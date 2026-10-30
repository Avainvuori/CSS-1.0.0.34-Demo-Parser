#pragma once

#include <string>

constexpr auto MAX_STRING_TABLES = 64;

struct StringTableData_t
{
	std::string		sName;
	int				nMaxEntries;
	int				nUserDataSize;
	int				nUserDataSizeBits;
	bool			bUserDataFixedSize;
};