#pragma once

#include <vector>
#include <string>
#include <cstdint>

enum class GameEventValueType
{
	NONE = 0,
	STRING = 1,
	FLOAT = 2,
	LONG = 3,
	SHORT = 4,
	BYTE = 5,
	BOOL = 6
};

struct GameEvent
{
	GameEvent( int id, const char *name );

	int id;
	std::string name;

	struct GameEventField
	{
		GameEventField( const char *name, GameEventValueType type );

		std::string name;
		GameEventValueType type;
	};

	void AddField( const char *name, GameEventValueType type );
	const GameEventField &GetField( std::uint32_t index ) const;
	size_t GetFieldCount( void ) const;

private:
	std::vector< GameEventField > fields;
};

using GameEventVector = std::vector< GameEvent >;