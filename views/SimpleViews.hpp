//
//  FolderView.hpp
//  Assignment2
//
//  Created by rick gessner on 2/15/21.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef SimpleViews_h
#define SimpleViews_h

#include <sstream>
#include <iomanip>
#include "View.hpp"
#include "../Errors.hpp"
#include "../keywords.hpp"
#include "../Timer.hpp"

namespace ECE141 {

    struct ViewUtils {

        static std::string getFormattedTime(double time) {
            std::stringstream elapsedTime;
            elapsedTime << std::fixed << std::setprecision(4) << time+0.00005;
            return elapsedTime.str();
        }

        static std::string getSuccessMessage(double time, size_t numRows = 0){
            std::stringstream ss;
            ss << "Query OK, " << numRows << " row" << ((numRows!=1)?"s":"") << " affected (" << getFormattedTime(time) << " sec)";
            return ss.str();
        }

        static std::string getErrorMessage(Errors error) {
            static std::map<ECE141::Errors, std::string> theMessages = {
                {ECE141::illegalIdentifier, "Illegal identifier"},
                {ECE141::unknownIdentifier, "Unknown identifier"},
                {ECE141::databaseExists,    "Database exists"},
                {ECE141::tableExists,       "Table Exists"},
                {ECE141::syntaxError,       "Syntax Error"},
                {ECE141::unknownCommand,    "Unknown command"},
                {ECE141::unknownDatabase,   "Unknown database"},
                {ECE141::unknownTable,      "Unknown table"},
                {ECE141::unknownError,      "Unknown error"}
            };

            std::string theMessage="Unknown Error";
            if(theMessages.count(error)) {
                theMessage=theMessages[error];
            }

            std::stringstream ss;
            ss << "Error (" << static_cast<int>(error) << ") " << theMessage;
            
            return ss.str();
        }

        static std::string getBlockTypeString(char blockTypeChar) {
            static std::map<char, std::string> blockTypeStrings {
                {'M', "Meta"},
                {'D', "Data"},
                {'E', "Entity"},
                {'F', "Free"},
                {'I', "Index"},
                {'U', "Unknown"},
            };

            return blockTypeStrings[blockTypeChar];
        }

    };

    class GenericView : public View {
    public:
        GenericView(size_t aRowsAffected=0, StatusResult aStatus=StatusResult{}) 
            : rowsAffected(aRowsAffected), status(aStatus) {}
        
        ~GenericView() {}

        virtual bool show(std::ostream &aStream, Timer& aTimer) {
            aTimer.stop();
            if (status) {
                aStream << ViewUtils::getSuccessMessage(aTimer.elapsed(), rowsAffected) << "\n";
            }
            else {
                aStream << ViewUtils::getErrorMessage(status.error) << "\n";
            }
            return true;
        }

        size_t          rowsAffected;
        StatusResult    status;
    };

    class CreateDBView : public GenericView {
    public:
        CreateDBView(size_t aRowsAffected=1, StatusResult aStatus=StatusResult{}) 
            : GenericView(aRowsAffected, aStatus) {}

        ~CreateDBView() {}
    };

    class CreateTableView : public GenericView {
    public:
        CreateTableView(size_t aRowsAffected=0, StatusResult aStatus=StatusResult{}) 
            : GenericView(aRowsAffected, aStatus) {}

        ~CreateTableView() {}
    };

    class DropDBView : public GenericView {
    public:
        DropDBView(size_t aRowsAffected=0, StatusResult aStatus=StatusResult{}) 
            : GenericView(aRowsAffected, aStatus) {}

        ~DropDBView() {}
    };

    class DropTableView : public GenericView {
    public:
        DropTableView(size_t aRowsAffected=0, StatusResult aStatus=StatusResult{}) 
            : GenericView(aRowsAffected, aStatus) {}

        ~DropTableView() {}
    };

    class InsertView : public GenericView {
    public:
        InsertView(size_t aRowsAffected=0, StatusResult aStatus=StatusResult{}) 
            : GenericView(aRowsAffected, aStatus) {}

        ~InsertView() {}
    };

    class UpdateView : public GenericView {
    public:
        UpdateView(size_t aRowsAffected=0, StatusResult aStatus=StatusResult{}) 
            : GenericView(aRowsAffected, aStatus) {}

        ~UpdateView() {}
    };

    class DeleteView : public GenericView {
    public:
        DeleteView(size_t aRowsAffected=0, StatusResult aStatus=StatusResult{}) 
            : GenericView(aRowsAffected, aStatus) {}

        ~DeleteView() {}
    };

}

#endif /* SimpleViews_h */