//
//  Query.hpp
//  Assignment5
//
//  Created by rick gessner on 4/26/21.
//

#ifndef Query_hpp
#define Query_hpp

#include <stdio.h>
#include <string>
#include <functional>
#include "Attribute.hpp"
#include "Row.hpp"
#include "Entity.hpp"
#include "Tokenizer.hpp"
#include "Filters.hpp"

namespace ECE141 {

    using RowSelector = std::function<bool(const Row&)>;
    using RowComparer = std::function<bool(RowPtr&, RowPtr&)>;

    class Query {
    public:

        Query();
        Query(Query &aQuery);
        Query& operator=(Query& aQuery);

        // getters and setters
        bool            addSelectField(std::string aField);
        bool            addOrderField(std::string aField);
        Entity&         getFrom();
        bool            getSelectAll() const;
        StringList      getSelectFields() const;
        Query&          setSelectAll();
        Query&          setFrom(Entity anEntity);
        Query&          setLimit(int aLimit);
        Query&          setOffset(int anOffset);
        Query&          setFilters(Filters &aFilters);

        bool            matches(const Row& aRow) const;
        StatusResult    run(RowCollection& aRows) const;

    protected:
        RowComparer     getRowComparer() const;

        Entity          _from;
        Filters         filters;
        size_t          limit;
        bool            selectsAll;
        StringList      selectFields;
        StringList      orderFields;
        int             offset;
    };

    class UpdateQuery : public Query {
    public:

        UpdateQuery();
        UpdateQuery(UpdateQuery &&aQuery) = default;
        UpdateQuery(UpdateQuery &aQuery);
        UpdateQuery& operator=(UpdateQuery& aQuery);

        bool addUpdateField(std::string aKey, Value aValue);
        KeyValues getUpdateFields() const;

    protected:

        KeyValues updateFields;

    };
}
#endif /* Query_hpp */
