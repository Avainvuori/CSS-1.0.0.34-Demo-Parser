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

#include "StringTable.h"
#include "DemoReader.h"
#include <stdexcept>
#include <iostream>

extern bool g_bPrintStringTables;

constexpr auto SUBSTRING_BITS = 5;
constexpr auto MAX_USERDATA_BITS = 12;
constexpr auto MAX_USERDATA_SIZE = ( 1 << MAX_USERDATA_BITS );

const StringTableData_t *DemoReader::CreateStringTable( const char *name, int max_entries, int user_data_size, int user_data_size_bits, bool user_data_fixed_size )
{
	if ( m_iNumStringTables >= MAX_STRING_TABLES )
		throw std::runtime_error( "Too many string tables" );

	m_StringTables[ m_iNumStringTables ].sName = name;
	m_StringTables[ m_iNumStringTables ].nMaxEntries = max_entries;
	m_StringTables[ m_iNumStringTables ].nUserDataSize = user_data_size;
	m_StringTables[ m_iNumStringTables ].nUserDataSizeBits = user_data_size_bits;
	m_StringTables[ m_iNumStringTables ].bUserDataFixedSize = user_data_fixed_size;

	return &m_StringTables[ m_iNumStringTables++ ];
}

void DemoReader::ParseStringTableUpdate( bf_read &reader, int entries, const StringTableData_t *pTable )
{
	const bool bIsUserInfo = !strcmp( pTable->sName.c_str(), "userinfo" );

	int lastEntry = -1;
	int lastDictionaryIndex = -1;

	const int nEntryBits = IntLog2( pTable->nMaxEntries );

	if ( ( 1 << nEntryBits ) != pTable->nMaxEntries )
		throw std::runtime_error( "String table size is not a power of two" );

	std::vector< std::string > string_history;

	for ( int i = 0; i < entries; i++ )
	{
		int entryIndex = lastEntry + 1;

		if ( !reader.ReadOneBit() )
		{
			entryIndex = reader.ReadUBitLong( nEntryBits );
		}

		lastEntry = entryIndex;

		if ( entryIndex < 0 || entryIndex >= pTable->nMaxEntries )
			throw std::runtime_error( "Invalid string index" );

		const char *pEntry = nullptr;
		char entry[ 1024 ];
		char substr[ 1024 ];
		entry[ 0 ] = 0;

		if ( reader.ReadOneBit() )
		{
			const bool substringcheck = reader.ReadOneBit() ? true : false;

			if ( substringcheck )
			{
				const int index = reader.ReadUBitLong( 5 );

				if ( size_t( index ) >= string_history.size() )
				{
					throw std::runtime_error( "Invalid string history index" );
				}

				const int bytestocopy = reader.ReadUBitLong( SUBSTRING_BITS );
				strncpy_s( entry, string_history[ index ].c_str(), bytestocopy + 1 );
				reader.ReadString( substr, sizeof( substr ) );
				strcat_s( entry, substr );
			}
			else
			{
				reader.ReadString( entry, sizeof( entry ) );
			}

			pEntry = entry;
		}

		char tempbuf[ MAX_USERDATA_SIZE ];
		memset( tempbuf, 0, sizeof( tempbuf ) );
		const char *pUserData = nullptr;
		int nBytes = 0;

		if ( reader.ReadOneBit() )
		{
			if ( pTable->bUserDataFixedSize )
			{
				nBytes = pTable->nUserDataSize;
				if ( nBytes <= 0 )
					throw std::runtime_error( "Invalid fixed user data size" );

				reader.ReadBits( tempbuf, pTable->nUserDataSizeBits );
			}
			else
			{
				nBytes = reader.ReadUBitLong( MAX_USERDATA_BITS );
				if ( nBytes > sizeof( tempbuf ) )
					throw std::runtime_error( "User data is too large" );

				reader.ReadBytes( tempbuf, nBytes );
			}

			tempbuf[ MAX_USERDATA_SIZE - 1 ] = 0;
			pUserData = tempbuf;
		}

		if ( !pEntry )
		{
			pEntry = "";
		}

		if ( g_bPrintStringTables )
			std::cout << "    L " << i + 1 << ". (" << entryIndex << ") \"" << pEntry << ( ( pUserData && !bIsUserInfo ) ? ( std::string( "\" \"" ) + pUserData + "\"\n" ) : "\"\n" );

		if ( pUserData && bIsUserInfo )
		{
			Player player( *(const Player *)pUserData );
			player.entityindex = entryIndex + 1;

			if ( m_bPOV && m_iPOVRecorderUserID < 0 )
			{
				if ( entryIndex == m_iPOVRecorderSlot )
				{
					m_iPOVRecorderUserID = player.userID;
				}
			}

			if ( g_bPrintStringTables )
			{
				std::cout
					<< "     L user id: " << player.userID
					<< "\n     L entity index: " << player.entityindex
					<< "\n     L name: " << player.name
					<< "\n     L guid: " << player.guid
					<< "\n     L fake player: " << ( player.fakeplayer ? "true" : "false" )
					<< "\n     L source tv: " << ( player.ishltv ? "true" : "false" )
					<< "\n     L friends id: " << player.friendsID
					<< "\n     L friends name: " << player.friendsName
					<< "\n     L files downloaded: " << player.filesDownloaded
					<< "\n     L custom files: ("
					<< player.customFiles[ 0 ] << " "
					<< player.customFiles[ 1 ] << " "
					<< player.customFiles[ 2 ] << " "
					<< player.customFiles[ 3 ] << ")\n";
			}

			Player *existing = GetPlayerByEntityIndex( player.entityindex );

			if ( !existing )
			{
				m_players.emplace_back( player );
			}
			else
			{
				if ( existing->userID != player.userID )
				{
					m_players.remove( *existing );
					m_players.emplace_back( player );
				}
				else
				{
					existing->Copy( player );
				}
			}
		}

		if ( string_history.size() > 31 )
		{
			string_history.erase( string_history.begin() );
		}

		string_history.emplace_back( pEntry );
	}
}

const StringTableData_t *DemoReader::GetStringTableData( std::uint32_t index )
{
	if ( index >= m_iNumStringTables )
		return nullptr;

	return &m_StringTables[ index ];
}