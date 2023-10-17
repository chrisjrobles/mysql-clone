//
//  FolderView.hpp
//
//

#ifndef IndexViews_h
#define IndexViews_h

#include <vector>
#include <filesystem>
#include "SimpleViews.hpp"
#include "View.hpp"
#include "../Database.hpp"
#include "../Index.hpp"

namespace ECE141 {

    class IndexView : public View {
    public:

        IndexView(Database* aDB, std::string aTable, StringList aFields)
            : activeDB(aDB), tableName(aTable), fieldNames(aFields) {}

        bool show(std::ostream& anOutput, Timer& aTimer) override {

            activeDB->forEachIndexPair(tableName, [&](IndexKeyType aKeyType, const IndexPair &aPair) -> bool {
                switch(aKeyType) {
                    case IndexKeyType::uint32_t: {
                        uint32_t indexName = std::get<uint32_t>(aPair.first);
                        std::string indexNameStr = std::to_string(indexName);
                        std::string blockNumStr = std::to_string(aPair.second);
                        indexKeys.push_back(indexNameStr);
                        blockNums.push_back(blockNumStr);
                        break;
                    }
                    case IndexKeyType::string: {
                        std::string indexName = std::get<std::string>(aPair.first);
                        std::string blockNumStr = std::to_string(aPair.second);
                        indexKeys.push_back(indexName);
                        blockNums.push_back(blockNumStr);
                        break;
                    }
                    default: break;
                }
                return true;
            });

            anOutput << std::left;

            anOutput << "+------+---------+\n"
                << "| key  | block#  |\n"
                << "+------+---------+\n";

            
            for (size_t i = 0; i < indexKeys.size(); i++) {
                anOutput << "| " << std::setw(4) << indexKeys[i] << " | ";
                anOutput << std::setw(7) << blockNums[i] << " |\n";
                anOutput << "+------+---------+\n";
            }

            int numRows = indexKeys.size();
            aTimer.stop();
            anOutput << numRows << " rows in set (" << ViewUtils::getFormattedTime(aTimer.elapsed()) << " sec)\n";

            return true;
        }

        Database* activeDB;
        std::string tableName;
        StringList fieldNames;
        StringList indexKeys;
        StringList blockNums;
    };

    class IndexesView : public View {
    public:

        IndexesView(Database* aDB) : activeDB(aDB) {}

        bool show(std::ostream& anOutput, Timer& aTimer) override {

            activeDB->forEachIndex([&](IndexKeyType aKeyType, const IndexPair &aPair) -> bool {
                switch(aKeyType) {
                    case IndexKeyType::uint32_t: {
                        uint32_t indexName = std::get<uint32_t>(aPair.first);
                        std::string indexNameStr = std::to_string(indexName);
                        std::string fieldStr = std::to_string(aPair.second);
                        indexes.push_back(indexNameStr);
                        fields.push_back(fieldStr);
                        break;
                    }
                    case IndexKeyType::string: {
                        std::string indexName = std::get<std::string>(aPair.first);
                        std::string fieldStr = std::to_string(aPair.second);
                        indexes.push_back(indexName);
                        fields.push_back(fieldStr);
                        break;
                    }
                    default: break;
                }
                return true;
            });

            anOutput << std::left;

            anOutput << "+--------------------+--------------------+\n"
                << "| table              | field(s)           |\n"
                << "+--------------------+--------------------+\n";

            for (size_t i = 0; i < indexes.size(); i++) {
                anOutput << "| " << std::setw(18) << indexes[i] << " | ";
                anOutput << std::setw(18) << fields[i] << " |\n";
                anOutput << "+--------------------+--------------------+\n";
            }

            int numRows = indexes.size();
            aTimer.stop();
            anOutput << numRows << " rows in set (" << ViewUtils::getFormattedTime(aTimer.elapsed()) << " sec)\n";

            return true;
        }

        Database* activeDB;
        StringList indexes;
        StringList fields;

    };

}

#endif /* IndexViews_h */