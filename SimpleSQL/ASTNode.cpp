#include "ASTNode.hpp"

kSQL::ASTNode::~ASTNode()
{
}

auto kSQL::ASTNode::Insert(std::pair<TokenType, std::string> token) -> ASTNode*
{
	const auto ret = new ASTNode(token);
	m_Nodes.push_back(ret);
	return ret;
}

auto kSQL::ASTNode::InsertNode(ASTNode* node) -> void
{
	m_Nodes.push_back(node);
}

auto kSQL::ASTNode::getNodes() -> std::vector<ASTNode*>
{
	return m_Nodes;
}

auto kSQL::ASTNode::getToken() -> std::pair<TokenType, std::string>
{
	return m_Token;
}
