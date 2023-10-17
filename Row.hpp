//
//  Row.hpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Row_hpp
#define Row_hpp

#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include "Entity.hpp"
#include "Storage.hpp"
#include "Attribute.hpp"


class Database;

namespace ECE141 {

  class Row : public Storable {
  public:

    Row(uint32_t anEntityHash=0, KeyValues aValues=KeyValues{}, uint32_t aBlockNumber=0);
    ~Row();

    StatusResult        dropValue(const std::string &aKey);
    StatusResult        setValue(const std::string &aKey, Value aValue);
    Value&              getValue(std::string aKey);
    Value               getValueCopy(std::string aKey) const;
    bool                hasValue(std::string aKey) const;
    KeyValues&          getData();
    KeyValues           getDataCopy() const;
    uint32_t            getBlockNum() const;
    void                setBlockNum(uint32_t aBlockNum);
    int                 getPrimaryKey() const;
    void                setPrimaryKey(int aPrimaryKey);

    // storable
    StatusResult        encode(std::ostream &aWriter) override;
    StatusResult        decode(std::istream &aReader) override;

    bool operator==(const Row& anotherRow) const;

  protected:
    uint32_t            entityHash;
    KeyValues           data;
    uint32_t            blockNum;
    int                 primaryKey;
  };
}

#endif /* Row_hpp */
