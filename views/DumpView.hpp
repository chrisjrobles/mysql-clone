//
//  Storage.hpp
//  Database
//
//

#ifndef DumpView_hpp
#define DumpView_hpp

#include <functional>
#include "../BasicTypes.hpp"
#include "../Database.hpp"
#include "SimpleViews.hpp"
#include "View.hpp"

namespace ECE141 {

	enum class DumpViewField {
		Type=1, Id, RefId, Count, Next, Pos
	};
	
	using BlockValueGetter = std::function<std::string(const Block&)>;
	using FieldSizePair = std::pair<DumpViewField, size_t>;
	using FieldSizeVector = std::vector<FieldSizePair>;

	class DumpView : public View {
	public:

		DumpView(std::string aName, Database* aDumpDB=nullptr)
			: name(aName), dumpDB(aDumpDB) {}

		~DumpView() {}

		bool show(std::ostream& anOutput, Timer& aTimer) override {
			if (!dumpDB) return false;

			FieldSizeVector dumpFields; // (FieldToBeOutput : ColumnWidth)
			dumpFields.push_back({DumpViewField::Pos, 	4});
			dumpFields.push_back({DumpViewField::Type, 	10});
			dumpFields.push_back({DumpViewField::Id, 	10});
			dumpFields.push_back({DumpViewField::RefId, 10});
			dumpFields.push_back({DumpViewField::Count, 5});
			dumpFields.push_back({DumpViewField::Next, 	5});

			anOutput << std::left;
			printHeader(anOutput, dumpFields);
			dumpDB->getStorage().each(getPrintBlockVisitor(anOutput, dumpFields));

			aTimer.stop();
			size_t theCount = dumpDB->getStorage().getBlockCount();
			anOutput << theCount << " rows in set (" << ViewUtils::getFormattedTime(aTimer.elapsed()) << " sec)\n";

			return true;
		}

		static void printHeader(std::ostream& anOutput, FieldSizeVector& aDumpFields) {
			printRowSeparator(anOutput, aDumpFields);
			for (auto& dumpField : aDumpFields) {
				anOutput << "| " << std::setw(dumpField.second) << getDumpFieldString(dumpField.first) << ' ';
			}
			anOutput << "|\n";
			printRowSeparator(anOutput, aDumpFields);
		}

		static void printRowSeparator(std::ostream& anOutput, FieldSizeVector& aDumpFields) {
			for (auto& dumpField : aDumpFields) {
				anOutput << "+-" << std::string(dumpField.second+1,'-');
			}
			anOutput << "+\n";
		}

		static BlockVisitor getPrintBlockVisitor(std::ostream& anOutput, FieldSizeVector& aDumpFields) {
			return [&](const Block& aBlock, uint32_t aBlockNum) -> bool {
				for (auto& dumpField : aDumpFields) {
					anOutput << "| " << std::setw(dumpField.second);
					anOutput << getBlockFieldValue(aBlock, dumpField.first) << ' ';
				}
				anOutput << "|\n";
				printRowSeparator(anOutput, aDumpFields);
				return true;
			};
		}

		static std::string getBlockFieldValue(const Block& aBlock, DumpViewField aDumpField) {
			switch(aDumpField) {
				case DumpViewField::Type:	return ViewUtils::getBlockTypeString(aBlock.header.type);
				case DumpViewField::Id:		return std::to_string(aBlock.header.id);
				case DumpViewField::RefId:	return std::to_string(aBlock.header.refId);
				case DumpViewField::Count: 	return std::to_string(aBlock.header.count);
				case DumpViewField::Next:	return std::to_string(aBlock.header.next);
				case DumpViewField::Pos:	return std::to_string(aBlock.header.pos);
				default:					return "NA";
			}
		}

		static std::string getDumpFieldString(DumpViewField aDumpField) {
			switch(aDumpField) {
				case DumpViewField::Type:	return "Type";
				case DumpViewField::Id:		return "Id";
				case DumpViewField::RefId:	return "RefId";
				case DumpViewField::Count: 	return "Count";
				case DumpViewField::Next:	return "Next";
				case DumpViewField::Pos:	return "#";
				default:					return "UNKNOWN";
			}
		}

		std::string name;
		Database* dumpDB;
	};
}

#endif /* DumpView_hpp */
