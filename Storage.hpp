//
//  Storage.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef Storage_hpp
#define Storage_hpp

#include <string>
#include <fstream>
#include <iostream>
#include <deque>
#include <functional>
#include "BlockIO.hpp"
#include "Errors.hpp"

namespace ECE141 {
  
  using BlockVisitor = std::function<bool(const Block&, uint32_t)>;
  using ChangeBlockVisitor = std::function<bool(Block&, uint32_t)>;
  using BlockList = std::deque<uint32_t>;
      
  struct CreateDB {}; //tags for db-open modes...
  struct OpenDB {};

  const int32_t kNewBlock=-1;

  struct BlockIterator {
    virtual bool each(const BlockVisitor)=0;
  };

  class Storable {
  public:
    virtual StatusResult  encode(std::ostream &anOutput)=0;
    virtual StatusResult  decode(std::istream &anInput)=0;
  };

  struct StorageInfo {
    
    StorageInfo(size_t theSize, uint32_t anId=0, uint32_t aRefId=0, 
                int32_t aStartPos=kNewBlock, BlockType aType=BlockType::data_block)
      : size(theSize), id(anId), refId(aRefId), start(aStartPos), type(aType) {}
    
    size_t     size;  //size of stream
    uint32_t   id;    //id of self
    uint32_t   refId; //id of owner
    int32_t    start; //block #
    BlockType  type;  //block type
  };

  class Storage : public BlockIO, BlockIterator {
  public:
        
    Storage(std::iostream &aStream);
    ~Storage();
 
    StatusResult save(std::iostream &aStream, StorageInfo &anInfo);
    StatusResult load(std::iostream &aStream, uint32_t aStartBlockNum);

    bool         each(const BlockVisitor aVisitor) override;

    StatusResult markBlockAsFree(uint32_t aPos);
    StatusResult releaseBlocks(uint32_t aPos, bool aInclusive=true);
    int32_t      getFreeBlock(); //pos of next free (or new)...

  protected:
    BlockList    available;
  };

}


#endif /* Storage_hpp */
