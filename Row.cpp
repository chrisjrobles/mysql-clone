//
//  Row.cpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include <iomanip>
#include <sstream>
#include <algorithm>
#include "Row.hpp"
#include "Database.hpp"
#include "Tokenizer.hpp"
#include "Helpers.hpp"
#include "BasicTypes.hpp"

namespace ECE141 {
  
  Row::Row(uint32_t anEntityHash, KeyValues aValues, uint32_t aBlockNumber)
    : entityHash(anEntityHash), data(aValues), blockNum(aBlockNumber) {}

  Row::~Row() {}

  StatusResult Row::dropValue(const std::string &aKey) {
    if (!data.count(aKey)) return StatusResult{ Errors::invalidAttribute };

    data.erase(aKey);
    return StatusResult{Errors::noError};
  }

  StatusResult Row::setValue(const std::string &aKey, Value aValue) {
    data[aKey] = aValue;
    return StatusResult{Errors::noError};
  }

  Value& Row::getValue(std::string aKey) {
    return data.at(aKey);
  }

  Value Row::getValueCopy(std::string aKey) const {
    if (!data.count(aKey)) return Value{NullType{}};
    return data.at(aKey);
  }

  bool Row::hasValue(std::string aKey) const {
    return data.find(aKey) != data.end();
  }

  KeyValues& Row::getData() {
    return data;
  }

  KeyValues Row::getDataCopy() const {
    return data;
  }

  uint32_t Row::getBlockNum() const {
    return blockNum;
  }

  void Row::setBlockNum(uint32_t aBlockNum) {
    blockNum = aBlockNum;
  }

  void Row::setPrimaryKey(int aPrimaryKey) {
    primaryKey = aPrimaryKey;
  }

  int Row::getPrimaryKey() const {
    return primaryKey;
  }

  StatusResult Row::encode(std::ostream &aWriter) {
    size_t valueAmount = data.size();
    aWriter << entityHash << " ";
    aWriter << blockNum << " ";
    aWriter << primaryKey << " ";
    aWriter << valueAmount << " ";
    for (auto pair : data) {
      aWriter << pair.first << " ";
      aWriter << Helpers::valueIndexToDataType(pair.second.index()) << " ";
      aWriter << pair.second << " ";
    }
    return StatusResult{ Errors::noError };
  }

  StatusResult Row::decode(std::istream &aReader) {
    StatusResult status;
    size_t valAmount;
    aReader >> entityHash;
    aReader >> blockNum;
    aReader >> primaryKey;
    aReader >> valAmount;
    for (size_t i=0; i<valAmount; i++) {
      std::string attrName;
      DataTypes type;
      Value aValue;
      aReader >> attrName;
      aReader >> type;
      aValue = Helpers::getValueOfType(type);
      aReader >> aValue;
      if (!(status = setValue(attrName, aValue))) return status;
    }
    return status;
  }

  bool Row::operator==(const Row& anotherRow) const {
    if (entityHash!=anotherRow.entityHash)    return false;
    if (data!=anotherRow.data)                return false;
    if (blockNum!=anotherRow.blockNum)        return false;
    return true;
  }

}
