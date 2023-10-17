//
//  Entity.cpp
//
//  Created by rick gessner on 4/03/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <algorithm>
#include <set>
#include <sstream>
#include "Entity.hpp"
#include "Helpers.hpp"

namespace ECE141 {

    //STUDENT: Implement the Entity class here...

    const int gMultiplier = 37;

    //hash given string to numeric quantity...
    uint32_t Entity::hashString(const char* str) {
        uint32_t h{ 0 };
        unsigned char* p;
        for (p = (unsigned char*)str; *p != '\0'; p++)
            h = gMultiplier * h + *p;
        return h;
    }

    uint32_t Entity::hashString(const std::string& aStr) {
        return hashString(aStr.c_str());
    }

    //OCF...
    Entity::Entity(std::string aName, uint32_t aBlockNum) : name(aName), blockNum(aBlockNum) {
    }

    Entity::~Entity() {
    }

    bool Entity::hasAttribute(std::string anAttributeName) const {
        return attributes.find(anAttributeName) != attributes.end();
    }

    StatusResult Entity::addAttribute(Attribute& anAttribute) {
        if (hasAttribute(anAttribute.getName())) return StatusResult { Errors::invalidAttribute };

        attributes[anAttribute.getName()] = anAttribute;
        return StatusResult{ Errors::noError };
    }

    StatusResult Entity::dropAttribute(std::string anAttributeName) {
        if (!hasAttribute(anAttributeName)) return StatusResult { Errors::invalidAttribute };

        attributes.erase(anAttributeName);
        return StatusResult{ Errors::noError };
    }

    StatusResult Entity::prepareRow(Row& aRow) {
        StatusResult status;
        for (auto& attr : attributes) {
            // set null value if no value for key present
            if (!aRow.hasValue(attr.first)) aRow.setValue(attr.first, NullType{});
            
            Value& value = aRow.getValue(attr.first);
            if (!(status = attr.second.prepareValue(value))) return status;
        }

        return StatusResult{ Errors::noError };
    }

    bool Entity::validateRow(const Row& aRow) const {
        // Check that there is a corresponding attribute for every value
        for (const auto& kv : aRow.getDataCopy()) {
            if (!hasAttribute(kv.first)) return false;
        }

        // Check that there is a valid value for every attribute
        for (const auto& attr : attributes) {
            // handle missing values
            if (!aRow.hasValue(attr.first)) return false;

            if (!attr.second.validateValue(aRow.getValueCopy(attr.first))) return false;
        }

        return true;
    }

    Attribute& Entity::getAttribute(std::string anAttributeName) {
        return attributes[anAttributeName];
    }

    AttributeMap& Entity::getAttributes() { return attributes; }

    std::string Entity::getName() const { return name; }

    uint32_t Entity::getBlockNum() const { return blockNum; }

    std::string Entity::getPrimaryAttributeName() const {
        for (auto& attr : attributes) {
            if (attr.second.isPrimaryKey()) return attr.first;
        }
        return std::string();
    }

    void Entity::setBlockNum(uint32_t aBlockNum) {
        blockNum = aBlockNum;
    }


    //other entity methods...
    StatusResult Entity::encode(std::ostream& aWriter) {
        aWriter << name << " ";
        aWriter << blockNum << " ";
        aWriter << attributes.size() << " ";
        StatusResult status;
        for (auto& attribute : attributes) {
            if (!(status = attribute.second.encode(aWriter))) return status;
        }
        return status;
    }

    StatusResult Entity::decode(std::istream& aReader) {
        aReader >> name;
        aReader >> blockNum;
        size_t attributeCount;
        aReader >> attributeCount;
        StatusResult status;
        for (size_t i = 0; i < attributeCount; i++) {
            Attribute tempAttribute;
            if (!(status = tempAttribute.decode(aReader))) return status;
            if (!(status = addAttribute(tempAttribute))) return status;
        }
        return StatusResult{ noError };
    }

    bool Entity::operator==(const Entity& anotherEntity) const {
        if (attributes!=anotherEntity.attributes)   return false;
        if (name!=anotherEntity.name)               return false;
        return true;
    }

    bool Entity::operator!=(const Entity& anotherEntity) const {
        return !(this->operator==(anotherEntity));
    }

    Entity::operator bool() const {
        return !name.empty();
    }
}
