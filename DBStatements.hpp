#ifndef DBStatement_hpp
#define DBStatement_hpp

#include "Statement.hpp"
#include "Tokenizer.hpp"

namespace ECE141 {

    class NamedDBStatement : public Statement {
    public:
        NamedDBStatement(Keywords aStatementType = Keywords::database_kw);
        NamedDBStatement(const NamedDBStatement& aCopy);
        virtual               ~NamedDBStatement();

        virtual StatusResult  parse(Tokenizer& aTokenizer);
        std::string           getName();

    protected:
        std::string dbName;
    };

}

#endif