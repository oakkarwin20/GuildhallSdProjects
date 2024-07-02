#pragma once

#include "Game/TileDefinition.hpp"

#include "Engine/Math/AABB3.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Tile
{
public:
	TileDefinition	m_tileDef;

public:
	IntVec2 m_hexCoord					= IntVec2( -1, -1 );
	int		m_hexDistToSelectedUnit		= -1;
	bool	m_isAlreadyVisted			= false;
};