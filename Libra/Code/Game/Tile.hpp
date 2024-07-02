#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"

//----------------------------------------------------------------------------------------------------------------------
// Red and blue added testing / debugging purposes
enum TileType
{
	TILE_TYPE_TEST_RED	,
	TILE_TYPE_GRASS		,
	TILE_TYPE_STONE		,
	TILE_TYPE_TEST_BLUE ,
	NUM_TILE_TYPES 
};

//----------------------------------------------------------------------------------------------------------------------
class Tile
{
public:
	AABB2 GetBounds() const;
	Rgba8 GetColor() const;

	TileType		 m_tileDefIndex = TILE_TYPE_TEST_RED;
	IntVec2			 m_tileCoords	= IntVec2(-1, -1);
};