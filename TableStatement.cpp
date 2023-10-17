
#include <vector>
#include "Database.hpp"
#include "Helpers.hpp"
#include "Query.hpp"
#include "TableStatement.hpp"

namespace ECE141 {

	TableStatement::TableStatement(Keywords aStatementType)
		: Statement(aStatementType) {}

	TableStatement::TableStatement(const TableStatement& aCopy) 
		: Statement(aCopy.stmtType), tableName(aCopy.tableName) {}

    StatusResult TableStatement::parse(Tokenizer& aTokenizer, Database* activeDB) {
        return StatusResult{};
    }

    // ----------------------------
    // ALTER
    // ----------------------------

    AlterTableStatement::AlterTableStatement()
		: TableStatement(Keywords::alter_kw) {}

	AlterTableStatement::AlterTableStatement(const AlterTableStatement& aCopy) 
		: TableStatement(aCopy.stmtType) {
            tableName = aCopy.tableName;
            changeAttributes = aCopy.changeAttributes;
            alterType = aCopy.alterType;
        }

    StatusResult AlterTableStatement::parse(Tokenizer& aTokenizer, Database* activeDB) {
        aTokenizer.restart();
        
        if (aTokenizer.skipIf(Keywords::alter_kw)) {
            if (aTokenizer.skipIf(Keywords::table_kw)) {
                tableName = aTokenizer.current().data;
                while (aTokenizer.more()) {

                    aTokenizer.next();
                    if (aTokenizer.current().data == ";") {
                        break;
                    }
                    
                    if (aTokenizer.current().data == "add") alterType = AlterType::add;
                    else if (aTokenizer.current().data == "drop") alterType = AlterType::drop;

                    aTokenizer.next();
                    Attribute theAttribute = parseAttribute(aTokenizer);
                    changeAttributes.push_back(theAttribute);
                }
                return StatusResult{ Errors::noError };
            }
        }
        
        return StatusResult{ Errors::syntaxError };
    }

    // Indicates the end of an attribute
    bool AlterTableStatement::endAttribute(std::string c) {
        if (c == "," || c == ";")
            return true;
        if (c == ")" && openParen <= 1)
            return true;
        return false;
    }

    Attribute AlterTableStatement::parseAttribute(Tokenizer& aTokenizer) {

        std::string theName = "";
        DataTypes theType = DataTypes::no_type;
        Value defaultValue = std::string();
        size_t theSize = 0;
        bool isPrimaryKey = false;
        bool nullable = true;
        bool autoIncrementable = false;
        bool hasDefaultValue = false;

        Token theToken = aTokenizer.current();

        while (!endAttribute(theToken.data)) {

            switch (theToken.type) {
            case TokenType::punctuation: {
                if (theToken.data == "(")
                    openParen++;
                if (theToken.data == ")")
                    openParen--;
                break;
            }
            case TokenType::number: {
                theSize = std::stoi(theToken.data);
                break;
            }
            case TokenType::identifier: {
                theName = theToken.data;
                break;
            }
            case TokenType::keyword: {

                DataTypes theDataType = ParsingHelper::getDataType(theToken.keyword);
                if (theDataType != DataTypes::no_type)
                    theType = theDataType;

                if (theToken.keyword == Keywords::primary_kw)
                    isPrimaryKey = true;

                else if (theToken.keyword == Keywords::not_kw) {
                    if (aTokenizer.more()) {
                        if (aTokenizer.peek().keyword == Keywords::null_kw) {
                            nullable = false;
                            aTokenizer.next();
                        }
                    }
                }

                else if (theToken.keyword == Keywords::auto_increment_kw)
                    autoIncrementable = true;

                else if (theToken.keyword == Keywords::default_kw) {
                    if (aTokenizer.more()) {
                        aTokenizer.next();
                        theToken = aTokenizer.current();
                        defaultValue = ParsingHelper::strToValue(theToken.data, theType);
                        hasDefaultValue = true;
                    }
                }

                break;
            }
            default: {
                break;
            }
            }

            aTokenizer.next();
            theToken = aTokenizer.current();

        }

        if (!hasDefaultValue) defaultValue = Helpers::getValueOfType(DataTypes::null_type);

        return Attribute(theName, theType, defaultValue, theSize, isPrimaryKey, nullable, autoIncrementable);
    }

    // ----------------------------
    // CREATE
    // ----------------------------

    CreateTableStatement::CreateTableStatement()
		: TableStatement(Keywords::create_kw) {}

	CreateTableStatement::CreateTableStatement(const CreateTableStatement& aCopy) 
		: TableStatement(aCopy.stmtType) {
            tableName = aCopy.tableName;
            attributes = aCopy.attributes;
            openParen = aCopy.openParen;
        }

    StatusResult CreateTableStatement::parse(Tokenizer& aTokenizer, Database* activeDB) {
        aTokenizer.restart();
        
        if (aTokenizer.skipIf(Keywords::create_kw)) {
            if (aTokenizer.skipIf(Keywords::table_kw)) {
                tableName = aTokenizer.current().data;
                while (aTokenizer.more()) {

                    aTokenizer.next();
                    if (aTokenizer.current().data == ";") {
                        break;
                    }

                    Attribute theAttribute = parseAttribute(aTokenizer);
                    attributes.push_back(theAttribute);
                }
                return StatusResult{ Errors::noError };
            }
        }
        
        return StatusResult{ Errors::syntaxError };
    }

    // Indicates the end of an attribute
    bool CreateTableStatement::endAttribute(std::string c) {
        if (c == "," || c == ";")
            return true;
        if (c == ")" && openParen <= 1)
            return true;
        return false;
    }

    Attribute CreateTableStatement::parseAttribute(Tokenizer& aTokenizer) {

        std::string theName = "";
        DataTypes theType = DataTypes::no_type;
        Value defaultValue = std::string();
        size_t theSize = 0;
        bool isPrimaryKey = false;
        bool nullable = true;
        bool autoIncrementable = false;
        bool hasDefaultValue = false;

        Token theToken = aTokenizer.current();

        while (!endAttribute(theToken.data)) {

            switch (theToken.type) {
            case TokenType::punctuation: {
                if (theToken.data == "(")
                    openParen++;
                if (theToken.data == ")")
                    openParen--;
                break;
            }
            case TokenType::number: {
                if (openParen >= 2)
                    theSize = std::stoi(theToken.data);
                break;
            }
            case TokenType::identifier: {
                theName = theToken.data;
                break;
            }
            case TokenType::keyword: {

                DataTypes theDataType = ParsingHelper::getDataType(theToken.keyword);
                if (theDataType != DataTypes::no_type)
                    theType = theDataType;

                if (theToken.keyword == Keywords::primary_kw)
                    isPrimaryKey = true;

                else if (theToken.keyword == Keywords::not_kw) {
                    if (aTokenizer.more()) {
                        if (aTokenizer.peek().keyword == Keywords::null_kw) {
                            nullable = false;
                            aTokenizer.next();
                        }
                    }
                }

                else if (theToken.keyword == Keywords::auto_increment_kw)
                    autoIncrementable = true;

                else if (theToken.keyword == Keywords::default_kw) {
                    if (aTokenizer.more()) {
                        aTokenizer.next();
                        theToken = aTokenizer.current();
                        defaultValue = ParsingHelper::strToValue(theToken.data, theType);
                        hasDefaultValue = true;
                    }
                }

                break;
            }
            default: {
                break;
            }
            }

            aTokenizer.next();
            theToken = aTokenizer.current();

        }

        if (!hasDefaultValue) defaultValue = Helpers::getValueOfType(DataTypes::null_type);

        return Attribute(theName, theType, defaultValue, theSize, isPrimaryKey, nullable, autoIncrementable);
    }

    // ----------------------------
    // DROP
    // ----------------------------

    DropTableStatement::DropTableStatement()
		: TableStatement(Keywords::drop_kw) {}

	DropTableStatement::DropTableStatement(const DropTableStatement& aCopy) 
		: TableStatement(aCopy.stmtType) {
            tableName = aCopy.tableName;
        }

    StatusResult DropTableStatement::parse(Tokenizer& aTokenizer, Database* activeDB) {
        aTokenizer.restart();

        if (aTokenizer.skipIf(Keywords::drop_kw)) {
            if (aTokenizer.skipIf(Keywords::table_kw)) {
                tableName = aTokenizer.current().data;
                aTokenizer.skipIf(';');

                if (!aTokenizer.more())
                    return StatusResult { Errors::noError };
            }
        }

        return StatusResult{ Errors::syntaxError };
    }


    // ----------------------------
    // SHOW
    // ----------------------------

    ShowTableStatement::ShowTableStatement()
		: TableStatement(Keywords::show_kw) {}

	ShowTableStatement::ShowTableStatement(const ShowTableStatement& aCopy) 
		: TableStatement(aCopy.stmtType) {
            tableName = aCopy.tableName;
        }

    StatusResult ShowTableStatement::parse(Tokenizer& aTokenizer, Database* activeDB) {
        aTokenizer.restart();

        if (aTokenizer.skipIf(Keywords::show_kw)) {
            if (aTokenizer.skipIf(Keywords::tables_kw)) {
                aTokenizer.skipIf(';');
                
                if (!aTokenizer.more())
                    return StatusResult { Errors::noError };
            }
        }

        return StatusResult{ Errors::syntaxError };
    }


    // ----------------------------
    // DESCRIBE
    // ----------------------------

    DescribeTableStatement::DescribeTableStatement()
		: TableStatement(Keywords::describe_kw) {}

	DescribeTableStatement::DescribeTableStatement(const DescribeTableStatement& aCopy) 
		: TableStatement(aCopy.stmtType) {
            tableName = aCopy.tableName;
        }

    StatusResult DescribeTableStatement::parse(Tokenizer& aTokenizer, Database* activeDB) {
        aTokenizer.restart();

        if (aTokenizer.skipIf(Keywords::describe_kw)) {
            tableName = aTokenizer.current().data;
            aTokenizer.skipIf(';');
            
            if (!aTokenizer.more())
                return StatusResult { Errors::noError };
        }

        return StatusResult{ Errors::syntaxError };
    }


    // ----------------------------
    // INSERT
    // ----------------------------
    
    InsertTableStatement::InsertTableStatement()
		: TableStatement(Keywords::insert_kw) {}

	InsertTableStatement::InsertTableStatement(const InsertTableStatement& aCopy) 
		: TableStatement(aCopy.stmtType) {
            tableName = aCopy.tableName;
            rows = aCopy.rows;
            keys = aCopy.keys;
            openParen = aCopy.openParen;
        }

    StatusResult InsertTableStatement::parse(Tokenizer& aTokenizer, Database* activeDB) {
        aTokenizer.restart();

        if (aTokenizer.skipIf(Keywords::insert_kw)) {
            if (aTokenizer.skipIf(Keywords::into_kw)) {
                tableName = aTokenizer.current().data;

                Entity theEntity = activeDB->getEntity(tableName);
                if (theEntity.getName() == "")
                    return StatusResult{ Errors::unknownEntity };
                
                aTokenizer.next();

                if (aTokenizer.skipIf('(')) {

                    Token theToken = aTokenizer.current();
                    while (theToken.data != ")") {
                        keys.push_back(theToken.data);
                        aTokenizer.next();
                        aTokenizer.skipIf(',');
                        theToken = aTokenizer.current();
                    }

                    aTokenizer.skipIf(')');
                    if (aTokenizer.skipIf(Keywords::values_kw)) {

                        while (aTokenizer.more()) {

                            aTokenizer.next();
                            if (aTokenizer.current().data == ";") {
                                break;
                            }

                            KeyValues theRow = parseRow(aTokenizer, theEntity);
                            rows.push_back(theRow);
                        }
                    }
                }

                aTokenizer.skipIf(';');

                if (!aTokenizer.more())
                    return StatusResult { Errors::noError };
            }
        }
        return StatusResult{ Errors::syntaxError };
    }

    KeyValues InsertTableStatement::parseRow(Tokenizer& aTokenizer, Entity& aEntity) {

        KeyValues thePairs;
        size_t index = 0;
        std::string theName = "";

        Token theToken = aTokenizer.current();

        while (theToken.data != ")" && index < keys.size()) {

            switch (theToken.type) {
            case TokenType::punctuation: {
                if (theToken.data == "(")
                    openParen++;
                if (theToken.data == ")")
                    openParen--;
                break;
            }
            case TokenType::number: {
                DataTypes theType = aEntity.getAttribute(keys[index]).getType();
                Value theValue = ParsingHelper::strToValue(theToken.data, theType);
                thePairs[keys[index]] = theValue;
                index++;
                break;
            }
            case TokenType::identifier: {
                theName = theToken.data;
                thePairs[keys[index]] = theName;
                index++;
                break;
            }
            default: {
                break;
            }
            }

            aTokenizer.next();
            theToken = aTokenizer.current();
        }
        return thePairs;
    }


    // ----------------------------
    // SELECT
    // ----------------------------

    SelectTableStatement::SelectTableStatement()
		: TableStatement(Keywords::select_kw) {}

	SelectTableStatement::SelectTableStatement(const SelectTableStatement& aCopy) 
		: TableStatement(aCopy.stmtType) {
            tableName = aCopy.tableName;
            //query = aCopy.query;
        }

    StatusResult SelectTableStatement::parse(Tokenizer& aTokenizer, Database* activeDB) {
        aTokenizer.restart();

        if (aTokenizer.skipIf(Keywords::select_kw)) {
            if (aTokenizer.skipIf('*')) {
                query.setSelectAll();
            }
            else {
                while (!ParsingHelper::endFields(aTokenizer.current())) {
                    aTokenizer.skipIf(',');
                    if (aTokenizer.more()) {
                        if (aTokenizer.peek(1).data == ".") {
                            aTokenizer.next(2);
                        }
                    }
                    query.addSelectField(aTokenizer.current().data);
                    aTokenizer.next();
                }
            }

            if (aTokenizer.skipIf(Keywords::from_kw)) {
                tableName = aTokenizer.current().data;

                Entity theEntity = activeDB->getEntity(tableName);
                if (theEntity.getName() == "")
                    return StatusResult{ Errors::unknownEntity };
                query.setFrom(theEntity);

                while (aTokenizer.more()) {

                    aTokenizer.next();
                    if (aTokenizer.current().data == ";") {
                        break;
                    }

                    if (aTokenizer.skipIf(Keywords::where_kw)) {
                        Filters theFilters;
                        Entity theFromEntity = query.getFrom();
                        theFilters.parse(aTokenizer, theFromEntity);
                        query.setFilters(theFilters);
                    }

                    if (aTokenizer.skipIf(Keywords::order_kw)) {
                        if (aTokenizer.skipIf(Keywords::by_kw)) {
                            while (!ParsingHelper::endFields(aTokenizer.current())) {
                                aTokenizer.skipIf(',');
                                query.addOrderField(aTokenizer.current().data);
                                aTokenizer.next();
                            }
                        }
                    }

                    if (aTokenizer.skipIf(Keywords::limit_kw)) {
                        try {
                            int theLimit = std::stoi(aTokenizer.current().data);
                            query.setLimit(theLimit);
                        }
                        catch (...) {
                            return StatusResult{ Errors::syntaxError };
                        }
                    }

                    if (aTokenizer.skipIf(Keywords::join_kw)) {
                        
                        Keywords theJoinType = aTokenizer.peek(-2).keyword;
                        Join theJoin(aTokenizer.current().data, theJoinType);
                        aTokenizer.next();
                        if (aTokenizer.skipIf(Keywords::on_kw)) {
                            Operand theLHS;
                            Operand theRHS;
                            theLHS.entityId = Entity::hashString(aTokenizer.current().data);
                            aTokenizer.next();
                            if (aTokenizer.skipIf('.')) {
                                theLHS.name = aTokenizer.current().data;
                                aTokenizer.next();
                                if (aTokenizer.current().op != Operators::unknown_op) {
                                    Operators theOp = aTokenizer.current().op;
                                    aTokenizer.next();
                                    theRHS.entityId = Entity::hashString(aTokenizer.current().data);
                                    aTokenizer.next();
                                    if (aTokenizer.skipIf('.')) {
                                        theRHS.name = aTokenizer.current().data;

                                        // Update Join
                                        Expression theExpr(theLHS, theOp, theRHS);
                                        theJoin.setExpr(theExpr);
                                        joins.push_back(theJoin);

                                        // return StatusResult{ Errors::noError };

                                    }
                                }
                            }
                        }
                        // return StatusResult{ Errors::syntaxError };
                    }
                }
                return StatusResult{ Errors::noError };
            }
        }
        return StatusResult{ Errors::syntaxError };
    }


    // ----------------------------
    // UPDATE
    // ----------------------------

    UpdateTableStatement::UpdateTableStatement()
		: TableStatement(Keywords::update_kw) {}

	UpdateTableStatement::UpdateTableStatement(const UpdateTableStatement& aCopy) 
		: TableStatement(aCopy.stmtType) {
            tableName = aCopy.tableName;
            //updateQuery = aCopy.updateQuery;
        }

    StatusResult UpdateTableStatement::parse(Tokenizer& aTokenizer, Database* activeDB) {
        aTokenizer.restart();

        if (aTokenizer.skipIf(Keywords::update_kw)) {
            tableName = aTokenizer.current().data;

            Entity theEntity = activeDB->getEntity(tableName);
            if (theEntity.getName() == "")
                return StatusResult{ Errors::unknownEntity };
            updateQuery.setFrom(theEntity);
            aTokenizer.next();

            if (aTokenizer.skipIf(Keywords::set_kw)) {
                
                while (!ParsingHelper::endFields(aTokenizer.current())) {
                    
                    aTokenizer.skipIf(',');
                    std::string theKey = aTokenizer.current().data;
                    aTokenizer.next();
                    
                    if (aTokenizer.skipIf('=')) {
                        std::string theStringVal = aTokenizer.current().data;
                        DataTypes theType = theEntity.getAttribute(theKey).getType();
                        Value theValue = ParsingHelper::strToValue(theStringVal, theType);
                        if (!updateQuery.addUpdateField(theKey, theValue))
                            return StatusResult{ Errors::syntaxError };
                    }
                    else {
                        return StatusResult{ Errors::syntaxError };
                    }

                    aTokenizer.next();
                }

                while (aTokenizer.more()) {
                    
                    if (aTokenizer.current().data == ";") {
                        break;
                    }

                    if (aTokenizer.skipIf(Keywords::where_kw)) {
                        Filters theFilters;
                        Entity theFromEntity = updateQuery.getFrom();
                        theFilters.parse(aTokenizer, theFromEntity);
                        updateQuery.setFilters(theFilters);
                    }
                    aTokenizer.next();
                }
                return StatusResult{ Errors::noError };
            }
        }
        return StatusResult{ Errors::syntaxError };
    }


    // ----------------------------
    // DELETE
    // ----------------------------

    DeleteTableStatement::DeleteTableStatement()
		: TableStatement(Keywords::delete_kw) {}

	DeleteTableStatement::DeleteTableStatement(const DeleteTableStatement& aCopy) 
		: TableStatement(aCopy.stmtType) {
            tableName = aCopy.tableName;
            //query = aCopy.query;
        }

    StatusResult DeleteTableStatement::parse(Tokenizer& aTokenizer, Database* activeDB) {
        aTokenizer.restart();

        if (aTokenizer.skipIf(Keywords::delete_kw)) {
            if (aTokenizer.skipIf(Keywords::from_kw)) {
                tableName = aTokenizer.current().data;

                Entity theEntity = activeDB->getEntity(tableName);
                if (theEntity.getName() == "")
                    return StatusResult{ Errors::unknownEntity };
                query.setFrom(theEntity);

                while (aTokenizer.more()) {

                    aTokenizer.next();
                    if (aTokenizer.current().data == ";") {
                        break;
                    }

                    if (aTokenizer.skipIf(Keywords::where_kw)) {
                        Filters theFilters;
                        Entity theFromEntity = query.getFrom();
                        theFilters.parse(aTokenizer, theFromEntity);
                        query.setFilters(theFilters);
                    }
                }
            }
        }
        return StatusResult{ Errors::noError };
    }

    // ----------------------------
    // SHOW INDEX
    // ----------------------------
    
    ShowIndexStatement::ShowIndexStatement()
		: TableStatement(Keywords::index_kw), _showPrimaryIndex(false) {}

	ShowIndexStatement::ShowIndexStatement(const ShowIndexStatement& aCopy) 
		: TableStatement(aCopy.stmtType) {
            tableName = aCopy.tableName;
            _showPrimaryIndex = aCopy._showPrimaryIndex;
            indexNames = aCopy.indexNames;
        }

    StatusResult ShowIndexStatement::parse(Tokenizer& aTokenizer, Database* activeDB) {
        aTokenizer.restart();

        if (aTokenizer.skipIf(Keywords::show_kw)) {
            if (aTokenizer.skipIf(Keywords::index_kw)) {

                while (!ParsingHelper::endFields(aTokenizer.current())) {
                    aTokenizer.skipIf(',');
                    indexNames.push_back(aTokenizer.current().data);
                    aTokenizer.next();
                }

                // Parsing only works for primary key rn
                _showPrimaryIndex = true;
                if (aTokenizer.skipIf(Keywords::from_kw)) {
                    tableName = aTokenizer.current().data;
                    return StatusResult{ Errors::noError };
                }
                
            }
        }
        return StatusResult{ Errors::syntaxError };
    }
}