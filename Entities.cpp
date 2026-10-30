//====== Copyright (c) 2014, Valve Corporation, All rights reserved. ========//
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation 
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
// THE POSSIBILITY OF SUCH DAMAGE.
//===========================================================================//

#include "Entities.h"
#include "DemoReader.h"
#include "PropDecode.h"
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <string_view>

extern bool g_bPrintEntities;

enum UpdateType
{
	EnterPVS = 0,
	LeavePVS,
	DeltaEnt,
	PreserveEnt,
	Finished,
	Failed
};

constexpr auto ENTITY_SENTINEL = 9999;

constexpr auto FHDR_ZERO = 0x0000;
constexpr auto FHDR_LEAVEPVS = 0x0001;
constexpr auto FHDR_DELETE = 0x0002;
constexpr auto FHDR_ENTERPVS = 0x0004;

std::string_view GetEntityUpdateString( std::uint32_t flags )
{
	if ( flags == ( FHDR_LEAVEPVS | FHDR_DELETE ) )
		return "delete";

	if ( flags & FHDR_LEAVEPVS )
		return "leave PVS";

	if ( flags & FHDR_ENTERPVS )
		return "enter PVS";

	return "delta";
}

std::string GetPropValueString( Prop_t *pProp )
{
	switch ( pProp->m_type )
	{
		case DPT_Int: return std::to_string( pProp->m_value.m_int );
		case DPT_Float: return std::to_string( pProp->m_value.m_float );
		case DPT_Vector: return '(' + std::to_string( pProp->m_value.m_vector.x ) + ' ' + std::to_string( pProp->m_value.m_vector.y ) + ' ' + std::to_string( pProp->m_value.m_vector.z ) + ')';
		case DPT_String: return pProp->m_value.m_pString;
		case DPT_Array:
		case DPT_DataTable:
		default: assert( false ); return "invalid value";
	}
}

void DemoReader::PrintEntityUpdate( const EntityEntry *pEntity, std::uint32_t flags )
{
	if ( !g_bPrintEntities )
		return;

	if ( !pEntity )
	{
		assert( false );
		std::cout << "   L no entity\n";
		return;
	}

	const auto &srvcls = m_ServerClasses[ pEntity->m_uClass ];
	std::cout << "   L " << GetEntityUpdateString( flags ) << " on entity " << pEntity->m_nEntity << " (" << srvcls.strName << ")\n";
}

EntityEntry *DemoReader::FindEntity( int nEntity )
{
	for ( auto e : m_Entities )
	{
		if ( e->m_nEntity == nEntity )
		{
			return e;
		}
	}

	return nullptr;
}

EntityEntry *DemoReader::AddEntity( int nEntity, std::uint32_t uClass, std::uint32_t uSerialNum )
{
	EntityEntry *pEntity = FindEntity( nEntity );

	if ( pEntity )
	{
		pEntity->m_uClass = uClass;
		pEntity->m_uSerialNum = uSerialNum;
	}
	else
	{
		pEntity = new EntityEntry( nEntity, uClass, uSerialNum );
		m_Entities.push_back( pEntity );
	}

	return pEntity;
}

void DemoReader::RemoveEntity( int nEntity )
{
	for ( auto i = m_Entities.begin(); i != m_Entities.end(); ++i )
	{
		EntityEntry *pEntity = *i;
		if ( pEntity->m_nEntity == nEntity )
		{
			m_Entities.erase( i );
			delete pEntity;
			break;
		}
	}
}

bool DemoReader::ProcessPacketEntities( bf_read &reader, int updatedentries, bool isdelta )
{
	const bool bAsDelta = isdelta;
	int nHeaderCount = updatedentries;
	int nHeaderBase = -1;
	int UpdateFlags = FHDR_ZERO;

	UpdateType updateType = PreserveEnt;

	while ( updateType < Finished )
	{
		--nHeaderCount;

		const bool bIsEntity = nHeaderCount >= 0;
		int nNewEntity = -1;

		if ( bIsEntity )
		{
			UpdateFlags = FHDR_ZERO;

			nNewEntity = nHeaderBase + 1 + reader.ReadUBitVar();
			nHeaderBase = nNewEntity;

			if ( reader.ReadOneBit() == 0 )
			{
				if ( reader.ReadOneBit() != 0 )
				{
					UpdateFlags |= FHDR_ENTERPVS;
				}
			}
			else
			{
				UpdateFlags |= FHDR_LEAVEPVS;

				if ( reader.ReadOneBit() != 0 )
				{
					UpdateFlags |= FHDR_DELETE;
				}
			}
		}

		updateType = PreserveEnt;

		while ( updateType == PreserveEnt )
		{
			if ( !bIsEntity || nNewEntity > ENTITY_SENTINEL )
			{
				updateType = Finished;
			}
			else
			{
				if ( UpdateFlags & FHDR_ENTERPVS )
				{
					updateType = EnterPVS;
				}
				else if ( UpdateFlags & FHDR_LEAVEPVS )
				{
					updateType = LeavePVS;
				}
				else
				{
					updateType = DeltaEnt;
				}
			}

			switch ( updateType )
			{
				case EnterPVS:
				{
					const std::uint32_t uClass = reader.ReadUBitLong( m_iServerClassBits );
					const std::uint32_t uSerialNum = reader.ReadUBitLong( NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS );

					EntityEntry *pEntity = AddEntity( nNewEntity, uClass, uSerialNum );
					PrintEntityUpdate( pEntity, UpdateFlags );

					if ( !ReadNewEntity( reader, pEntity ) )
					{
						throw std::runtime_error( "Error reading entity in enter PVS" );
					}
				}
				break;

				case LeavePVS:
				{
					if ( !bAsDelta )
					{
						throw std::runtime_error( "Leave PVS on full update" );
					}
					else if ( UpdateFlags & FHDR_DELETE )
					{
						PrintEntityUpdate( FindEntity( nNewEntity ), UpdateFlags );
						RemoveEntity( nNewEntity );
					}
				}
				break;

				case DeltaEnt:
				{
					EntityEntry *pEntity = FindEntity( nNewEntity );

					if ( pEntity )
					{
						PrintEntityUpdate( pEntity, UpdateFlags );
						if ( !ReadNewEntity( reader, pEntity ) )
						{
							throw std::runtime_error( "Error reading entity in delta entity" );
						}
					}
					else
					{
						throw std::runtime_error( "Entity not found on delta entity" );
					}
				}
				break;

				case PreserveEnt:
				{
					assert( false );
					if ( !bAsDelta )
					{
						throw std::runtime_error( "Preserve entity on full update" );
					}
					else if ( nNewEntity >= MAX_EDICTS )
					{
						throw std::runtime_error( "New entity >= MAX_EDICTS in preserve entity" );
					}
				}
				break;

				default:
				{
					if ( updateType != Finished )
						throw std::runtime_error( "Invalid update type" );
				}
				break;
			}
		}
	}

	return true;
}

bool DemoReader::ReadNewEntity( bf_read &reader, EntityEntry *pEntity )
{
	int index = -1;

	while ( reader.ReadOneBit() )
	{
		index += reader.ReadUBitVar() + 1;

		FlattenedPropEntry *pSendProp = GetSendPropByIndex( pEntity->m_uClass, index );

		if ( !pSendProp )
		{
			return false;
		}

		const PropEntry *pOldProp = pEntity->FindProp( pSendProp->m_prop->m_propName );
		const std::string sOldValue = pOldProp ? GetPropValueString( pOldProp->m_pPropValue ) : "";

		Prop_t *pProp = DecodeProp( reader, pSendProp, pEntity->m_uClass, index );
		pEntity->AddOrUpdateProp( pSendProp, pProp );

		if ( g_bPrintEntities )
		{
			std::cout << "    L " << ( sOldValue.empty() ? "set" : "updated" ) << " \"" << pSendProp->m_prop->m_propName << '\"'
				<< ( sOldValue.empty() ? "" : ( " from " + sOldValue ) )
				<< " to " << GetPropValueString( pProp ) << '\n';
		}
	}

	return true;
}

FlattenedPropEntry *DemoReader::GetSendPropByIndex( std::uint32_t uClass, std::uint32_t uIndex )
{
	if ( uIndex < m_ServerClasses[ uClass ].flattenedProps.size() )
	{
		return &m_ServerClasses[ uClass ].flattenedProps[ uIndex ];
	}
	return nullptr;
}