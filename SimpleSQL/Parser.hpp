#pragma once
#include <string>
#include <vector>
#include <utility>
#include <stdexcept>
#include <iostream>

#include "ASTNode.hpp"
#include "Token.hpp"

namespace kSQL{

	struct ParserNode {
		std::pair<TokenType, std::string> token;
		std::vector<ParserNode*> children;
	};

	class Parser
	{
		TokenType m_QueryType;

		ParserNode* m_Root;
		std::vector<std::pair<TokenType, std::string>> m_Tokens;
		std::string m_ErrorMsg;
		int m_CurrentTokenIndex;
		std::pair<TokenType, std::string> m_CurrentToken;

		std::vector<std::string> m_Columns;

		std::vector<std::pair<TokenType, std::string>> m_NewColumns;
		std::string m_NewTablePk;

		std::string m_Table;
		std::string m_JoinTable;
		std::vector<std::string> m_JoinValues;
		std::vector<std::pair<std::string, std::pair<std::string, TokenType>>> m_InsertValues;
		// false - asc, true - desc
		std::vector<std::pair<std::string, bool>> m_OrderBy;
		int m_Limit;
		int m_Offset;
		std::pair<std::string, TokenType> m_ColumnToAdd;
		std::string m_ColumnToRemove;

		std::pair<std::string, std::string> m_RefToAdd;
		std::pair<std::string, std::string> m_RefToRemove;

		/*
		ex CREATE TABLE ex (Id integer pk, Email text)
		*/
		auto HandleCreate() -> void;
		auto HandleDrop() -> void;
		/*
		ex: INSERT INTO User Id = 2, Email = 'text.pl', 
		*/
		auto HandleInsert() -> void;
		auto HandleDelete() -> void;
		auto HandleSaveDb() -> void;
		auto HandleLoadDb() -> void;
		auto HandleSelect() -> bool;
		auto HandleAlter() -> void;
		auto HandleUpdate() -> void;
		auto ParseColumns() -> void;
		auto ParseConditions() -> void;

		auto ParseLogical() -> ParserNode*;
		auto ParseStatment() -> ParserNode*;
		auto ParseInput() -> ParserNode*;

		auto ParseOrderBy() -> void;
		auto ParseJoin() -> void;

		auto isCmpOp(TokenType type) -> bool;
		auto isLogicalOp(TokenType type) -> bool;
		auto isData(TokenType type) -> bool;

		auto AdvanceToken() -> void;

		auto DealocateAST(ParserNode* node) -> void;
	public:
		Parser(std::vector<std::pair<TokenType, std::string>> tokens) : m_Tokens(tokens), m_Root(nullptr),
			m_ErrorMsg(""), m_CurrentTokenIndex(0), m_QueryType(TokenType::none), m_Limit(-1), m_Offset(-1){}
		~Parser();
		auto Parse() -> bool;

		auto GetQueryType() -> TokenType;
		auto GetCondtions() -> ParserNode*;
		auto GetTargetTable() -> std::string;
		auto GetColumns() -> std::vector<std::string>;
		auto GetJoinTable() -> std::string;
		auto GetJoinValues() -> std::vector<std::string>;
		auto GetOrderBy() -> std::vector<std::pair<std::string, bool>>;
		// first limit second offset
		auto GetLimitAndOffset() -> std::pair<int, int>;
		auto GetInsertValues() -> std::vector<std::pair<std::string, std::pair<std::string, TokenType>>>;
		auto GetNewColumns() -> std::vector<std::pair<TokenType, std::string>>;
		auto GetNewPk() -> std::string;
		auto GetColumnToAdd() -> std::pair<std::string, TokenType>;
		auto GetColumnToRemove() -> std::string;
		auto GetRefToAdd() -> std::pair<std::string, std::string>;
		auto GetRefToRemove() -> std::pair<std::string, std::string>;
	};
}

