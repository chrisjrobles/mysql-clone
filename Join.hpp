//
//  Join.hpp
//  RGAssignment8
//
//  Created by rick gessner on 5/05/21.
//

#ifndef Join_h
#define Join_h

#include <string>
#include <vector>
#include "BasicTypes.hpp"
#include "Database.hpp"
#include "Errors.hpp"
#include "Filters.hpp"
#include "keywords.hpp"


namespace ECE141 {

  class Join  {
  public:
    Join(const std::string &aTable, Keywords aType)
      : table(aTable), joinType(aType), expr() {}
            
    bool validate(Database* theDB, Entity& rootEntity);
    bool execute(Database* theDB, Entity& rootEntity, RowCollection &rootRows);
    void setExpr(Expression &anExpr);

    std::string table;
    Keywords    joinType;
    Expression  expr;

  protected:
    bool executeLeft(RowCollection &resultRows, const RowCollection &leftRows, const RowCollection &rightRows);
  };

  using JoinList = std::vector<Join>;

}

#endif /* Join_h */
