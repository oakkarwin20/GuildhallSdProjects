#pragma once

#include "Game/TileDefinition.hpp"

#include "Engine/Math/AABB3.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Tile
{
public:
	TileDefinition	m_tileDef;
	AABB3			m_bounds;
};