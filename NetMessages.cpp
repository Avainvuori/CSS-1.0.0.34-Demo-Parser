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

#include "NetMessages.h"
#include "DemoReader.h"
#include "Entities.h"
#include <stdexcept>
#include <iostream>

extern bool g_bPrintStringTables;
extern bool g_bPrintVerbose;
extern bool g_bPrintNoDefaults;

std::string_view GetNetMessageString( byte msg )
{
	switch ( msg )
	{
		case NET_NOP:					return "NET_NOP";
		case NET_Disconnect:			return "NET_Disconnect";
		case NET_File:					return "NET_File";
		case NET_Tick:					return "NET_Tick";
		case NET_StringCmd:				return "NET_StringCmd";
		case NET_SetConVar:				return "NET_SetConVar";
		case NET_SignOnState:			return "NET_SignOnState";
		case SVC_Print:					return "SVC_Print";
		case SVC_ServerInfo:			return "SVC_ServerInfo";
		case SVC_SendTable:				return "SVC_SendTable";
		case SVC_ClassInfo:				return "SVC_ClassInfo";
		case SVC_SetPause:				return "SVC_SetPause";
		case SVC_CreateStringTable:		return "SVC_CreateStringTable";
		case SVC_UpdateStringTable:		return "SVC_UpdateStringTable";
		case SVC_VoiceInit:				return "SVC_VoiceInit";
		case SVC_VoiceData:				return "SVC_VoiceData";
		case SVC_Sounds:				return "SVC_Sounds";
		case SVC_SetView:				return "SVC_SetView";
		case SVC_FixAngle:				return "SVC_FixAngle";
		case SVC_CrosshairAngle:		return "SVC_CrosshairAngle";
		case SVC_BSPDecal:				return "SVC_BSPDecal";
		case SVC_UserMessage:			return "SVC_UserMessage";
		case SVC_EntityMessage:			return "SVC_EntityMessage";
		case SVC_GameEvent:				return "SVC_GameEvent";
		case SVC_PacketEntities:		return "SVC_PacketEntities";
		case SVC_TempEntities:			return "SVC_TempEntities";
		case SVC_Prefetch:				return "SVC_Prefetch";
		case SVC_Menu:					return "SVC_Menu";
		case SVC_GameEventList:			return "SVC_GameEventList";
		case SVC_GetCvarValue:			return "SVC_GetCvarValue";
		default:
			assert( false );
			return "invalid message type";
	}
}

std::string_view GetUserMessageString( UserMessage msg )
{
	switch ( msg )
	{
		case UserMessage::Geiger:			return "Geiger";
		case UserMessage::Train:			return "Train";
		case UserMessage::HudText:			return "HudText";
		case UserMessage::SayText:			return "SayText";
		case UserMessage::SayText2:			return "SayText2";
		case UserMessage::TextMsg:			return "TextMsg";
		case UserMessage::HudMsg:			return "HudMsg";
		case UserMessage::ResetHUD:			return "ResetHUD";
		case UserMessage::GameTitle:		return "GameTitle";
		case UserMessage::ItemPickup:		return "ItemPickup";
		case UserMessage::ShowMenu:			return "ShowMenu";
		case UserMessage::Shake:			return "Shake";
		case UserMessage::Fade:				return "Fade";
		case UserMessage::VGUIMenu:			return "VGUIMenu";
		case UserMessage::CloseCaption:		return "CloseCaption";
		case UserMessage::SendAudio:		return "SendAudio";
		case UserMessage::RawAudio:			return "RawAudio";
		case UserMessage::VoiceMask:		return "VoiceMask";
		case UserMessage::RequestState:		return "RequestState";
		case UserMessage::BarTime:			return "BarTime";
		case UserMessage::Damage:			return "Damage";
		case UserMessage::RadioText:		return "RadioText";
		case UserMessage::HintText:			return "HintText";
		case UserMessage::ReloadEffect:		return "ReloadEffect";
		case UserMessage::PlayerAnimEvent:	return "PlayerAnimEvent";
		case UserMessage::AmmoDenied:		return "AmmoDenied";
		case UserMessage::UpdateRadar:		return "UpdateRadar";
		case UserMessage::KillCam:			return "KillCam";
		case UserMessage::SPHapWeapEvent:	return "SPHapWeapEvent";
		case UserMessage::HapDmg:			return "HapDmg";
		case UserMessage::HapPunch:			return "HapPunch";
		case UserMessage::HapSetDrag:		return "HapSetDrag";
		case UserMessage::HapSetConst:		return "HapSetConst";
		case UserMessage::HapMeleeContact:	return "HapMeleeContact";
		default:
			assert( false );
			return "invalid message type";
	}
}

void DemoReader::HandleDemoPacket( void )
{
	democmdinfo_t cmdinfo;
	m_reader.ReadBytes( &cmdinfo, sizeof( democmdinfo_t ) );

	const std::uint32_t seqNrIn = m_reader.ReadLong();
	const std::uint32_t seqNrOut = m_reader.ReadLong();
	const std::uint32_t datasize = m_reader.ReadLong();

	std::string data( datasize, ' ' );
	m_reader.ReadBytes( &data[ 0 ], datasize );
	bf_read packetReader( &data[ 0 ], datasize );

	while ( (std::uint32_t)packetReader.GetNumBytesRead() < datasize )
	{
		const byte message = packetReader.ReadUBitLong( 5 );

		if ( message < 0 || message >= NumMessageTypes || packetReader.IsOverflowed() )
		{
			throw std::runtime_error( "Invalid NET/SVC message type encountered" );
		}

		std::cout << "  L " << GetNetMessageString( message ) << '\n';

		switch ( message )
		{
			default:
			{
				assert( false );
				throw std::runtime_error( "Invalid packet message type" );
			}
			case NET_NOP:
			{
				break;
			}
			case NET_Disconnect:
			{
				HandleNETDisconnect( packetReader );
				break;
			}
			case NET_File:
			{
				HandleNETFile( packetReader );
				break;
			}
			case NET_Tick:
			{
				HandleNETTick( packetReader );
				break;
			}
			case NET_StringCmd:
			{
				HandleNETStringCmd( packetReader );
				break;
			}
			case NET_SetConVar:
			{
				HandleNETSetConVar( packetReader );
				break;
			}
			case NET_SignOnState:
			{
				HandleNETSignOnState( packetReader );
				break;
			}
			case SVC_Print:
			{
				HandleSVCPrint( packetReader );
				break;
			}
			case SVC_ServerInfo:
			{
				HandleSVCServerInfo( packetReader );
				break;
			}
			case SVC_SendTable:
			{
				HandleSVCSendTable( packetReader );
				break;
			}
			case SVC_ClassInfo:
			{
				HandleSVCClassInfo( packetReader );
				break;
			}
			case SVC_SetPause:
			{
				HandleSVCSetPause( packetReader );
				break;
			}
			case SVC_CreateStringTable:
			{
				HandleSVCCreateStringTable( packetReader );
				break;
			}
			case SVC_UpdateStringTable:
			{
				HandleSVCUpdateStringTable( packetReader );
				break;
			}
			case SVC_VoiceInit:
			{
				HandleSVCVoiceInit( packetReader );
				break;
			}
			case SVC_VoiceData:
			{
				HandleSVCVoiceData( packetReader );
				break;
			}
			case SVC_Sounds:
			{
				HandleSVCSounds( packetReader );
				break;
			}
			case SVC_SetView:
			{
				HandleSVCSetView( packetReader );
				break;
			}
			case SVC_FixAngle:
			{
				HandleSVCFixAngle( packetReader );
				break;
			}
			case SVC_CrosshairAngle:
			{
				HandleSVCCrosshairAngle( packetReader );
				break;
			}
			case SVC_BSPDecal:
			{
				HandleSVCBSPDecal( packetReader );
				break;
			}
			case SVC_UserMessage:
			{
				HandleSVCUserMessage( packetReader );
				break;
			}
			case SVC_EntityMessage:
			{
				HandleSVCEntityMessage( packetReader );
				break;
			}
			case SVC_GameEvent:
			{
				HandleSVCGameEvent( packetReader );
				break;
			}
			case SVC_PacketEntities:
			{
				HandleSVCPacketEntities( packetReader );
				break;
			}
			case SVC_TempEntities:
			{
				HandleSVCTempEntities( packetReader );
				break;
			}
			case SVC_Prefetch:
			{
				HandleSVCPrefetch( packetReader );
				break;
			}
			case SVC_Menu:
			{
				HandleSVCMenu( packetReader );
				break;
			}
			case SVC_GameEventList:
			{
				HandleSVCGameEventList( packetReader );
				break;
			}
			case SVC_GetCvarValue:
			{
				HandleSVCGetCvarValue( packetReader );
				break;
			}
		}
	}
}

void DemoReader::HandleNETDisconnect( bf_read &reader )
{
	assert( false );
	throw std::runtime_error( "Message handler not implemented" );
}

void DemoReader::HandleNETFile( bf_read &reader )
{
	const int transferid = reader.ReadLong();
	char filename[ 1024 ];
	reader.ReadString( filename, sizeof( filename ) );
	const bool filerequested = reader.ReadOneBit();

	if ( g_bPrintVerbose )
		std::cout << "   L filename: " << filename << '\n';
}

void DemoReader::HandleNETTick( bf_read &reader )
{
	const int tick = reader.ReadLong();

	if ( g_bPrintVerbose )
		std::cout << "   L tick: " << tick << '\n';
}

void DemoReader::HandleNETStringCmd( bf_read &reader )
{
	char command[ 1024 ];
	reader.ReadString( command, sizeof( command ) );

	if ( g_bPrintVerbose )
		std::cout << "   L " << command << '\n';
}

void DemoReader::HandleNETSetConVar( bf_read &reader )
{
	int num = reader.ReadByte();
	while ( num-- > 0 )
	{
		char convar[ 256 ];
		reader.ReadString( convar, sizeof( convar ) );
		char value[ 256 ];
		reader.ReadString( value, sizeof( value ) );

		if ( g_bPrintVerbose )
			std::cout << "   L " << convar << " = " << value << '\n';
	}
}

void DemoReader::HandleNETSignOnState( bf_read &reader )
{
	const int signonstate = reader.ReadByte();
	const int spawncount = reader.ReadLong();

	if ( g_bPrintVerbose )
		std::cout << "   L sign on state: " << signonstate << "\n   L spawn count: " << spawncount << '\n';
}

void DemoReader::HandleSVCPrint( bf_read &reader )
{
	char message[ 1024 ];
	reader.ReadString( message, sizeof( message ) );

	if ( g_bPrintVerbose )
		std::cout << "   L message: " << message << '\n';
}

void DemoReader::HandleSVCServerInfo( bf_read &reader )
{
	const int protocol = reader.ReadShort();
	const int servercount = reader.ReadLong();
	const bool ishltv = reader.ReadOneBit();
	const bool isdedicated = reader.ReadOneBit();
	const int clientcrc = reader.ReadLong();
	const int maxclasses = reader.ReadShort();
	const int mapcrc = reader.ReadLong();
	const int playerslot = reader.ReadByte();
	const int maxclients = reader.ReadByte();
	const float tickinterval = reader.ReadFloat();
	const char platform = reader.ReadChar();

	m_bPOV = !ishltv;
	m_iPOVRecorderSlot = playerslot;

	if ( platform != 'w' && platform != 'l' )
		m_bUse5BitStringTableIndices = false;

	char gamedir[ 256 ];
	reader.ReadString( gamedir, sizeof( gamedir ) );

	char mapname[ 256 ];
	reader.ReadString( mapname, sizeof( mapname ) );

	char skyname[ 256 ];
	reader.ReadString( skyname, sizeof( skyname ) );

	char hostname[ 256 ];
	reader.ReadString( hostname, sizeof( hostname ) );

	if ( !g_bPrintNoDefaults )
	{
		std::cout << "   L host name: " << hostname << '\n';
		std::cout << "   L map name: " << mapname << '\n';
		std::cout << "   L " << ( ishltv ? "Source TV" : "POV" ) << " demo\n";
		std::cout << "   L " << ( isdedicated ? "dedicated" : "listen" ) << " server\n";
		std::cout << "   L running on " << ( 'W' == toupper( platform ) ? "Windows" : "Linux" ) << '\n';
		std::cout << "   L tick rate: " << (int)floor( 1 / tickinterval ) << '\n';
		std::cout << "   L server count: " << servercount << '\n';
		std::cout << "   L max clients: " << maxclients << '\n';
	}
}

void DemoReader::HandleSVCSendTable( bf_read &reader )
{
	assert( false );
	throw std::runtime_error( "Message handler not implemented" );
}

void DemoReader::HandleSVCClassInfo( bf_read &reader )
{
	const int num = reader.ReadShort();
	const bool createonclient = reader.ReadOneBit();
	if ( !createonclient )
	{
		assert( false );
		throw std::runtime_error( "Class info not created on client" );
	}
}

void DemoReader::HandleSVCSetPause( bf_read &reader )
{
	const bool paused = reader.ReadOneBit();
	if ( !g_bPrintNoDefaults )
		std::cout << "   L game " << ( paused ? "paused" : "unpaused" ) << '\n';
}

void DemoReader::HandleSVCCreateStringTable( bf_read &reader )
{
	char name[ 512 ];
	reader.ReadString( name, sizeof( name ) );

	const int max_entries = reader.ReadShort();
	const int num_entries = reader.ReadUBitLong( IntLog2( max_entries ) + 1 );
	const int datasize = reader.ReadUBitLong( 20 );

	const bool user_data_fixed_size = reader.ReadOneBit();
	int user_data_size = 0;
	int user_data_size_bits = 0;

	if ( user_data_fixed_size )
	{
		reader.ReadBits( &user_data_size, 12 );
		reader.ReadBits( &user_data_size_bits, 4 );
	}

	if ( !name || !strcmp( name, "" ) )
	{
		throw std::runtime_error( "String table has no name" );
	}

	auto pTable = CreateStringTable( name, max_entries, user_data_size, user_data_size_bits, user_data_fixed_size );

	if ( g_bPrintStringTables )
		std::cout << "   L created \"" << name << "\"\n";

	ParseStringTableUpdate( reader, num_entries, pTable );
}

void DemoReader::HandleSVCUpdateStringTable( bf_read &reader )
{
	const int tableID = reader.ReadUBitLong( m_bUse5BitStringTableIndices ? 5 : 4 );

	const bool multiple_changed_entries = reader.ReadOneBit();
	const int num_changed_entries = multiple_changed_entries ? reader.ReadShort() : 1;

	const int datasize = reader.ReadWord();

	auto pTable = GetStringTableData( tableID );

	if ( !pTable || num_changed_entries >= pTable->nMaxEntries )
		throw std::runtime_error( "Bad string table update" );

	if ( g_bPrintStringTables )
		std::cout << "   L updated \"" << pTable->sName << "\"\n";

	ParseStringTableUpdate( reader, num_changed_entries, pTable );
}

void DemoReader::HandleSVCVoiceInit( bf_read &reader )
{
	char codec[ 256 ];
	reader.ReadString( codec, sizeof( codec ) );
	const int quality = reader.ReadByte();

	if ( g_bPrintVerbose )
		std::cout << "   L codec: \"" << codec << "\" - quality: " << quality << '\n';
}

void DemoReader::HandleSVCVoiceData( bf_read &reader )
{
	const int client = reader.ReadByte();
	const int datalength = reader.ReadWord();
	reader.SeekRelative( datalength );

	if ( g_bPrintVerbose )
		std::cout << "   L client: " << client << '\n';
}

void DemoReader::HandleSVCSounds( bf_read &reader )
{
	const bool reliablesound = reader.ReadOneBit();
	int numsounds = 1;
	if ( !reliablesound )
	{
		numsounds = reader.ReadByte();
	}
	const int datalength = reliablesound ? reader.ReadByte() : reader.ReadShort();
	reader.SeekRelative( datalength );

	if ( g_bPrintVerbose )
		std::cout << "   L " << numsounds << ( numsounds > 1 ? " sounds" : " sound" ) << ( reliablesound ? " (reliable)" : " (unreliable)" ) << '\n';
}

void DemoReader::HandleSVCSetView( bf_read &reader )
{
	const int entityindex = reader.ReadUBitLong( 11 );

	if ( g_bPrintVerbose )
		std::cout << "   L entity: " << entityindex << '\n';
}

void DemoReader::HandleSVCFixAngle( bf_read &reader )
{
	const bool relative = reader.ReadOneBit();
	QAngle angle;
	angle.x = reader.ReadBitAngle( 16 );
	angle.y = reader.ReadBitAngle( 16 );
	angle.z = reader.ReadBitAngle( 16 );

	if ( g_bPrintVerbose )
		std::cout << "   L angle: (" << angle.x << " " << angle.y << " " << angle.z << ")\n";
}

void DemoReader::HandleSVCCrosshairAngle( bf_read &reader )
{
	assert( false );
	throw std::runtime_error( "Message handler not implemented" );
}

void DemoReader::HandleSVCBSPDecal( bf_read &reader )
{
	Vector pos;
	reader.ReadBitVec3Coord( pos );

	const int decaltextureindex = reader.ReadUBitLong( 9 );
	const bool onentity = reader.ReadOneBit();

	if ( onentity )
	{
		const int entityindex = reader.ReadUBitLong( 11 );
		const int modelindex = reader.ReadUBitLong( 11 );
	}

	const bool lowpriority = reader.ReadOneBit();

	if ( g_bPrintVerbose )
		std::cout << "   L decal index " << decaltextureindex << " at " << "(" << pos.x << " " << pos.y << " " << pos.z << ")" << ( lowpriority ? " (low priority)" : "" ) << '\n';
}

void DemoReader::HandleSVCUserMessage( bf_read &reader )
{
	const UserMessage msgtype = (UserMessage)reader.ReadByte();
	const int datalength = reader.ReadUBitLong( 11 );

	if ( g_bPrintVerbose || ( msgtype == UserMessage::SayText2 && !g_bPrintNoDefaults ) )
		std::cout << "   L " << GetUserMessageString( msgtype ) << '\n';

	if ( msgtype == UserMessage::SayText2 && !g_bPrintNoDefaults )
	{
		const byte sender = reader.ReadByte();
		const byte unknown = reader.ReadByte();

		char msg[ 5 ][ 256 ];
		for ( int i = 0; i < 5; ++i )
		{
			reader.ReadString( msg[ i ], sizeof( msg[ i ] ) );
			if ( msg[ i ][ 0 ] )
				std::cout << "    L " << msg[ i ] << '\n';
		}
	}
	else
	{
		reader.SeekRelative( datalength );
	}
}

void DemoReader::HandleSVCEntityMessage( bf_read &reader )
{
	assert( false );
	throw std::runtime_error( "Message handler not implemented" );
}

void DemoReader::HandleSVCGameEvent( bf_read &reader )
{
	const int datalength = reader.ReadUBitLong( 11 );
	HandleGameEvent( reader );
}

void DemoReader::HandleSVCPacketEntities( bf_read &reader )
{
	const int maxentries = reader.ReadUBitLong( 11 );
	const bool isdelta = reader.ReadOneBit();
	const int deltafrom = isdelta ? reader.ReadLong() : -1;
	const int baseline = reader.ReadUBitLong( 1 );
	const int updatedentries = reader.ReadUBitLong( 11 );

	const int datalength = reader.ReadUBitLong( 20 );

	const bool updatebaseline = reader.ReadOneBit();

	const int databytes = BitsToBytes( datalength );
	std::string data( databytes, ' ' );
	reader.ReadBits( &data[ 0 ], datalength );
	bf_read forkedReader( &data[ 0 ], databytes );

	ProcessPacketEntities( forkedReader, updatedentries, isdelta );
}

void DemoReader::HandleSVCTempEntities( bf_read &reader )
{
	const int numentries = reader.ReadByte();
	const int datalength = reader.ReadUBitLong( 17 );

	reader.SeekRelative( datalength );
}

void DemoReader::HandleSVCPrefetch( bf_read &reader )
{
	assert( false );
	throw std::runtime_error( "Message handler not implemented" );
}

void DemoReader::HandleSVCMenu( bf_read &reader )
{
	const int menutype = reader.ReadShort();
	const int datalength = reader.ReadWord();
	reader.SeekRelative( BytesToBits( datalength ) );
}

void DemoReader::HandleSVCGameEventList( bf_read &reader )
{
	int events = reader.ReadUBitLong( 9 );
	const int datalength = reader.ReadUBitLong( 20 );

	const int databytes = BitsToBytes( datalength );
	std::string data( databytes, ' ' );
	reader.ReadBits( &data[ 0 ], datalength );
	bf_read forkedReader( &data[ 0 ], databytes );

	while ( events-- > 0 )
	{
		ParseGameEvent( forkedReader );
	}
}

void DemoReader::HandleSVCGetCvarValue( bf_read &reader )
{
	const int cookie = reader.ReadLong();
	char cvarname[ 512 ];
	reader.ReadString( cvarname, sizeof( cvarname ) );

	if ( g_bPrintVerbose )
		std::cout << "   L " << cvarname << " (cookie: " << cookie << ")\n";
}