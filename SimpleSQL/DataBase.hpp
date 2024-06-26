#pragma once
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <regex>
#include <filesystem>

#include "tokenizer.hpp"
#include "Parser.hpp"
#include "Table.hpp"
#include "Relation.hpp"

namespace kSQL {

	

	class DataBase
	{
		std::map<std::string, Table*> m_Tables;
		std::vector<Relation> m_Relations;
		std::string m_DatabasePath;

		auto ExecuteDrop(Parser& parser) -> void;
		auto ExecuteUpdate(Parser& parser) -> void;
		auto ExecuteAlter(Parser& parser) -> void;
		auto ExecuteCreate(Parser& parser) -> void;
		auto ExecuteInsert(Parser& parser) -> void;
		auto ExecuteSelect(Parser& parser) -> bool;
		auto ExecuteQuit() -> void;
		auto ExecuteDelete(Parser& parser) -> void;

		auto EvaluateConditions(ParserNode* root, Table* table, int row) -> bool;
		auto EvaluateLogicalOp(ParserNode* root, Table* table, int row) -> bool;
		auto EvaluateCmpOperation(ParserNode* root, Table* table, int row) -> bool;

		auto SortOutputByOrderBy(const std::vector<std::string>& columns, std::vector<std::pair<std::vector<std::string>, int>>& result, Parser& parser, Table* table) -> void;
		auto LimitResults(std::vector<std::pair<std::vector<std::string>, int>>& result, Parser& parser) -> void;

		auto PopulateJoinTable(Table* joinTable,const std::string& table1, const std::string& tabl2, Parser& parser) -> void;
		auto InvalidateDataBase() -> void;
		
	public:
		DataBase() : m_Tables({}), m_Relations({}) {}

		auto Execute(const std::string query) -> bool;
		// DEBUG
		auto AddTable(const std::string& name, Table* table) -> void { m_Tables[name] = table; }
		auto AddRelation(Relation rel) -> void { m_Relations.push_back(rel); }
		auto ValidateRelations(const std::string& updateTableName) -> bool;
		auto LoadFromMemory(const std::string& path) -> bool;
		auto SaveToFile(const std::string& path) -> void;
	};
}

