//
//  FolderView.hpp
//  Assignment2
//
//  Created by rick gessner on 2/15/21.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef FolderView_h
#define FolderView_h

#include <vector>
#include <filesystem>
#include "SimpleViews.hpp"
#include "View.hpp"

namespace ECE141 {

    class FolderView : public View {
    public:

        FolderView(std::string aPath, std::string anExtension = ".db")
            : path(aPath), extension(anExtension) {}

        bool updateDatabases() {
            for (auto theFile : std::filesystem::directory_iterator(path)) {
                if (theFile.path().extension() == extension) {
                    databases.push_back(theFile.path().filename().string());
                }
            }
            return true;
        }

        bool show(std::ostream& anOutput, Timer& aTimer) override {

            updateDatabases();

            anOutput << std::left;

            anOutput << "+--------------------+\n"
                << "| Database           |\n"
                << "+--------------------+\n";

            for (std::string theFile : databases) {
                anOutput << "| " << std::setw(18) << theFile << " |\n";
            }

            anOutput << "+--------------------+\n";

            int numRows = databases.size();
            aTimer.stop();
            anOutput << numRows << " rows in set (" << ViewUtils::getFormattedTime(aTimer.elapsed()) << " sec)\n";

            return true;
        }

        std::string path;
        std::string extension;
        std::vector<std::string> databases;
    };

}

#endif /* FolderView_h */