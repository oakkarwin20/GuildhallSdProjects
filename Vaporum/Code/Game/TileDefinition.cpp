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
    m_symbol		= ParseXmlAttribute( tileDefElement,    "symbol", m_symbol	    );
    m_name          = ParseXmlAttribute( tileDefElement,      "name", m_name        );
    m_isBlocked     = ParseXmlAttribute( tileDefElement, "isBlocked", m_isBlocked   );
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
TileDefinition const* TileDefinition::GetTileDefBySymbol( char name )
{
	for ( int i = 0; i < s_tileDefinitions.size(); i++ )
	{
		if ( s_tileDefinitions[i]->m_symbol[0] == name )
		{
			return s_tileDefinitions[i];
		}
	}
	return nullptr;
}
