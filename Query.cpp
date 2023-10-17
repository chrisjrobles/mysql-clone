//
//  Query.cpp
//  Assignment5
//
//  Created by rick gessner on 4/26/21.
//

#include <utility>
#include "Query.hpp"
#include "Helpers.hpp"

namespace ECE141 {

    Query::Query() : limit(0), selectsAll(false), offset(0) {}

    Query::Query(Query &aQuery) {
        _from = aQuery._from;
        filters = std::move(aQuery.filters);
        limit = aQuery.limit;
        selectsAll = aQuery.selectsAll;
        selectFields = aQuery.selectFields;
        orderFields = aQuery.orderFields;
        offset = aQuery.offset;
    }

    Query& Query::operator=(Query& aQuery) {
        _from = aQuery._from;
        filters = std::move(aQuery.filters);
        limit = aQuery.limit;
        selectsAll = aQuery.selectsAll;
        selectFields = aQuery.selectFields;
        orderFields = aQuery.orderFields;
        offset = aQuery.offset;
        return *this;
    }

    bool Query::addSelectField(std::string aField) {
        selectFields.push_back(aField);
        return true;
    }

    bool Query::addOrderField(std::string aField) {
        orderFields.push_back(aField);
        return true;
    }

    Entity& Query::getFrom() { return _from; }

    bool Query::getSelectAll() const { return selectsAll; }

    StringList Query::getSelectFields() const { return selectFields; }

    Query& Query::setSelectAll() {
        selectsAll = true;
        return *this;
    }

    Query& Query::setFrom(Entity anEntity) {
        _from = anEntity;
        return *this;
    }

    Query& Query::setLimit(int aLimit) {
        limit = aLimit;
        return *this;
    }

    Query& Query::setOffset(int anOffset) {
        offset = anOffset;
        return *this;
    }

    Query& Query::setFilters(Filters &aFilters) {
        filters = std::move(aFilters);
        return *this;
    }

    bool Query::matches(const Row& aRow) const {
        return filters.matches(aRow.getDataCopy());
    }

    StatusResult Query::run(RowCollection& aRows) const {
        if (!orderFields.empty())           std::sort(aRows.begin(), aRows.end(), getRowComparer());
        if (limit && limit<aRows.size())    aRows.erase(aRows.begin()+limit, aRows.end());

        return StatusResult{ Errors::noError };
    }

    RowComparer Query::getRowComparer() const {
        return [this](RowPtr& row1, RowPtr& row2)->bool {
            for (const auto& orderField: orderFields) {
                if (row1->getValue(orderField) != row2->getValue(orderField)) {
                    return row1->getValue(orderField) < row2->getValue(orderField);
                }
            }
            return true;
        };
    }




    // ------------------------------
    // Update Query
    // ------------------------------

    UpdateQuery::UpdateQuery() {}
    UpdateQuery::UpdateQuery(UpdateQuery &aQuery) {}
    UpdateQuery& UpdateQuery::operator=(UpdateQuery& aQuery) { return *this; }

    bool UpdateQuery::addUpdateField(std::string aKey, Value aValue) {
        try {
            updateFields[aKey] = aValue;
        }
        catch (...) {
            return false;
        }
        return true;
    }

    KeyValues UpdateQuery::getUpdateFields() const { return updateFields; }

}