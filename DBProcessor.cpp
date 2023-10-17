
#include <filesystem>
#include "Config.hpp"
#include "Database.hpp"
#include "DBProcessor.hpp"
#include "DBStatements.hpp"
#include "views/DumpView.hpp"
#include "views/FolderView.hpp"
#include "views/SimpleViews.hpp"
#include "views/IndexViews.hpp"

namespace fs = std::filesystem;

namespace ECE141 {

    DBProcessor::DBProcessor(std::ostream &anOutput)
        : CmdProcessor(anOutput), activeDB(nullptr), sqlProc(nullptr) {}

    DBProcessor::~DBProcessor() {
        if (activeDB) delete activeDB;
        if (sqlProc) delete sqlProc;
    }

    bool DBProcessor::isKnown(Keywords aKeyword) {
        static Keywords theKnown[] = { Keywords::database_kw, Keywords::databases_kw, Keywords::use_kw, Keywords::indexes_kw };
        auto theIt = std::find(std::begin(theKnown),
            std::end(theKnown), aKeyword);

        return theIt != std::end(theKnown);
    }

    CmdProcessor* DBProcessor::recognizes(Tokenizer &aTokenizer) {
        if (isKnown(aTokenizer.current().keyword)) {
            return this;
        }

        if (!sqlProc) sqlProc = new SQLProcessor(output, this);

        return sqlProc->recognizes(aTokenizer);
    }

    Statement* DBProcessor::makeStatement(Tokenizer &aTokenizer) {
        Token theToken = aTokenizer.current();
        if (theToken.keyword == Keywords::databases_kw || theToken.keyword == Keywords::database_kw) {
            theToken = aTokenizer.peek(-1);
            if (theToken.keyword==Keywords::show_kw) {
                return new Statement(theToken.keyword);
            }
            NamedDBStatement* stmt = new NamedDBStatement(theToken.keyword);
            stmt->parse(aTokenizer);
            return stmt;
        }
        else if (isKnown(theToken.keyword)) {
            NamedDBStatement* stmt = new NamedDBStatement(theToken.keyword);
            stmt->parse(aTokenizer);
            return stmt;
        }

        return nullptr;
    }

    StatusResult DBProcessor::createDatabase(std::ostream& anOutput, Statement *aStmt) {
        StatusResult status;
        NamedDBStatement *namedDBStatement = static_cast<NamedDBStatement*>(aStmt);
        std::string dbName = namedDBStatement->getName();
        std::string pathToDb = Config::getDBPath(dbName);
        
        if (fs::exists(pathToDb)) {
            status = StatusResult{ Errors::databaseExists };
        } else {
            Database(dbName, CreateDB{});
        }

        CreateDBView view{1, status};
        view.show(anOutput, timer);
        return view.status;
    }

    StatusResult DBProcessor::dropDatabase(std::ostream& anOutput, Statement *aStmt) {
        StatusResult status;
        NamedDBStatement *namedDBStatement = static_cast<NamedDBStatement*>(aStmt);
        std::string dbName = namedDBStatement->getName();
        std::string pathToDb = Config::getDBPath(dbName);

        if (activeDB && activeDB->getName()==dbName) {
            delete activeDB;
            activeDB = nullptr;
        }

        uint32_t amountOfTables;
        if (fs::exists(pathToDb)) {
            Database* deleteDb = new Database(dbName, OpenDB{});
            amountOfTables = deleteDb->getTableAmount();
            fs::remove(pathToDb);
        } else {
            status = StatusResult{Errors::unknownDatabase};
        }

        DropDBView view{amountOfTables, status};
        view.show(anOutput, timer);
        return status;
    }

    StatusResult DBProcessor::showDatabases(std::ostream& anOutput, Statement* aStmt) {
        FolderView theShowView(Config::getStoragePath());
        theShowView.show(anOutput, timer);
        return StatusResult{ Errors::noError };
    }

    StatusResult DBProcessor::useDatabase(std::ostream& anOutput, Statement *aStmt) {
        NamedDBStatement* namedDBStatement = static_cast<NamedDBStatement*>(aStmt);
        std::string newDBName = namedDBStatement->getName();
        std::string newDBPath = Config::getDBPath(newDBName);

        if (fs::exists(newDBPath)) {
            if (activeDB) {
                delete activeDB;
                activeDB = nullptr;
            }
            activeDB = new Database(newDBName, OpenDB{});
            anOutput << "Database changed\n";
            return StatusResult{ Errors::noError };
        }

        return StatusResult{ Errors::unknownDatabase };
    }

    StatusResult DBProcessor::dumpDatabase(std::ostream& anOutput, Statement *aStmt) {
        NamedDBStatement* namedDBStatement = static_cast<NamedDBStatement*>(aStmt);
        std::string dumpDBName = namedDBStatement->getName();
        std::string dumpDBPath = Config::getDBPath(dumpDBName);

        if (fs::exists(dumpDBPath)) {
            Database* theNewDB = new Database(dumpDBName, OpenDB{});
            DumpView theView(dumpDBName, theNewDB);
            theView.show(anOutput, timer);
            delete theNewDB;
            return StatusResult{ Errors::noError };
        }

        return StatusResult{ Errors::unknownDatabase };
    }

    StatusResult DBProcessor::showIndexes(std::ostream& anOutput, Statement *aStmt) {
        if (!activeDB) return StatusResult{ Errors::noDatabaseSpecified };

        IndexesView theIndexesView(activeDB);
        theIndexesView.show(anOutput, timer);
        return StatusResult{ Errors::noError };
    }

    StatusResult  DBProcessor::run(Statement *aStmt, const Timer& aTimer) {
        timer = aTimer;
        switch (aStmt->getType()) {
        case Keywords::create_kw:   return createDatabase(output, aStmt);
        case Keywords::drop_kw:     return dropDatabase(output, aStmt);
        case Keywords::show_kw:     return showDatabases(output, aStmt);
        case Keywords::use_kw:      return useDatabase(output, aStmt);
        case Keywords::dump_kw:     return dumpDatabase(output, aStmt);
        case Keywords::indexes_kw:  return showIndexes(output, aStmt);
        default: break;
        }
        return StatusResult{ Errors::unknownCommand };
    }

    Database* DBProcessor::getDB() {
        return activeDB;
    }
}