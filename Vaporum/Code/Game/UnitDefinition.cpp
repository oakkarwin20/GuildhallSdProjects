#include "Game/UnitDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Model.hpp"

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
std::vector<UnitDefinition*> UnitDefinition::s_UnitDefinitionsList;

//----------------------------------------------------------------------------------------------------------------------
UnitDefinition::UnitDefinition( XmlElement const& mapDefElement )
{
    m_model                 = new Model();
	m_symbol				= ParseXmlAttribute( mapDefElement,               "symbol", "UNKNOWN _symbol"        );
	m_name					= ParseXmlAttribute( mapDefElement,                 "name", "UNKNOWN _name"          );
    m_modelFileName			= ParseXmlAttribute( mapDefElement,        "modelFilename", "UNKNOWN _modelFilename" );
	m_model->ParseXmlData( m_modelFileName );
    m_type					= ParseXmlAttribute( mapDefElement,                 "type", "UNKNOWN _type"          );
    m_groundAttackDamage	= ParseXmlAttribute( mapDefElement,   "groundAttackDamage", 0                        );
    m_groundAttackRangeMin	= ParseXmlAttribute( mapDefElement, "groundAttackRangeMin", 0                        );
    m_groundAttackRangeMax	= ParseXmlAttribute( mapDefElement, "groundAttackRangeMax", 0                        );
    m_movementRange			= ParseXmlAttribute( mapDefElement,        "movementRange", 0                        );
    m_defense				= ParseXmlAttribute( mapDefElement,              "defense", 0                        );
    m_health				= ParseXmlAttribute( mapDefElement,               "health", 0                        );
}

//----------------------------------------------------------------------------------------------------------------------
UnitDefinition::~UnitDefinition()
{
    delete m_model;
    m_model = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void UnitDefinition::InitializeUnitDef( char const* filepath )
{
    XmlDocument unitDefs;

    XmlResult result = unitDefs.LoadFile( filepath );
    GUARANTEE_OR_DIE( result == tinyxml2::XML_SUCCESS, Stringf( "Failed to open required unit defs file %s", filepath ) );

    XmlElement* rootElement = unitDefs.RootElement();
    GUARANTEE_OR_DIE( rootElement, "is invalid" );

    XmlElement* unitDefElement = rootElement->FirstChildElement();
    while ( unitDefElement )
    {
        UnitDefinition* newUnitDef = new UnitDefinition( *unitDefElement );
	
        // Get subChild element of firstChild
        XmlElement* childSpawnInfoElement = unitDefElement->FirstChildElement( "SpawnInfos" );
		if ( childSpawnInfoElement != nullptr )
		{
            XmlElement* subChildSpawnInfoElement = childSpawnInfoElement->FirstChildElement( "SpawnInfo" );
            while ( subChildSpawnInfoElement != nullptr )
            {
                subChildSpawnInfoElement = subChildSpawnInfoElement->NextSiblingElement();
            }
        }

        s_UnitDefinitionsList.push_back( newUnitDef ); 
        unitDefElement = unitDefElement->NextSiblingElement();
    }
}

//----------------------------------------------------------------------------------------------------------------------
UnitDefinition const* UnitDefinition::GetUnitDefBySymbol( char name )
{
    for ( int i = 0; i < s_UnitDefinitionsList.size(); i++ )
    {
        if ( s_UnitDefinitionsList[i]->m_symbol[0] == name )
        {
            return s_UnitDefinitionsList[i];
        }
    }
    return nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
void UnitDefinition::DeleteDefinitions()
{
	for ( int i = 0; i < s_UnitDefinitionsList.size(); i++ )
	{
        delete s_UnitDefinitionsList[i];
	}
}
