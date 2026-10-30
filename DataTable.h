#pragma once

#include "Entities.h"
#include <string>
#include <vector>

class SendTable
{
public:
	char m_tableName[ 256 ];
	bool m_bNeedsDecoder;
	int m_nProps;
	std::vector< SendProp > m_props;
};

struct ServerClass_t
{
	int nClassID;
	char strName[ 256 ];
	char strDTName[ 256 ];
	int nDataTable;

	std::vector< FlattenedPropEntry > flattenedProps;
};

using ServerClassVector = std::vector< ServerClass_t >;
using SendTableVector = std::vector< SendTable >;
using ExcludeEntryVector = std::vector< ExcludeEntry >;