#pragma once
#include <string>
#include <vector>
#include <utility>

#include "ASTNode.hpp"
#include "Token.hpp"

namespace kSQL {

	struct GroupInfo
	{
		bool isExpr;

		std::pair<TokenType, std::string> leftOp; // data type
		std::pair<TokenType, std::string> op; // =, !=, <, <=, >, >=
		std::pair<TokenType, std::string> rightOp; // data type or numeric or int
	};

	class AST
	{
	public:
		AST(std::vector<std::pair<TokenType, std::string>> tokens) : m_Root(nullptr), m_CurrentNode(nullptr), m_CurrentTokenIndex(0), m_ParsingError(false){ m_Tokens = tokens; }
		auto CreateTree(std::string& errorMessage) -> bool;
		auto GetTree() -> ASTNode*;
	private:
		ASTNode* m_Root;
		ASTNode* m_CurrentNode;
		std::vector<std::pair<TokenType, std::string>> m_Tokens;

		auto HandleWhareCase(std::string& errorMessage, int& currentTokenIndex, ASTNode* node) -> bool;
		// for where case
		int m_CurrentTokenIndex;
		bool m_ParsingError;
		auto parseFactor(const std::vector<GroupInfo>&tokens, std::string& error) -> ASTNode*;
		auto parseTerm(const std::vector<GroupInfo>& tokens, std::string& error) -> ASTNode*;
		auto parseExpression(const std::vector<GroupInfo>& tokens, std::string& error) -> ASTNode*;
	};

}
