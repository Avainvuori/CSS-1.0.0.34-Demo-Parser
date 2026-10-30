#pragma once

#include "SharedDefs.h"
#include "GameEvent.h"
#include "StringTable.h"
#include "DataTable.h"
#include "Player.h"
#include "bitbuf.h"
#include <list>

class DemoFile;

class DemoReader
{
public:
	DemoReader( DemoFile *pDemo );
	~DemoReader( void );

	bool ReadDemo( void );

private:
	DemoReader( const DemoReader & ) = delete;
	DemoReader &operator=( const DemoReader & ) = delete;

	void ReadUserCmd( void );

	void HandleDemoPacket( void );
	void HandleNETDisconnect( bf_read &reader );
	void HandleNETFile( bf_read &reader );
	void HandleNETTick( bf_read &reader );
	void HandleNETStringCmd( bf_read &reader );
	void HandleNETSetConVar( bf_read &reader );
	void HandleNETSignOnState( bf_read &reader );
	void HandleSVCPrint( bf_read &reader );
	void HandleSVCServerInfo( bf_read &reader );
	void HandleSVCSendTable( bf_read &reader );
	void HandleSVCClassInfo( bf_read &reader );
	void HandleSVCSetPause( bf_read &reader );
	void HandleSVCCreateStringTable( bf_read &reader );
	void HandleSVCUpdateStringTable( bf_read &reader );
	void HandleSVCVoiceInit( bf_read &reader );
	void HandleSVCVoiceData( bf_read &reader );
	void HandleSVCSounds( bf_read &reader );
	void HandleSVCSetView( bf_read &reader );
	void HandleSVCFixAngle( bf_read &reader );
	void HandleSVCCrosshairAngle( bf_read &reader );
	void HandleSVCBSPDecal( bf_read &reader );
	void HandleSVCUserMessage( bf_read &reader );
	void HandleSVCEntityMessage( bf_read &reader );
	void HandleSVCGameEvent( bf_read &reader );
	void HandleSVCPacketEntities( bf_read &reader );
	void HandleSVCTempEntities( bf_read &reader );
	void HandleSVCPrefetch( bf_read &reader );
	void HandleSVCMenu( bf_read &reader );
	void HandleSVCGameEventList( bf_read &reader );
	void HandleSVCGetCvarValue( bf_read &reader );

	GameEventVector	m_GameEvents;
	const GameEvent &GetGameEvent( std::uint32_t id );
	void ParseGameEvent( bf_read &reader );
	void HandleGameEvent( bf_read &reader );
	void HandlePlayerDeathEvent( bf_read &reader, const GameEvent &event );

	std::uint32_t m_iNumStringTables;
	StringTableData_t m_StringTables[ MAX_STRING_TABLES ];
	const StringTableData_t *CreateStringTable( const char *name, int max_entries, int user_data_size, int user_data_size_bits, bool user_data_fixed_size );
	void ParseStringTableUpdate( bf_read &reader, int entries, const StringTableData_t *pTable );
	const StringTableData_t *GetStringTableData( std::uint32_t index );

	int	m_iServerClassBits;
	ServerClassVector m_ServerClasses;
	SendTableVector	m_DataTables;
	ExcludeEntryVector m_currentExcludes;
	void ParseDataTables( bf_read &reader );
	SendTable *GetTableByName( const char *pName );
	SendTable *GetTableByClassID( std::uint32_t iClassID );
	void GatherExcludes( SendTable *pTable );
	void GatherProps( SendTable *pTable, int nServerClass );
	void GatherProps_IterateProps( SendTable *pTable, int nServerClass, std::vector< FlattenedPropEntry > &flattenedProps );
	bool IsPropExcluded( SendTable *pTable, const SendProp &checkSendProp );
	void FlattenDataTable( int nServerClass );

	EntityVector m_Entities;
	void PrintEntityUpdate( const EntityEntry *pEntity, std::uint32_t flags );
	bool ProcessPacketEntities( bf_read &reader, int updatedentries, bool isdelta );
	bool ReadNewEntity( bf_read &reader, EntityEntry *pEntity );
	EntityEntry *FindEntity( int nEntity );
	EntityEntry *AddEntity( int nEntity, std::uint32_t uClass, std::uint32_t uSerialNum );
	void RemoveEntity( int nEntity );
	FlattenedPropEntry *GetSendPropByIndex( std::uint32_t uClass, std::uint32_t uIndex );

	std::list<Player> m_players;
	Player *GetPlayerByEntityIndex( int entityindex );
	Player *GetPlayerByUserID( int userID );

	DemoFile *m_pDemo;
	bf_read m_reader;

	demoheader_t m_demoheader;

	int m_iCurrentTick;

	bool m_bPOV;
	int m_iPOVRecorderUserID;
	int m_iPOVRecorderSlot;

	bool m_bUse5BitStringTableIndices;
};