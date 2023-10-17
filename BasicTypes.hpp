//
//  BasicTypes.hpp
//  RGAssignement1
//
//  Created by rick gessner on 3/9/21.
//

#ifndef BasicTypes_h
#define BasicTypes_h

#include <map>
#include <ctime>
#include <vector>
#include <string>
#include <variant>
#include <iomanip>
#include <iostream>
#include <optional>

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

namespace ECE141
{
  enum class AlterType : char {
    add, drop
  };

  enum class DataTypes : char {
    no_type = 'N',
    bool_type = 'B',
    datetime_type = 'D',
    float_type = 'F',
    int_type = 'I',
    varchar_type = 'V',
    null_type = 'U'
  };

  struct NullType {
    NullType() {};
    template <typename T>
    NullType(const T&) {}
    operator std::string() const {return std::string();};
    operator int() const {return 0;};
    bool operator!=(const NullType &other) const { return true; };
    bool operator==(const NullType &other) const { return true; };
    bool operator<(const NullType &other) const { return true; };
  };

  using IndexKey = std::variant<uint32_t, std::string>;

  using NamedIndex = std::map<std::string, uint32_t>;
  using StringList = std::vector<std::string>;
  using StringMap = std::map<std::string, std::string>;
  using StringOpt = std::optional<std::string>;
  using IntOpt = std::optional<uint32_t>;

  using Value = std::variant<NullType, bool, int, double, std::string, time_t>;
  using KeyValues = std::map<const std::string, Value>; //attributeName : value

  std::ostream &operator<<(std::ostream &out, const NullType &aType);
  std::istream &operator>>(std::istream &in, NullType &aType);
  std::ostream &operator<<(std::ostream &out, const DataTypes &aType);
  std::istream &operator>>(std::istream &in, DataTypes &aType);
  std::ostream &operator<<(std::ostream &out, const Value &aValue);
  std::istream &operator>>(std::istream &in, Value &aValue);
}

#endif /* BasicTypes_h */
