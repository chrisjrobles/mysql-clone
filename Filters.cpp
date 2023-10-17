//
//  Filters.cpp
//  Datatabase5
//
//  Created by rick gessner on 3/5/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <string>
#include <limits>
#include "Attribute.hpp"
#include "Compare.hpp"
#include "Entity.hpp"
#include "Filters.hpp"
#include "Helpers.hpp"
#include "keywords.hpp"

namespace ECE141 {

    using Comparitor = bool (*)(Value& aLHS, Value& aRHS);

    bool equals(Value& aLHS, Value& aRHS) {
        bool theResult = false;

        std::visit([&](auto const& aLeft) {
            std::visit([&](auto const& aRight) {
                theResult = isEqual(aLeft, aRight);
            }, aRHS);
        }, aLHS);
        return theResult;
    }

    bool lessThan(Value& aLHS, Value& aRHS) {
        bool theResult = false;

        std::visit([&](auto const& aLeft) {
            std::visit([&](auto const& aRight) {
                theResult = isLessThan(aLeft, aRight);
            }, aRHS);
        }, aLHS);
        return theResult;
    }

    bool greaterThan(Value& aLHS, Value& aRHS) {
        bool theResult = false;

        std::visit([&](auto const& aLeft) {
            std::visit([&](auto const& aRight) {
                theResult = (!isLessThan(aLeft, aRight)) && (!isEqual(aLeft, aRight));
            }, aRHS);
        }, aLHS);
        return theResult;
    }

    bool lessThanOrEqual(Value& aLHS, Value& aRHS) {
        bool theResult = false;

        std::visit([&](auto const& aLeft) {
            std::visit([&](auto const& aRight) {
                theResult = (isLessThan(aLeft, aRight)) || (isEqual(aLeft, aRight));
            }, aRHS);
        }, aLHS);
        return theResult;
    }

    bool greaterThanOrEqual(Value& aLHS, Value& aRHS) {
        bool theResult = false;

        std::visit([&](auto const& aLeft) {
            std::visit([&](auto const& aRight) {
                theResult = !isLessThan(aLeft, aRight);
            }, aRHS);
        }, aLHS);
        return theResult;
    }

    bool notEquals(Value& aLHS, Value& aRHS) {
        bool theResult = false;

        std::visit([&](auto const& aLeft) {
            std::visit([&](auto const& aRight) {
                theResult = !isEqual(aLeft, aRight);
            }, aRHS);
        }, aLHS);
        return theResult;
    }

    static std::map<Operators, Comparitor> comparitors{
      {Operators::equal_op, equals},
      {Operators::lt_op, lessThan},
      {Operators::gt_op, greaterThan},
      {Operators::lte_op, lessThanOrEqual},
      {Operators::gte_op, greaterThanOrEqual},
      {Operators::notequal_op, notEquals},

      //STUDENT: Add more for other operators...
    };

    bool Expression::operator()(const KeyValues& aList) {
        Value theLHS{ lhs.value };
        Value theRHS{ rhs.value };

        if (TokenType::identifier == lhs.ttype) {
            theLHS = aList.at(lhs.name); //get row value
        }

        if (TokenType::identifier == rhs.ttype) {
            theRHS = aList.at(rhs.name); //get row value
        }

        return comparitors.count(op)
            ? comparitors[op](theLHS, theRHS) : false;
    }

    //--------------------------------------------------------------

    Filters::Filters() {}

    Filters& Filters::operator=(Filters&& aFilters) {
        this->expressions = std::move(aFilters.expressions);
        return *this;
    }

    Filters::~Filters() {
    }

    Filters& Filters::add(Expression* anExpression) {
        expressions.push_back(std::unique_ptr<Expression>(anExpression));
        return *this;
    }

    bool Filters::matches(const KeyValues& aList) const {
        bool exprValue = true;
        for (auto& theExpr : expressions) {
            switch (theExpr->logic) {
                case Logical::and_op: exprValue = exprValue && (*theExpr)(aList); break;
                case Logical::not_op: exprValue = !(*theExpr)(aList); break;
                case Logical::or_op:  exprValue = exprValue || (*theExpr)(aList); break;
                case Logical::no_op:  exprValue = (*theExpr)(aList); break;
                default:              exprValue = (*theExpr)(aList); break;
            }
        }
        return exprValue;
    }


    //where operand is field, number, string...
    StatusResult parseOperand(Tokenizer& aTokenizer,
        Entity& anEntity, Operand& anOperand) {
        StatusResult theResult{ noError };
        Token& theToken = aTokenizer.current();
        if (TokenType::identifier == theToken.type) {
            if (auto& theAttr = anEntity.getAttribute(theToken.data)) {
                anOperand.ttype = theToken.type;
                anOperand.name = theToken.data; //hang on to name...
                anOperand.entityId = Entity::hashString(theToken.data);
                anOperand.dtype = theAttr.getType();
            }
            else {
                anOperand.ttype = TokenType::string;
                anOperand.dtype = DataTypes::varchar_type;
                anOperand.value = theToken.data;
            }
        }
        else if (TokenType::number == theToken.type) {
            anOperand.ttype = TokenType::number;
            anOperand.dtype = DataTypes::int_type;
            if (theToken.data.find('.') != std::string::npos) {
                anOperand.dtype = DataTypes::float_type;
                anOperand.value = std::stof(theToken.data);
            }
            else anOperand.value = std::stoi(theToken.data);
        }
        else theResult.error = syntaxError;
        if (theResult) aTokenizer.next();
        return theResult;
    }

    //STUDENT: Add validation here...
    bool validateOperands(Operand& aLHS, Operand& aRHS, Entity& anEntity) {
        if (TokenType::identifier == aLHS.ttype) { //most common case...
          //STUDENT: Add code for validation as necessary
            return true;
        }
        else if (TokenType::identifier == aRHS.ttype) {
            //STUDENT: Add code for validation as necessary
            return true;
        }
        return false;
    }

    //STUDENT: This starting point code may need adaptation...
    StatusResult Filters::parse(Tokenizer& aTokenizer, Entity& anEntity) {
        StatusResult  theResult{ noError };

        while (theResult && (2 < aTokenizer.remaining())) {
            Operand theLHS, theRHS;
            Token& theToken = aTokenizer.current();
            if (theToken.type != TokenType::identifier) return theResult;
            if ((theResult = parseOperand(aTokenizer, anEntity, theLHS))) {
                Token& theToken = aTokenizer.current();
                if (theToken.type == TokenType::operators) {
                    Operators theOp = Helpers::toOperator(theToken.data);
                    aTokenizer.next();
                    if ((theResult = parseOperand(aTokenizer, anEntity, theRHS))) {
                        if (validateOperands(theLHS, theRHS, anEntity)) {
                            add(new Expression(theLHS, theOp, theRHS));
                            if (aTokenizer.skipIf(semicolon)) {
                                break;
                            }
                        }
                        else theResult.error = syntaxError;
                    }
                }
            }
            else theResult.error = syntaxError;
        }
        return theResult;
    }

}