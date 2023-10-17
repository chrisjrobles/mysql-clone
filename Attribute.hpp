//
//  Attribute.hpp
//
//  Created by rick gessner on 4/02/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef Attribute_hpp
#define Attribute_hpp

#include <stdio.h>
#include <string>
#include "Storage.hpp"
#include "keywords.hpp"
#include "BasicTypes.hpp"

namespace ECE141 {

    class Attribute : public Storable {
    public:

        Attribute();
        Attribute(std::string aName, DataTypes aType, Value aValue, size_t aSize, bool aIsPrimaryKey, bool aNullable, bool aAutoIncrementable, int aNext=1);
        virtual ~Attribute();

        StatusResult    prepareValue(Value &aValue) ;    // Populate value if applicable
        bool            validateValue(Value aValue) const;   // Return error if value does not conform to attribute

        std::string     getName() const;
        DataTypes       getType() const;
        Value           getDefaultValue() const;
        size_t          getSizeLimit() const;
        bool            isPrimaryKey() const;
        bool            isNullable() const;
        bool            isAutoIncrementable() const;
        int             getNext();

        virtual StatusResult        encode(std::ostream& anOutput) override;
        virtual StatusResult        decode(std::istream& anInput) override;

        bool operator==(const Attribute& anotherAttribute) const;
        operator bool() const;

    protected:
        std::string   name;
        DataTypes     type;
        Value         defaultValue;

        size_t        size;
        bool          primaryKey;
        bool          nullable;
        bool          autoIncrementable;
        int           next;
    };

}


#endif /* Attribute_hpp */
