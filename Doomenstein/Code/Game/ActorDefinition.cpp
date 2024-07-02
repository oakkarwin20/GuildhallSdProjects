#include "Game/ActorDefinition.hpp"
#include "Game/GameCommon.hpp"	

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"

//----------------------------------------------------------------------------------------------------------------------
std::vector<ActorDefinition*> ActorDefinition::s_actorDefinitionsList;

//----------------------------------------------------------------------------------------------------------------------
ActorDefinition::ActorDefinition( XmlElement const& actorDefElement )
{
	//----------------------------------------------------------------------------------------------------------------------
	// ProjectileActorDefinitions

	//----------------------------------------------------------------------------------------------------------------------
	// ActorDefinitions 
	m_name					= ParseXmlAttribute( actorDefElement, "name"				,	m_name				 );
	m_visible				= ParseXmlAttribute( actorDefElement, "visible"				,	m_visible			 );
	m_health				= ParseXmlAttribute( actorDefElement, "health"				,	m_health			 );
	m_corpseLifetime		= ParseXmlAttribute( actorDefElement, "corpseLifetime"		,	m_corpseLifetime	 );
	m_faction				= ParseXmlAttribute( actorDefElement, "faction"				,	m_faction			 );
	m_renderForward			= ParseXmlAttribute( actorDefElement, "renderForward"		,	m_renderForward		 );
	m_solidColor			= ParseXmlAttribute( actorDefElement, "solidColor"			,	m_solidColor		 );
	m_wireframeColor		= ParseXmlAttribute( actorDefElement, "wireframeColor"		,	m_wireframeColor	 );
	m_canBePossessed		= ParseXmlAttribute( actorDefElement, "canBePossessed"		,	m_canBePossessed	 );
//	m_physicsRadius			= ParseXmlAttribute( actorDefElement, "physicsRadius"		,	m_physicsRadius		 );
	m_physicsHeight			= ParseXmlAttribute( actorDefElement, "physicsHeight"		,	m_physicsHeight		 );
	m_collidesWithWorld		= ParseXmlAttribute( actorDefElement, "collidesWithWorld"	,	m_collidesWithWorld	 );
	m_collidesWithActors	= ParseXmlAttribute( actorDefElement, "collidesWithActors"	,	m_collidesWithActors );
	m_dieOnCollide			= ParseXmlAttribute( actorDefElement, "dieOnCollide"		,	m_dieOnCollide		 );
	m_damageOnCollide		= ParseXmlAttribute( actorDefElement, "damageOnCollide"		,	m_damageOnCollide	 );
	m_impulseOnCollide		= ParseXmlAttribute( actorDefElement, "impulseOnCollide"	,	m_impulseOnCollide	 );
	m_simulated				= ParseXmlAttribute( actorDefElement, "simulated"			,	m_simulated			 );
	m_flying				= ParseXmlAttribute( actorDefElement, "flying"				,	m_flying			 );
	m_walkSpeed				= ParseXmlAttribute( actorDefElement, "walkSpeed"			,	m_walkSpeed			 );
	m_runSpeed				= ParseXmlAttribute( actorDefElement, "runSpeed"			,	m_runSpeed			 );
	m_drag					= ParseXmlAttribute( actorDefElement, "drag"				,	m_drag				 );
	m_turnSpeed				= ParseXmlAttribute( actorDefElement, "turnSpeed"			,	m_turnSpeed			 );
	m_eyeHeight				= ParseXmlAttribute( actorDefElement, "eyeHeight"			,	m_eyeHeight			 );
	m_cameraFOVDegrees		= ParseXmlAttribute( actorDefElement, "cameraFOVDegrees"	,	m_cameraFOVDegrees	 );
	m_aiEnabled				= ParseXmlAttribute( actorDefElement, "aiEnabled"			,	m_aiEnabled			 );
	m_sightRadius			= ParseXmlAttribute( actorDefElement, "sightRadius"			,	m_sightRadius		 );
	m_sightAngle			= ParseXmlAttribute( actorDefElement, "sightAngle"			,	m_sightAngle		 );
	m_dieOnSpawn			= ParseXmlAttribute( actorDefElement, "dieOnSpawn"			,	m_dieOnSpawn		 );

	// Missile Mice
	m_slidesAlongWalls		= ParseXmlAttribute( actorDefElement, "slidesAlongWalls"	, m_slidesAlongWalls	  );
	m_isProjectileActor		= ParseXmlAttribute( actorDefElement, "isProjectileActor"	, m_isProjectileActor	  );
	
	// Ranged Demon
	m_chasePlayerRange		= ParseXmlAttribute( actorDefElement, "chasePlayerRange"	, m_chasePlayerRange	  );
}																					

//----------------------------------------------------------------------------------------------------------------------
ActorDefinition::~ActorDefinition()
{
	delete m_spriteSheet;
	m_spriteSheet = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void ActorDefinition::InitializeActorDef( char const* filepath )
{
	// Create instance of an Xml document
	XmlDocument actorDefs;

	// Load the Xml file based on filepath
	XmlResult result = actorDefs.LoadFile( filepath );
	GUARANTEE_OR_DIE( result == tinyxml2::XML_SUCCESS, Stringf( "Failed to open required actor defs file %s", filepath ) );

	// Get the root element
	XmlElement* rootElement = actorDefs.RootElement();
	GUARANTEE_OR_DIE( rootElement, "is invalid" );

	// Get first child element of root element
	XmlElement* actorDefElement = rootElement->FirstChildElement();
	
	// Loop until end of Xml document and there are no more child elements in the root element
	while ( actorDefElement )
	{	
		// Create a new actorDef at current child element
		ActorDefinition* newActorDef = new ActorDefinition( *actorDefElement );

		// Get subChild element of firstChild
		XmlElement* childCollisionElement = actorDefElement->FirstChildElement( "Collision" );
		if ( childCollisionElement != nullptr )
		{
			newActorDef->m_physicsRadius		= ParseXmlAttribute( *childCollisionElement,  		     "radius", newActorDef->m_physicsRadius		 );
			newActorDef->m_explosionRadius		= ParseXmlAttribute( *childCollisionElement,    "explosionRadius", newActorDef->m_explosionRadius	 );
			newActorDef->m_physicsHeight		= ParseXmlAttribute( *childCollisionElement,  			 "height", newActorDef->m_physicsHeight		 );
			newActorDef->m_collidesWithWorld	= ParseXmlAttribute( *childCollisionElement,  "collidesWithWorld", newActorDef->m_collidesWithWorld	 );
			newActorDef->m_collidesWithActors	= ParseXmlAttribute( *childCollisionElement, "collidesWithActors", newActorDef->m_collidesWithActors );
			newActorDef->m_damageOnCollide		= ParseXmlAttribute( *childCollisionElement,	"damageOnCollide", newActorDef->m_damageOnCollide	 );
			newActorDef->m_impulseOnCollide		= ParseXmlAttribute( *childCollisionElement,   "impulseOnCollide", newActorDef->m_impulseOnCollide	 );
			newActorDef->m_dieOnCollide			= ParseXmlAttribute( *childCollisionElement,	   "dieOnCollide", newActorDef->m_dieOnCollide		 );
		}
		XmlElement* childPhysicsElement = actorDefElement->FirstChildElement( "Physics" );
		if ( childPhysicsElement != nullptr )
		{
			newActorDef->m_simulated = ParseXmlAttribute( *childPhysicsElement, "simulated", newActorDef->m_simulated );
			newActorDef->m_walkSpeed = ParseXmlAttribute( *childPhysicsElement, "walkSpeed", newActorDef->m_walkSpeed );
			newActorDef->m_runSpeed	 = ParseXmlAttribute( *childPhysicsElement,  "runSpeed", newActorDef->m_runSpeed  );
			newActorDef->m_turnSpeed = ParseXmlAttribute( *childPhysicsElement, "turnSpeed", newActorDef->m_turnSpeed );
			newActorDef->m_flying	 = ParseXmlAttribute( *childPhysicsElement,	   "flying", newActorDef->m_flying	  );
			newActorDef->m_drag		 = ParseXmlAttribute( *childPhysicsElement,      "drag", newActorDef->m_drag	  );
		}
		XmlElement* childCameraElement = actorDefElement->FirstChildElement( "Camera" );
		if ( childCameraElement != nullptr )
		{
			newActorDef->m_eyeHeight		= ParseXmlAttribute( *childCameraElement, "eyeHeight", newActorDef->m_eyeHeight );
			newActorDef->m_cameraFOVDegrees = ParseXmlAttribute( *childCameraElement, "cameraFOV", newActorDef->m_cameraFOVDegrees );
		}
		XmlElement* childVisualsElement = actorDefElement->FirstChildElement( "Visuals" );
		if ( childVisualsElement != nullptr )
		{
			newActorDef->m_size				= ParseXmlAttribute( *childVisualsElement,		 	 "size", newActorDef->m_size		  );
			newActorDef->m_pivot			= ParseXmlAttribute( *childVisualsElement,			"pivot", newActorDef->m_pivot		  );
			newActorDef->m_billboardType	= ParseXmlAttribute( *childVisualsElement,  "billboardType", newActorDef->m_billboardType );
			newActorDef->m_renderLit		= ParseXmlAttribute( *childVisualsElement,		"renderLit", newActorDef->m_renderLit	  );
			newActorDef->m_renderRounded	= ParseXmlAttribute( *childVisualsElement,	"renderRounded", newActorDef->m_renderRounded );
			newActorDef->m_shaderName		= ParseXmlAttribute( *childVisualsElement,		   "shader", newActorDef->m_shaderName	  );
			newActorDef->m_shader			= g_theRenderer->CreateOrGetShaderByName( newActorDef->m_shaderName.c_str(), VertexType::VERTEX_PCUTBN				  );
			newActorDef->m_spriteSheetName	= ParseXmlAttribute( *childVisualsElement,	  "spriteSheet", newActorDef->m_spriteSheetName	  );
 			newActorDef->m_cellCount		= ParseXmlAttribute( *childVisualsElement,		"cellCount", newActorDef->m_cellCount	  );
			
			XmlElement* childAnimationGroupElement = childVisualsElement->FirstChildElement( "AnimationGroup" );
			while ( childAnimationGroupElement )
			{
				// Create texture and spriteSheet
//				Texture* texture			= g_theRenderer->CreateOrGetTextureFromFile( newActorDef->m_spriteSheet.c_str() );
//				SpriteSheet spriteSheet		= SpriteSheet( *texture, IntVec2( (int)newActorDef->m_cellCount.x, (int)newActorDef->m_cellCount.y) );
				newActorDef->m_texture		= g_theRenderer->CreateOrGetTextureFromFile( newActorDef->m_spriteSheetName.c_str() );
//				SpriteSheet spriteSheet		= SpriteSheet( *newActorDef->m_texture, IntVec2( (int)newActorDef->m_cellCount.x, (int)newActorDef->m_cellCount.y) );
				newActorDef->m_spriteSheet	= new SpriteSheet( *newActorDef->m_texture, IntVec2( (int)newActorDef->m_cellCount.x, (int)newActorDef->m_cellCount.y) );
//				newActorDef->m_spriteSheet	= spriteSheet;
//				newActorDef->m_spriteSheet	= SpriteSheet( *newActorDef->m_texture, IntVec2( (int)newActorDef->m_cellCount.x, (int)newActorDef->m_cellCount.y) );

				// Create spriteAnimGroupDef
				SpriteAnimationGroupDefinition* spriteAnimGroupDef = new SpriteAnimationGroupDefinition( childAnimationGroupElement, *newActorDef->m_spriteSheet );		// pass the xml element of spriteAnimGroup and spriteSheet
//				SpriteAnimationGroupDefinition* spriteAnimGroupDef = new SpriteAnimationGroupDefinition( childAnimationGroupElement, spriteSheet );		// pass the xml element of spriteAnimGroup and spriteSheet
				newActorDef->m_spriteAnimGroupDefList.push_back( spriteAnimGroupDef );
				childAnimationGroupElement = childAnimationGroupElement->NextSiblingElement();
			}
		}
		XmlElement* childInventoryElement = actorDefElement->FirstChildElement( "Inventory" );
		if ( childInventoryElement != nullptr )
		{
			XmlElement* childWeaponElement = childInventoryElement->FirstChildElement( "Weapon" );
			while ( childWeaponElement )
			{
				std::string weaponName = ParseXmlAttribute( *childWeaponElement, "name", "invalid Weapon");
				newActorDef->m_weaponNameList.push_back( weaponName );
				childWeaponElement = childWeaponElement->NextSiblingElement();
			}
		}
//		XmlElement* childAIElement = actorDefElement->FirstChildElement( "AIController" );
		XmlElement* childAIElement = actorDefElement->FirstChildElement( "AI" );
		if ( childAIElement != nullptr )
		{
			newActorDef->m_aiEnabled   = ParseXmlAttribute( *childAIElement,   "aiEnabled", newActorDef->m_aiEnabled   );
			newActorDef->m_sightRadius = ParseXmlAttribute( *childAIElement, "sightRadius", newActorDef->m_sightRadius );
			newActorDef->m_sightAngle  = ParseXmlAttribute( *childAIElement,  "sightAngle", newActorDef->m_sightAngle  );
		}

		// Add created newActorDef to list of actorDefs
		s_actorDefinitionsList.push_back( newActorDef );

		// Move onto the next child aka "sibling" of the root element
		actorDefElement = actorDefElement->NextSiblingElement();
	}
}

//----------------------------------------------------------------------------------------------------------------------
ActorDefinition const* ActorDefinition::GetActorDefByName( std::string const& name )
{
	// Loop through list of actorDef and find matching name to the xml element[i]'s 
	for ( int i = 0; i < s_actorDefinitionsList.size(); i++ )
	{
		if ( s_actorDefinitionsList[i]->m_name == name )
		{
			return s_actorDefinitionsList[i];
		}
	}
	return nullptr;
}