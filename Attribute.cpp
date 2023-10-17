//
//  Attribute.hpp
//
//  Created by rick gessner on 4/02/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <iostream>
#include "Attribute.hpp"
#include "Helpers.hpp"

namespace ECE141 {

    Attribute::Attribute() : next(1) {}

    Attribute::Attribute(std::string aName, DataTypes aType, Value aValue, size_t aSize, bool aIsPrimaryKey, bool aNullable, bool aAutoIncrementable, int aNext)
        : name(aName), type(aType), defaultValue(aValue), size(aSize), primaryKey(aIsPrimaryKey), nullable(aNullable), autoIncrementable(aAutoIncrementable), next(aNext) {}

    Attribute::~Attribute() {}

    StatusResult Attribute::prepareValue(Value &aValue) {
        if (autoIncrementable && Helpers::valueIsNull(aValue)) aValue = getNext();
        if (autoIncrementable && !Helpers::valueIsNull(aValue)) next = std::get<int>(aValue)+1;
        if (Helpers::valueIsNull(aValue)) aValue = defaultValue;

        return StatusResult{ Errors::noError };
    }

    bool Attribute::validateValue(Value aValue) const {
        if (!nullable && Helpers::valueIsNull(aValue))                  return false;
        if (type != Helpers::valueIndexToDataType(aValue.index()) &&
            !(nullable && Helpers::valueIsNull(aValue)))                return false;
        if (!size && Helpers::getValueSize(aValue)>size)                return false;
        
        return true;
    }

    std::string   Attribute::getName() const                { return name; }
    DataTypes     Attribute::getType() const                { return type; }
    Value         Attribute::getDefaultValue() const        { return defaultValue; }
    size_t        Attribute::getSizeLimit() const           { return size; }
    bool          Attribute::isPrimaryKey() const           { return primaryKey; }
    bool          Attribute::isNullable() const             { return nullable; }
    bool          Attribute::isAutoIncrementable() const    { return autoIncrementable; }
    int           Attribute::getNext()                      { return next++; }
                  Attribute::operator bool() const          { return !name.empty();}

    StatusResult Attribute::encode(std::ostream& anOutput) {
        anOutput << name << " ";
        anOutput << type << " ";
        bool hasDefault = DataTypes::null_type!=Helpers::valueIndexToDataType(defaultValue.index());
        anOutput << hasDefault << " ";
        anOutput << defaultValue << " ";
        anOutput << size << " ";
        anOutput << primaryKey << " ";
        anOutput << nullable << " ";
        anOutput << autoIncrementable << " ";
        anOutput << next << " ";
        return StatusResult{ Errors::noError };
    }

    StatusResult Attribute::decode(std::istream& anInput) {
        anInput >> name;
        anInput >> type;
        bool hasDefault;
        anInput >> hasDefault;
        defaultValue = Helpers::getValueOfType(hasDefault ? type : DataTypes::null_type);
        anInput >> defaultValue;
        anInput >> size;
        anInput >> primaryKey;
        anInput >> nullable;
        anInput >> autoIncrementable;
        anInput >> next;
        return StatusResult{ Errors::noError };
    }

    bool Attribute::operator==(const Attribute& anotherAttribute) const {
        if (name != anotherAttribute.name) return false;
        if (defaultValue != anotherAttribute.defaultValue) return false;
        if (size != anotherAttribute.size) return false;
        if (primaryKey != anotherAttribute.primaryKey) return false;
        if (nullable != anotherAttribute.nullable) return false;
        if (autoIncrementable != anotherAttribute.autoIncrementable) return false;
        if (next != anotherAttribute.next) return false;
        return true;   
    }

}
