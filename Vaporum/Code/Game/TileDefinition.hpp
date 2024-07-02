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
	static TileDefinition const* GetTileDefBySymbol( char name );
	static std::vector<TileDefinition*> s_tileDefinitions;

public:
/*
	std::string		m_symbol				= "UNDEFINED SYMBOL";
	std::string		m_name					= "UNNAMED TILE TYPE";
	bool			m_isBlocked				= true;
*/
	std::string		m_symbol				= "X";
	std::string		m_name					= "Blocked";
	bool			m_isBlocked				= true;

};