#include "Game/TileDefinition.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
std::vector<TileDefinition*> TileDefinition::s_tileDefinitions;

//----------------------------------------------------------------------------------------------------------------------
TileDefinition::TileDefinition()
{
}

//----------------------------------------------------------------------------------------------------------------------
TileDefinition::TileDefinition( XmlElement const& tileDefElement )
{
    m_name                = ParseXmlAttribute( tileDefElement,                 "name", m_name                );
    m_isSolid             = ParseXmlAttribute( tileDefElement,              "isSolid", m_isSolid             );
    m_mapImagePixelColor  = ParseXmlAttribute( tileDefElement,   "mapImagePixelColor", m_mapImagePixelColor	 );
    m_floorSpriteCoords   = ParseXmlAttribute( tileDefElement,    "floorSpriteCoords", m_floorSpriteCoords   );
    m_ceilingSpriteCoords = ParseXmlAttribute( tileDefElement,  "ceilingSpriteCoords", m_ceilingSpriteCoords );
    m_wallSpriteCoords    = ParseXmlAttribute( tileDefElement,     "wallSpriteCoords", m_wallSpriteCoords    );
}

//----------------------------------------------------------------------------------------------------------------------
TileDefinition::~TileDefinition()
{
}

//----------------------------------------------------------------------------------------------------------------------
void TileDefinition::InitializeTileDef( char const* filepath )
{
    XmlDocument tileDefs;

    XmlResult result = tileDefs.LoadFile( filepath );
    GUARANTEE_OR_DIE( result == tinyxml2::XML_SUCCESS, Stringf( "Failed to open required tile defs file %s", filepath ) );

    XmlElement* rootElement = tileDefs.RootElement();
	GUARANTEE_OR_DIE( rootElement, "is invalid" );

    XmlElement* tileDefElement = rootElement->FirstChildElement();
    while ( tileDefElement )
    {
        std::string elementName = tileDefElement->Name();
        GUARANTEE_OR_DIE( elementName == "TileDefinition", Stringf( "Root child element in %s was <%s>, must be <TileDefinition>", filepath, elementName.c_str() ) );
        TileDefinition* newTileDef = new TileDefinition( *tileDefElement );
        s_tileDefinitions.push_back( newTileDef ); 
        tileDefElement = tileDefElement->NextSiblingElement();
    }
}

//----------------------------------------------------------------------------------------------------------------------
TileDefinition const* TileDefinition::GetTileDefByColor( Rgba8 const& color )
{
	for ( int i = 0; i < s_tileDefinitions.size(); i++ )
	{
		if ( s_tileDefinitions[i]->m_mapImagePixelColor == color )
		{
			return s_tileDefinitions[i];
		}
	}
	return nullptr;
}
