//
//  Filters.hpp
//  RGAssignment5
//
//  Created by rick gessner on 4/4/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef Filters_h
#define Filters_h

#include <stdio.h>
#include <vector>
#include <memory>
#include <string>
#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "Tokenizer.hpp"

namespace ECE141 {

  bool equals(Value& aLHS, Value& aRHS);
  
  class Row;
  class Entity;
  
  struct Operand {
    Operand() : ttype(TokenType::unknown), dtype(DataTypes::no_type), name(""),
        value(NullType{}), entityId(0) {}
    Operand(std::string &aName, TokenType aType, Value aValue, uint32_t anId=0)
      : ttype(aType), dtype(DataTypes::varchar_type), name(aName),
        value(aValue), entityId(anId) {}
    
    TokenType   ttype; //is it a field, or const (#, string)...
    DataTypes   dtype;
    std::string name;  //attr name
    Value       value;
    uint32_t    entityId;
  };
  
  //---------------------------------------------------

  struct Expression {
    Operand     lhs;  //id
    Operand     rhs;  //usually a constant; maybe a field...
    Operators   op;   //=     //users.id=books.author_id
    Logical     logic; //and, or, not...
    
    Expression() {}

    Expression(Operand &aLHSOperand, Operators anOp,
               Operand &aRHSOperand)
      : lhs(aLHSOperand), rhs(aRHSOperand),
        op(anOp), logic(Logical::no_op) {}

    Expression(const Expression &anExpression)
      : lhs(anExpression.lhs), rhs(anExpression.rhs),
        op(anExpression.op), logic(anExpression.logic) {}
    
    bool operator()(const KeyValues &aList);
  };
  
  using Expressions = std::vector<std::unique_ptr<Expression> >;

  //---------------------------------------------------

  class Filters {
  public:
    
    Filters();
    // Filters(const Filters &aFilters);
    Filters(Filters &&aFilters) = default;
    Filters& operator=(Filters&& aFilters);
    ~Filters();
    
    size_t        getCount() const {return expressions.size();}
    bool          matches(const KeyValues &aList) const;
    Filters&      add(Expression *anExpression);
        
    StatusResult  parse(Tokenizer &aTokenizer, Entity &anEntity);
    
  protected:
    Expressions   expressions;
  };
 
}

#endif /* Filters_h */
