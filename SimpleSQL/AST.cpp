#include "AST.hpp"

auto kSQL::AST::CreateTree(std::string& errorMessage) -> bool
{
    errorMessage = std::string("");

    if (m_Tokens.size() == 0) {
        errorMessage = std::string("No command found");
        return false;
    }

    int tokenCounter = 0;

    // first one must be the command
    const auto commandToken = m_Tokens[tokenCounter];
    if (commandToken.first != TokenType::select) {
        errorMessage = std::string(commandToken.second + " is invalid command");
        return false;
    }
    m_Root = new ASTNode(m_Tokens[tokenCounter]);
    m_CurrentNode = m_Root;
    tokenCounter++;

    while (tokenCounter < m_Tokens.size()) {
        const auto previousTokenType = m_Tokens[tokenCounter - 1].first;
        const auto currentToken = m_Tokens[tokenCounter];
        switch (previousTokenType)
        {
        case TokenType::select:
            // check if this token is input
            if (currentToken.first == TokenType::inputField) {
                /*m_CurrentNode->Insert(currentToken);
                m_CurrentNode = m_CurrentNode->getNodes()[0];*/
                const auto newNode = m_Root->Insert({ TokenType::fields, "Fields for command" });
                m_CurrentNode = newNode;
                m_CurrentNode->Insert(currentToken);
            }
            else {
                errorMessage = std::string("Invalid rows syntax for SELECT command");
                return false;
            }
            break;
        case TokenType::from:
            // check if cur token is input data
            if (currentToken.first == TokenType::inputField) {
                const auto newNode = m_Root->Insert({TokenType::tables, "Table for command"});
                m_CurrentNode = newNode;
                m_CurrentNode->Insert(currentToken);
            }
            else {
                errorMessage = std::string("Invalid syntax: you must specify table after \"FROM\"");
                return false;
            }
            break;
        case TokenType::where:
        {
            //const auto newNode = m_Root->Insert({ TokenType::conditions, "Table for conditions" });
            //m_CurrentNode = newNode;

            if (!HandleWhareCase(errorMessage, tokenCounter, nullptr))
                return false;
        }
            break;
        case TokenType::comma:
            // next one must be data
            if (currentToken.first == TokenType::inputField) {
                m_CurrentNode->Insert(currentToken);
            }
            else {
                errorMessage = std::string("Invalid syntax: comma should separate input fields");
                return false;
            }
            break;
        case TokenType::inputField:
            // ignore if
            if (currentToken.first != TokenType::inputField) {
                if (currentToken.first == TokenType::from || currentToken.first == TokenType::where) {
                    m_Root->Insert(currentToken);
                }
                else if (currentToken.first == TokenType::comma) {
                    // ignore
                }
                else {
                    errorMessage = std::string("Invalid operation: " + currentToken.second);
                    return false;
                }
            }
            else {
                errorMessage = std::string("Invalid syntax: data fields must be separated be comma");
                return false;
            }
            break;
        default:
            break;
        }

        tokenCounter++;
    }

    return true;
}

auto kSQL::AST::GetTree() -> ASTNode*
{
    return m_Root;
}

auto kSQL::AST::HandleWhareCase(std::string& errorMessage, int& currentTokenIndex, ASTNode* node) -> bool
{

    

    const auto groupStatments = [&](int curTInx) -> std::pair<std::vector<GroupInfo>, int> {

        std::vector<GroupInfo> res;
        int endIndex = m_Tokens.size();

        for (int i = curTInx; i < m_Tokens.size() - 2; ++i) {

            if (m_Tokens[i].first == TokenType::orderby || m_Tokens[i].first == TokenType::limit) {
                endIndex = i;
                break;
            }

            int lOpIndx = i;
            int opIndx = i + 1;
            int rOpIndx = i + 2;
            if (m_Tokens[lOpIndx].first == TokenType::inputField) {
                if (m_Tokens[opIndx].first == TokenType::eq ||
                    m_Tokens[opIndx].first == TokenType::neq ||
                    m_Tokens[opIndx].first == TokenType::gt ||
                    m_Tokens[opIndx].first == TokenType::gtq ||
                    m_Tokens[opIndx].first == TokenType::lt ||
                    m_Tokens[opIndx].first == TokenType::ltq) {

                    if (m_Tokens[rOpIndx].first == TokenType::numeric ||
                        m_Tokens[rOpIndx].first == TokenType::integer ||
                        m_Tokens[rOpIndx].first == TokenType::inputField) {
                        
                        res.push_back({true, m_Tokens[lOpIndx], m_Tokens[opIndx], m_Tokens[rOpIndx] });
                    }
                }
            }
            else if (m_Tokens[lOpIndx].first == TokenType::andt ||
                m_Tokens[lOpIndx].first == TokenType::ort ||
                m_Tokens[lOpIndx].first == TokenType::leftp ||
                m_Tokens[lOpIndx].first == TokenType::rightp) {
                res.push_back({ false, m_Tokens[lOpIndx], m_Tokens[lOpIndx], m_Tokens[lOpIndx] });
            
            }
        }

        return std::make_pair(res, endIndex);
    };

    const auto groups = groupStatments(currentTokenIndex);
    if (!groups.first.size()) {
        errorMessage = std::string("Invalid syntax: no statments");
        currentTokenIndex = groups.second;
        return false;
    }

    const auto cond = new ASTNode({ TokenType::conditions, "Conditions" });
    m_Root->InsertNode(cond);

    cond->InsertNode(parseExpression(groups.first, errorMessage));

    if (errorMessage != "")
        return false;

    currentTokenIndex = groups.second;
    return true;
}

auto kSQL::AST::parseFactor(const std::vector<GroupInfo>& tokens, std::string& error) -> ASTNode*
{
    // check for expresions

    if (tokens[m_CurrentTokenIndex].isExpr) {

        // create node *
        //            / \
        //            *  *
        const auto ret = new ASTNode(tokens[m_CurrentTokenIndex].op);
        ret->Insert(tokens[m_CurrentTokenIndex].leftOp);
        ret->Insert(tokens[m_CurrentTokenIndex].rightOp);

        m_CurrentTokenIndex++;

        return ret;

    }
    else if (tokens[m_CurrentTokenIndex].leftOp.first == TokenType::leftp) {
        m_CurrentTokenIndex++;
        auto expr = parseExpression(tokens, error);

        if (tokens[m_CurrentTokenIndex].leftOp.first == TokenType::rightp) {
            const auto ret = new ASTNode({ TokenType::parenthesis, "Paranthesis" });
            ret->InsertNode(expr);
            return ret;
        }
        else {
            error = std::string("Invalid syntax: mismatched parentheses");
            return new ASTNode({ TokenType::none, "invalid token" });
        }
    }
    else {
        error = std::string("Invalid syntax: unexpected token: " + tokens[m_CurrentTokenIndex].leftOp.second);
        return new ASTNode({ TokenType::none, "invalid token" });
    }
}

auto kSQL::AST::parseTerm(const std::vector<GroupInfo>& tokens, std::string& error) -> ASTNode*
{
    auto factor = parseFactor(tokens, error);

    // continue parrsing if ther is AND
    while (m_CurrentTokenIndex < tokens.size() && tokens[m_CurrentTokenIndex].leftOp.first == TokenType::andt) {

        const auto token = tokens[m_CurrentTokenIndex].leftOp;
        // advance token
        m_CurrentTokenIndex++;
        const auto nextFactor = parseFactor(tokens, error);
        const auto newNode = new ASTNode(token);
        newNode->InsertNode(factor);
        newNode->InsertNode(nextFactor);
        factor = newNode;

    }

    return factor;
}

auto kSQL::AST::parseExpression(const std::vector<GroupInfo>& tokens, std::string& error) -> ASTNode*
{
    auto term = parseTerm(tokens, error);

    // continue parrsing if ther is OR
    while (m_CurrentTokenIndex < tokens.size() && tokens[m_CurrentTokenIndex].leftOp.first == TokenType::ort) {
        const auto token = tokens[m_CurrentTokenIndex].leftOp;
        // advance token
        m_CurrentTokenIndex++;
        const auto nextTerm = parseTerm(tokens, error);
        const auto newNode = new ASTNode(token);
        newNode->InsertNode(term);
        newNode->InsertNode(nextTerm);
        term = newNode;
    }

    return term;
}
