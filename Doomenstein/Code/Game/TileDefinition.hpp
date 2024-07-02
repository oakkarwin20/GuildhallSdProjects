#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/XmlUtils.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class TileDefinition
{
public:
	TileDefinition();
	TileDefinition( XmlElement const& tileDefElement );
	~TileDefinition();

	static void InitializeTileDef( char const* path );
	static TileDefinition const* GetTileDefByColor( Rgba8 const& color  );	
	static std::vector<TileDefinition*> s_tileDefinitions;

public:
	std::string		m_name					= "UNNAMED TILE TYPE";
	bool			m_isSolid				= false;
	Rgba8			m_mapImagePixelColor	= Rgba8::WHITE;
	IntVec2			m_floorSpriteCoords		= IntVec2( -1, -1 );
	IntVec2			m_ceilingSpriteCoords	= IntVec2( -1, -1 );
	IntVec2			m_wallSpriteCoords		= IntVec2( -1, -1 );
};