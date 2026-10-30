#pragma once

#include "SharedDefs.h"
#include "vector.h"
#include <vector>
#include <string_view>

struct EntityEntry;

using EntityVector = std::vector<EntityEntry *>;

constexpr auto MAX_EDICTS = ( 1 << MAX_EDICT_BITS );
constexpr auto NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS = 10;

constexpr auto SPROP_UNSIGNED = ( 1 << 0 );
constexpr auto SPROP_COORD = ( 1 << 1 );
constexpr auto SPROP_NOSCALE = ( 1 << 2 );
constexpr auto SPROP_ROUNDDOWN = ( 1 << 3 );
constexpr auto SPROP_ROUNDUP = ( 1 << 4 );
constexpr auto SPROP_NORMAL = ( 1 << 5 );
constexpr auto SPROP_EXCLUDE = ( 1 << 6 );
constexpr auto SPROP_XYZE = ( 1 << 7 );
constexpr auto SPROP_INSIDEARRAY = ( 1 << 8 );
constexpr auto SPROP_PROXY_ALWAYS_YES = ( 1 << 9 );
constexpr auto SPROP_CHANGES_OFTEN = ( 1 << 10 );
constexpr auto SPROP_IS_A_VECTOR_ELEM = ( 1 << 11 );
constexpr auto SPROP_COLLAPSIBLE = ( 1 << 12 );

enum SendPropType
{
	DPT_Int = 0,
	DPT_Float,
	DPT_Vector,
	DPT_String,
	DPT_Array,
	DPT_DataTable,

	DPT_NUMSendPropTypes
};

inline std::string_view SendPropTypeToString( SendPropType type )
{
	switch ( type )
	{
		case DPT_Int: return "int";
		case DPT_Float: return "float";
		case DPT_Vector: return "vector";
		case DPT_String: return "string";
		case DPT_Array: return "array";
		case DPT_DataTable: return "data table";
		default: assert( false ); return "invalid type";
	}
}

struct SendProp
{
public:
	char m_propName[ 256 ];
	SendPropType m_propType;
	int m_flags;
	float m_fLowValue;
	float m_fHighValue;
	int m_nBits;
	int m_nNumElements;
	char m_dtname[ 256 ];
};

struct FlattenedPropEntry
{
	FlattenedPropEntry( const SendProp *prop, const SendProp *arrayElementProp )
		: m_prop( prop ), m_arrayElementProp( arrayElementProp )
	{
	}

	const SendProp *m_prop;
	const SendProp *m_arrayElementProp;
};

struct ExcludeEntry
{
	ExcludeEntry( const char *pVarName, const char *pDTName, const char *pDTExcluding )
		: m_pVarName( pVarName ), m_pDTName( pDTName ), m_pDTExcluding( pDTExcluding )
	{
	}

	const char *m_pVarName;
	const char *m_pDTName;
	const char *m_pDTExcluding;
};

struct Prop_t
{
	Prop_t()
	{
	}

	Prop_t( SendPropType type )
		: m_type( type ), m_nNumElements( 0 )
	{
		m_value.m_vector.Init();
	}

	~Prop_t()
	{
		if ( m_type == DPT_String )
			delete[] m_value.m_pString;
	}

	SendPropType m_type;
	union value
	{
		value() { m_vector.Init(); };
		int m_int;
		float m_float;
		const char *m_pString;
		std::int64_t m_int64;
		Vector m_vector;
	}
	m_value;
	int m_nNumElements;
};

struct PropEntry
{
	PropEntry( FlattenedPropEntry *pFlattenedProp, Prop_t *pPropValue )
		: m_pFlattenedProp( pFlattenedProp ), m_pPropValue( pPropValue )
	{
	}

	~PropEntry()
	{
		if ( m_pFlattenedProp->m_prop->m_propType == DPT_Array )
			delete[] m_pPropValue;
		else
			delete m_pPropValue;
	}

	FlattenedPropEntry *m_pFlattenedProp;
	Prop_t *m_pPropValue;
};

struct EntityEntry
{
	EntityEntry( int nEntity, std::uint32_t uClass, std::uint32_t uSerialNum )
		: m_nEntity( nEntity ), m_uClass( uClass ), m_uSerialNum( uSerialNum )
	{
	}

	~EntityEntry()
	{
		for ( auto i : m_props )
		{
			delete i;
		}
	}

	PropEntry *FindProp( const char *pName ) const
	{
		for ( auto i : m_props )
		{
			PropEntry *pProp = i;
			if ( strcmp( pProp->m_pFlattenedProp->m_prop->m_propName, pName ) == 0 )
			{
				return pProp;
			}
		}
		return nullptr;
	}

	void AddOrUpdateProp( FlattenedPropEntry *pFlattenedProp, Prop_t *pPropValue )
	{
		PropEntry *pProp = FindProp( pFlattenedProp->m_prop->m_propName );
		if ( pProp )
		{
			if ( pFlattenedProp->m_prop->m_propType == DPT_Array )
				delete[] pProp->m_pPropValue;
			else
				delete pProp->m_pPropValue;

			pProp->m_pPropValue = pPropValue;
		}
		else
		{
			pProp = new PropEntry( pFlattenedProp, pPropValue );
			m_props.push_back( pProp );
		}
	}

	int m_nEntity;
	std::uint32_t m_uClass;
	std::uint32_t m_uSerialNum;
	std::vector< PropEntry * > m_props;
};