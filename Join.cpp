#include "Entity.hpp"
#include "Helpers.hpp"
#include "Join.hpp"

namespace ECE141 {

    bool Join::validate(Database* theDB, Entity& rootEntity) {
        // Check if join table exists
        if (!theDB->hasTable(table)) return false;

        // Check if join type is valid
        if (!Helpers::in_array<Keywords>(gJoinTypes, joinType)) return false;

        // Check if join operand tables match statement tables
        bool leftTableIsTable = expr.lhs.entityId==Entity::hashString(table);
        bool rightTableIsTable = expr.rhs.entityId==Entity::hashString(table);
        bool leftTableIsRootTable = expr.lhs.entityId==Entity::hashString(rootEntity.getName());
        bool rightTableIsRootTable = expr.rhs.entityId==Entity::hashString(rootEntity.getName());
        if (!(leftTableIsTable && rightTableIsRootTable) && !(rightTableIsTable && leftTableIsRootTable)) return false;

        // Check if operand fields exist
        Entity leftEntity, rightEntity;
        if (leftTableIsTable) {
            leftEntity = theDB->getEntity(table);
            rightEntity = rootEntity;
        } else {
            leftEntity = rootEntity;
            rightEntity = theDB->getEntity(table);
        }
        if (!leftEntity.hasAttribute(expr.lhs.name)) return false;
        if (!rightEntity.hasAttribute(expr.rhs.name)) return false;

        return true;
    }

    bool Join::execute(Database* theDB, Entity& rootEntity, RowCollection &rootRows) {
        Query selectAllQuery;
        Entity joinEntity = theDB->getEntity(table);
        selectAllQuery.setFrom(joinEntity);
        RowCollection rightRows, joinedRows;
        if (!theDB->selectRows(selectAllQuery, rightRows)) return false;

        // Join rows
        switch(joinType) {
            case Keywords::left_kw:     executeLeft(joinedRows, rootRows, rightRows); break;
            case Keywords::right_kw:    executeLeft(joinedRows, rightRows, rootRows); break;
            default:                    executeLeft(joinedRows, rootRows, rightRows); break;
        }

        // Update root entity with join fields
        for (auto &attrPair : joinEntity.getAttributes()) {
            if (!rootEntity.hasAttribute(attrPair.first)) {
                rootEntity.addAttribute(attrPair.second);
            }
        }

        rootRows = std::move(joinedRows);
        
        return true;
    }

    bool Join::executeLeft(RowCollection &joinedRows, const RowCollection &leftRows, const RowCollection &rightRows) {
        for (const auto &leftRow : leftRows) {
            bool foundMatch = false;
            for (const auto &rightRow : rightRows) {
                Value leftValue = leftRow->getValueCopy(expr.lhs.name);
                Value rightValue = rightRow->getValueCopy(expr.rhs.name);
                // Check if field values match
                if (equals(leftValue, rightValue)) {
                    Row combinedRow;
                    foundMatch = true;
                    // Add data from each row
                    for (auto &kv : leftRow->getDataCopy()) {
                        combinedRow.setValue(kv.first, kv.second);
                    }
                    for (auto &kv : rightRow->getDataCopy()) {
                        combinedRow.setValue(kv.first, kv.second);
                    }
                    // Add combined row to joined rows
                    joinedRows.push_back(std::make_unique<Row>(combinedRow));
                } 
            }
            // Add left row if did not find matching values
            if (!foundMatch) {
                joinedRows.push_back(std::make_unique<Row>(*leftRow.get()));
            }
        }
        return true;
    }

    void Join::setExpr(Expression &anExpr) {
        expr = anExpr;
    }

}