//
//  SelectView.hpp
//
//

#ifndef SelectView_h
#define SelectView_h

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
#include "../Query.hpp"
#include "../Row.hpp"

namespace ECE141 {

    class SelectView : public View {
    public:

        SelectView(RowCollection& aRows, Query aQuery, Entity anEntity)
            : rows(std::move(aRows)), query(aQuery), entity(anEntity) {}

        bool show(std::ostream& anOutput, Timer& aTimer) override {

            anOutput << std::left;
            // no selected rows
            if (!rows.size()) {
                anOutput << "Empty set (" << ViewUtils::getFormattedTime(aTimer.elapsed()) << " sec)\n";
                return true;
            }

            StringList selectedFields;
            if (query.getSelectAll()) {
                for (auto &attr : entity.getAttributes()) {                    
                    if (attr.second.isPrimaryKey()) {
                        selectedFields.insert(selectedFields.begin(), attr.first);
                    }
                    else {
                        selectedFields.push_back(attr.first);
                    }
                }
            }
            else {
                selectedFields = query.getSelectFields();
            }

            printHeader(anOutput, selectedFields);
            for (auto& row : rows) {
                printRow(anOutput, row, selectedFields);
            }
            printFooter(anOutput, selectedFields, aTimer);
            
            return true;
        }

        void printHeader(std::ostream& anOutput, StringList& selectedFields) {
            static std::string displaySection = "+-----------------------";
            for (size_t i=0; i<selectedFields.size(); i++) {
                anOutput << displaySection;
            }
            anOutput << "+\n";

            for (auto& aField : selectedFields) {
                anOutput << "| " << std::setw(22) << aField;
            }
            anOutput << "|\n";

            for (size_t i=0; i<selectedFields.size(); i++) {
                anOutput << displaySection;
            }
            anOutput << "+\n";
        }

        void printRow(std::ostream& anOutput, const RowPtr& aRow, StringList& selectedFields) {
            for (auto& field : selectedFields) {
                anOutput << "| " << std::setw(22);
                if (!aRow->hasValue(field)) anOutput << "NULL";
                else                        anOutput << aRow->getValue(field);
            }
            anOutput << "|\n";
        }

        void printFooter(std::ostream& anOutput, StringList& selectedFields, Timer& aTimer) {
            static std::string displaySection = "+-----------------------";
            for (size_t i=0; i<selectedFields.size(); i++) {
                anOutput << displaySection;
            }
            anOutput << "+\n";
            
            aTimer.stop();
            int numRows = rows.size();
            anOutput << numRows << " rows in set (" << ViewUtils::getFormattedTime(aTimer.elapsed()) << " sec)\n";

        }

    protected:
        RowCollection rows;
        Query query;
        Entity entity;
    };

}

#endif /* SelectView_h */
