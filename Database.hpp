//
//  Database.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>
#include <string>
#include <fstream>
#include <functional>
#include <map>
#include <vector>
#include "BasicTypes.hpp"
#include "Entity.hpp"
#include "Index.hpp"
#include "Query.hpp"
#include "Storage.hpp"

namespace ECE141 {

    using EntityVisitor = std::function<bool(const Entity&)>;
    using RowVisitor = std::function<bool(const Row&)>;
    using ChangeRowVisitor = std::function<bool(Row&)>;
    using AlterEntityFunc = std::function<bool(Entity&, Attribute)>;

    const std::string PRIMARY_KEY = "primary_key";
    const std::string ENTITY_INDEX = "entity_index";
    const std::string TABLE_INDEX_INDEX = "table_index_index";

    class Database : public Storable {
    public:

        Database(const std::string aPath, CreateDB);
        Database(const std::string aPath, OpenDB);
        virtual ~Database();

        // table operations
        StatusResult    addTable(Entity& anEntity);
        StatusResult    alterTable(std::string anEntityName, AlterType anAlterType, Attribute anAlterAttr);
        StatusResult    dropTable(std::string anEntityName);
        bool            hasTable(std::string anEntityName);

        // row operations
        StatusResult    deleteRow(std::string anEntityName, IndexKey aRowId);
        StatusResult    deleteRows(Query &aQuery);
        StatusResult    insertRow(std::string anEntityName, KeyValues rowValues);
        StatusResult    selectRows(Query &aQuery, RowCollection &aRows);
        StatusResult    updateRows(UpdateQuery &aQuery);

        // getters
        Entity          getEntity(std::string anEntityName);
        std::string&    getName();
        size_t          getTableAmount();
        Index           getTableIndexIndexCopy(); // TODO
        std::fstream&   getStream();
        Storage&        getStorage();

        // iterators
        bool            forEachEntity(const EntityVisitor& aBlockVisitor);
        bool            forEachRow(std::string aTableName, const RowVisitor& aRowVisitor);
        bool            changeEachRow(std::string aTableName, const ChangeRowVisitor& aRowVisitor);
        bool            forEachIndex(const IndexVisitor& anIndexVisitor);
        bool            forEachIndexPair(std::string anEntityName, const IndexVisitor& anIndexVisitor, std::string aFieldName=PRIMARY_KEY); // TODO

        // storable
        StatusResult    encode(std::ostream& anOutput) override;
        StatusResult    decode(std::istream& anInput) override;

    protected:
        bool            hasTable(uint32_t entityHash);

        StatusResult    saveMetaBlock();
        StatusResult    loadMetaBlock();
        StatusResult    saveIndexBlock();
        StatusResult    loadIndexBlock();
        StatusResult    saveDatabase();
        StatusResult    loadDatabase();
        StatusResult    saveEntity(Entity& anEntity);
        StatusResult    loadEntity(Entity& anEntity);
        StatusResult    saveTableIndex(Index& anIndex, uint32_t anEntityHash);
        StatusResult    loadTableIndex(Index& anIndex);
        StatusResult    saveRow(Row& aRow, uint32_t anEntityHash);

        std::string     name;               // dbName
        Storage         storage;            // storage object for saving and loading to file
        bool            changed;            // whether database has been changed since initialized
        Index           entityIndex;        // index of entity locations in db
        Index           tableIndexIndex;    // index of table primary key indexes
        std::fstream    stream;             // stream storage uses for IO

        static const uint32_t metaBlockLocation = 0;
        static const uint32_t tableIndexIndexBlockLocation = 1;
    };

}
#endif /* Database_hpp */
