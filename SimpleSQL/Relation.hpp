#pragma once
#include <vector>
#include <string>

namespace kSQL {

	// many to many
	/*
		example
		relations.kRel
	parentTable ParentColumn -> childTable childColumn
	Owner id -> user ownerId
	
	*/
	struct Relation
	{
		std::string parentTableName;
		std::string childTableName;
		std::string childColumnName;
	};
}

