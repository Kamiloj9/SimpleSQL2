#include "tokenizer.hpp"

auto kSQL::Tokenizer::Tokenize() -> void
{
	SplitQueryToSubqueriesAndLiterals();

	// replace special chars with witespaces for easier tokenizing
	// comma, (, ), =, !=, <, <=, >, >=
	std::vector<std::pair<std::string, std::string>> toReplace = { {",", ","}, {"\\(", "("}, {"\\)", ")"},
		/*{"=", "="}, {"!=", "!="}, {"<", "<"}, {"<=", "<="}, {">", ">"}, {">=", ">="}, {"'", "'"}*/};

	std::ranges::for_each(m_SubQueries, [&](auto query) {
		if (query.second) {
			AddWihiteSpaces(query.first, toReplace);
			TokenizeSubQuerry(query.first);
		}
		else {
			Token tmp(query.first);
			tmp.Parse();
			/*TokenType ret = TokenType::literalField;
			if (tmp.getType() == TokenType::integer || tmp.getType() == TokenType::numeric)
				ret = tmp.getType();
			m_Tokens.push_back({ ret, query.first });*/
			m_Tokens.push_back({ TokenType::literalField, query.first });
		}
		});

	m_Tokens.push_back({ TokenType::eor, "End Of Request" });

	return;

	AddWihiteSpaces(m_Query, toReplace);

	//std::cout << m_Query << "\n";

	std::stringstream queryStream(m_Query);
	std::string potentialToken;

	std::string strLiteral("");
	bool readLiteral = false;

	while (queryStream >> potentialToken) {

		std::string upperPToken = potentialToken;
		std::ranges::transform(upperPToken.begin(), upperPToken.end(), upperPToken.begin(), toupper);

		Token upperToken(upperPToken);
		Token normalToken(potentialToken);

		if (readLiteral && potentialToken != "'") {
			strLiteral += std::string(potentialToken + " ");
			continue;
		}

		// Parse key words seperated by white space
		if (upperToken.Parse()) {

			if (upperToken.getType() == TokenType::literal) {
				if (readLiteral) {
					// trim space at the end
					auto finalLiteral = std::string("");
					if(strLiteral.size() != 0)
						finalLiteral = std::string(strLiteral.begin(), strLiteral.end() - 1);

					m_Tokens.push_back({ TokenType::inputField, finalLiteral });
					readLiteral = false;
					continue;
				}
				else {
					readLiteral = true;
					continue;
				}
			}

			m_Tokens.push_back({upperToken.getType(), potentialToken});

			continue;
		}

		normalToken.Parse();

		m_Tokens.push_back({ normalToken.getType(), potentialToken });
		//switch (normalToken.getType())
		//{
		//case TokenType::integer:
		//	m_Tokens.push_back({ TokenType::integer, potentialToken });
		//	break;
		//case TokenType::numeric:
		//	m_Tokens.push_back({TokenType::numeric, potentialToken});
		//	break;
		//case TokenType::inputField:
		//	// might contain key words like ","

		//	if (std::ranges::find(potentialToken, ',') != potentialToken.end()) {
		//		//const auto commaNum = std::ranges::count(potentialToken, ',');
		//		
		//		int pointer = 0;
		//		for (int i = 0; i < potentialToken.size(); ++i) {
		//			if (potentialToken[i] == ',') {
		//				m_Tokens.push_back({ TokenType::inputField, std::string(potentialToken.begin() + pointer, potentialToken.begin() + i) });
		//				m_Tokens.push_back({ TokenType::comma, "," });
		//				pointer = i+1;
		//			}
		//			else if (i == potentialToken.size() - 1) {
		//				m_Tokens.push_back({ TokenType::inputField, std::string(potentialToken.begin() + pointer, potentialToken.end()) });
		//			}
		//		}
		//	}
		//	else {
		//		m_Tokens.push_back({ TokenType::inputField, potentialToken });
		//	}

		//	break;
		//default:
		//	break;
		//}
	}

	if (readLiteral)
		throw std::invalid_argument("Syntax error: no closing ' sign");

	m_Tokens.push_back({ TokenType::eor, "End Of Request" });
}

auto kSQL::Tokenizer::getTokens() -> std::vector<std::pair<TokenType, std::string>>
{
	return m_Tokens;
}

auto kSQL::Tokenizer::AddWihiteSpaces(std::string& querry, const std::vector<std::pair<std::string, std::string>>& chars) -> void
{
	/*for (int i = 0; i < chars.size(); i++) {
		auto replacment = std::string(" " + chars[i].second + " ");
		m_Query = std::regex_replace(m_Query, std::regex(chars[i].first), replacment);
		std::cout << "replaced " << chars[i].second << "\n";
	}*/

	std::ranges::for_each(chars, [&](auto ch) {
		auto replacment = std::string(" " + ch.second + " ");
		querry = std::regex_replace(querry, std::regex(ch.first), replacment);
		});
}

auto kSQL::Tokenizer::SplitQueryToSubqueriesAndLiterals() -> void
{
	auto currentQuery = std::string("");
	for (int i = 0; i < m_Query.size(); ++i) {
		if (m_Query[i] == '\'') {
			//std::cout << "subquery: " << currentQuery << "\n";
			m_SubQueries.push_back(std::make_pair(currentQuery, true));
			currentQuery = std::string("");

			auto stringLiteral = std::string("");
			for (int j = i + 1; j < m_Query.size(); ++j, ++i) {
				if (m_Query[j] != '\'')
					stringLiteral += m_Query[j];
				else {
					// skip last '
					i++;
					break;
				}
			}

			//std::cout << "literal: " << stringLiteral << "\n";
			m_SubQueries.push_back(std::make_pair(stringLiteral, false));
		}
		else {
			currentQuery += m_Query[i];
		}
	}
	// push remaining querry
	if (currentQuery.size() != 0) {
		//std::cout << "subquery: " << currentQuery << "\n";
		m_SubQueries.push_back(std::make_pair(currentQuery, true));
	}

}

auto kSQL::Tokenizer::TokenizeSubQuerry(std::string& query) -> void
{

	std::stringstream queryStream(query);
	std::string potentialToken;

	while (queryStream >> potentialToken) {

		std::string upperPToken = potentialToken;
		std::transform(upperPToken.begin(), upperPToken.end(), upperPToken.begin(), toupper);

		Token upperToken(upperPToken);
		Token normalToken(potentialToken);

		// Parse key words seperated by white space
		if (upperToken.Parse()) {

			m_Tokens.push_back({ upperToken.getType(), potentialToken });

			continue;
		}

		normalToken.Parse();

		m_Tokens.push_back({ normalToken.getType(), potentialToken });
	}

}
