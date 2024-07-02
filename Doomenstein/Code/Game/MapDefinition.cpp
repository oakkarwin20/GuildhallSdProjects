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
	m_name                                  = ParseXmlAttribute( mapDefElement,                 "name", m_name                      );
	std::string imageFilePath               = ParseXmlAttribute( mapDefElement,                "image", "invalid image file path"   );
    m_image                                 = new Image( imageFilePath.c_str()                                                      );
	std::string spriteSheetTextureFilePath  = ParseXmlAttribute( mapDefElement,   "spriteSheetTexture", "invalid Texture path"      );
    m_spriteSheetTexture                    = g_theRenderer->CreateOrGetTextureFromFile( spriteSheetTextureFilePath.c_str()         );
	m_spriteSheetCellCount	                = ParseXmlAttribute( mapDefElement, "spriteSheetCellCount", m_spriteSheetCellCount      );
	std::string shaderName                  = ParseXmlAttribute( mapDefElement,               "shader", "UnNamed Shader"    );
    m_shader                                = g_theRenderer->CreateOrGetShaderByName( shaderName.c_str() );
}

//----------------------------------------------------------------------------------------------------------------------
MapDefinition::~MapDefinition()
{
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
                SpawnInfo currentSpawnInfo;
                currentSpawnInfo.m_actorName         = ParseXmlAttribute( *subChildSpawnInfoElement,       "actor", "INVALID ACTOR NAME"      );
                currentSpawnInfo.m_actorPosition     = ParseXmlAttribute( *subChildSpawnInfoElement,    "position", Vec3(-1.0f, -1.0f, -1.0f) );
                currentSpawnInfo.m_actorOrientation  = ParseXmlAttribute( *subChildSpawnInfoElement, "orientation", EulerAngles()             );
                
                newMapDef->m_spawnInfoList.push_back( currentSpawnInfo );
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