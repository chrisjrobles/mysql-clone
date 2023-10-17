//
//  SQLProcessor.cpp
//  RGAssignment3
//
//  Created by rick gessner on 4/1/21.
//

#include "DBProcessor.hpp"
#include "SQLProcessor.hpp"
#include "Database.hpp"
#include "TableStatement.hpp"
#include "Timer.hpp"
#include "views/SelectView.hpp"
#include "views/SimpleViews.hpp"
#include "views/TabularView.hpp"
#include "views/DescribeView.hpp"
#include "views/IndexViews.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace ECE141 {

    SQLProcessor::SQLProcessor(std::ostream& anOutput, DBProcessor* aDBProc)
        : CmdProcessor(anOutput), dbProc(aDBProc) {}

    SQLProcessor::~SQLProcessor() {}

    bool SQLProcessor::isKnown(Keywords aKeyword) {
        static Keywords theKnown[] = {  Keywords::alter_kw,
                                        Keywords::table_kw,
                                        Keywords::tables_kw,
                                        Keywords::describe_kw,
                                        Keywords::insert_kw,
                                        Keywords::select_kw,
                                        Keywords::update_kw,
                                        Keywords::delete_kw,
                                        Keywords::index_kw };

        auto theIt = std::find(std::begin(theKnown), std::end(theKnown), aKeyword);
        return theIt != std::end(theKnown);
    }

    CmdProcessor* SQLProcessor::recognizes(Tokenizer& aTokenizer) {
        if (isKnown(aTokenizer.current().keyword)) {
            return this;
        }

        return nullptr;
    }

    Statement* SQLProcessor::makeStatement(Tokenizer& aTokenizer) {
        Keywords theCommandKeyword = aTokenizer.tokenAt(0).keyword;
        if (theCommandKeyword == Keywords::show_kw) {
            if (aTokenizer.tokenAt(1).keyword == Keywords::index_kw)
                theCommandKeyword = Keywords::index_kw;
        }
        TableStatement* stmt;
        switch (theCommandKeyword) {
        case Keywords::alter_kw:    stmt = new AlterTableStatement(); break;
        case Keywords::create_kw:   stmt = new CreateTableStatement(); break;
        case Keywords::drop_kw:     stmt = new DropTableStatement(); break;
        case Keywords::show_kw:     stmt = new ShowTableStatement(); break;
        case Keywords::describe_kw: stmt = new DescribeTableStatement(); break;
        case Keywords::insert_kw:   stmt = new InsertTableStatement(); break;
        case Keywords::select_kw:   stmt = new SelectTableStatement(); break;
        case Keywords::update_kw:   stmt = new UpdateTableStatement(); break;
        case Keywords::delete_kw:   stmt = new DeleteTableStatement(); break;
        case Keywords::index_kw:    stmt = new ShowIndexStatement(); break;
        default: break;
        }
        stmt->parse(aTokenizer, dbProc->getDB());
        return stmt;
    }

    StatusResult SQLProcessor::alterTable(std::ostream& anOutput, Statement* aStmt) {
        Database* db = dbProc->getDB();
        if (!db) return StatusResult{ Errors::noDatabaseSpecified };
        
        AlterTableStatement* stmt = static_cast<AlterTableStatement*>(aStmt);
        if (!db->hasTable(stmt->getName())) return StatusResult{ Errors::unknownTable };

        StatusResult status;
        Entity entity{stmt->getName()};

        for (auto &attr : stmt->getChangeAttributes()) {
            if (!(status = db->alterTable(stmt->getName(), stmt->getAlterType(), attr))) return status;
        }
        
        GenericView view{};
        view.show(anOutput, timer);

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLProcessor::createTable(std::ostream& anOutput, Statement* aStmt) {
        Database* db = dbProc->getDB();
        if (!db) return StatusResult{ Errors::noDatabaseSpecified };

        StatusResult status;
        CreateTableStatement* stmt = static_cast<CreateTableStatement*>(aStmt);
        Entity entity{stmt->getName()};

        for (auto& attr : stmt->getAttributes()) {
            if (!(status = entity.addAttribute(attr))) return status;
        }
        if (!(status = db->addTable(entity))) return status;

        CreateTableView view{};
        view.show(anOutput, timer);

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLProcessor::deleteRows(std::ostream& anOutput, Statement* aStmt) {
        Database* db = dbProc->getDB();
        if (!db) return StatusResult{ Errors::noDatabaseSpecified };

        DeleteTableStatement* stmt = static_cast<DeleteTableStatement*>(aStmt);
        if (!db->hasTable(stmt->getName())) return StatusResult{ Errors::unknownTable };

        StatusResult status;
        Query& theQuery = stmt->getQuery();

        if (!(status = db->deleteRows(theQuery))) return status;

        DeleteView view{status.value};
        view.show(anOutput, timer);

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLProcessor::describeTable(std::ostream& anOutput, Statement* aStmt) {
        Database* db = dbProc->getDB();
        if (!db) return StatusResult{ Errors::noDatabaseSpecified };

        DescribeTableStatement* stmt = static_cast<DescribeTableStatement*>(aStmt);
        DescribeView theView{db, stmt->getName()};

        if (!db->hasTable(stmt->getName())) return StatusResult{ Errors::unknownTable };
        if (!theView.show(anOutput, timer)) return StatusResult{ Errors::unknownTable };

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLProcessor::dropTable(std::ostream& anOutput, Statement* aStmt) {
        Database* db = dbProc->getDB();
        if (!db) return StatusResult{ Errors::noDatabaseSpecified };

        StatusResult status;
        DropTableStatement* stmt = static_cast<DropTableStatement*>(aStmt);
        
        if (!(status = db->dropTable(stmt->getName()))) return status;

        DropTableView view{};
        view.show(anOutput, timer);

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLProcessor::insertRows(std::ostream& anOutput, Statement* aStmt) {
        Database* db = dbProc->getDB();
        if (!db) return StatusResult{ Errors::noDatabaseSpecified };

        StatusResult status;
        InsertTableStatement* stmt = static_cast<InsertTableStatement*>(aStmt);

        if (!db->hasTable(stmt->getName())) return StatusResult{ Errors::unknownTable };

        for (auto& kv : stmt->getKVs()) {
            if (!(status = db->insertRow(stmt->getName(), kv))) return status;
        }

        InsertView view{stmt->getKVs().size()};
        view.show(anOutput, timer);

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLProcessor::selectRows(std::ostream& anOutput, Statement* aStmt) {
        Database* db = dbProc->getDB();
        if (!db) return StatusResult{ Errors::noDatabaseSpecified };

        SelectTableStatement* stmt = static_cast<SelectTableStatement*>(aStmt);
        if (!db->hasTable(stmt->getName())) return StatusResult{ Errors::unknownTable };

        StatusResult status;
        Query& theQuery = stmt->getQuery();
        Entity theEntity = theQuery.getFrom();
        RowCollection rows;

        // validate joins
        for (auto &join : stmt->getJoins()) {
            if (!join.validate(db, theEntity)) return StatusResult{ Errors::unknownTable };
        }

        // filter rows using any where clause
        if (!(status = db->selectRows(theQuery, rows))) return status;

        // do any joins
        for (auto &join : stmt->getJoins()) {
            join.execute(db, theEntity, rows);
        }

        return showQuery(anOutput, theQuery, rows);
    }
    
    StatusResult SQLProcessor::showIndex(std::ostream& anOutput, Statement* aStmt) {
        Database* db = dbProc->getDB();
        if (!db) return StatusResult{ Errors::noDatabaseSpecified };

        StatusResult status;
        ShowIndexStatement* stmt = static_cast<ShowIndexStatement*>(aStmt);

        if (!db->hasTable(stmt->getName())) return StatusResult{ Errors::unknownEntity };

        IndexView theIndexView(db, stmt->getName(), stmt->getIndexNames());
        theIndexView.show(anOutput, timer);

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLProcessor::showQuery(std::ostream& anOutput, Query &aQuery, RowCollection &aRows) {
        Database* db = dbProc->getDB();
        if (!db) return StatusResult{ Errors::noDatabaseSpecified };
        StatusResult status;

        if (!(status = aQuery.run(aRows))) return status;

        SelectView view{aRows, aQuery, db->getEntity(aQuery.getFrom().getName())};
        view.show(anOutput, timer);
        
        return StatusResult{ Errors::noError };
    }

    StatusResult SQLProcessor::showTables(std::ostream& anOutput, Statement* aStmt) {
        Database* db = dbProc->getDB();
        if (!db) return StatusResult{ Errors::noDatabaseSpecified };

        TabularView view{db};
        view.show(anOutput, timer);

        return StatusResult{ Errors::noError };
    }

    StatusResult SQLProcessor::updateRows(std::ostream& anOutput, Statement* aStmt) {
        Database* db = dbProc->getDB();
        if (!db) return StatusResult{ Errors::noDatabaseSpecified };
        
        UpdateTableStatement* stmt = static_cast<UpdateTableStatement*>(aStmt);
        if (!db->hasTable(stmt->getName())) return StatusResult{ Errors::unknownTable };

        StatusResult status;
        UpdateQuery& theQuery = stmt->getQuery();

        if (!(status = db->updateRows(theQuery))) return status;

        UpdateView view{status.value};
        view.show(anOutput, timer);

        return StatusResult{ Errors::noError };
    }

    StatusResult  SQLProcessor::run(Statement* aStmt, const Timer& aTimer) {
        timer = aTimer;
        switch (aStmt->getType()) {
        case Keywords::alter_kw:    return alterTable(output, aStmt);
        case Keywords::create_kw:   return createTable(output, aStmt);
        case Keywords::delete_kw:   return deleteRows(output, aStmt);
        case Keywords::describe_kw: return describeTable(output, aStmt);
        case Keywords::drop_kw:     return dropTable(output, aStmt);
        case Keywords::index_kw:    return showIndex(output, aStmt);
        case Keywords::insert_kw:   return insertRows(output, aStmt);
        case Keywords::select_kw:   return selectRows(output, aStmt);
        case Keywords::show_kw:     return showTables(output, aStmt);
        case Keywords::update_kw:   return updateRows(output, aStmt);
        default: break;
        }
        return StatusResult{ Errors::unknownCommand };
    }
}
