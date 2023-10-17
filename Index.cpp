
#include <variant>
#include <sstream>
#include "Helpers.hpp"
#include "Index.hpp"

namespace ECE141 {

    std::istream& operator>>(std::istream &in, IndexKeyType &aKeyType) {
        size_t aTypeVal;
        in >> aTypeVal;
        aKeyType = static_cast<IndexKeyType>(aTypeVal);
        return in;
    }

    std::ostream& operator<<(std::ostream &out, const IndexKeyType &aKeyType) {
        out << static_cast<size_t>(aKeyType);
        return out;
    }
    
    std::istream& operator>>(std::istream &in, IndexKey &anIndexKey) {
        std::visit([&in](auto &anIndexKey) {
            in >> anIndexKey;
        }, anIndexKey);
        return in;
    }

    std::ostream& operator<<(std::ostream &out, const IndexKey &anIndexKey) {
        std::visit([&out](auto const &anIndexKey) {
            out << anIndexKey;
        }, anIndexKey);
        return out;
    }

    std::istream& operator>>(std::istream &in, IndexPair &anIndexPair) {
        char aTypeChar;
        in >> aTypeChar;
        switch (static_cast<IndexKeyType>(aTypeChar)) {
            case IndexKeyType::uint32_t:    anIndexPair.first = IndexKey{uint32_t()}; break;
            case IndexKeyType::string:      anIndexPair.first = IndexKey{std::string()}; break;
            default: break;
        }
        in >> anIndexPair.first >> anIndexPair.second;
        return in;
    }

    std::ostream& indexPairEncode(std::ostream &out, const IndexPair &aValue) {
        IndexKeyType keyType = Helpers::toindexKeyType(aValue.first.index());
        out << static_cast<char>(keyType) << " " << aValue.first << " " << aValue.second;
        return out;
    }

    Index::Index(std::iostream &aStream, std::string aName, uint32_t aBlockNum)
        : stream(aStream), name(aName), blockNum(aBlockNum) {}
    Index::~Index() {}

    StatusResult Index::insert(IndexKey anIndexKey, uint32_t aBlockNum) {
        indexMap.insert(IndexPair{anIndexKey, aBlockNum});

        return StatusResult{ Errors::noError };
    }

    IndexMap::iterator Index::get(IndexKey anIndexKey) {
        return indexMap.find(anIndexKey);
    }

    bool Index::hasKey(IndexKey anIndexKey) {
        return indexMap.find(anIndexKey) != indexMap.end();
    }

    bool Index::erase(IndexKey anIndexKey) {
        if (!hasKey(anIndexKey)) return false;

        return indexMap.erase(anIndexKey);
    }

    bool Index::each(const BlockVisitor aVisitor) {
        BlockIO blockIO{stream};
        for (auto& kvPair : indexMap) {
            Block tempBlock;
            if (!blockIO.readBlock(kvPair.second, tempBlock)) return false;
            if (!aVisitor(tempBlock, tempBlock.header.id)) return false;
        }
        return true;
    }

    bool Index::changeEach(const ChangeBlockVisitor aVisitor) {
        BlockIO blockIO{stream};
        for (auto& kvPair : indexMap) {
            Block tempBlock;
            if (!blockIO.readBlock(kvPair.second, tempBlock)) return false;
            if (!aVisitor(tempBlock, tempBlock.header.id)) return false;
            if (!blockIO.writeBlock(tempBlock.header.pos, tempBlock)) return false;
        }
        return true;
    }

    bool Index::eachPair(const IndexVisitor aVisitor) {
        for (auto& kvPair : indexMap) {
            IndexKeyType keyType = Helpers::toindexKeyType(kvPair.first.index());
            if (!aVisitor(keyType, kvPair)) return false;
        }
        return true;
    }

    size_t Index::size() {
        return indexMap.size();
    }

    void Index::setBlockNum(uint32_t aBlockNum) {
        blockNum = aBlockNum;
    }

    uint32_t Index::getBlockNum() const {
        return blockNum;
    }

    std::string Index::getName() const {
        return name;
    }

    StatusResult Index::encode(std::ostream &anOutput) {
        anOutput << name << " ";
        anOutput << blockNum << " ";
        anOutput << indexMap.size() << " ";
        for (auto& indexEntry : indexMap) {
            indexPairEncode(anOutput, indexEntry);
            anOutput << " ";
            // anOutput << indexEntry << " ";
        }
        return StatusResult { Errors::noError };
    }

    StatusResult Index::decode(std::istream &anInput) {
        size_t indexSize;
        IndexPair indexEntry;

        anInput >> name;
        anInput >> blockNum;
        anInput >> indexSize;
        for (size_t i = 0; i < indexSize; i++) {
            anInput >> indexEntry;
            indexMap.insert(indexEntry);
        }

        return StatusResult { Errors::noError };
    }
}

