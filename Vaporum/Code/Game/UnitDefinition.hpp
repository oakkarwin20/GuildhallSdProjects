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
class Model;


//----------------------------------------------------------------------------------------------------------------------
class UnitDefinition
{
public:
	UnitDefinition( XmlElement const& mapDefElement );
	~UnitDefinition();

	static void InitializeUnitDef( char const* path );
	static UnitDefinition const* GetUnitDefBySymbol( char name );
	static std::vector<UnitDefinition*> s_UnitDefinitionsList;
	static void DeleteDefinitions();

public:
	Model*			m_model					= nullptr;
	std::string		m_symbol				= "UN-NAMED symbol";
	std::string		m_name					= "UN-NAMED name";
	std::string		m_modelFileName			= "UN-NAMED modelFilename";
	std::string		m_type					= "UN-NAMED type";
	int				m_groundAttackDamage	= 0;
	int				m_groundAttackRangeMin	= 0;
	int				m_groundAttackRangeMax	= 0;
	int				m_movementRange			= 0;
	int				m_defense				= 0;
	int				m_health				= 0;
};