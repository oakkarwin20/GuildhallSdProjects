#include "Game/TileDefinition.hpp"

std::vector<TileDefinition> TileDefinition::s_definitions;

//----------------------------------------------------------------------------------------------------------------------
TileDefinition::TileDefinition()
{
}

//----------------------------------------------------------------------------------------------------------------------
TileDefinition::TileDefinition( bool isSolid, IntVec2 spriteIndex, Rgba8 tint )
{
	m_isSolid		= isSolid;
	m_spriteCoords	= spriteIndex;
	m_tint			= tint;
}

//----------------------------------------------------------------------------------------------------------------------
TileDefinition::~TileDefinition()
{
}
