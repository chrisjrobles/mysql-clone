#ifndef TableStatement_hpp
#define TableStatement_hpp

#include <vector>
#include "Attribute.hpp"
#include "BasicTypes.hpp"
#include "Database.hpp"
#include "Entity.hpp"
#include "Join.hpp"
#include "ParsingHelper.hpp"
#include "Query.hpp"
#include "Statement.hpp"
#include "Tokenizer.hpp"

namespace ECE141 {

    class TableStatement : public Statement {
    public:

        TableStatement(Keywords aStatementType = Keywords::table_kw);
        TableStatement(const TableStatement& aCopy);
        virtual ~TableStatement() {}

        virtual StatusResult    parse(Tokenizer& aTokenizer, Database* activeDB);
        std::string             getName() { return tableName; }

    protected:
        std::string tableName = "";

    };

    class AlterTableStatement : public TableStatement {
    public:
        AlterTableStatement();
        AlterTableStatement(const AlterTableStatement& aCopy);
        virtual ~AlterTableStatement() {}

        virtual StatusResult    parse(Tokenizer& aTokenizer, Database* activeDB);
        AttributeList           getChangeAttributes()   { return changeAttributes; }
        AlterType               getAlterType()          { return alterType; }

    protected:

        bool endAttribute(std::string c);
        Attribute parseAttribute(Tokenizer& aTokenizer);

        AttributeList   changeAttributes;
        AlterType       alterType;
        int openParen = 0;
    };

    class CreateTableStatement : public TableStatement {
    public:
        CreateTableStatement();
        CreateTableStatement(const CreateTableStatement& aCopy);
        virtual ~CreateTableStatement() {}

        virtual StatusResult    parse(Tokenizer& aTokenizer, Database* activeDB);
        AttributeList&          getAttributes() { return attributes; }

    protected:

        bool endAttribute(std::string c);
        Attribute parseAttribute(Tokenizer& aTokenizer);

        AttributeList attributes;
        int openParen = 0;

    };

    class DropTableStatement : public TableStatement {
    public:
        DropTableStatement();
        DropTableStatement(const DropTableStatement& aCopy);
        virtual ~DropTableStatement() {}

        virtual StatusResult    parse(Tokenizer& aTokenizer, Database* activeDB);

    };

    class ShowTableStatement : public TableStatement {
    public:
        ShowTableStatement();
        ShowTableStatement(const ShowTableStatement& aCopy);
        virtual ~ShowTableStatement() {}

        virtual StatusResult    parse(Tokenizer& aTokenizer, Database* activeDB);

    };

    class DescribeTableStatement : public TableStatement {
    public:
        DescribeTableStatement();
        DescribeTableStatement(const DescribeTableStatement& aCopy);
        virtual ~DescribeTableStatement() {}

        virtual StatusResult    parse(Tokenizer& aTokenizer, Database* activeDB);

    };

    class InsertTableStatement : public TableStatement {
    public:
        InsertTableStatement();
        InsertTableStatement(const InsertTableStatement& aCopy);
        virtual ~InsertTableStatement() {}

        virtual StatusResult    parse(Tokenizer& aTokenizer, Database* activeDB);
        std::vector<KeyValues>  getKVs() { return rows; }

    protected:

        KeyValues parseRow(Tokenizer& aTokenizer, Entity& aEntity);

        std::vector<KeyValues> rows;
		StringList keys;
        int openParen = 0;

    };

    class SelectTableStatement : public TableStatement {
    public:
        SelectTableStatement();
        SelectTableStatement(const SelectTableStatement& aCopy);
        virtual ~SelectTableStatement() {}

        virtual StatusResult    parse(Tokenizer& aTokenizer, Database* activeDB);
        Query&                  getQuery() { return query; }
        JoinList                getJoins() { return joins; }
        void                    addJoin(Join &aJoin) { joins.push_back(aJoin); };

    protected:
        JoinList joins;
        Query query;
    };

    class UpdateTableStatement : public TableStatement {
    public:
        UpdateTableStatement();
        UpdateTableStatement(const UpdateTableStatement& aCopy);
        virtual ~UpdateTableStatement() {}

        virtual StatusResult    parse(Tokenizer& aTokenizer, Database* activeDB);
        UpdateQuery&            getQuery() { return updateQuery; }

    protected:
        UpdateQuery updateQuery;
    };

    class DeleteTableStatement : public TableStatement {
    public:
        DeleteTableStatement();
        DeleteTableStatement(const DeleteTableStatement& aCopy);
        virtual ~DeleteTableStatement() {}

        virtual StatusResult    parse(Tokenizer& aTokenizer, Database* activeDB);
        Query&                  getQuery() { return query; }

    protected:
        Query query;
    };

    class ShowIndexStatement : public TableStatement {
    public:
        ShowIndexStatement();
        ShowIndexStatement(const ShowIndexStatement& aCopy);
        virtual ~ShowIndexStatement() {}

        virtual StatusResult    parse(Tokenizer& aTokenizer, Database* activeDB);
        bool                    showPrimaryIndex()  { return _showPrimaryIndex; }
        StringList              getIndexNames()     { return indexNames; }

    protected:
        bool        _showPrimaryIndex;
        StringList  indexNames;
    };

}

#endif /* TableStatement_hpp */