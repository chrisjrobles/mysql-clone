
#include <ctime>
#include "BasicTypes.hpp"

namespace ECE141 {
  std::ostream &operator<<(std::ostream &out, const NullType &aType) {
    out << "Null";
    return out;
  }

  std::istream &operator>>(std::istream &in, NullType &aType) {
    char typeChar;
    in >> typeChar;
    aType = NullType{};
    return in;
  }

  std::ostream &operator<<(std::ostream &out, const DataTypes &aType) {
    out << static_cast<char>(aType);
    return out;
  }

  std::istream &operator>>(std::istream &in, DataTypes &aType) {
    char typeChar;
    in >> typeChar;
    aType = static_cast<DataTypes>(typeChar);
    return in;
  }

  std::ostream &operator<<(std::ostream &out, const Value &aValue) {
    std::visit(overload{
      [&out](bool aValue)               { out << (aValue ? "T" : "F"); },
      [&out](const std::string &aValue) { out << std::quoted(aValue); },
      [&out](int aValue)                { out << std::to_string(aValue); },
      [&out](double aValue)             { out << std::to_string(aValue); },
      [&out](time_t aValue)             { out << std::ctime(&aValue); },
      [&out](const NullType &aValue)    { out << "NULL"; },
    }, aValue);
    return out;
  }

  std::istream &operator>>(std::istream &in, Value &aValue) {
    aValue = std::visit(overload{
      [&in](bool aValue) {
        char aValueChar;
        in >> aValueChar;
        return Value(aValueChar == 'T');
      },
      [&in](std::string &aValue) {
        in >> std::quoted(aValue);
        return Value(aValue);
      },
      [&in](NullType &aValue) {
        std::string pass;
        in >> pass;
        return Value{NullType{}};
      },
      [&in](auto aValue) {
        in >> aValue;
        return Value(aValue);
      },
    }, aValue);
    return in;
  }
}