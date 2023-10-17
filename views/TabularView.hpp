//
//  TabularView.hpp
//
//  Created by rick gessner on 4/26/20.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef TabularView_h
#define TabularView_h

#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include "SimpleViews.hpp"
#include "View.hpp"
#include "../Database.hpp"
#include "../Row.hpp"

namespace ECE141 {

    class TabularView : public View {
    public:

        TabularView(Database* aDB) : database(aDB) {}

        bool show(std::ostream& anOutput, Timer& aTimer) override {

            anOutput << std::left;
            size_t width = 25;
            int numRows = 0;
            printHeader(anOutput, width);
            printTables(anOutput, width, numRows);
            printRowSeparator(anOutput, width);

            aTimer.stop();
            anOutput << numRows << " rows in set (" << ViewUtils::getFormattedTime(aTimer.elapsed()) << " sec)\n";

            return true;
        }

        void printHeader(std::ostream& anOutput, size_t width) {
            std::string heading = "Tables_in_" + database->getName();
            printRowSeparator(anOutput, width);
            anOutput << "| " << std::setw(width) << heading << " |\n";
            printRowSeparator(anOutput, width);
        }

        void printTables(std::ostream& anOutput, size_t width, int& numRows) {
            database->forEachEntity([&anOutput, &width, &numRows](const Entity& anEntity) -> bool {
                anOutput << "| " << std::setw(width) << anEntity.getName() << " |\n";
                numRows++;
                return true;
            });
        }
        
        void printRowSeparator(std::ostream& anOutput, size_t width) {
            anOutput << "+-" << std::string(width, '-') << "-+\n";
        }

    protected:
        Database* database;
    };

}

#endif /* TabularView_h */
