//
//  Helpers.hpp
//  Database3
//
//  Created by rick gessner on 4/14/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#include "BasicTypes.hpp"
#include "keywords.hpp"
#include "Helpers.hpp"
#include <algorithm>
#include <sstream>

namespace ECE141 {
    
    Keywords Helpers::getKeywordId(const std::string aKeyword) {
        auto theIter = gDictionary.find(aKeyword);
        if (theIter != gDictionary.end()) {
        return theIter->second;
        }
        return Keywords::unknown_kw;
    }

    //convert from char to keyword...
    Keywords Helpers::charToKeyword(char aChar) {
        switch(toupper(aChar)) {
            case 'I': return Keywords::integer_kw;
            case 'T': return Keywords::datetime_kw;
            case 'B': return Keywords::boolean_kw;
            case 'F': return Keywords::float_kw;
            case 'V': return Keywords::varchar_kw;
            default:  return Keywords::unknown_kw;
        }
    }

    //convert from char to keyword...
    DataTypes Helpers::charToDatatype(char aChar) {
        switch(toupper(aChar)) {
            case 'I': return DataTypes::int_type;
            case 'B': return DataTypes::bool_type;
            case 'F': return DataTypes::float_type;
            case 'V': return DataTypes::varchar_type;
            case 'D': return DataTypes::datetime_type;
            case 'N': return DataTypes::no_type;
            default:  return DataTypes::no_type;
        }
    }
            
    const char* Helpers::dataTypeToString(DataTypes aType) {
        switch(aType) {
            case DataTypes::no_type:        return "none";
            case DataTypes::bool_type:      return "bool";
            case DataTypes::datetime_type:  return "datetime";
            case DataTypes::float_type:     return "float";
            case DataTypes::int_type:       return "int";
            case DataTypes::varchar_type:   return "varchar";
            default: return "none";
        }
    }

    const char* Helpers::keywordToString(Keywords aType) {
        switch(aType) {
            case Keywords::boolean_kw:    return "bool";
            case Keywords::create_kw:     return "create";
            case Keywords::database_kw:   return "database";
            case Keywords::databases_kw:  return "databases";
            case Keywords::datetime_kw:   return "datetime";
            case Keywords::describe_kw:   return "describe";
            case Keywords::drop_kw:       return "drop";
            case Keywords::float_kw:      return "float";
            case Keywords::integer_kw:    return "integer";
            case Keywords::show_kw:       return "show";
            case Keywords::table_kw:      return "table";
            case Keywords::tables_kw:     return "tables";
            case Keywords::use_kw:        return "use";
            case Keywords::varchar_kw:    return "varchar";
            default:                      return "unknown";
        }
    }

    // USE: ---validate that given keyword is a datatype...
    bool Helpers::isDatatype(Keywords aKeyword) {
        switch(aKeyword) {
            case Keywords::char_kw:
            case Keywords::datetime_kw:
            case Keywords::float_kw:
            case Keywords::integer_kw:
            case Keywords::varchar_kw:
                return true;
            default: return false;
        }
    }
        
    Operators Helpers::toOperator(std::string aString) {
        auto theIter = gOperators.find(aString);
        if (theIter != gOperators.end()) {
        return theIter->second;
        }
        return Operators::unknown_op;
    }

    int Helpers::getFunctionId(const std::string anIdentifier) {
        auto theIter = gFunctions.find(anIdentifier);
        if (theIter != gFunctions.end()) {
        return theIter->second;
        }
        return 0;
    }

    bool Helpers::isNumericKeyword(Keywords aKeyword) {
        static Keywords theTypes[]={Keywords::decimal_kw, Keywords::double_kw, Keywords::float_kw, Keywords::integer_kw};
        for(auto k : theTypes) {
        if(aKeyword==k) return true;
        }
        return false;
    }


    DataTypes Helpers::valueIndexToDataType(size_t anIndex) {
        switch(anIndex) {
            case 0:     return DataTypes::null_type;
            case 1:     return DataTypes::bool_type;
            case 2:     return DataTypes::int_type;
            case 3:     return DataTypes::float_type;
            case 4:     return DataTypes::varchar_type;
            case 5:     return DataTypes::datetime_type;
            default:    return DataTypes::no_type;
        }
    }

    IndexKeyType Helpers::toindexKeyType(size_t anIndex) {
        switch(anIndex) {
            case 0:     return IndexKeyType::uint32_t;
            case 1:     return IndexKeyType::string;
            default:    return IndexKeyType::uint32_t;
        }
    }

    Value Helpers::getValueOfType(DataTypes type) {
        switch(type) {
            case DataTypes::bool_type:      return Value{bool{}};
            case DataTypes::int_type:       return Value{int{}};
            case DataTypes::float_type:     return Value{double{}};
            case DataTypes::varchar_type:   return Value{std::string{}};
            case DataTypes::datetime_type:  return Value{time_t{}};
            case DataTypes::null_type:      return Value{NullType{}};
            default:                        return Value();
        }
    }

    bool Helpers::valueIsNull(const Value &aValue) {
        return std::visit(overload{
            [](const auto &aValue)     {return false;},
            [](const NullType &aValue) {return true;},
        }, aValue);
    }

    size_t Helpers::getValueSize(const Value &aValue) {
        return std::visit(overload{
            [](std::string &aValue)       { return aValue.size();},
            [](auto aValue)               { return size_t{0};},
        }, aValue);
    }
}
