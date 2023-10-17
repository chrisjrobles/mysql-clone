//
//  Database.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <cstring>
#include "Config.hpp"
#include "Database.hpp"
#include "Entity.hpp"
#include "Helpers.hpp"
#include "Storage.hpp"

namespace ECE141 {

    bool addAttrToEntity(Entity &anEntity, Attribute addAttr) {
        if (!anEntity.addAttribute(addAttr)) return false;
        return true;
    }

    bool dropAttrFromEntity(Entity &anEntity, Attribute dropAttr) {
        if (!anEntity.dropAttribute(dropAttr.getName())) return false;
        return true;
    }

    Database::Database(const std::string aName, CreateDB)
        : name(aName), storage(stream), changed(true), 
          entityIndex(stream, ENTITY_INDEX, metaBlockLocation), 
          tableIndexIndex(stream, TABLE_INDEX_INDEX, tableIndexIndexBlockLocation) {
        std::string thePath = Config::getDBPath(name);
        stream.clear(); // Clear Flag, then create file...
        stream.open(thePath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
        stream.close();
        stream.open(thePath.c_str(), std::fstream::binary | std::fstream::binary | std::fstream::in | std::fstream::out);

        saveDatabase();
    }

    Database::Database(const std::string aName, OpenDB)
        : name(aName), storage(stream), changed(false), 
          entityIndex(stream, ENTITY_INDEX, metaBlockLocation), 
          tableIndexIndex(stream, TABLE_INDEX_INDEX, tableIndexIndexBlockLocation)  {

        std::string thePath = Config::getDBPath(name);
        stream.open(thePath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out);

        loadDatabase();
    }

    Database::~Database() {
        if (changed) {
            saveDatabase();
        }
        stream.close();
    }

    StatusResult Database::addTable(Entity& anEntity) {
        StatusResult status;

        if (hasTable(anEntity.getName())) return StatusResult{ Errors::tableExists };
        changed = true;

        uint32_t entityHash = Entity::hashString(anEntity.getName());
        // save block and make entityIndex entry
        if (!(status = saveEntity(anEntity))) return status;
        uint32_t entityBlockNum = status.value;
        if (!(status = entityIndex.insert(entityHash, entityBlockNum))) return status;

        // create tableIndex and make indexIndex entry
        std::string primaryIndexName = anEntity.getName() + '_' + anEntity.getPrimaryAttributeName();
        Index tableIndex{stream, primaryIndexName};
        if (!(status = saveTableIndex(tableIndex, entityHash))) return status;
        uint32_t indexBlockNum = status.value;
        if (!(status = tableIndexIndex.insert(primaryIndexName, indexBlockNum))) return status;

        // save
        if (!(status = saveDatabase())) return status;

        return StatusResult{ Errors::noError };
    }

    StatusResult Database::alterTable(std::string anEntityName, AlterType anAlterType, Attribute anAlterAttr) {
        StatusResult status;

        if (!hasTable(anEntityName)) return StatusResult{ Errors::unknownTable };
        Entity entity = getEntity(anEntityName);


        // Set alter functionality
        AlterEntityFunc  anAlterEntityFunc;
        ChangeRowVisitor anAlterRowFunc;
        switch(anAlterType) {
            case AlterType::add: {
                anAlterEntityFunc = addAttrToEntity;
                anAlterRowFunc = [&anAlterAttr] (Row& aRow)->bool {
                    aRow.setValue(anAlterAttr.getName(), NullType{});
                    return true;
                };
                break;
            }
            case AlterType::drop: {
                anAlterEntityFunc = dropAttrFromEntity;
                anAlterRowFunc = [&anAlterAttr] (Row& aRow)->bool {
                    aRow.dropValue(anAlterAttr.getName());
                    return true;
                };
                break;
            }
        }

        if (!anAlterEntityFunc(entity, anAlterAttr)) return StatusResult{ Errors::invalidAttribute };
        if (!(status = saveEntity(entity))) return status;
        if (!changeEachRow(anEntityName, anAlterRowFunc)) return StatusResult{ Errors::unknownError };

        return StatusResult{ Errors::noError };
    }

    StatusResult Database::dropTable(std::string anEntityName) {
        StatusResult status;

        if (!hasTable(anEntityName)) return StatusResult{ Errors::unknownTable };
        changed = true;

        uint32_t entityHash = Entity::hashString(anEntityName);
        // erase rows
        Entity entityToDelete = getEntity(anEntityName);
        Query deleteRowsQuery{};
        deleteRowsQuery.setFrom(entityToDelete);
        if (!(status = deleteRows(deleteRowsQuery))) return status;
        uint32_t amountOfDroppedRows = status.value;

        // erase entity block and entityIndex entry
        uint32_t entityBlockNum = entityIndex.get(entityHash)->second;
        if (!(status = storage.releaseBlocks(entityBlockNum))) return status;
        if (!(entityIndex.erase(entityHash))) return StatusResult{ Errors::unknownIndex };

        // find indexes that need to be erased
        StringList indexesToErase;
        tableIndexIndex.eachPair([&anEntityName, &indexesToErase, this](IndexKeyType aKeyType, const IndexPair &aPair) -> bool {
            switch(aKeyType) {
                case IndexKeyType::uint32_t:    break;
                case IndexKeyType::string:      {
                    std::string indexName = std::get<std::string>(aPair.first);
                    if (indexName.find(anEntityName) != std::string::npos) {
                        indexesToErase.push_back(indexName);
                    }
                    break;
                }
                default:                        break;
            }
            return true;
        });

        // erase indexes
        for (auto& indexName : indexesToErase) {
            uint32_t indexLocation = tableIndexIndex.get(indexName)->second;
            if (!(status = storage.releaseBlocks(indexLocation))) return status;
            if (!tableIndexIndex.erase(indexName)) return StatusResult{ Errors::unknownIndex };
        }

        // save
        if (!(status = saveDatabase())) return status;
        status.value = amountOfDroppedRows;

        return status;
    }

    bool Database::hasTable(std::string anEntityName) {
        return hasTable(Entity::hashString(anEntityName));
    }

    bool Database::hasTable(uint32_t entityHash) {
        return entityIndex.hasKey(entityHash);
    }

    Entity Database::getEntity(std::string anEntityName) {
        if (!hasTable(anEntityName)) return Entity{};

        Entity entity{anEntityName};
        if (!loadEntity(entity)) return Entity{};

        return entity;
    }

    StatusResult Database::insertRow(std::string anEntityName, KeyValues rowValues) {
        StatusResult status;
        Entity entity;
        if (!(entity = getEntity(anEntityName))) return StatusResult{ Errors::unknownEntity };

        Row row = Row{Entity::hashString(entity.getName()), rowValues};

        if (!(status = entity.prepareRow(row))) return status;
        if (!entity.validateRow(row)) return StatusResult{ Errors::invalidArguments };
        if (!(status = saveEntity(entity))) return status;

        uint32_t entityHash = Entity::hashString(anEntityName);

        // save row
        std::string primaryAttributeName = entity.getPrimaryAttributeName();
        if (primaryAttributeName.empty()) return StatusResult{ Errors::unknownAttribute };
        int rowPrimaryKey = std::get<int>(row.getValueCopy(primaryAttributeName));
        row.setPrimaryKey(rowPrimaryKey);
        if (!(status = saveRow(row, entityHash))) return status;
        uint32_t rowBlockNum = status.value;

        // update and save table indexes
        tableIndexIndex.eachPair([&](IndexKeyType aKeyType, const IndexPair &aPair) -> bool {
            switch(aKeyType) {
                case IndexKeyType::uint32_t:    break;
                case IndexKeyType::string:      {
                    std::string indexName = std::get<std::string>(aPair.first);
                    if (indexName.find(anEntityName) != std::string::npos) {
                        Index tableIndex{stream, indexName};
                        if (!loadTableIndex(tableIndex)) return false;
                        if (!tableIndex.insert((uint32_t) rowPrimaryKey, rowBlockNum)) return false; // does not insert proper key value yet
                        if (!saveTableIndex(tableIndex, entityHash)) return false;
                    }
                    break;
                }
                default:                        break;
            }
            return true;
        });

        return StatusResult{ Errors::noError };
    }

    StatusResult Database::deleteRow(std::string anEntityName, IndexKey aRowId) {
        StatusResult status;
        uint32_t entityHash = Entity::hashString(anEntityName);
        if (!hasTable(entityHash)) return StatusResult{ Errors::unknownEntity };

        // update and save table indexes
        uint32_t rowLocation;
        if (!tableIndexIndex.eachPair([&](IndexKeyType aKeyType, const IndexPair &aPair) -> bool {
            switch(aKeyType) {
                case IndexKeyType::uint32_t:    break;
                case IndexKeyType::string:      {
                    std::string indexName = std::get<std::string>(aPair.first);
                    if (indexName.find(anEntityName) != std::string::npos) {
                        Index tableIndex{stream, indexName};
                        if (!loadTableIndex(tableIndex)) return false;
                        rowLocation = tableIndex.get(aRowId)->second;
                        if (!tableIndex.erase(aRowId)) return false;
                        if (!saveTableIndex(tableIndex, entityHash)) return false;
                    }
                    break;
                }
                default:                        break;
            }
            return true;
        })) return StatusResult{ Errors::unknownIndex };

        if (!rowLocation) return StatusResult{ Errors::unknownIndex };
        if (!(status = storage.releaseBlocks(rowLocation))) return status;

        return StatusResult{ Errors::noError };
    }

    StatusResult Database::deleteRows(Query &aQuery) {
        StatusResult status;
        std::string entityName = aQuery.getFrom().getName();
        if (!forEachRow(entityName, [this, &entityName, &aQuery, &status](const Row& aRow) {
            if (aQuery.matches(aRow)) {
                if (!deleteRow(entityName, (uint32_t) aRow.getPrimaryKey())) return false;
                status.value++;
            }
            return true;
        })) return StatusResult{ Errors::unknownError };

        return status;

    }

    StatusResult Database::selectRows(Query &aQuery, RowCollection &aRows) {
        forEachRow(aQuery.getFrom().getName(), [&aQuery, &aRows](const Row& aRow) {
            if (aQuery.matches(aRow)) {
                aRows.push_back(std::make_unique<Row>(aRow));
            }
            return true;
        });

        return StatusResult{ Errors::noError };
    }

    StatusResult Database::updateRows(UpdateQuery &aQuery) {
        StatusResult status;
        std::string entityName = aQuery.getFrom().getName();
        forEachRow(entityName, [this, &aQuery, &status, &entityName](const Row& aRow) {
            if (aQuery.matches(aRow)) {
                for (auto& kv : aQuery.getUpdateFields()) {
                    Row theRow(aRow);
                    theRow.setValue(kv.first, kv.second);
                    saveRow(theRow, Entity::hashString(entityName));
                }
                status.value++;
            }
            return true;
        });

        return status;
    }

    std::string& Database::getName() {
        return name;
    }

    size_t Database::getTableAmount() {
        return entityIndex.size();
    }

    std::fstream& Database::getStream() {
        return stream;
    }

    Storage& Database::getStorage() {
        return storage;
    }

    bool Database::forEachEntity(const EntityVisitor& anEntityVisitor) {
        return entityIndex.each([&](const Block& aBlock, uint32_t aBlockNum) {
            std::stringstream payload;
            Entity anEntity;
            payload << aBlock.payload;
            if (!anEntity.decode(payload)) return false;
            return anEntityVisitor(anEntity);
        });
    }

    bool Database::forEachRow(std::string aTableName, const RowVisitor& aRowVisitor) {
        StatusResult status;
        Entity anEntity = getEntity(aTableName);
        std::string primaryIndexName = aTableName + '_' + anEntity.getPrimaryAttributeName();
        Index tableIndex{stream, primaryIndexName};
        
        if (!(status = loadTableIndex(tableIndex))) return false;

        return tableIndex.each([&](const Block& aBlock, uint32_t aBlockNum) {
            std::stringstream payload;
            Row aRow;
            payload << aBlock.payload;
            if (!aRow.decode(payload)) return false;
            return aRowVisitor(aRow);
        });
    }

    bool Database::changeEachRow(std::string aTableName, const ChangeRowVisitor& aRowVisitor) {
        StatusResult status;
        Entity anEntity = getEntity(aTableName);
        std::string primaryIndexName = aTableName + '_' + anEntity.getPrimaryAttributeName();
        Index tableIndex{stream, primaryIndexName};
        
        if (!(status = loadTableIndex(tableIndex))) return false;

        return tableIndex.changeEach([&](Block& aBlock, uint32_t aBlockNum) {
            std::stringstream payload;
            Row aRow;
            payload << aBlock.payload;
            if (!aRow.decode(payload)) return false;
            return aRowVisitor(aRow);
        });
    }

    bool Database::forEachIndex(const IndexVisitor& anIndexVisitor) {
        return tableIndexIndex.eachPair(anIndexVisitor);
    }

    bool Database::forEachIndexPair(std::string anEntityName, const IndexVisitor& anIndexVisitor, std::string aFieldName) {
        Entity entity;
        if (!(entity = getEntity(anEntityName))) return false;

        std::string fieldName = aFieldName==PRIMARY_KEY ? entity.getPrimaryAttributeName() : aFieldName;
        std::string tableIndexName = anEntityName + '_' + fieldName;
        Index tableIndex{stream, tableIndexName};

        if (!loadTableIndex(tableIndex)) return false;
        return tableIndex.eachPair(anIndexVisitor);
    }

    StatusResult Database::saveMetaBlock() {
        StatusResult status;
        std::stringstream payload;

        if (!(status = encode(payload))) return status;
        payload.seekg(0, std::ios::end);
        StorageInfo info(payload.tellg());
        info.id    = Entity::hashString(name);
        info.start = metaBlockLocation;
        info.type  = BlockType::meta_block;

        return storage.save(payload, info);
    }

    StatusResult Database::loadMetaBlock() {
        StatusResult status;
        std::stringstream payload;

        if (!(status = storage.load(payload, metaBlockLocation))) return status;
        return decode(payload);
    }

    StatusResult Database::saveIndexBlock() {
        StatusResult status;
        std::stringstream payload{};

        if (!(status = tableIndexIndex.encode(payload))) return status;
        payload.seekg(0, std::ios::end);
        StorageInfo info(payload.tellg());
        info.refId = Entity::hashString(name);
        info.start = tableIndexIndexBlockLocation;
        info.type  = BlockType::index_block;

        return storage.save(payload, info);
    }
    
    StatusResult Database::loadIndexBlock() {
        StatusResult status;
        std::stringstream payload;

        if (!(status = storage.load(payload, tableIndexIndexBlockLocation))) return status;
        return tableIndexIndex.decode(payload);
    }

    StatusResult Database::saveDatabase() {
        StatusResult status;

        if (!(status = saveMetaBlock())) return status;
        if (!(status = saveIndexBlock())) return status;

        return StatusResult{ Errors::noError };
    }

    StatusResult Database::loadDatabase() {
        StatusResult status;

        if (!(status = loadMetaBlock())) return status;
        if (!(status = loadIndexBlock())) return status;

        return status;
    }

    StatusResult Database::saveEntity(Entity& anEntity) {
        StatusResult status;
        std::stringstream payload;
        
        if (anEntity.getBlockNum()==0) anEntity.setBlockNum(storage.getFreeBlock());

        if (!(status = anEntity.encode(payload))) return status;
        payload.seekg(0, std::ios::end);
        StorageInfo info(payload.tellg());
        info.id    = Entity::hashString(anEntity.getName());
        info.refId = Entity::hashString(name);
        info.start = anEntity.getBlockNum();
        info.type  = BlockType::entity_block;

        return storage.save(payload, info);
    }

    StatusResult Database::loadEntity(Entity& anEntity) {
        StatusResult status;
        std::stringstream payload;

        uint32_t entityHash = Entity::hashString(anEntity.getName());
        uint32_t entityBlockNum = entityIndex.get(entityHash)->second;

        if (!(status = storage.load(payload, entityBlockNum))) return status;
        if (!(status = anEntity.decode(payload))) return status;

        return StatusResult{ Errors::noError };
    }
    
    StatusResult Database::saveTableIndex(Index& anIndex, uint32_t anEntityHash) {
        StatusResult status;
        std::stringstream payload;

        if (anIndex.getBlockNum()==0) anIndex.setBlockNum(storage.getFreeBlock());

        if (!(status = anIndex.encode(payload))) return status;
        payload.seekg(0, std::ios::end);
        StorageInfo info(payload.tellg());
        info.refId = anEntityHash;
        info.start = anIndex.getBlockNum();
        info.type  = BlockType::index_block;

        return storage.save(payload, info);
    }

    StatusResult Database::loadTableIndex(Index& anIndex) {
        StatusResult status;
        std::stringstream payload;
        uint32_t tableIndexBlockNum = tableIndexIndex.get(anIndex.getName())->second;

        if (!(status = storage.load(payload, tableIndexBlockNum))) return status;
        if (!(status = anIndex.decode(payload))) return status;

        return StatusResult{ Errors::noError };
    }

    StatusResult Database::saveRow(Row& aRow, uint32_t anEntityHash) {
        StatusResult status;
        std::stringstream payload;

        if (aRow.getBlockNum()==0) aRow.setBlockNum(storage.getFreeBlock());

        if (!(status = aRow.encode(payload))) return status;
        payload.seekg(0, std::ios::end);
        StorageInfo info(payload.tellg());
        info.id    = (uint32_t) aRow.getPrimaryKey();
        info.refId = anEntityHash;
        info.start = aRow.getBlockNum();
        info.type  = BlockType::data_block;

        return storage.save(payload, info);
    }

    StatusResult Database::encode(std::ostream& anOutput) {
        anOutput << name << " ";
        return entityIndex.encode(anOutput);
    }

    StatusResult Database::decode(std::istream& anInput) {
        anInput >> name;
        return entityIndex.decode(anInput);
    }

}
