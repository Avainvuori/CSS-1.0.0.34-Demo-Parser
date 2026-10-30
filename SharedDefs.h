#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

#include "Vector.h"
#include <cstdint>
#include <stdexcept>

using byte = std::uint8_t;

constexpr auto MAX_EDICT_BITS = 11;

struct demoheader_t
{
	char			demofilestamp[ 8 ];
	std::int32_t	demoprotocol;
	std::int32_t	networkprotocol;
	char			servername[ MAX_PATH ];
	char			clientname[ MAX_PATH ];
	char			mapname[ MAX_PATH ];
	char			gamedirectory[ MAX_PATH ];
	float			playback_time;
	std::int32_t	playback_ticks;
	std::int32_t	playback_frames;
	std::int32_t	signonlength;
};

struct demousercmd_t
{
	int				command_number;
	int				tick_count;
	QAngle			viewangles;
	float			forwardmove;
	float			sidemove;
	float			upmove;
	std::uint32_t	buttons;
	byte			impulse;
	int				weaponselect;
	int				weaponsubtype;
	short			mousedx;
	short			mousedy;
};

struct democmdinfo_t
{
	std::int32_t	flags;

	Vector			viewOrigin;
	QAngle			viewAngles;
	QAngle			localViewAngles;

	Vector			viewOrigin2;
	QAngle			viewAngles2;
	QAngle			localViewAngles2;
};

enum
{
	dem_signon = 1,
	dem_packet,
	dem_synctick,
	dem_consolecmd,
	dem_usercmd,
	dem_datatables,
	dem_stop,
};


// ===========================================================
// Utility
// ===========================================================

inline int IntLog2( int val )
{
	if ( val == 0 )
		throw std::runtime_error( "Invalid value in log2" );

	int tmp{ 0 };
	while ( val >>= 1 )
		++tmp;

	return tmp;
}