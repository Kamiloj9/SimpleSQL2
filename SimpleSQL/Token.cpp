#include "Token.hpp"

kSQL::Token::Token(const std::string& tokenName) : m_InputName(tokenName), m_Type(TokenType::none)
{
}

bool kSQL::Token::Parse()
{
	if (g_KeyLookUp.contains(m_InputName)) {
		m_Type = g_KeyLookUp[m_InputName];

		return true;
	}

	std::regex integerR("[-+]?[0-9]+");
	if (std::regex_match(m_InputName, integerR)) {
		m_Type = TokenType::integer;

		return false;
	}

	std::regex floatOrIntegerR("[-+]?([0-9]*\.[0-9]+|[0-9]+)");
	if (std::regex_match(m_InputName, floatOrIntegerR)) {
		m_Type = TokenType::numeric;

		return false;
	}

	m_Type = TokenType::inputField;

	return false;
}

kSQL::TokenType kSQL::Token::getType()
{
	return m_Type;
}
