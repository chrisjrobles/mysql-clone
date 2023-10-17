
#ifndef Index_hpp
#define Index_hpp

#include <map>
#include <variant>
#include "Storage.hpp"

namespace ECE141 {

    enum class IndexKeyType : char {
        uint32_t='u', string='s'
    };

    using IndexKey  = std::variant<uint32_t, std::string>;
    using IndexPair = std::pair<IndexKey, uint32_t>;
    using IndexMap  = std::multimap<IndexKey, uint32_t>;
    using IndexVisitor = std::function<bool(IndexKeyType, const IndexPair&)>;

    class Index : public Storable, BlockIterator {
    public:

        Index(std::iostream &aStream, std::string aName, uint32_t aBlockNum=0);
        ~Index();

        StatusResult          insert(IndexKey anIndexKey, uint32_t aBlockNum);
        IndexMap::iterator    get(IndexKey anIndexKey);
        bool                  hasKey(IndexKey anIndexKey);
        bool                  erase(IndexKey anIndexKey);

        bool                  each(const BlockVisitor aVisitor) override;
        bool                  changeEach(const ChangeBlockVisitor aVisitor);
        bool                  eachPair(const IndexVisitor aVisitor);

        size_t                size();
        void                  setBlockNum(uint32_t aBlockNum);
        uint32_t              getBlockNum() const;
        std::string           getName() const;

        // storable
        virtual StatusResult  encode(std::ostream &anOutput);
        virtual StatusResult  decode(std::istream &anInput);

    protected:
        std::iostream &stream;
        std::string name;
        uint32_t blockNum;
        IndexMap indexMap; // key : block number
    };
}

#endif