#include "Parser.hpp"

auto kSQL::Parser::HandleCreate() -> void
{
    AdvanceToken();
    if(m_CurrentToken.first != TokenType::table)
        throw std::invalid_argument("Invalid syntax: missing table keyword");
    AdvanceToken();
    if (m_CurrentToken.first != TokenType::inputField)
        throw std::invalid_argument("Invalid syntax: no table name");
    m_Table = m_CurrentToken.second;
    AdvanceToken();

    if(m_CurrentToken.first != TokenType::leftp)
        throw std::invalid_argument("Invalid syntax: missing '('");
    AdvanceToken();

    while (m_CurrentToken.first != TokenType::rightp)
    {
        if(m_CurrentToken.first == TokenType::eor)
            throw std::invalid_argument("Invalid syntax: missing ')'");

        if (m_CurrentToken.first != TokenType::inputField)
            throw std::invalid_argument("Invalid syntax: missing column name");

        std::pair<TokenType, std::string> newColumn;
        newColumn.second = m_CurrentToken.second;

        AdvanceToken();

        if (m_CurrentToken.first != TokenType::inputField)
            throw std::invalid_argument("Invalid syntax: missing column type");

        if (m_CurrentToken.second == "integer") {
            newColumn.first = TokenType::integer;
        }
        else if (m_CurrentToken.second == "numeric") {
            newColumn.first = TokenType::numeric;
        }
        else if (m_CurrentToken.second == "text") {
            newColumn.first = TokenType::literalField;
        }
        else {
            throw std::invalid_argument("Invalid syntax: " + m_CurrentToken.second + " is invalid data type");
        }
        AdvanceToken();
        if (m_CurrentToken.first == TokenType::pk) {
            m_NewTablePk = newColumn.second;
            AdvanceToken();
        }

        m_NewColumns.push_back(newColumn);

        if (m_CurrentToken.first == TokenType::comma)
            AdvanceToken();
    }

    //std::cout << "test\n";
}

auto kSQL::Parser::HandleDrop() -> void
{
    AdvanceToken();
    if (m_CurrentToken.first != TokenType::table)
        throw std::invalid_argument("Invalid syntax: missing table keyword");
    AdvanceToken();

    if (m_CurrentToken.first != TokenType::inputField)
        throw std::invalid_argument("Invalid syntax: no table name");
    m_Table = m_CurrentToken.second;
    AdvanceToken();
}

auto kSQL::Parser::HandleInsert() -> void
{
    AdvanceToken();
    if(m_CurrentToken.first != TokenType::into)
        throw std::invalid_argument("Invalid syntax: missing into keyword");
    AdvanceToken();
    if (m_CurrentToken.first != TokenType::inputField)
        throw std::invalid_argument("Invalid syntax: no table name");
    m_Table = m_CurrentToken.second;
    AdvanceToken();

    while (m_CurrentToken.first == TokenType::inputField) {
        std::pair<std::string, std::pair<std::string, TokenType>> columnValue;
        columnValue.first = m_CurrentToken.second;
        AdvanceToken();
        if(m_CurrentToken.first != TokenType::eq)
            throw std::invalid_argument("Invalid syntax: no = operator");
        AdvanceToken();
        if(m_CurrentToken.first != TokenType::literalField &&
            m_CurrentToken.first != TokenType::integer &&
            m_CurrentToken.first != TokenType::numeric &&
            m_CurrentToken.first != TokenType::null)
            throw std::invalid_argument("Invalid syntax: " + m_CurrentToken.second + " unknown data type");
        columnValue.second = std::make_pair(m_CurrentToken.second, m_CurrentToken.first);
        AdvanceToken();
        if (m_CurrentToken.first == TokenType::comma)
            AdvanceToken();
        m_InsertValues.push_back(columnValue);
    }
    //std::cout << "done\n";
}

auto kSQL::Parser::HandleDelete() -> void
{
    AdvanceToken();
    if(m_CurrentToken.first != TokenType::from)
        throw std::invalid_argument("Invalid syntax: missing from keyword");
    AdvanceToken();
    if (m_CurrentToken.first != TokenType::inputField)
        throw std::invalid_argument("Invalid syntax: no table name");
    m_Table = m_CurrentToken.second;

    AdvanceToken();

    if (m_CurrentToken.first == TokenType::where) {
        AdvanceToken();
        ParseConditions();
    }
}

auto kSQL::Parser::HandleSaveDb() -> void
{
    AdvanceToken();
    if (m_CurrentToken.first == TokenType::literalField) {
        m_Table = m_CurrentToken.second;
    }
}

auto kSQL::Parser::HandleLoadDb() -> void
{
    AdvanceToken();
    if(m_CurrentToken.first != TokenType::literalField)
        throw std::invalid_argument("Invalid syntax: expected literal after LOADDB");
    m_Table = m_CurrentToken.second;
}

auto kSQL::Parser::HandleSelect() -> bool
{
    AdvanceToken(); // advance select token
    ParseColumns();

    // FROM
    if (m_CurrentToken.first != TokenType::from)
        throw std::invalid_argument("Invalid syntax: expected FROM keyword");
    AdvanceToken();
    if (m_CurrentToken.first != TokenType::inputField)
        throw std::invalid_argument("Invalid syntax: no table name");
    m_Table = m_CurrentToken.second;
    AdvanceToken();

    if (m_CurrentToken.first == TokenType::join) {
        AdvanceToken();
        ParseJoin();
    }

    if (m_CurrentToken.first == TokenType::where) {
          AdvanceToken();
          ParseConditions();
    }

    if (m_CurrentToken.first == TokenType::orderby) {
          AdvanceToken();
          ParseOrderBy();
    }

    if (m_CurrentToken.first == TokenType::limit) {
        AdvanceToken();
        if(m_CurrentToken.first != TokenType::integer)
            throw std::invalid_argument("Invalid syntax: no integer after limit");
        m_Limit = std::atoi(m_CurrentToken.second.c_str());
        AdvanceToken();
        if (m_CurrentToken.first == TokenType::offset) {
            AdvanceToken();
            if (m_CurrentToken.first != TokenType::integer)
                throw std::invalid_argument("Invalid syntax: no integer after offset");
            m_Offset = std::atoi(m_CurrentToken.second.c_str());
        }

        //std::cout << "limit " << m_Limit << " offset " << m_Offset << "\n";
    }
}

auto kSQL::Parser::HandleAlter() -> void
{
    AdvanceToken();
    if(m_CurrentToken.first != TokenType::table)
        throw std::invalid_argument("Invalid syntax: expected TABLE keyword");
    AdvanceToken();
    if (m_CurrentToken.first != TokenType::inputField)
        throw std::invalid_argument("Invalid syntax: no table name");
    m_Table = m_CurrentToken.second;
    AdvanceToken();

    if (m_CurrentToken.first == TokenType::add) {

        AdvanceToken();
        if (m_CurrentToken.first != TokenType::inputField)
            throw std::invalid_argument("Invalid syntax: no column name");
        m_ColumnToAdd.first = m_CurrentToken.second;
        AdvanceToken();
        if (m_CurrentToken.second == "integer") {
            m_ColumnToAdd.second = TokenType::integer;
        }
        else if (m_CurrentToken.second == "numeric") {
            m_ColumnToAdd.second = TokenType::numeric;
        }
        else if (m_CurrentToken.second == "text") {
            m_ColumnToAdd.second = TokenType::literalField;
        }
        else {
            throw std::invalid_argument("Invalid syntax: " + m_CurrentToken.second + " is invalid data type");
        }
    }
    else if (m_CurrentToken.first == TokenType::drop) {
        AdvanceToken();
        if (m_CurrentToken.first != TokenType::inputField)
            throw std::invalid_argument("Invalid syntax: no column name");
        m_ColumnToRemove = m_CurrentToken.second;
    }
    else if (m_CurrentToken.first == TokenType::addkey) {
        AdvanceToken();
        if (m_CurrentToken.first != TokenType::inputField)
            throw std::invalid_argument("Invalid syntax: no table name");

        m_RefToAdd.first = m_CurrentToken.second;
        AdvanceToken();

        if (m_CurrentToken.first != TokenType::on)
            throw std::invalid_argument("Invalid syntax: no 'on' keyword");
        AdvanceToken();
        if (m_CurrentToken.first != TokenType::inputField)
            throw std::invalid_argument("Invalid syntax: no column name");
        m_RefToAdd.second = m_CurrentToken.second;
    }
    else if (m_CurrentToken.first == TokenType::dropkey) {
        AdvanceToken();
        if (m_CurrentToken.first != TokenType::inputField)
            throw std::invalid_argument("Invalid syntax: no table name");

        m_RefToRemove.first = m_CurrentToken.second;
        AdvanceToken();

        if (m_CurrentToken.first != TokenType::on)
            throw std::invalid_argument("Invalid syntax: no 'on' keyword");
        AdvanceToken();
        if (m_CurrentToken.first != TokenType::inputField)
            throw std::invalid_argument("Invalid syntax: no column name");
        m_RefToRemove.second = m_CurrentToken.second;
    }
    else {
        throw std::invalid_argument("Invalid syntax: no operation selected");
    }
}

auto kSQL::Parser::HandleUpdate() -> void
{
    AdvanceToken();
    if (m_CurrentToken.first != TokenType::inputField)
        throw std::invalid_argument("Invalid syntax: no table name");
    m_Table = m_CurrentToken.second;
    AdvanceToken();
    if (m_CurrentToken.first != TokenType::set)
        throw std::invalid_argument("Invalid syntax: no set keyword");
    AdvanceToken();

    while (m_CurrentToken.first == TokenType::inputField) {
        std::pair<std::string, std::pair<std::string, TokenType>> columnValue;
        columnValue.first = m_CurrentToken.second;
        AdvanceToken();
        if (m_CurrentToken.first != TokenType::eq)
            throw std::invalid_argument("Invalid syntax: no = operator");
        AdvanceToken();
        if (m_CurrentToken.first != TokenType::literalField &&
            m_CurrentToken.first != TokenType::integer &&
            m_CurrentToken.first != TokenType::numeric &&
            m_CurrentToken.first != TokenType::null)
            throw std::invalid_argument("Invalid syntax: " + m_CurrentToken.second + " unknown data type");
        columnValue.second = std::make_pair(m_CurrentToken.second, m_CurrentToken.first);
        AdvanceToken();
        if (m_CurrentToken.first == TokenType::comma)
            AdvanceToken();
        m_InsertValues.push_back(columnValue);
    }

    if(m_CurrentToken.first != TokenType::where)
        throw std::invalid_argument("Invalid syntax: no where keyword");
    AdvanceToken();
    ParseConditions();

    //std::cout << "here";
}

auto kSQL::Parser::ParseColumns() -> void
{
    // fix for insert
    while (m_CurrentToken.first == TokenType::inputField ||
        m_CurrentToken.first == TokenType::numeric ||
        m_CurrentToken.first == TokenType::integer) {

        m_Columns.push_back(m_CurrentToken.second);

        AdvanceToken();
        if (m_CurrentToken.first == TokenType::comma)
            AdvanceToken(); // ignore commas
        else
            break;
    }

}

// Na podstawie tego bloga: https://ruslanspivak.com/lsbasi-part7/
auto kSQL::Parser::ParseConditions() -> void
{
    //m_Root.children.push_back(ParseCondition());
    m_Root = ParseLogical();
}

auto kSQL::Parser::ParseLogical() -> ParserNode*
{
    auto leftOp = ParseStatment();

    while (isLogicalOp(m_CurrentToken.first) && m_CurrentTokenIndex < m_Tokens.size()-1) {
        const auto op = m_CurrentToken;
        AdvanceToken();
        const auto rightOp = ParseStatment();

        const auto newNode = new ParserNode({ op, {} });
        newNode->children.push_back(leftOp);
        newNode->children.push_back(rightOp);

        

        leftOp = newNode;
    }

    return leftOp;
}

auto kSQL::Parser::ParseStatment() -> ParserNode*
{
    auto leftArg = ParseInput();
    // notka z 'a'
    // is notnull

    while ((isCmpOp(m_CurrentToken.first) || m_CurrentToken.first == TokenType::isnotnull || m_CurrentToken.first == TokenType::isnull || m_CurrentToken.first == TokenType::like)) {
        const auto op = m_CurrentToken;
        const auto newNode = new ParserNode({ op, {} });
        AdvanceToken();

        if (op.first == TokenType::isnotnull || op.first == TokenType::isnull) {
            newNode->children.push_back(leftArg);
        }
        else {
            const auto rightArg = ParseInput();
            newNode->children.push_back(leftArg);
            newNode->children.push_back(rightArg);
        }

        leftArg = newNode;
        
    }

    return leftArg;
}

auto kSQL::Parser::ParseInput() -> ParserNode*
{
    if (isData(m_CurrentToken.first) || m_CurrentToken.first == TokenType::null ){
        const auto ret = new ParserNode({ m_CurrentToken, {} });
        AdvanceToken();
        return ret;
    }
    else if (m_CurrentToken.first == TokenType::leftp) {
        AdvanceToken();
        const auto logical = ParseLogical();
        if (m_CurrentToken.first != TokenType::rightp)
            throw std::invalid_argument("Invalid syntax: missing )");
        AdvanceToken();
        return logical;
    }

    throw std::invalid_argument("Invalid syntax: expected data got: " + m_CurrentToken.second);
}

auto kSQL::Parser::ParseOrderBy() -> void
{
    while (m_CurrentToken.first == TokenType::inputField) {
        std::pair<std::string, bool> newOrder;
        //std::cout << "ordering by " << m_CurrentToken.second << "\n";
        newOrder.first = m_CurrentToken.second;
        newOrder.second = false; // ascending
        AdvanceToken();
        if (m_CurrentToken.first == TokenType::asc || m_CurrentToken.first == TokenType::desc) {
            newOrder.second = m_CurrentToken.first == TokenType::desc;
            AdvanceToken();
        }

        if (m_CurrentToken.first == TokenType::comma)
            AdvanceToken();

        m_OrderBy.push_back(newOrder);
    }
}

auto kSQL::Parser::ParseJoin() -> void
{
    if (m_CurrentToken.first != TokenType::inputField)
        throw std::invalid_argument("Invalid syntax: invalid argument on join");
    m_JoinTable = m_CurrentToken.second;
    AdvanceToken();
    if (m_CurrentToken.first != TokenType::on)
        throw std::invalid_argument("Invalid syntax: expected ON after join table");
    AdvanceToken();
    
    if (m_CurrentToken.first != TokenType::inputField)
        throw std::invalid_argument("Invalid syntax: invalid argument on join after ON");

    m_JoinValues.push_back(m_CurrentToken.second);
    AdvanceToken();

    if (m_CurrentToken.first != TokenType::eq)
        throw std::invalid_argument("Invalid syntax: expected =");
    AdvanceToken();

    if (m_CurrentToken.first != TokenType::inputField)
        throw std::invalid_argument("Invalid syntax: invalid argument on join after ON");

    m_JoinValues.push_back(m_CurrentToken.second);
    AdvanceToken();
}

auto kSQL::Parser::isCmpOp(TokenType type) -> bool
{
    return type == TokenType::eq || type == TokenType::neq ||
            type == TokenType::lt || type == TokenType::ltq ||
            type == TokenType::gt || type == TokenType::gtq;
}

auto kSQL::Parser::isLogicalOp(TokenType type) -> bool
{
    return type == TokenType::ort || type == TokenType::andt;
}

auto kSQL::Parser::isData(TokenType type) -> bool
{
    return type == TokenType::literalField || type == TokenType::inputField || type == TokenType::numeric || type == TokenType::integer;
}

auto kSQL::Parser::AdvanceToken() -> void
{
    m_CurrentTokenIndex++;
    if (m_CurrentTokenIndex >= m_Tokens.size())
        throw std::invalid_argument("Invalid syntax: out of tokens");
    m_CurrentToken = m_Tokens[m_CurrentTokenIndex];
}

auto kSQL::Parser::DealocateAST(ParserNode* node) -> void
{
    if (!node)
        return;

    //std::cout << "deallocating node: " << node->token.second << "\n";

    for (int i = 0; i < node->children.size(); ++i) {
        DealocateAST(node->children[i]);
    }

    delete node;
}

kSQL::Parser::~Parser()
{
    DealocateAST(m_Root);
}

auto kSQL::Parser::Parse() -> bool
{
    if (m_Tokens.size() == 0)
        throw std::invalid_argument("Invalid syntax: no operation selected");

    m_CurrentToken = m_Tokens[0];
    m_QueryType = m_CurrentToken.first;

    switch (m_CurrentToken.first)
    {
    case TokenType::create:
        HandleCreate();
        break;
    case TokenType::drop:
        HandleDrop();
        break;
    case TokenType::insert:
        HandleInsert();
        break;
    case TokenType::_delete:
        HandleDelete();
        break;
    case TokenType::savedb:
        HandleSaveDb();
        break;
    case TokenType::loaddb:
        HandleLoadDb();
        break;
    case TokenType::quit:
        break;
    case TokenType::select:
        HandleSelect();
        break;
    case TokenType::alter:
        HandleAlter();
        break;
    case TokenType::update:
        HandleUpdate();
        break;
    default:
        throw std::invalid_argument("Invalid syntax: " + m_CurrentToken.second + " is unknown operation");
        break;
    }

    return true;
}

auto kSQL::Parser::GetQueryType() -> TokenType
{
    return m_QueryType;
}

auto kSQL::Parser::GetCondtions() -> ParserNode*
{
    return m_Root;
}

auto kSQL::Parser::GetTargetTable() -> std::string
{
    return m_Table;
}

auto kSQL::Parser::GetColumns() -> std::vector<std::string>
{
    return m_Columns;
}

auto kSQL::Parser::GetJoinTable() -> std::string
{
    return m_JoinTable;
}

auto kSQL::Parser::GetJoinValues() -> std::vector<std::string>
{
    return m_JoinValues;
}

auto kSQL::Parser::GetOrderBy() -> std::vector<std::pair<std::string, bool>>
{
    return m_OrderBy;
}

auto kSQL::Parser::GetLimitAndOffset() -> std::pair<int, int>
{
    return std::make_pair(m_Limit, m_Offset);
}

auto kSQL::Parser::GetInsertValues() -> std::vector<std::pair<std::string, std::pair<std::string, TokenType>>>
{
    return m_InsertValues;
}

auto kSQL::Parser::GetNewColumns() -> std::vector<std::pair<TokenType, std::string>>
{
    return m_NewColumns;
}

auto kSQL::Parser::GetNewPk() -> std::string
{
    return m_NewTablePk;
}

auto kSQL::Parser::GetColumnToAdd() -> std::pair<std::string, TokenType>
{
    return m_ColumnToAdd;
}

auto kSQL::Parser::GetColumnToRemove() -> std::string
{
    return m_ColumnToRemove;
}

auto kSQL::Parser::GetRefToAdd() -> std::pair<std::string, std::string>
{
    return m_RefToAdd;
}

auto kSQL::Parser::GetRefToRemove() -> std::pair<std::string, std::string>
{
    return m_RefToRemove;
}
