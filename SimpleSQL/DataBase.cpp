#include "DataBase.hpp"

auto kSQL::DataBase::ExecuteDrop(Parser& parser) -> void
{
	const auto targetTable = parser.GetTargetTable();

	if (!m_Tables.contains(targetTable)) {
		std::cerr << "ERROR: no table named " << targetTable << " found\n";
		return;
	}

	auto it = std::ranges::find_if(m_Relations, [&](Relation rel) {
		return rel.parentTableName == targetTable
		|| rel.childTableName == targetTable; });

	if (it != m_Relations.end()) {
		std::cerr << "ERROR: table named " << targetTable << " has relations\n";
		return;
	}

	delete m_Tables[targetTable];
	m_Tables.erase(m_Tables.find(targetTable));
}

auto kSQL::DataBase::ExecuteUpdate(Parser& parser) -> void
{
	const auto targetTable = parser.GetTargetTable();

	if (!m_Tables.contains(targetTable)) {
		std::cerr << "ERROR: no table named " << targetTable << " found\n";
		return;
	}

	const auto dataToUpdate = parser.GetInsertValues();
	const auto table = m_Tables[targetTable];

	int invalidCount = 0;

	for (int i = 0; i < table->GetSize(); ++i) {
		if (!table->IsRowValid(i))
			continue;

		try {

			if (!EvaluateConditions(parser.GetCondtions(), table, i))
				continue;

		}
		catch (const std::invalid_argument& e) {
			std::cerr << "EVALUATION ERROR: " << e.what() << "\n";
			return ;
		}

		try {
			const auto rowCopy = table->GetRowCopy(i);

			table->UpdateRow(dataToUpdate, i);
			if (!ValidateRelations(targetTable)) {
				invalidCount++;
				table->UpdateRow(rowCopy, i);
			}
		}
		catch (const std::invalid_argument& e) {
			//std::cerr << "ERROR: " << e.what() << "\n";
			invalidCount++;
		}

	}

	if (invalidCount > 0) {
		std::cerr << "ERROR: failed to update " << invalidCount << " rows\n";
	}
}

auto kSQL::DataBase::ExecuteAlter(Parser& parser) -> void
{
	const auto columnToAdd = parser.GetColumnToAdd();
	const auto columnToRemove = parser.GetColumnToRemove();
	const auto targetTable = parser.GetTargetTable();
	const auto relationToAdd = parser.GetRefToAdd();
	const auto relationToRemove = parser.GetRefToRemove();

	if (!relationToAdd.first.empty()) {
		// add foreign key
		Relation new_key;
		new_key.parentTableName = targetTable;
		new_key.childColumnName = relationToAdd.second;
		new_key.childTableName = relationToAdd.first;

		m_Relations.push_back(new_key);
		if (!ValidateRelations(targetTable)) {
			std::cerr << "ERROR: collisions found\n";

			auto it = std::ranges::find_if(m_Relations, [&](Relation rel) {
				return rel.parentTableName == targetTable 
				&& rel.childColumnName == relationToAdd.second 
				&& rel.childTableName == relationToAdd.first; });

			if (it != m_Relations.end())
				m_Relations.erase(it);
		}
		return;
	}
	else if (!relationToRemove.first.empty()) {
		// remove foreign key

		auto it = std::ranges::find_if(m_Relations, [&](Relation rel) {
			return rel.parentTableName == targetTable
			&& rel.childColumnName == relationToRemove.second
			&& rel.childTableName == relationToRemove.first; });

		if (it == m_Relations.end()) {
			std::cerr << "ERROR no relation found\n";
		}
		else {
			m_Relations.erase(it);
		}

		return;
	}

	if (!m_Tables.contains(targetTable)) {
		std::cerr << "ERROR: table named " << targetTable << " doesnt exist\n";
		return;
	}

	if (columnToAdd.first.empty()) {
		// remove column
		if (!m_Tables[targetTable]->HasColumn(columnToRemove)) {
			std::cerr << "ERROR: cannot remove column, no column found\n";
			return;
		}

		for (auto& rel : m_Relations) {
			if (rel.childColumnName == columnToRemove) {
				std::cerr << "ERROR: cannot remove column\n";
				return;
			}
		}

		if (m_Tables[targetTable]->GetPrimeyKeyName() == columnToRemove) {
			std::cerr << "ERROR: cannot remove primary column\n";
			return;
		}

		m_Tables[targetTable]->RemoveColumn(columnToRemove);
	}
	else if(columnToRemove.empty()){
		// add column
		m_Tables[targetTable]->AddColumn(columnToAdd.first, columnToAdd.second);
	}
	else {
		std::cerr << "ERROR: Invalid syntax\n";
		return;
	}
}

auto kSQL::DataBase::ExecuteCreate(Parser& parser) -> void
{
	const auto TableName = parser.GetTargetTable();
	const auto TablePk = parser.GetNewPk();
	const auto TableColumns = parser.GetNewColumns();

	if (m_Tables.contains(TableName)) {
		std::cerr << "ERROR: table named " << TableName << " already exists\n";
		return;
	}

	const auto iter = std::ranges::find_if(TableColumns, [&](std::pair<TokenType, std::string> entry) {
		return entry.second == TablePk;
		});

	if (iter == TableColumns.end()) {
		std::cerr << "ERROR: no column named " << TablePk << "\n";
		return;
	}

	const auto newTable = new Table();
	for (const auto& entry : TableColumns)
		newTable->AddColumn(entry.second, entry.first);
	newTable->SetPrimaryKey(TablePk);
	m_Tables[TableName] = newTable;
}

auto kSQL::DataBase::ExecuteInsert(Parser& parser) -> void
{
	const auto insertToTable = parser.GetTargetTable();
	if (!m_Tables.contains(insertToTable)) {
		std::cerr << "ERROR: no table named " << insertToTable << " found\n";
		return;
	}

	auto dataToInsert = parser.GetInsertValues();
	std::vector<std::pair<std::string, DataCell>> newRow(dataToInsert.size());
	std::ranges::transform(dataToInsert.begin(), dataToInsert.end(), newRow.begin(), [&](std::pair<std::string, std::pair<std::string, TokenType>> val) {
		DataCell cell;
		cell.data = val.second.first;
		cell.type = val.second.second;
		return std::make_pair(val.first, cell);
		});
	try {
		const auto rowID = m_Tables[insertToTable]->InsertRow(newRow);
		if (!ValidateRelations(insertToTable)) {
			std::cerr << "ERROR: Relations criteria not met\n";
			m_Tables[insertToTable]->RemoveRow(rowID);
		}
	}
	catch (const std::invalid_argument& e) {
		std::cerr << "ERROR " << e.what() << "\n";
	}
}

auto kSQL::DataBase::ExecuteSelect(Parser& parser) -> bool
{
	auto table = parser.GetTargetTable();
	auto colums = parser.GetColumns();

	auto joinTableName = parser.GetJoinTable();
	Table* joinTable = nullptr;
	if (joinTableName.size() > 0 && m_Tables.contains(joinTableName))
		joinTable = new Table();

	if (m_Tables.contains(table)) {
		Table* tablePtr = nullptr;
		if (!joinTable)
			tablePtr = m_Tables[table];
		else {
			tablePtr = joinTable;
			PopulateJoinTable(joinTable, table, joinTableName, parser);
			//joinTable->DebugPrint();
		}

		// print all columns
		if (std::ranges::find(colums, "*") != colums.end())
			colums = tablePtr->GetColumnNames();
		else {
			for (auto col : colums) {
				if (!tablePtr->HasColumn(col)) {
					std::cerr << "EVALUATION ERROR: invalid column name " << col << "\n";
					if (joinTable)
						delete joinTable;
					return false;
				}
			}
		}

		std::vector<std::pair<std::vector<std::string>, int>> output;

		for (int i = 0; i < tablePtr->GetSize(); ++i) {
			if (!tablePtr->IsRowValid(i))
				continue;

			try {

				if (!EvaluateConditions(parser.GetCondtions(), tablePtr, i))
					continue;

			}
			catch (const std::invalid_argument& e) {
				std::cerr << "EVALUATION ERROR: " << e.what() << "\n";
				if (joinTable)
					delete joinTable;
				return false;
			}

			std::vector<std::string> tmp;
			for (auto h : colums) {
				tmp.push_back(tablePtr->GetCell(h, i).toString());
				//std::cout << tablePtr->GetCell(h, i).toString() << "\t\t";
			}
			output.push_back({ tmp, i });
			
		}

		SortOutputByOrderBy(colums, output, parser, tablePtr);
		LimitResults(output, parser);

		// print headers
		for (auto h : colums) {
			std::cout << std::setw(15) << h;
		}
		std::cout << "\n";
		for (auto vec : output) {
			for (auto val : vec.first) {
				std::cout << std::setw(15) << val;
			}
			std::cout << "\n";
		}
		std::cout << "\n\n";

		if (joinTable)
			delete joinTable;

		return true;
	}

	std::cerr << "SELECT ERROR: no table named: " + table + "\n";

	return false;
}

auto kSQL::DataBase::ExecuteQuit() -> void
{
	// TODO: save tables and relatins to files
	InvalidateDataBase();
	exit(0);
}

auto kSQL::DataBase::ExecuteDelete(Parser& parser) -> void
{
	auto table = parser.GetTargetTable();
	if (!m_Tables.contains(table)) {
		std::cerr << "DELETE ERROR: no table named: " + table + "\n";
		return;
	}

	const auto tablePtr = m_Tables[table];

	std::vector<std::vector<std::pair<std::string, DataCell>>> rowsToInserBack;

	for (int i = 0; i < tablePtr->GetSize(); ++i) {
		if (!tablePtr->IsRowValid(i))
			continue;
		try {
			if (EvaluateConditions(parser.GetCondtions(), tablePtr, i)) {
				// TODO check relations
				const auto row = tablePtr->GetRowWithNames(i);
				tablePtr->RemoveRow(i);

				if (!ValidateRelations(table)) {
					std::cerr << "ERROR: connot remove row because of realtions\n";
					rowsToInserBack.push_back(row);
				}
			}
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "ERROR: " << e.what() << "\n";
		}
	}

	for (auto& row : rowsToInserBack)
		tablePtr->InsertRow(row);
}

auto kSQL::DataBase::EvaluateConditions(ParserNode* root, Table* table, int row) -> bool
{
	if (!root)
		return true;

	const auto childenSize = root->children.size();

	const auto validateData = [&](ParserNode* columnToken, ParserNode* valueToken) {
		if (columnToken->token.first != TokenType::inputField)
			throw std::invalid_argument("left side at operator " + root->token.second + " must be input field");

		if (valueToken->token.first == TokenType::inputField && !table->HasColumn(valueToken->token.second))
			throw std::invalid_argument("no table named " + valueToken->token.second + " found");

		if (valueToken->token.first != TokenType::inputField && table->GetColumnType(columnToken->token.second) != valueToken->token.first)
			throw std::invalid_argument("data type missmatch");
	};

	const auto compare = [&](auto l, auto r, TokenType operation) -> bool {
		switch (operation) {
		case TokenType::eq:
			return l == r;
		case TokenType::neq:
			return l != r;
		case TokenType::gt:
			return l > r;
		case TokenType::lt:
			return l < r;
		case TokenType::ltq:
			return l <= r;
		case TokenType::gtq:
			return l >= r;
		default:
			return false;
		}
	};


	switch (root->token.first)
	{
	case TokenType::ort:
		if (childenSize != 2)
			throw std::invalid_argument(root->token.second + " should have two arguments");
		return EvaluateConditions(root->children[0], table, row) || EvaluateConditions(root->children[1], table, row);
	case TokenType::andt:
		if (childenSize != 2)
			throw std::invalid_argument(root->token.second + " should have two arguments");
		return EvaluateConditions(root->children[0], table, row) && EvaluateConditions(root->children[1], table, row);
	case TokenType::eq:
	case TokenType::neq:
	case TokenType::lt:
	case TokenType::ltq:
	case TokenType::gt:
	case TokenType::gtq:
	case TokenType::like:
	{
		if (childenSize != 2)
			throw std::invalid_argument(root->token.second + " should have two arguments");

		auto columnToken = root->children[0];
		auto valueToken = root->children[1];
		validateData(columnToken, valueToken);

		if (table->GetCell(columnToken->token.second, row).type == TokenType::null)
			return false;

		if (root->token.first == TokenType::like) {
			const auto data = table->GetCell(columnToken->token.second, row).data;
			auto pattern = valueToken->token.second;
			// simple patterns:
			// % - represents zero, one, or multiple characters
			// _ represents one, single character

			auto patterToRegex = std::string();
			patterToRegex += "^";

			for (int i = 0; i < pattern.size(); ++i) {
				switch (pattern[i])
				{
				case '%':
					patterToRegex += ".*";
					break;
				case '_':
					patterToRegex += ".";
					break;
				case '^':
					patterToRegex += "\\^";
					break;
				case '.':
					patterToRegex += "\\.";
					break;
				case '$':
					patterToRegex += "\\$";
					break;
				case '+':
					patterToRegex += "\\+";
					break;
				case '?':
					patterToRegex += "\\?";
					break;
				case '|':
					patterToRegex += "\\|";
					break;
				case '*':
					patterToRegex += "\\*";
					break;
				default:
					patterToRegex += pattern[i];
					break;
				}
			}

			

			/*const auto replace = [&](const std::string& what, const std::string& to) {
				if (!what.size())
					return;

				size_t pos = 0;
				while ((pos = pattern.find(what, pos)) != std::string::npos) {
					pattern.replace(pos, what.length(), to);
					pos += what.length();
				}
			};

			replace("%", ".*");
			replace("_", ".");*/

			patterToRegex += "$";

			//std::cout << "data: " << data << " like " << patterToRegex << "\n";

			std::regex regex(patterToRegex);
			return std::regex_match(data, regex);
		}

		if (valueToken->token.first == TokenType::inputField) {
			return compare(table->GetCell(columnToken->token.second, row).data, table->GetCell(valueToken->token.second, row).data, root->token.first);
		}
		else if (valueToken->token.first == TokenType::integer) {
			return compare(std::atoi(table->GetCell(columnToken->token.second, row).data.c_str()), std::atoi(valueToken->token.second.c_str()), root->token.first);
		}
		else if (valueToken->token.first == TokenType::numeric){
			return compare(std::atof(table->GetCell(columnToken->token.second, row).data.c_str()), std::atof(valueToken->token.second.c_str()), root->token.first);
		}
		else if (valueToken->token.first == TokenType::literalField) {
			return compare(table->GetCell(columnToken->token.second, row).data, valueToken->token.second, root->token.first);
		}
		else
			throw std::invalid_argument("Syntax Error: Unknown value");

	}
	case TokenType::isnull:
		if (childenSize != 1)
			throw std::invalid_argument(root->token.second + " should have one argument");
		return table->GetCell(root->children[0]->token.second, row).type == TokenType::null;
	case TokenType::isnotnull:
		if (childenSize != 1)
			throw std::invalid_argument(root->token.second + " should have one argument");
		return table->GetCell(root->children[0]->token.second, row).type != TokenType::null;
	default:
		throw std::invalid_argument("unknown comperison operation");
		break;
	}
	return false;
}

auto kSQL::DataBase::EvaluateLogicalOp(ParserNode* root, Table* table, int row) -> bool
{
	const auto childenSize = root->children.size();
	return false;
}

auto kSQL::DataBase::SortOutputByOrderBy(const std::vector<std::string>& columns, std::vector<std::pair<std::vector<std::string>, int>>& result, Parser& parser, Table* table) -> void
{
	auto orderByVec = parser.GetOrderBy();
	std::ranges::reverse(orderByVec);

	for (auto orderBy : orderByVec) {
		if (!table->HasColumn(orderBy.first))
			continue;

		bool inResult = false;
		int index = 0;
		auto iter = std::ranges::find(columns, orderBy.first);
		if (iter != columns.end()) {
			inResult = true;
			// get vector index in result
			index = iter - columns.begin();
		}

		auto type = table->GetColumnType(orderBy.first);

		static const auto cmpareData = [&](const std::string& val1, const std::string& val2, bool desc) {
			switch (type)
			{
			case TokenType::literalField:
				if (desc)
					return val1 > val2;
				return val1 < val2;
			case TokenType::integer:
			{
				if (val1 == "NULL" || val2 == "NULL")
					return true;

				auto intval1 = std::atoi(val1.c_str());
				auto intval2 = std::atoi(val2.c_str());
				if (desc)
					return intval1 > intval2;
				return intval1 < intval2;
			}
			case TokenType::numeric:
			{
				if (val1 == "NULL" || val2 == "NULL")
					return true;

				auto fval1 = std::atof(val1.c_str());
				auto fval2 = std::atof(val2.c_str());
				if (desc)
					return fval1 > fval2;
				return fval1 < fval2;
			}
			default:
				return true;
			}
		};

		std::ranges::sort(result, [&](std::pair<std::vector<std::string>, int> vec1, std::pair<std::vector<std::string>, int> vec2) -> bool {
			
			

			if (inResult) {
				// false - asc, true - desc
				auto val1 = vec1.first[index];
				auto val2 = vec2.first[index];
				return cmpareData(val1, val2, orderBy.second);
			}
			else {
				auto val1 = table->GetCell(orderBy.first, vec1.second).toString();
				auto val2 = table->GetCell(orderBy.first, vec2.second).toString();
				return cmpareData(val1, val2, orderBy.second);
			}
			});
	}

}

auto kSQL::DataBase::LimitResults(std::vector<std::pair<std::vector<std::string>, int>>& result, Parser& parser) -> void
{
	const auto limits = parser.GetLimitAndOffset();
	
	if (limits.second > 0) {
		if (limits.second > result.size() - 1)
			result.clear();
		else {
			// std::ranges::shift_left since cpp 23 :<
			std::shift_left(result.begin(), result.end(), limits.second);
		}
	}
	if (limits.first > 0 && limits.first < result.size()) {
		//auto iter = result.begin();
		//std::advance(iter, limits.first);
		result.erase(result.begin() + limits.first, result.end());
		//std::cout << "done\n";
	}

}

auto kSQL::DataBase::PopulateJoinTable(Table* newjoinTable, const std::string& table, const std::string& joinTable, Parser& parser) -> void
{
	const auto joinValues = parser.GetJoinValues();

	// parse Join Values
	auto TableArg = std::string();
	auto onJoinTableArg = std::string();

	for (auto joinVal : joinValues) {
		auto name = std::string();
		auto ssJoin = std::stringstream(joinVal);
		if (std::getline(ssJoin, name, '.')) {
			if (name == table) {
				if (!std::getline(ssJoin, name, '.')) {
					std::cerr << "Syntax error: expected table name after '.' on join command\n";
					return;
				}
				TableArg = name;
			}
			else if (name == joinTable) {
				if (!std::getline(ssJoin, name, '.')) {
					std::cerr << "Syntax error: expected table name after '.' on join command\n";
					return;
				}
				onJoinTableArg = name;
			}
			else {
				std::cerr << "ERROR: no table named " << name << " found\n";
				return;
			}
		}
		else {
			std::cerr << "Syntax error: expected '.' on join command\n";
			return;
		}
	}
	//TableArg = std::string();
	//auto onJoinTableArg
	//std::cout << "TableArg = " << TableArg << " onJoinTableArg = " << onJoinTableArg << "\n";
	
	const auto TablePtr = m_Tables[table];
	const auto joinTablePtr = m_Tables[joinTable];

	if (!TablePtr->HasColumn(TableArg)) {
		std::cerr << "ERROR: no collumn named " << TableArg << " in " << table << "\n";
		return;
	}

	if (!joinTablePtr->HasColumn(onJoinTableArg)) {
		std::cerr << "ERROR: no collumn named " << onJoinTableArg << " in " << joinTable << "\n";
		return;
	}

	auto tableColumnNames = TablePtr->GetColumnNamesAndTypes();
	for (auto& col : tableColumnNames)
		col.first = table + "." + col.first;

	auto joinTableColumnNames = joinTablePtr->GetColumnNamesAndTypes();
	for (auto& col : joinTableColumnNames)
		col.first = joinTable + "." + col.first;

	for (auto col : tableColumnNames)
		newjoinTable->AddColumn(col.first, col.second);

	for (auto col : joinTableColumnNames)
		newjoinTable->AddColumn(col.first, col.second);

	newjoinTable->AddColumn("id", TokenType::integer);
	newjoinTable->SetPrimaryKey("id");
	int id = 0;

	for (int i = 0; i < TablePtr->GetSize(); ++i) {
		if (TablePtr->IsRowValid(i)) {

			auto tableCell = TablePtr->GetCell(TableArg, i);
			

			for (int j = 0; j < joinTablePtr->GetSize(); j++) {
				if (joinTablePtr->IsRowValid(j)) {
					auto joinTableCell = joinTablePtr->GetCell(onJoinTableArg, j);

					if (tableCell.type == joinTableCell.type && tableCell.data == joinTableCell.data) {
						// insert to dataToInsert

						std::vector<std::pair<std::string, DataCell>> dataToInsert;

						auto tableRowWithNames = TablePtr->GetRowWithNames(i);
						for (auto& tEntry : tableRowWithNames) {
							tEntry.first = table + "." + tEntry.first;
						}

						dataToInsert.insert(dataToInsert.end(), tableRowWithNames.begin(), tableRowWithNames.end());

						auto tableOnJoinRowWithNames = joinTablePtr->GetRowWithNames(j);
						for (auto& tEntry : tableOnJoinRowWithNames) {
							tEntry.first = joinTable + "." + tEntry.first;
						}

						dataToInsert.insert(dataToInsert.end(), tableOnJoinRowWithNames.begin(), tableOnJoinRowWithNames.end());

						DataCell idCell;
						idCell.data = std::to_string(id);
						idCell.type = TokenType::integer;

						dataToInsert.push_back(std::make_pair("id", idCell));
						newjoinTable->InsertRow(dataToInsert);
						id++;

					}
				}
			}
		}
	}

}

auto kSQL::DataBase::InvalidateDataBase() -> void
{
	m_Relations.clear();
	for (auto& [_, table] : m_Tables) {
		//std::cout << "Crearing " << _ << "\n";
		delete table;
	}
	m_Tables.clear();
	m_DatabasePath = std::string();
}

auto kSQL::DataBase::LoadFromMemory(const std::string& path) -> bool
{
	/*const auto mainFilePath = path + "/main.db";
	auto mainFile = std::fstream(mainFilePath, std::ios::in);
	if (!mainFile) {
		std::cerr << "ERROR: failed to open " << mainFilePath << " file.\n";
		return false;
	}
	auto line = std::string();

	while (std::getline(mainFile, line)) {
		auto newTable = new Table();
		auto tablePathName = pa
	}*/

	
	
	std::filesystem::path databasePath(path);
	if (!std::filesystem::is_directory(databasePath) || !std::filesystem::exists(databasePath)) {
		std::cerr << "ERROR: " << databasePath << " is not valid database\n";
		return false;
	}
	m_DatabasePath = path;
	std::cout << "Loading database from path: " << databasePath << "\n";

	for (const auto& fileEntry : std::filesystem::directory_iterator{ databasePath }) {
		if (fileEntry.is_directory())
			continue;

		const auto extension = fileEntry.path().extension().generic_string();
		const auto tableName = fileEntry.path().stem().generic_string();
		if (extension == ".table") {
			std::cout << "\tLoading table: " << tableName << " ";
			Table* newTable = new Table();
			if (newTable->LoadFromFile(fileEntry.path().generic_string())) {
				this->AddTable(tableName, newTable);
				std::cout << "OK\n";
			}
			else {
				std::cout << "ERROR\n";
			}
		}
		else if (extension == ".rel") {
			std::cout << "\t Loading relations: ";

			auto relFile = std::fstream(fileEntry.path(), std::ios::in);
			if (!relFile) {
				std::cout << "ERROR\n";
			}
			else {
				auto line = std::string();
				while (std::getline(relFile, line)) {
					if (line.empty())
						continue;
					Relation relation;
					auto stream = std::stringstream(line);
					auto token = std::string();

					stream >> token;
					relation.parentTableName = token;
					stream >> token;
					relation.childTableName = token;
					stream >> token;
					relation.childColumnName = token;

					m_Relations.push_back(relation);

					std::cout << "\t Added relation: parentTable: " << relation.parentTableName << " childTableName: " << relation.childTableName
						<< " childColumnName: " << relation.childColumnName << "\n";
				}
			}
		}

		//std::cout << "path " << fileEntry.path() << " extension " << fileEntry.path().extension() << " filename " << fileEntry.path().stem() << "\n";
	}

	return true;
}

auto kSQL::DataBase::SaveToFile(const std::string& path) -> void
{
	auto targetPath = path;
	if (!path.size())
		targetPath = m_DatabasePath;
	if (!targetPath.size()) {
		std::cerr << "ERROR: no database name selected\n";
		return;
	}

	std::filesystem::path databasePath(targetPath);
	if (!std::filesystem::is_directory(databasePath) || !std::filesystem::exists(databasePath)) {
		std::filesystem::create_directory(databasePath);
	}

	for (const auto& fileEntry : std::filesystem::directory_iterator{ databasePath }) {
		if (fileEntry.is_directory())
			continue;

		const auto extension = fileEntry.path().extension().generic_string();
		const auto tableName = fileEntry.path().stem().generic_string();

		if (extension == ".table" || extension == ".rel") {
			std::filesystem::remove(fileEntry);
		}
	}

	//const auto createAndSaveTable = [&](const std::string& tableName) {
	//	//auto tmpPath = path + "/" + tableName + ".table";
	//	//auto std::fstream(tmpPath, ios::out)
	//};

	for (const auto& [tableName, tablePtr] : m_Tables) {
		if (tablePtr->SaveToFile(targetPath + "/" + tableName + ".table")) {
			std::cout << "\tSaved " << tableName << " to file (path: " << targetPath + "/" + tableName + ".table" << " )\n";
		}
		else {
			std::cerr << "\tFailed to save " << tableName << " to file\n";
		}
		
	}

	// save relations

	auto relFile = std::fstream(targetPath + "/rel.rel", std::ios::out | std::ios::trunc);
	if (!relFile) {
		std::cerr << "\tFailed to save relations to file\n";
		return;
	}
	
	for (const auto& rel : m_Relations) {
		//std::string parentTableName;
		//std::string childTableName;
		//std::string childColumnName;
		relFile << rel.parentTableName << " ";
		relFile << rel.childTableName << " ";
		relFile << rel.childColumnName << "\n";
	}

	std::cout << "\tSaved relations to file\n";
}

auto kSQL::DataBase::Execute(const std::string query) -> bool
{
	// measure time
	bool executed = false;
	
	/*auto curTime = std::chrono::steady_clock::now();*/

	Tokenizer tokenizer(query);
	tokenizer.Tokenize();

	Parser parser(tokenizer.getTokens());

	try {
		parser.Parse();
	}
	catch (const std::invalid_argument& e) {
		std::cerr << "PARSER ERROR: " << e.what() << "\n";
		return false;
	}

	switch (parser.GetQueryType())
	{
	case TokenType::drop:
		ExecuteDrop(parser);
		break;
	case TokenType::update:
		ExecuteUpdate(parser);
		break;
	case TokenType::alter:
		ExecuteAlter(parser);
		break;
	case TokenType::create:
		ExecuteCreate(parser);
		break;
	case TokenType::insert:
		ExecuteInsert(parser);
		break;
	case TokenType::_delete:
		ExecuteDelete(parser);
		break;
	case TokenType::loaddb:
		InvalidateDataBase();
		LoadFromMemory(parser.GetTargetTable());
		break;
	case TokenType::savedb:
		SaveToFile(parser.GetTargetTable());
		break;
	case TokenType::quit:
		ExecuteQuit();
	case TokenType::select:
		executed = ExecuteSelect(parser);
		break;
	default:
		std::cerr << "ERROR: Token not handled\n";
		break;
	}

	/*if (executed) {
		auto timeAfterExecution = std::chrono::steady_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeAfterExecution - curTime);
		std::cout << "Executed in " << ms << "\n";
	}*/

	return executed;
}

auto kSQL::DataBase::ValidateRelations(const std::string& updateTableName) -> bool
{
	// skip invalid entries
	if (!m_Tables.contains(updateTableName))
		return true;


	for (auto rel : m_Relations) {
		//if (rel.parentTableName == updateTableName || rel.childTableName == updateTableName) {
			// skip invalid entries
			if (!m_Tables.contains(rel.parentTableName))
				continue;

			auto childColumns = m_Tables[rel.childTableName]->GetColumn(rel.childColumnName);
			auto parentColumns = m_Tables[rel.parentTableName]->GetColumn(m_Tables[rel.parentTableName]->GetPrimeyKeyName());

			
			for (auto childCell : childColumns) {

				bool found = false;

				for (auto parentCell : parentColumns) {
					if (childCell.data == parentCell.data || childCell.type == TokenType::null ) {
						found = true;
						continue;
						//return true;
					}
				}

				if (!found)
					return false;
			}

			//return true;
		//}
	}

	return true;
}
