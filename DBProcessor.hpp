#ifndef DBProcessor_hpp
#define DBProcessor_hpp

#include "CmdProcessor.hpp"
#include "Database.hpp"
#include "SQLProcessor.hpp"

namespace ECE141 {

    class DBProcessor : public CmdProcessor {
    public:
    
        DBProcessor(std::ostream &anOutput);
        virtual ~DBProcessor();
                
        virtual bool          isKnown(Keywords aKeyword) override;
        virtual CmdProcessor* recognizes(Tokenizer &aTokenizer) override;
        virtual Statement*    makeStatement(Tokenizer &aTokenizer) override;
        virtual StatusResult  run(Statement *aStmt, const Timer& aTimer) override;

        StatusResult createDatabase(std::ostream& anOutput, Statement *aStmt);
        StatusResult dropDatabase(std::ostream& anOutput, Statement *aStmt);
        StatusResult showDatabases(std::ostream& anOutput, Statement *aStmt);
        StatusResult useDatabase(std::ostream& anOutput, Statement *aStmt);
        StatusResult dumpDatabase(std::ostream& anOutput, Statement *aStmt);
        StatusResult showIndexes(std::ostream& anOutput, Statement *aStmt);

        Database* getDB();

    protected:
        Database        *activeDB;
        SQLProcessor    *sqlProc; 
    };
}

#endif