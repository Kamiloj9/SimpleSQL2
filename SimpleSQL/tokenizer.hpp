#pragma once
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <regex>
#include <iostream>

#include "Token.hpp"

namespace kSQL {
	class Tokenizer
	{
	public:
		Tokenizer(const std::string& query) : m_Query(query) {};
		auto Tokenize() -> void;
		auto getTokens() -> std::vector<std::pair<TokenType, std::string>>;
	private:
		auto AddWihiteSpaces(std::string& querry, const std::vector<std::pair<std::string, std::string>>& chars) -> void;
		auto SplitQueryToSubqueriesAndLiterals() -> void;
		auto TokenizeSubQuerry(std::string& query) -> void;
		std::string m_Query;
		// true is query false if literal
		std::vector<std::pair<std::string, bool>> m_SubQueries;
		std::vector<std::pair<TokenType, std::string>> m_Tokens;
	};
}