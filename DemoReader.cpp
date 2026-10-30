#include "DemoReader.h"
#include "DemoFile.h"
#include "SharedDefs.h"
#include <iostream>
#include <conio.h>

extern bool g_bPrintUserCmds;
extern bool g_bPrintVerbose;

DemoReader::DemoReader( DemoFile *pDemo )
	: m_pDemo{ pDemo }, m_iCurrentTick{ -1 }, m_bUse5BitStringTableIndices{ true }
{
	memset( &m_demoheader, 0, sizeof( m_demoheader ) );

	m_bPOV = false;
	m_iPOVRecorderUserID = -1;
	m_iPOVRecorderSlot = -1;
	m_iNumStringTables = 0;
	m_iServerClassBits = 0;
}

DemoReader::~DemoReader( void )
{
	for ( auto e : m_Entities )
	{
		delete e;
	}
}

std::string_view GetMessageString( byte msg )
{
	switch ( msg )
	{
		case dem_synctick:
			return "sync tick";
		case dem_stop:
			return "stop";
		case dem_signon:
			return "sign on";
		case dem_packet:
			return "packet";
		case dem_consolecmd:
			return "console command";
		case dem_datatables:
			return "datatables";
		case dem_usercmd:
			return "user commands";
		default:
			assert( false );
			return "invalid message type";
	}
}

bool DemoReader::ReadDemo( void )
{
	assert( m_pDemo && m_pDemo->IsValid() );

	m_reader.StartReading( m_pDemo->GetFileBuffer(), m_pDemo->GetFilesize() );

	m_reader.ReadBytes( &m_demoheader, sizeof( demoheader_t ) );

	std::cout << "\nReading " << m_pDemo->GetFilename() << "...\n";
	std::cout << "recorder: " << m_demoheader.clientname << '\n'
		<< "server: " << m_demoheader.servername << '\n'
		<< "map name: " << m_demoheader.mapname << "\n\n\n";

	while ( true )
	{
		const byte message = m_reader.ReadByte();

		if ( message < dem_signon || message > dem_stop || m_reader.IsOverflowed() )
			throw std::runtime_error( "Invalid demo message type" );

		if ( message == dem_stop )
			return true;

		const int tick{ m_reader.ReadLong() };

		if ( tick != m_iCurrentTick )
			std::cout << "Tick " << tick << '\n';

		std::cout << " L " << GetMessageString( message ) << '\n';

		m_iCurrentTick = tick;

		switch ( message )
		{
			case dem_synctick:
				break;

			case dem_stop:
			default:
			{
				assert( false );
				break;
			}

			case dem_signon:
			case dem_packet:
			{
				HandleDemoPacket();
				break;
			}

			case dem_consolecmd:
			{
				const size_t datasize( m_reader.ReadLong() );
				std::string cmdstring( datasize, ' ' );
				m_reader.ReadString( &cmdstring[ 0 ], datasize );

				if ( g_bPrintVerbose )
					std::cout << "  L " << cmdstring << '\n';
				break;
			}

			case dem_datatables:
			{
				const size_t datasize = m_reader.ReadLong();
				std::string data( datasize, ' ' );
				m_reader.ReadBytes( &data[ 0 ], datasize );
				bf_read forkedReader( &data[ 0 ], datasize );

				ParseDataTables( forkedReader );
				break;
			}

			case dem_usercmd:
			{
				ReadUserCmd();
				break;
			}
		}

		if ( _kbhit() )
		{
			constexpr int ESC = 27;

			if ( _getch() == ESC )
				return false;
		}
	}

	return true;
}

// HL2 SDK in_buttons.h
std::string GetButtonsString( std::uint32_t buttons )
{
	constexpr std::uint32_t IN_ATTACK{ 1 << 0 };
	constexpr std::uint32_t IN_JUMP{ 1 << 1 };
	constexpr std::uint32_t IN_DUCK{ 1 << 2 };
	constexpr std::uint32_t IN_FORWARD{ 1 << 3 };
	constexpr std::uint32_t IN_BACK{ 1 << 4 };
	constexpr std::uint32_t IN_USE{ 1 << 5 };
	constexpr std::uint32_t IN_CANCEL{ 1 << 6 };
	constexpr std::uint32_t IN_LEFT{ 1 << 7 };
	constexpr std::uint32_t IN_RIGHT{ 1 << 8 };
	constexpr std::uint32_t IN_MOVELEFT{ 1 << 9 };
	constexpr std::uint32_t IN_MOVERIGHT{ 1 << 10 };
	constexpr std::uint32_t IN_ATTACK2{ 1 << 11 };
	constexpr std::uint32_t IN_RUN{ 1 << 12 };
	constexpr std::uint32_t IN_RELOAD{ 1 << 13 };
	constexpr std::uint32_t IN_ALT1{ 1 << 14 };
	constexpr std::uint32_t IN_ALT2{ 1 << 15 };
	constexpr std::uint32_t IN_SCORE{ 1 << 16 };
	constexpr std::uint32_t IN_SPEED{ 1 << 17 };
	constexpr std::uint32_t IN_WALK{ 1 << 18 };
	constexpr std::uint32_t IN_ZOOM{ 1 << 19 };
	constexpr std::uint32_t IN_WEAPON1{ 1 << 20 };
	constexpr std::uint32_t IN_WEAPON2{ 1 << 21 };
	constexpr std::uint32_t IN_BULLRUSH{ 1 << 22 };
	constexpr std::uint32_t IN_GRENADE1{ 1 << 23 };
	constexpr std::uint32_t IN_GRENADE2{ 1 << 24 };

	std::string output{ "" };

	if ( buttons & IN_ATTACK )
		output += "attack, ";
	if ( buttons & IN_JUMP )
		output += "jump, ";
	if ( buttons & IN_DUCK )
		output += "crouch, ";
	if ( buttons & IN_FORWARD )
		output += "move forward, ";
	if ( buttons & IN_BACK )
		output += "move back, ";
	if ( buttons & IN_USE )
		output += "use, ";
	if ( buttons & IN_CANCEL )
		output += "cancel, ";
	if ( buttons & IN_LEFT )
		output += "left, ";
	if ( buttons & IN_RIGHT )
		output += "right, ";
	if ( buttons & IN_MOVELEFT )
		output += "move left, ";
	if ( buttons & IN_MOVERIGHT )
		output += "move right, ";
	if ( buttons & IN_ATTACK2 )
		output += "attack 2, ";
	if ( buttons & IN_RUN )
		output += "run, ";
	if ( buttons & IN_RELOAD )
		output += "reload, ";
	if ( buttons & IN_ALT1 )
		output += "alt 1, ";
	if ( buttons & IN_ALT2 )
		output += "alt 2, ";
	if ( buttons & IN_SCORE )
		output += "score, ";
	if ( buttons & IN_SPEED )
		output += "speed, ";
	if ( buttons & IN_WALK )
		output += "walk, ";
	if ( buttons & IN_ZOOM )
		output += "zoom, ";
	if ( buttons & IN_WEAPON1 )
		output += "weapon 1, ";
	if ( buttons & IN_WEAPON2 )
		output += "weapon 2, ";
	if ( buttons & IN_BULLRUSH )
		output += "bullrush, ";
	if ( buttons & IN_GRENADE1 )
		output += "grenade 1, ";
	if ( buttons & IN_GRENADE2 )
		output += "grenade 2, ";

	if ( !output.empty() )
		output = output.substr( 0, output.size() - 2 );

	return output;
}

// HL2 SDK usercmd.cpp
void DemoReader::ReadUserCmd( void )
{
	const std::int32_t outgoing_sequence{ m_reader.ReadLong() };
	const size_t datasize( m_reader.ReadLong() );

	if ( !g_bPrintUserCmds )
	{
		m_reader.SeekRelative( BytesToBits( datasize ) );
		return;
	}

	std::string buffer( datasize, ' ' );
	m_reader.ReadBytes( &buffer[ 0 ], datasize );
	bf_read forkedReader( &buffer[ 0 ], datasize );

	demousercmd_t usercmd;
	memset( &usercmd, 0, sizeof( usercmd ) );

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.command_number = forkedReader.ReadLong();
		std::cout << "  L command number: " << usercmd.command_number << '\n';
	}

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.tick_count = forkedReader.ReadLong();
		std::cout << "  L tick count: " << usercmd.tick_count << '\n';
	}

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.viewangles.x = forkedReader.ReadBitAngle( 16 );
		std::cout << "  L view angle x: " << usercmd.viewangles.x << '\n';
	}

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.viewangles.y = forkedReader.ReadBitAngle( 16 );
		std::cout << "  L view angle y: " << usercmd.viewangles.y << '\n';
	}

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.viewangles.z = forkedReader.ReadBitAngle( 8 );
		std::cout << "  L view angle z: " << usercmd.viewangles.z << '\n';
	}

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.forwardmove = (float)forkedReader.ReadSBitLong( 16 );
		std::cout << "  L forward move: " << usercmd.forwardmove << '\n';
	}

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.sidemove = (float)forkedReader.ReadSBitLong( 16 );
		std::cout << "  L side move: " << usercmd.sidemove << '\n';
	}

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.upmove = (float)forkedReader.ReadSBitLong( 16 );
		std::cout << "  L up move: " << usercmd.upmove << '\n';
	}

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.buttons = forkedReader.ReadUBitLong( 32 );
		std::cout << "  L buttons: " << GetButtonsString( usercmd.buttons ) << '\n';
	}

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.impulse = forkedReader.ReadByte();
		std::cout << "  L impulse: " << usercmd.impulse << '\n';
	}

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.weaponselect = forkedReader.ReadUBitLong( MAX_EDICT_BITS );
		std::cout << "  L weapon selection: " << usercmd.weaponselect << '\n';

		if ( forkedReader.ReadOneBit() )
		{
			constexpr auto WEAPON_SUBTYPE_BITS = 6;
			usercmd.weaponsubtype = forkedReader.ReadUBitLong( WEAPON_SUBTYPE_BITS );
			std::cout << "  L weapon subtype: " << usercmd.weaponsubtype << '\n';
		}
	}

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.mousedx = forkedReader.ReadShort();
		std::cout << "  L mouse dx: " << usercmd.mousedx << '\n';
	}

	if ( forkedReader.ReadOneBit() )
	{
		usercmd.mousedy = forkedReader.ReadShort();
		std::cout << "  L mouse dy: " << usercmd.mousedy << '\n';
	}
}

Player *DemoReader::GetPlayerByEntityIndex( int entityindex )
{
	for ( Player &p : m_players )
	{
		if ( p.entityindex == entityindex )
			return &p;
	}

	return nullptr;
}

Player *DemoReader::GetPlayerByUserID( int userID )
{
	for ( Player &p : m_players )
	{
		if ( p.userID == userID )
			return &p;
	}

	return nullptr;
}