
#include "DBStatements.hpp"

namespace ECE141 {

    NamedDBStatement::NamedDBStatement(Keywords aStatementType)
        : Statement(aStatementType) {}

    NamedDBStatement::NamedDBStatement(const NamedDBStatement& aCopy)
        : Statement(aCopy.stmtType), dbName(aCopy.dbName) {}

    NamedDBStatement::~NamedDBStatement() {}

    StatusResult  NamedDBStatement::parse(Tokenizer& aTokenizer) {

        aTokenizer.restart();
        Token current = aTokenizer.current();

        if (current.type == TokenType::keyword)
            stmtType = current.keyword;
        else
            return StatusResult{ Errors::keywordExpected };

        aTokenizer.next();

        if (aTokenizer.skipIf(Keywords::database_kw)) {
            dbName = aTokenizer.current().data;
        }
        else if (stmtType == Keywords::use_kw) {
            dbName = aTokenizer.current().data;
        }
        else if (stmtType == Keywords::show_kw) {
            if (aTokenizer.skipIf(Keywords::indexes_kw)) {
                stmtType = Keywords::indexes_kw;
            }
        }
        else
            return StatusResult{ Errors::unexpectedKeyword };

        return StatusResult{ Errors::noError };
    }

    std::string NamedDBStatement::getName() { return dbName; }

}