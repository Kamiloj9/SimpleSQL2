#pragma once
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Token.hpp"

namespace kSQL {

	struct DataCell
	{
		std::string data;
		TokenType type;

		std::string toString() {
			if (type == TokenType::null)
				return "NULL";
			return data;
		}
	};

	class Table
	{
		std::map<std::string, std::map<int, DataCell>> m_Data;
		std::map<std::string, TokenType> m_DataType;
		int m_RowCount;
		std::string m_PrimaryKey;
	public:
		Table() : m_RowCount(0), m_Data({}), m_DataType({}), m_PrimaryKey("") {}
		//~Table() {
		//	std::cout << "table destructor\n"; 
		//}
		auto GetPrimeyKeyName() -> std::string;
		auto SetPrimaryKey(const std::string& key) -> void;
		auto GetRow(int id) -> std::vector<DataCell>;
		auto GetRowWithNames(int id) -> std::vector<std::pair<std::string, DataCell>>;
		auto RemoveRow(int id) -> void;
		auto IsRowValid(int id) -> bool;
		auto AddColumn(std::string columName, TokenType type) -> void;
		auto RemoveColumn(std::string columnName) -> void;
		auto GetCell(std::string columName, int id) -> DataCell;
		auto GetColumn(std::string columName) -> std::vector<DataCell>;
		auto SetCell(std::string columName, int id, DataCell cell) -> void;
		auto InsertRow(const std::vector<std::pair<std::string, DataCell>>& row) -> int;
		auto UpdateRow(const std::vector<std::pair<std::string, std::pair<std::string, TokenType>>>& row, int i) -> void;
		auto GetRowCopy(int i) -> std::vector<std::pair<std::string, std::pair<std::string, TokenType>>>;
		auto DebugPrint() -> void;
		// returns row count
		auto GetSize() -> int;
		auto GetColumnType(const std::string& column) -> TokenType;
		auto GetColumnNames() -> std::vector<std::string>;
		auto GetColumnNamesAndTypes() -> std::vector<std::pair<std::string, TokenType>>;
		auto HasColumn(const std::string& column) -> bool;
		auto LoadFromFile(const std::string& filePath) -> bool;
		auto SaveToFile(const std::string& filePath) -> bool;
		
	};
}

