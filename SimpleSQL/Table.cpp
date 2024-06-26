#include "Table.hpp"

auto kSQL::Table::GetPrimeyKeyName() -> std::string
{
    return m_PrimaryKey;
}

auto kSQL::Table::SetPrimaryKey(const std::string& key) -> void
{
    m_PrimaryKey = key;
}

auto kSQL::Table::GetRow(int id) -> std::vector<DataCell>
{
    if(id > m_RowCount)
        return std::vector<DataCell>();

    std::vector<DataCell> ret;

    for (auto it = m_Data.begin(); it != m_Data.end(); ++it) {
        auto column = (*it).second;
        if (column.contains(id)) {
            ret.push_back(column[id]);
        }
        else {
            ret.push_back({ "NULL", TokenType::null });
        }
    }

    return ret;
}

auto kSQL::Table::GetRowWithNames(int id) -> std::vector<std::pair<std::string, DataCell>>
{
    std::vector<std::pair<std::string, DataCell>> ret;

    if (id > m_RowCount)
        return ret;

    for (auto it = m_Data.begin(); it != m_Data.end(); ++it) {
        auto column = (*it).second;
        if (column.contains(id)) {
            ret.push_back({ (*it).first, column[id] });
        }
        else {
            ret.push_back({ (*it).first, { "NULL", TokenType::null } });
        }
    }

    return ret;
}

auto kSQL::Table::RemoveRow(int id) -> void
{
    for (auto& col : m_Data) {

        const auto it = col.second.find(id);
        if( it != col.second.end())
            col.second.erase(it);
    }
}

auto kSQL::Table::IsRowValid(int id) -> bool
{
    if(!m_Data.contains(m_PrimaryKey))
        return false;
    if (!m_Data[m_PrimaryKey].contains(id) || m_Data[m_PrimaryKey][id].type == TokenType::null)
        return false;
    return true;
}

auto kSQL::Table::AddColumn(std::string columName, TokenType type) -> void
{
    m_Data.insert({ columName, {} });
    m_DataType.insert({ columName, type });
}

auto kSQL::Table::RemoveColumn(std::string columnName) -> void
{
    m_Data.erase(m_Data.find(columnName));
    m_DataType.erase(m_DataType.find(columnName));
}

auto kSQL::Table::GetCell(std::string columName, int id) -> DataCell
{
    if (m_Data.contains(columName)) {
        if (m_Data[columName].contains(id)) {
            return m_Data[columName][id];
        }
    }
    return { "NULL", TokenType::null };
}

auto kSQL::Table::GetColumn(std::string columName) -> std::vector<DataCell>
{
    if(!m_Data.contains(columName))
        return std::vector<DataCell>();

    std::vector<DataCell> ret;
    for (auto colEntry : m_Data[columName]) {
        ret.push_back(colEntry.second);
    }

    return ret;
}

auto kSQL::Table::SetCell(std::string columName, int id, DataCell cell) -> void
{
    m_Data[columName][id] = cell;
}

auto kSQL::Table::InsertRow(const std::vector<std::pair<std::string, DataCell>>& row) -> int
{
    // allow partial data inserts
    /*if (m_Data.size() != row.size())
        throw std::invalid_argument("Invalid data: data size mismatch");*/

    // check if primary key is present in insert
    bool pkPresent = false;
    for (auto el : row) {
        if (m_PrimaryKey == el.first)
            pkPresent = true;
    }
    if (!pkPresent)
        throw std::invalid_argument("Invalid data: no primary key present");

    for (auto el : row) {
        if (m_PrimaryKey == el.first && el.second.type == TokenType::null)
            throw std::invalid_argument("Invalid data: primary key must not be NULL");
        if (m_PrimaryKey == el.first) {
            for (auto entry : m_Data[m_PrimaryKey]) {
                if (entry.second.data == el.second.data)
                    throw std::invalid_argument("Invalid data: primary key must be unique");
            }
        }
        if (!m_DataType.contains(el.first))
            throw std::invalid_argument("Invalid data: no column named: " + el.first);
        if (el.second.type != TokenType::null && m_DataType[el.first] != el.second.type)
            throw std::invalid_argument("Invalid data: data type missmatch at colum: " + el.first + " value: " + el.second.data);

        m_Data[el.first][m_RowCount] = el.second;
    }
    auto ret = m_RowCount;
    m_RowCount++;
    return ret;
}

auto kSQL::Table::UpdateRow(const std::vector<std::pair<std::string, std::pair<std::string, TokenType>>>& row, int i) -> void
{
    bool pkPresent = false;
    for (auto el : row) {
        if (!this->m_Data.contains(el.first))
            throw std::invalid_argument("Invalid data: no column found");

        if(el.second.second != TokenType::null && this->m_DataType[el.first] != el.second.second)
            throw std::invalid_argument("Invalid data: invalid data type");

        if (m_PrimaryKey == el.first)
            pkPresent = true;
    }
    if (pkPresent)
        throw std::invalid_argument("Invalid data: cant change primary key");

    

    for (auto el : row) {

        DataCell cell;
        cell.data = el.second.first;
        cell.type = el.second.second;

        m_Data[el.first][i] = cell;
    }
}

auto kSQL::Table::GetRowCopy(int i) -> std::vector<std::pair<std::string, std::pair<std::string, TokenType>>>
{
    auto ret =  std::vector<std::pair<std::string, std::pair<std::string, TokenType>>>();

    for (auto& _t : m_Data) {
        if(_t.first != m_PrimaryKey)
            ret.push_back(std::make_pair(_t.first, std::make_pair(_t.second[i].data, _t.second[i].type)));
    }

    return ret;
}

auto kSQL::Table::DebugPrint() -> void
{
    for (auto it = m_Data.begin(); it != m_Data.end(); ++it) {
        auto columnName = (*it).first;
        std::cout << columnName << "\t";
    }
    std::cout << "\n";

    for (int i = 0; i < m_RowCount; ++i) {

        if (!IsRowValid(i))
            continue;

        for (auto it = m_Data.begin(); it != m_Data.end(); ++it) {
            const auto cell = GetCell((*it).first, i);
            if (cell.type != TokenType::null)
                std::cout << cell.data << "\t";
            else
                std::cout << "NULL" << "\t";
       }
        std::cout << "\n";
    }
    std::cout << "\n";
}

auto kSQL::Table::GetSize() -> int
{
    return m_RowCount;
}

auto kSQL::Table::GetColumnType(const std::string& column) -> TokenType
{
    if (!m_DataType.contains(column))
        return TokenType::none;
    return m_DataType[column];
}

auto kSQL::Table::GetColumnNames() -> std::vector<std::string>
{
    std::vector<std::string> ret;
    for (auto el : m_Data) {
        ret.push_back(el.first);
    }

    return ret;
}

auto kSQL::Table::GetColumnNamesAndTypes() -> std::vector<std::pair<std::string, TokenType>>
{
    auto names = GetColumnNames();
    std::vector<std::pair<std::string, TokenType>> ret;

    for (auto name : names) {
        ret.push_back({ name, GetColumnType(name) });
    }

    return ret;
}

auto kSQL::Table::HasColumn(const std::string& column) -> bool
{
    return m_Data.contains(column);
}

auto kSQL::Table::LoadFromFile(const std::string& filePath) -> bool
{
    auto file = std::fstream(filePath, std::ios::in);
    if (!file)
        return false;

    auto line = std::string();
    int lineCount = 0;
    int dataCount = 0;
    // column name to index index
    std::vector<std::string> columnsIds;

    while (std::getline(file, line)) {
        std::stringstream data(line);
        //std::cout << "\nline: " << line << "\n";
        auto token = std::string();
        if (lineCount == 0) {
            while (data >> token) {
                //std::cout << "token: " << token << " ";

                    // parse Column names and types
                auto type = std::string();
                auto tokenType = TokenType::none;
                if (!(data >> type))
                    return false;
                if (type == "integer")
                    tokenType = TokenType::integer;
                else if (type == "text")
                    tokenType = TokenType::literalField;
                else if (type == "numeric")
                    tokenType = TokenType::numeric;
                else
                    return false;

                //std::cout << "adding column: " << token << " type " << (int)tokenType << "\n";
                this->AddColumn(token, tokenType);
                columnsIds.push_back(token);
                dataCount++;
            }
        }
        else if (lineCount == 1) {
            // parse primarykey
            if (!this->HasColumn(line))
                return false;
            //std::cout << "setting primary key to: " << line << "\n";
            this->SetPrimaryKey(line);
        }
        else {
            std::vector<std::pair<std::string, DataCell>> row;
            int currentDataindex = 0;
            for (int i = 0; i < line.size(); i++) {
                auto dataToInsert = std::string();
                if (line[i] == '\'') {
                    
                    for (int j = i + 1; j < line.size(); ++j, ++i) {
                        if (line[j] == '\'') {
                            DataCell cell;
                            cell.data = dataToInsert;
                            cell.type = TokenType::literalField;
                            row.push_back(std::make_pair(columnsIds[currentDataindex], cell));
                            currentDataindex++;
                            i++; // skip '
                            break;
                        }
                        else {
                            dataToInsert += line[j];
                        }
                    }
                }
                else {
                    for (int j = i; j < line.size(); ++j, ++i) {
                        if (line[j] == ' ' || j == line.size() - 1) {
                            if (j == line.size() - 1)
                                dataToInsert += line[j];
                            if (line[j] == ' ' && dataToInsert.empty())
                                continue;
                            DataCell cell;
                            // make sure no ' chars
                            dataToInsert = std::regex_replace(dataToInsert, std::regex("'"), std::string(""));
                            cell.data = dataToInsert;
                            if (dataToInsert == "NULL")
                                cell.type = TokenType::null;
                            else
                                cell.type = this->GetColumnType(columnsIds[currentDataindex]);
                            row.push_back(std::make_pair(columnsIds[currentDataindex], cell));
                            currentDataindex++;
                            break;
                        }
                        else {
                            dataToInsert += line[j];
                        }
                    }
                }
            }

            /*for (auto el : row) {
                std::cout << "inserting to " << el.first << " data: " << el.second.data << "\n";
            }*/

            this->InsertRow(row);
        }
        lineCount++;
    }

    return true;
}

auto kSQL::Table::SaveToFile(const std::string& filePath) -> bool
{
    auto file = std::fstream(filePath, std::ios::out | std::ios::trunc);
    if (!file)
        return false;

    auto columnsAndTypes = this->GetColumnNamesAndTypes();
    for (auto col : columnsAndTypes) {
        file << col.first << " ";
        if (col.second == TokenType::literalField) {
            file << "text ";
        }
        else if (col.second == TokenType::numeric) {
            file << "numeric ";
        }
        else if (col.second == TokenType::integer) {
            file << "integer ";
        }
    }

    file << "\n";
    file << this->GetPrimeyKeyName();
    file << "\n";

    for (int i = 0; i < this->GetSize(); ++i) {
        if (!this->IsRowValid(i))
            continue;
        for (int j = 0; j < columnsAndTypes.size(); j++) {
            auto dataToSave = this->GetCell(columnsAndTypes[j].first, i).toString();
            auto dataType = this->GetCell(columnsAndTypes[j].first, i).type;
            if (dataType == TokenType::literalField) {
                file << '\'' << dataToSave << "\'";
            }
            else {
                file << dataToSave;
            }
            if (j < columnsAndTypes.size() - 1)
                file << " ";
        }
        file << "\n";
    }


    return true;
}
