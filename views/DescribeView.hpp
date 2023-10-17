//
//  DescribeView.hpp
//
//

#ifndef DescribeView_h
#define DescribeView_h

#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include <ctime>
#include "SimpleViews.hpp"
#include "View.hpp"
#include "../Database.hpp"
#include "../Helpers.hpp"
#include "../Row.hpp"

namespace ECE141 {

    class DescribeView : public View {
    public:

        DescribeView(Database* aDB, std::string aTableName)
            : database(aDB), tableName(aTableName) {}

        bool show(std::ostream& anOutput, Timer& aTimer) override {

            if (!updateAttributes())
                return false;

            anOutput << std::left;

            anOutput << "+-----------------+--------------+------+-----+---------------------+-----------------------------+\n"
                     << "| Field           | Type         | Null | Key | Default             | Extra                       |\n"
                     << "+-----------------+--------------+------+-----+---------------------+-----------------------------+\n";

            for (auto& attrPair : attributes) {
                Attribute& theAttribute = attrPair.second;
                std::string theType = formatDataType(theAttribute.getType(), theAttribute.getSizeLimit());
                std::string theNullable = formatNullable(theAttribute.isNullable());
                std::string thePrimaryKey = formatPrimaryKey(theAttribute.isPrimaryKey());
                // std::string theValue = formatValue(theAttribute.getDefaultValue());
                std::string theAutoIncrementable = formatAutoIncrementable(theAttribute.isAutoIncrementable());

                anOutput << "| "
                    << std::setw(15) << theAttribute.getName()          << " | "
                    << std::setw(12) << theType                         << " | "
                    << std::setw(4)  << theNullable                     << " | "
                    << std::setw(3)  << thePrimaryKey                   << " | "
                    << std::setw(19) << theAttribute.getDefaultValue()  << " | "
                    << std::setw(27) << theAutoIncrementable            << " |\n";
            }

            anOutput << "+-----------------+--------------+------+-----+---------------------+-----------------------------+\n";

            int numRows = attributes.size();
            aTimer.stop();
            anOutput << numRows << " rows in set (" << ViewUtils::getFormattedTime(aTimer.elapsed()) << " sec)\n";

            return true;
        }

    protected:

        bool updateAttributes() {

            Entity theEntity = database->getEntity(tableName);

            if (theEntity.getName() == "")
                return false;

            attributes = theEntity.getAttributes();
            return true;
        }

        std::string formatDataType(DataTypes aType, size_t aSize) {
            std::string theType;
            switch (aType) {
            case DataTypes::int_type: theType = "integer"; break;
            case DataTypes::bool_type: theType = "boolean"; break;
            case DataTypes::float_type: theType = "float"; break;
            case DataTypes::datetime_type: theType = "date"; break;
            case DataTypes::varchar_type: theType = "varchar(" + std::to_string(aSize) + ")"; break;
            default: theType = ""; break;
            }
            return theType;
        }

        std::string formatNullable(bool aBool) {
            return (aBool) ? "YES" : "NO";
        }

        std::string formatPrimaryKey(bool aBool) {
            return (aBool) ? "YES" : "";
        }

        std::string formatAutoIncrementable(bool aBool) {
            return (aBool) ? "auto_increment primary key" : "";
        }

        Database* database;
        AttributeMap attributes;
        std::string tableName;
    };

}

#endif /* DescribeView_h */
