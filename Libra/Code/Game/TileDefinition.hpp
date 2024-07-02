#pragma once

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class TileDefinition
{
public:
	TileDefinition();

	TileDefinition( bool isSolid, IntVec2 spriteIndex, Rgba8 tint = Rgba8( 255, 255, 255, 255 ) );
	~TileDefinition();

	bool		m_isSolid		= true;
	IntVec2		m_spriteCoords	= IntVec2( -1, -1 );
	Rgba8		m_tint			= Rgba8( 255, 255, 255, 255 );

	static std::vector<TileDefinition> s_definitions;
};