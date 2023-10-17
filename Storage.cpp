//
//  Storage.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//


#include <sstream>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <cstring>
#include "Storage.hpp"
#include "Config.hpp"

namespace ECE141 {

  Storage::Storage(std::iostream &aStream) : BlockIO(aStream) {
  }

  Storage::~Storage() {
  }

  bool Storage::each(const BlockVisitor aVisitor) {
    for(uint32_t i=0; i<getBlockCount(); i++) {
      Block tempBlock;
      if (!readBlock(i, tempBlock)) return false;
      if (!aVisitor(tempBlock, i)) return false;
    }
    return true;
  }
  
  int32_t Storage::getFreeBlock() {
    // check if there are any known free blocks
    if (available.empty()) {
      uint32_t blockCount = getBlockCount();
      for(uint32_t i=0; i<blockCount; i++) {
        Block tempBlock;
        if (!readBlock(i, tempBlock)) return kNewBlock; // block read error
        if (tempBlock.header.type == static_cast<char>(BlockType::free_block)) {
          return i;
        }
      }
      return blockCount;
    }

    uint32_t freeBlockNum = available.front();
    available.pop_front();
    return freeBlockNum;
  }
  
  //write logic to mark a block as free...
  StatusResult Storage::markBlockAsFree(uint32_t aPos) {
    Block tempBlock;
    StatusResult status;

    // attempt to read block
    if (!(status = readBlock(aPos, tempBlock))) return status;
    else {
      tempBlock.header.type = static_cast<char>(BlockType::free_block);
      if (!(status = writeBlock(aPos, tempBlock))) return status;
      status.value = tempBlock.header.next;
    }
    return status;
  }

  StatusResult Storage::releaseBlocks(uint32_t aPos, bool aInclusive) {
    Block tempBlock;
    StatusResult status;

    // attempt to read block
    if(!(status = readBlock(aPos, tempBlock))) return status;
    else {
      if (aInclusive) {
        if (!(status = markBlockAsFree(aPos))) return status;
      }
      // read each block in list and mark as free
      while (status.value) {
        if (!(status = markBlockAsFree(status.value))) return status;
      }
    }
    return StatusResult{Errors::noError};
  }

  StatusResult Storage::save(std::iostream &aStream, StorageInfo &anInfo) {
    StatusResult status;
    size_t blockAmount = anInfo.size / kPayloadSize + (anInfo.size % kPayloadSize != 0);
    int32_t nextPos;
    
    // Set save location to either free block or specified location
    if (anInfo.start==kNewBlock) {
      nextPos = getFreeBlock();
      if (nextPos==kNewBlock) return StatusResult{Errors::readError};
    } else {
      nextPos = anInfo.start;
    }
    // set value so user can see where first block is located
    status.value = nextPos;

    // go to beginning of stream and save to blocks
    aStream.seekg(0, std::ios::beg);
    for (size_t i=1; i<=blockAmount; i++) {
      // initialize block
      Block tempBlock = Block{anInfo.type};
      tempBlock.header.count = blockAmount;  // total amount of saved blocks
      tempBlock.header.id    = anInfo.id;    // position in sequence of saved blocks
      tempBlock.header.refId = anInfo.refId; // refId from storage info
      tempBlock.header.pos   = nextPos;      // absolute block position in db

      // save part of stream to block payload
      memset(tempBlock.payload,0,kPayloadSize);
      size_t readSize = i==blockAmount ? anInfo.size - (i-1)*(kPayloadSize-1) : kPayloadSize-1;
      aStream.read(tempBlock.payload, readSize);
      tempBlock.payload[readSize-1] = '\0';
      
      // if not last block, get next free block id and move stream forward
      if (i!=blockAmount) {
        Block existingBlock = tempBlock;
        if (!readBlock(existingBlock.header.pos, existingBlock)) {
          nextPos = getFreeBlock();
          stream.clear();
        } else nextPos = existingBlock.header.next ? existingBlock.header.next : getFreeBlock();
        // nextPos = getFreeBlock();
        tempBlock.header.next = nextPos;
      } else if (aStream.eof()) {
        aStream.clear();
      }

      status.error = writeBlock(tempBlock.header.pos, tempBlock).error;
      if (!status) return status;
    }

    return status;
  }

  StatusResult Storage::load(std::iostream &anOut, uint32_t aBlockNum) {
    Block tempBlock;
    StatusResult status;
    uint32_t next = aBlockNum;
    size_t count = 0;

    do {
      if (!(status = readBlock(next, tempBlock))) return status;
      anOut << tempBlock.payload;
      next = tempBlock.header.next;
      count++;
    } while(next);
    anOut.seekg(0, std::ios::beg);
    
    return StatusResult{Errors::noError};
  }
 
}

