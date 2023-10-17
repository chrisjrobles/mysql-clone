
#include "Tokenizer.hpp"
#include "BasicTypes.hpp"
#include <algorithm>

namespace ECE141 {
	class ParsingHelper {
	public:

		ParsingHelper() {}
		~ParsingHelper() {}

		static bool endFields(Token aToken) {
			if (aToken.data == ";" || aToken.type == TokenType::keyword)
				return true;
			return false;
		}

		static DataTypes getDataType(Keywords aKey) {
			switch (aKey) {
			case Keywords::boolean_kw:	return DataTypes::bool_type;
			case Keywords::float_kw:	return DataTypes::float_type;
			case Keywords::integer_kw:	return DataTypes::int_type;
			case Keywords::datetime_kw: return DataTypes::datetime_type;
			case Keywords::varchar_kw:	return DataTypes::varchar_type;
			default:					return DataTypes::no_type;
			}
			return DataTypes::no_type;
		}

		// Convert string to variant Value based on DataType specified
		static Value strToValue(std::string aStr, DataTypes aType) {
			Value theVal;
			switch (aType) {
			case DataTypes::bool_type: {
				std::string temp(aStr);
				std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
				theVal = (temp == "true");
				break;
			}
			case DataTypes::float_type: {
				theVal = std::stod(aStr);
				break;
			}
			case DataTypes::int_type: {
				theVal = std::stoi(aStr);
				break;
			}
			case DataTypes::datetime_type: {
				time_t theTime = std::stoi(aStr);
				theVal = theTime;
				break;
			}
			case DataTypes::varchar_type: {
				theVal = aStr;
				break;
			}
			default: break;
			}
			return theVal;
		}
	};
}