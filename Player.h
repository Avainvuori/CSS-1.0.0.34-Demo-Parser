#pragma once

#include "SharedDefs.h"
#include "DemoReader.h"
#include <cstdint>

constexpr auto MAX_PLAYER_NAME_LENGTH = 32;
constexpr auto SIGNED_GUID_LEN = 32;
constexpr auto MAX_CUSTOM_FILES = 4;

struct Player
{
	char			name[ MAX_PLAYER_NAME_LENGTH ];
	int				userID;
	char			guid[ SIGNED_GUID_LEN + 1 ];
	std::uint32_t	friendsID;
	char			friendsName[ MAX_PLAYER_NAME_LENGTH ];
	bool			fakeplayer;
	bool			ishltv;
	std::uint32_t	customFiles[ MAX_CUSTOM_FILES ];
	byte			filesDownloaded;

	int				entityindex;

	Player( const Player &other );
	void Copy( const Player &other );
	bool operator==( const Player &other ) const;
};

inline Player::Player( const Player &other )
{
	Copy( other );
}

inline void Player::Copy( const Player &other )
{
	strcpy_s( name, sizeof( name ), other.name );
	strcpy_s( guid, sizeof( guid ), other.guid );
	strcpy_s( friendsName, sizeof( friendsName ), other.friendsName );

	userID = other.userID;
	friendsID = other.friendsID;
	fakeplayer = other.fakeplayer;
	ishltv = other.ishltv;
	filesDownloaded = other.filesDownloaded;
	entityindex = other.entityindex;

	for ( int i = 0; i < MAX_CUSTOM_FILES; ++i )
		customFiles[ i ] = other.customFiles[ i ];
}

inline bool Player::operator==( const Player &other ) const
{
	return this->userID == other.userID;
}