#include "GameEvent.h"
#include "DemoReader.h"
#include "Player.h"
#include "Entities.h"
#include <stdexcept>
#include <iostream>
#include <string_view>

extern bool g_bPrintVerbose;
extern bool g_bPrintNoDefaults;

std::string_view GameEventValueTypeToString( GameEventValueType type )
{
	switch ( type )
	{
		case GameEventValueType::NONE:		return "no type";
		case GameEventValueType::STRING:	return "string";
		case GameEventValueType::FLOAT:		return "float";
		case GameEventValueType::LONG:		return "long";
		case GameEventValueType::SHORT:		return "short";
		case GameEventValueType::BYTE:		return "byte";
		case GameEventValueType::BOOL:		return "bool";
		default: assert( false );			return "invalid type";
	}
}

GameEvent::GameEvent( int _id, const char *_name )
	: id{ _id }, name{ _name }
{
}

void GameEvent::AddField( const char *_name, GameEventValueType type )
{
	fields.emplace_back( _name, type );
}

const GameEvent::GameEventField &GameEvent::GetField( std::uint32_t index ) const
{
	assert( index < fields.size() );

	return fields[ index ];
}

size_t GameEvent::GetFieldCount( void ) const
{
	return fields.size();
}

GameEvent::GameEventField::GameEventField( const char *_name, GameEventValueType _type )
	: name{ _name }, type{ _type }
{
}

const GameEvent &DemoReader::GetGameEvent( std::uint32_t id )
{
	if ( id >= m_GameEvents.size() || m_GameEvents[ id ].id != id )
		throw std::runtime_error( "Invalid game event ID" );

	return m_GameEvents[ id ];
}

void DemoReader::ParseGameEvent( bf_read &reader )
{
	const int eventID = reader.ReadUBitLong( 9 );

	char name[ 32 ];
	if ( !reader.ReadString( name, sizeof( name ) ) )
		throw std::runtime_error( "Failed to read event name in SVC_GameEventList" );

	if ( g_bPrintVerbose )
		std::cout << "   L event: " << name << " (id: " << eventID << ")\n";

	GameEvent event( eventID, name );

	while ( true )
	{
		const GameEventValueType type = (GameEventValueType)reader.ReadUBitLong( 3 );

		if ( type == GameEventValueType::NONE )
			break;

		char fieldname[ 32 ];
		if ( !reader.ReadString( fieldname, sizeof( fieldname ) ) )
			throw std::runtime_error( "Failed to read event field name in SVC_GameEventList" );

		if ( g_bPrintVerbose )
			std::cout << "    L field: " << fieldname << " (" << GameEventValueTypeToString( type ) << ")\n";

		event.AddField( fieldname, type );
	}

	m_GameEvents.push_back( event );
}

void ReadGameEventFieldValue( bf_read &reader, const GameEvent::GameEventField &field )
{
	if ( g_bPrintVerbose )
		std::cout << "    L " << field.name << ": ";

	switch ( field.type )
	{
		case GameEventValueType::NONE:
		{
			if ( g_bPrintVerbose )
				std::cout << "no value\n";
			break;
		}
		case GameEventValueType::STRING:
		{
			char data[ 128 ];
			reader.ReadString( data, sizeof( data ) );
			if ( g_bPrintVerbose )
				std::cout << data << '\n';
			break;
		}
		case GameEventValueType::FLOAT:
		{
			const float value = reader.ReadFloat();
			if ( g_bPrintVerbose )
				std::cout << value << '\n';
			break;
		}
		case GameEventValueType::LONG:
		{
			const long value = reader.ReadLong();
			if ( g_bPrintVerbose )
				std::cout << value << '\n';
			break;
		}
		case GameEventValueType::SHORT:
		{
			const short value = reader.ReadShort();
			if ( g_bPrintVerbose )
				std::cout << value << '\n';
			break;
		}
		case GameEventValueType::BYTE:
		{
			const byte value = reader.ReadByte();
			if ( g_bPrintVerbose )
				std::cout << value << '\n';
			break;
		}
		case GameEventValueType::BOOL:
		{
			const bool value = reader.ReadOneBit();
			if ( g_bPrintVerbose )
				std::cout << value << '\n';
			break;
		}
		default:
		{
			throw std::runtime_error( "Unknown field type encountered in a game event" );
			break;
		}
	}
}

void DemoReader::HandlePlayerDeathEvent( bf_read &reader, const GameEvent &event )
{
	const Player *attacker = { nullptr };
	const Player *victim{ nullptr };
	char weaponName[ 32 ];
	bool headshot{ false };

	for ( size_t iField = 0; iField < event.GetFieldCount(); ++iField )
	{
		auto &field = event.GetField( iField );

		if ( field.name == "userid" )
		{
			victim = GetPlayerByUserID( reader.ReadShort() );
		}
		else if ( field.name == "attacker" )
		{
			attacker = GetPlayerByUserID( reader.ReadShort() );
		}
		else if ( field.name == "weapon" )
		{
			reader.ReadString( weaponName, 32 );
		}
		else if ( field.name == "headshot" )
		{
			headshot = reader.ReadOneBit();
		}
		else
		{
			ReadGameEventFieldValue( reader, field );
		}
	}

	if ( !victim )
		throw std::runtime_error( "Victim info not found" );

	if ( !attacker || victim == attacker )
	{
		std::cout << "    L " << victim->name << " suicided\n";
		return;
	}

	const EntityEntry *entAttacker = FindEntity( attacker->entityindex );
	const EntityEntry *entVictim = FindEntity( victim->entityindex );
	float distance = -1.f;

	if ( entAttacker && entVictim )
	{
		Vector vecAttacker, vecVictim;

		vecAttacker = entAttacker->FindProp( "m_vecOrigin" )->m_pPropValue->m_value.m_vector;
		vecVictim = entVictim->FindProp( "m_vecOrigin" )->m_pPropValue->m_value.m_vector;

		distance = ( vecAttacker - vecVictim ).Length();
	}

	std::cout << "    L "
		<< attacker->name << " killed "
		<< victim->name << " with " << weaponName
		<< ( headshot ? " by headshot" : "" )
		<< ( distance >= 0 ? ( " (distance: " + std::to_string( (int)distance ) + "u)\n" ) : "\n" );
}

void DemoReader::HandleGameEvent( bf_read &reader )
{
	const int eventID = reader.ReadUBitLong( 9 );
	const GameEvent &event = GetGameEvent( eventID );

	if ( g_bPrintVerbose || (!g_bPrintNoDefaults && (event.name == "player_death" || event.name == "round_start")) )
		std::cout << "   L " << event.name << '\n';

	if ( event.name == "player_death" && !g_bPrintNoDefaults )
	{
		HandlePlayerDeathEvent( reader, event );
		return;
	}

	const size_t fields = event.GetFieldCount();

	for ( size_t i = 0; i < fields; ++i )
	{
		auto &field = event.GetField( i );

		ReadGameEventFieldValue( reader, field );
	}
}