#include "Game/WeaponDefinition.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
std::vector<WeaponDefinition*> WeaponDefinition::s_weaponDefinitionsList;

//----------------------------------------------------------------------------------------------------------------------
WeaponDefinition::WeaponDefinition( XmlElement const& weaponDefElement )
{
	// Core attributes
	m_name				= ParseXmlAttribute( weaponDefElement, "name",				m_name			  );
	m_refireTime		= ParseXmlAttribute( weaponDefElement, "refireTime",		m_refireTime	  );
	
	// Raycast aka Pistol attributes 
	m_rayCount			= ParseXmlAttribute( weaponDefElement, "rayCount",			m_rayCount		  );
	m_rayCone			= ParseXmlAttribute( weaponDefElement, "rayCone",			m_rayCone		  );
	m_rayRange			= ParseXmlAttribute( weaponDefElement, "rayRange",			m_rayRange		  );
	m_rayDamage			= ParseXmlAttribute( weaponDefElement, "rayDamage",			m_rayDamage		  );
	m_rayImpulse		= ParseXmlAttribute( weaponDefElement, "rayImpulse",		m_rayImpulse	  );
	
	// Projectile actors aka Plasma Rifle attributes 
	m_projectileCount		= ParseXmlAttribute( weaponDefElement,		 "projectileCount",	m_projectileCount		);
	m_projectileCone		= ParseXmlAttribute( weaponDefElement,		  "projectileCone",	m_projectileCone		);
	m_projectileSpeed		= ParseXmlAttribute( weaponDefElement,		 "projectileSpeed",	m_projectileSpeed		);
	m_projectileAttackRange	= ParseXmlAttribute( weaponDefElement, "projectileAttackRange",	m_projectileAttackRange	);
	m_projectileActor		= ParseXmlAttribute( weaponDefElement,		 "projectileActor",	m_projectileActor		);

	// Demon melee attributes
	m_meleeCount		= ParseXmlAttribute( weaponDefElement, "meleeCount",		m_meleeCount	  );
	m_meleeRange		= ParseXmlAttribute( weaponDefElement, "meleeRange",		m_meleeRange	  );
	m_meleeArc			= ParseXmlAttribute( weaponDefElement, "meleeArc",			m_meleeArc		  );
	m_meleeDamage		= ParseXmlAttribute( weaponDefElement, "meleeDamage",		m_meleeDamage	  );
 	m_meleeImpulse		= ParseXmlAttribute( weaponDefElement, "meleeImpulse",		m_meleeImpulse	  );
}

//----------------------------------------------------------------------------------------------------------------------
WeaponDefinition::~WeaponDefinition()
{
}

//----------------------------------------------------------------------------------------------------------------------
void WeaponDefinition::InitializeWeaponDef( char const* filepath )
{
	XmlDocument weaponDefs;

	XmlResult result = weaponDefs.LoadFile( filepath );
	GUARANTEE_OR_DIE( result == tinyxml2::XML_SUCCESS, Stringf( "Failed to open required weapon defs file %s", filepath ) );

	XmlElement* rootElement = weaponDefs.RootElement();
	GUARANTEE_OR_DIE( rootElement, "is invalid" );

	XmlElement* weaponDefElement = rootElement->FirstChildElement();
	while ( weaponDefElement )
	{
		std::string elementName = weaponDefElement->Name();
//      GUARANTEE_OR_DIE( elementName == "MapDefinition", Stringf( "Root child element in %s was <%s>, must be <MapDefinition>", filepath, elementName.c_str() ) );
		WeaponDefinition* newWeaponDef = new WeaponDefinition( *weaponDefElement );

		XmlElement* childHudElement = weaponDefElement->FirstChildElement( "HUD" );
		if ( childHudElement != nullptr )
		{
			// shader
			newWeaponDef->m_shaderName	= ParseXmlAttribute( *childHudElement, "shader", newWeaponDef->m_shaderName );
			newWeaponDef->m_shader		= g_theRenderer->CreateOrGetShaderByName( newWeaponDef->m_shaderName.c_str() );

			// baseTexture
			newWeaponDef->m_baseTextureName = ParseXmlAttribute( *childHudElement, "baseTexture", newWeaponDef->m_baseTextureName );
			newWeaponDef->m_baseTexture		= g_theRenderer->CreateOrGetTextureFromFile( newWeaponDef->m_baseTextureName.c_str() );

			// reticleTexture
			newWeaponDef->m_reticleTextureName	= ParseXmlAttribute( *childHudElement, "reticleTexture", newWeaponDef->m_reticleTextureName );
			newWeaponDef->m_reticleTexture		= g_theRenderer->CreateTextureFromFile( newWeaponDef->m_reticleTextureName.c_str() );

			// ReticleSize
			newWeaponDef->m_reticleSize	= ParseXmlAttribute( *childHudElement, "reticleSize", newWeaponDef->m_reticleSize );
			
			// spriteSize
			newWeaponDef->m_spriteSize = ParseXmlAttribute( *childHudElement, "spriteSize", newWeaponDef->m_spriteSize );
			
			// spritePivot
			newWeaponDef->m_spritePivot = ParseXmlAttribute( *childHudElement, "spritePivot", newWeaponDef->m_spritePivot );
			
			// Parse HUD attributes
			XmlElement* childAnimGroupElement = childHudElement->FirstChildElement( "Animation" );
			while ( childAnimGroupElement != nullptr )
			{
				// Base data
				WeaponAnimGroupDef weaponGroupDef;
 				weaponGroupDef.m_name			= ParseXmlAttribute( *childAnimGroupElement, "name", weaponGroupDef.m_name );
				
				// Shaders data
				weaponGroupDef.m_shaderName		= ParseXmlAttribute( *childAnimGroupElement, "shader", weaponGroupDef.m_shaderName );
				weaponGroupDef.m_shader			= g_theRenderer->CreateOrGetShaderByName( weaponGroupDef.m_shaderName.c_str() );

				// Textures data
				weaponGroupDef.m_textureName	= ParseXmlAttribute( *childAnimGroupElement, "spriteSheet", weaponGroupDef.m_textureName );
				weaponGroupDef.m_texture		= g_theRenderer->CreateOrGetTextureFromFile( weaponGroupDef.m_textureName.c_str() );

				// SpriteSheet and cellCount
				weaponGroupDef.m_cellCount		= ParseXmlAttribute( *childAnimGroupElement, "cellCount", weaponGroupDef.m_cellCount );
				SpriteSheet* spriteSheet		= new SpriteSheet( *weaponGroupDef.m_texture, weaponGroupDef.m_cellCount );
				newWeaponDef->m_spriteSheet		= spriteSheet;

				// Create spriteAnimDef
				weaponGroupDef.m_secondsPerFrame	= ParseXmlAttribute( *childAnimGroupElement, "secondsPerFrame", weaponGroupDef.m_secondsPerFrame );
				weaponGroupDef.m_framesPerSecond	= 1 / weaponGroupDef.m_secondsPerFrame;
				weaponGroupDef.m_startFrame			= ParseXmlAttribute( *childAnimGroupElement, "startFrame", -2	);
				weaponGroupDef.m_endFrame			= ParseXmlAttribute( *childAnimGroupElement,   "endFrame", -2	);
				weaponGroupDef.m_spriteAnimDef		= new SpriteAnimDefinition( *newWeaponDef->m_spriteSheet, weaponGroupDef.m_startFrame, weaponGroupDef.m_endFrame, weaponGroupDef.m_framesPerSecond, SpriteAnimPlaybackType::ONCE );
							
				// add WeaponGroupDef to WeaponGroupDefList
				newWeaponDef->m_weaponAnimGroupDefList.push_back( weaponGroupDef );

				// Move to next sibling element
				childAnimGroupElement = childAnimGroupElement->NextSiblingElement();
			}
		}
		XmlElement* childSoundsElement = weaponDefElement->FirstChildElement( "Sounds" );
		if ( childSoundsElement != nullptr )
		{
			XmlElement* subChildSoundsElement	= childSoundsElement->FirstChildElement( "Sound" );
			while ( subChildSoundsElement )
			{
				newWeaponDef->m_sound			= ParseXmlAttribute( *subChildSoundsElement, "sound", newWeaponDef->m_sound	  );
				newWeaponDef->m_soundName		= ParseXmlAttribute( *subChildSoundsElement,  "name", newWeaponDef->m_soundName  );
				subChildSoundsElement			= subChildSoundsElement->NextSiblingElement();
			}
		}

		// Add parse newWeaponDef data into list of newWeaponDefs
		s_weaponDefinitionsList.push_back( newWeaponDef );
		weaponDefElement = weaponDefElement->NextSiblingElement();
	}
}

//----------------------------------------------------------------------------------------------------------------------
WeaponDefinition const* WeaponDefinition::GetWeaponDefByName( std::string const& name )
{
	// Loop through list of weaponDef and find name which matches the xml element[i]'s 
	for ( int i = 0; i < s_weaponDefinitionsList.size(); i++ )
	{
		if ( s_weaponDefinitionsList[i]->m_name == name )
		{
			return s_weaponDefinitionsList[i];
		}
	}
	return nullptr;
}
