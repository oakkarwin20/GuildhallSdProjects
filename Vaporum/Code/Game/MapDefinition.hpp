#pragma once

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

#include <string>
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class MapDefinition
{
public:
	MapDefinition( XmlElement const& mapDefElement );
	~MapDefinition();

	static void InitializeMapDef( char const* path );
	static MapDefinition const* GetMapDefByName( std::string const& name );
	static std::vector<MapDefinition*> s_mapDefinitionsList;

public:
	std::string		m_name				= "UN-NAMED GRID";
	Shader*			m_overlayShader		= nullptr;
	IntVec2			m_gridSize			= IntVec2::ZERO;
	Vec3			m_worldBoundsMin	= Vec3::ZERO;
	Vec3			m_worldBoundsMax	= Vec3::ZERO;
	std::string		m_tileCharacterData	= "UN-DEFINED CDATA";
	std::string		m_player1Data		= "UN-DEFINED PLAYER 1 CDATA";
	std::string		m_player2Data		= "UN-DEFINED PLAYER 2 CDATA";
};