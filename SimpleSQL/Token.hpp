#pragma once
#include <string>
#include <unordered_map>
#include <regex>

namespace kSQL {

	enum class TokenType
	{
		none,
		select,
		from,
		where,
		orderby,
		limit,
		offset,

		asc,
		desc,

		eq,
		neq,
		lt,
		gt,
		ltq,
		gtq,

		andt,
		ort,

		leftp,
		rightp,
		parenthesis,

		semicolon,
		comma,

		inputField,
		integer,
		numeric,
		literalField,

		fields,
		tables,
		conditions,

		literal,
		eor,
		null,

		isnull,
		isnotnull,

		join,
		on,

		quit,
		like,

		loaddb,
		savedb,
		newdb,
		_delete,
		insert,
		into,
		create,
		table,
		pk,
		alter,
		add,
		addkey,
		dropkey,
		drop,

		update,
		set,
	};

	static inline std::unordered_map<std::string, TokenType> g_KeyLookUp = {
		{"SELECT", TokenType::select},
		{"FROM", TokenType::from},
		//{";", TokenType::semicolon},
		{",", TokenType::comma},
		{"WHERE", TokenType::where},
		{"ORDERBY", TokenType::orderby},
		{"LIMIT", TokenType::limit},
		{"=", TokenType::eq},
		{"!=", TokenType::neq},
		{"<", TokenType::lt},
		{">", TokenType::gt},
		{"<=", TokenType::ltq},
		{">=", TokenType::gtq},
		{"AND", TokenType::andt},
		{"OR", TokenType::ort},
		{"(", TokenType::leftp},
		{")", TokenType::rightp},
		{"ASC", TokenType::asc},
		{"DESC", TokenType::desc},
		{"'", TokenType::literal},
		{"NULL", TokenType::null},
		{"ISNULL", TokenType::isnull},
		{"ISNOTNULL", TokenType::isnotnull},
		{"JOIN", TokenType::join},
		{"ON", TokenType::on},
		{"QUIT", TokenType::quit},
		{"LIKE", TokenType::like},
		{"OFFSET", TokenType::offset},
		{"LOADDB", TokenType::loaddb},
		{"SAVEDB", TokenType::savedb},
		{"NEWDB", TokenType::newdb},
		{"DELETE", TokenType::_delete},
		{"INSERT", TokenType::insert},
		{"INTO", TokenType::into},
		{"CREATE", TokenType::create},
		{"TABLE", TokenType::table},
		{"PK", TokenType::pk},
		{"ALTER", TokenType::alter},
		{"ADD", TokenType::add},
		{"ADDKEY", TokenType::addkey},
		{"DROPKEY", TokenType::dropkey},
		{"DROP", TokenType::drop},
		{"UPDATE", TokenType::update},
		{"SET", TokenType::set},
	};

	class Token
	{
	public:
		Token(const std::string& tokenName);

		/*
			Return true if Token is KeyWord, else otherwise
		*/
		bool Parse();

		TokenType getType();
	private:
		TokenType m_Type;
		std::string m_InputName;
	};
}

