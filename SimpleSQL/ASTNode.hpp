#pragma once
#include <vector>
#include <utility>

#include "Token.hpp"

namespace kSQL {

	class ASTNode
	{
	public:
		ASTNode(std::pair<TokenType, std::string> token) : /*m_Parrent(nullptr),*/ m_Token(token) {}
		~ASTNode();
		auto Insert(std::pair<TokenType, std::string> token) -> ASTNode*;
		auto InsertNode(ASTNode* node) -> void;
		//auto InsertToFirstType
		auto getNodes() -> std::vector<ASTNode*>;
		auto getToken() -> std::pair<TokenType, std::string>;
	private:
		//ASTNode* m_Parrent;
		std::vector<ASTNode*> m_Nodes;
		std::pair<TokenType, std::string> m_Token;
	};
}

