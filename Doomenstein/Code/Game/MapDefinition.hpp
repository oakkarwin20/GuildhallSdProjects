#pragma once

#include "Game/SpawnInfo.hpp"

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
	std::string		m_name;
	Image*			m_image					= nullptr;
	Shader*			m_shader				= nullptr;
	Texture*		m_spriteSheetTexture	= nullptr;
	IntVec2			m_spriteSheetCellCount  = IntVec2( -1, -1 );

	std::vector<SpawnInfo> m_spawnInfoList;

//	std::string		m_actor					= "UNINITIALIZED_ACTOR";
//	Vec3			m_position				= Vec3( 0.0f, 0.0f, 0.0f);
//	EulerAngles		m_orientation			= EulerAngles( 0.0f, 0.0f, 0.0f );
//	Vec3			m_velocity				= Vec3( 0.0f, 0.0f, 0.0f );
};