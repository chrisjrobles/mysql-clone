//
//  Entity.hpp
//
//  Created by rick gessner on 4/03/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//


#ifndef Entity_hpp
#define Entity_hpp

#include <stdio.h>
#include <vector>
#include <optional>
#include <memory>
#include <string>

#include "Attribute.hpp"
#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "Index.hpp"
#include "Storage.hpp"
#include "Row.hpp"

namespace ECE141 {

    class Row;

    using AttributeOpt = std::optional<Attribute>;
    using AttributeList = std::vector<Attribute>;
    using AttributeMap = std::map<std::string, Attribute>;
    using RowPtr = std::unique_ptr<Row>;
    using RowCollection = std::vector<RowPtr>;


    //------------------------------------------------

    class Entity : public Storable {
    public:

        static uint32_t       hashString(const char* str);
        static uint32_t       hashString(const std::string& aStr);

        Entity(std::string aName = "", uint32_t aBlockNum=0);
        Entity(const Entity& anEntity) = default;
        virtual ~Entity();

        bool           hasAttribute(std::string anAttributeName) const;
        StatusResult   addAttribute(Attribute& anAttribute);
        StatusResult   dropAttribute(std::string anAttributeName);
        StatusResult   prepareRow(Row& aRow);
        bool           validateRow(const Row& aRow) const;

        // getters & setters
        Attribute&     getAttribute(std::string anAttributeName);
        AttributeMap&  getAttributes();
        std::string    getName() const;
        uint32_t       getBlockNum() const;
        std::string    getPrimaryAttributeName() const;
        void           setBlockNum(uint32_t aBlockNum);

        // storable
        virtual StatusResult          encode(std::ostream& aWriter) override;
        virtual StatusResult          decode(std::istream& aReader) override;

        bool operator==(const Entity& anotherEntity) const;
        bool operator!=(const Entity& anotherEntity) const;
        operator bool() const;

    protected:
        AttributeMap    attributes;
        std::string     name;
        uint32_t        blockNum;
    };

}
#endif /* Entity_hpp */
