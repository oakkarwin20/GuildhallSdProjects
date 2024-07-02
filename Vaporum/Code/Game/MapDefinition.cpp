#include "Game/MapDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
std::vector<MapDefinition*> MapDefinition::s_mapDefinitionsList;

//----------------------------------------------------------------------------------------------------------------------
MapDefinition::MapDefinition( XmlElement const& mapDefElement )
{
	m_name                          = ParseXmlAttribute( mapDefElement,              "name", "UNKNOWN NAME"             );
	std::string shaderName          = ParseXmlAttribute( mapDefElement,     "overlayShader", "INVALID overlayShader"    );
    m_overlayShader                 = g_theRenderer->CreateOrGetShaderByName( shaderName.c_str(), VertexType::VERTEX_PCUTBN );
    m_gridSize                      = ParseXmlAttribute( mapDefElement,          "gridSize", IntVec2::ZERO              );
    m_worldBoundsMin                = ParseXmlAttribute( mapDefElement,    "worldBoundsMin", Vec3::ZERO                 );
    m_worldBoundsMax                = ParseXmlAttribute( mapDefElement,    "worldBoundsMax", Vec3::ZERO                 );
    XmlElement const* cDataElement  = mapDefElement.FirstChildElement( "Tiles" );
    if ( cDataElement != nullptr )
    {
        XmlNode const* node = cDataElement->FirstChild();
        if ( node != nullptr )
        {
           m_tileCharacterData = node->Value();
        }
    }
    XmlElement const* unitElement = mapDefElement.FirstChildElement( "Units" );
	while ( unitElement != nullptr )
	{
		XmlNode const* node = unitElement->FirstChild();
		int playerNum       = ParseXmlAttribute( *unitElement, "player", 0 );
		if ( node != nullptr )
		{
            if ( playerNum == 1 )
            {
                m_player1Data = node->Value();
            }
            if ( playerNum == 2 )
            {
                m_player2Data = node->Value();
            }
		}
        unitElement = unitElement->NextSiblingElement();
	}
}

//----------------------------------------------------------------------------------------------------------------------
MapDefinition::~MapDefinition()
{
    delete m_overlayShader;
    m_overlayShader = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void MapDefinition::InitializeMapDef( char const* filepath )
{
    XmlDocument mapDefs;

    XmlResult result = mapDefs.LoadFile( filepath );
    GUARANTEE_OR_DIE( result == tinyxml2::XML_SUCCESS, Stringf( "Failed to open required map defs file %s", filepath ) );

    XmlElement* rootElement = mapDefs.RootElement();
    GUARANTEE_OR_DIE( rootElement, "is invalid" );

    XmlElement* mapDefElement = rootElement->FirstChildElement();
    while ( mapDefElement )
    {
        MapDefinition* newMapDef = new MapDefinition( *mapDefElement );
	
        // Get subChild element of firstChild
        XmlElement* childSpawnInfoElement = mapDefElement->FirstChildElement( "SpawnInfos" );
		if ( childSpawnInfoElement != nullptr )
		{
            XmlElement* subChildSpawnInfoElement = childSpawnInfoElement->FirstChildElement( "SpawnInfo" );
            while ( subChildSpawnInfoElement != nullptr )
            {
                subChildSpawnInfoElement = subChildSpawnInfoElement->NextSiblingElement();
            }
        }

        s_mapDefinitionsList.push_back( newMapDef ); 
        mapDefElement = mapDefElement->NextSiblingElement();
    }
}

//----------------------------------------------------------------------------------------------------------------------
MapDefinition const* MapDefinition::GetMapDefByName( std::string const& name )
{
    for ( int i = 0; i < s_mapDefinitionsList.size(); i++ )
    {
        if ( s_mapDefinitionsList[i]->m_name == name )
        {
            return s_mapDefinitionsList[i];
        }
    }
    return nullptr;
}