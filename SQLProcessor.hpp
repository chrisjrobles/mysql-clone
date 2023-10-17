//
//  SQLProcessor.hpp
//  RGAssignment3
//
//  Created by rick gessner on 4/1/21.
//

#ifndef SQLProcessor_hpp
#define SQLProcessor_hpp

#include <stdio.h>
#include "CmdProcessor.hpp"
#include "Query.hpp"
#include "Statement.hpp"
#include "Tokenizer.hpp"

namespace ECE141 {

    class DBProcessor;

    class SQLProcessor : public CmdProcessor {
    public:

        SQLProcessor(std::ostream& anOutput, DBProcessor* aDBProc=nullptr);
        virtual ~SQLProcessor();

        virtual bool          isKnown(Keywords aKeyword) override;
        virtual CmdProcessor* recognizes(Tokenizer& aTokenizer) override;
        virtual Statement*    makeStatement(Tokenizer& aTokenizer) override;
        virtual StatusResult  run(Statement* aStmt, const Timer& aTimer) override;

        StatusResult alterTable(std::ostream& anOutput, Statement* aStmt);
        StatusResult createTable(std::ostream& anOutput, Statement* aStmt);
        StatusResult deleteRows(std::ostream& anOutput, Statement* aStmt);
        StatusResult describeTable(std::ostream& anOutput, Statement* aStmt);
        StatusResult dropTable(std::ostream& anOutput, Statement* aStmt);
        StatusResult insertRows(std::ostream& anOutput, Statement* aStmt);
        StatusResult selectRows(std::ostream& anOutput, Statement* aStmt);
        StatusResult showIndex(std::ostream& anOutput, Statement* aStmt);
        StatusResult showQuery(std::ostream& anOutput, Query &aQuery, RowCollection &aRows);
        StatusResult showTables(std::ostream& anOutput, Statement* aStmt);
        StatusResult updateRows(std::ostream& anOutput, Statement* aStmt);

    protected:
        DBProcessor* dbProc;
    };

}

#endif /* SQLProcessor_hpp */
