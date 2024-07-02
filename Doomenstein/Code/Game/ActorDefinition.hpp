#pragma once

#include "Game/SpriteAnimationGroupDefinition.hpp"

#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Renderer/Camera.hpp"

#include <string>
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class ActorDefinition
{
public:
	ActorDefinition( XmlElement const& actorDefElement );
	~ActorDefinition();

	static void InitializeActorDef( char const* filepath );
	static ActorDefinition const* GetActorDefByName( std::string const& name );
	static std::vector<ActorDefinition*> s_actorDefinitionsList;
	
public:
	//----------------------------------------------------------------------------------------------------------------------
	// ActorDefinitions
	bool			m_visible				= false;
	float			m_health				= 1.0f;
	float			m_corpseLifetime		= 0.0f;
	std::string		m_faction				= "NEUTRAL";
	bool			m_renderForward			= false;
	Rgba8			m_solidColor			= Rgba8(  255,  255,  255 );
	Rgba8			m_wireframeColor		= Rgba8( 192, 192, 192 );
	bool			m_canBePossessed		= false;
	float			m_physicsRadius			= 0.0f;
	float			m_explosionRadius		= 0.0f;
	float			m_physicsHeight			= 0.0f;
	bool			m_collidesWithWorld		= false;
	bool			m_collidesWithActors	= false;
	bool			m_dieOnCollide			= false;
	FloatRange		m_damageOnCollide		= FloatRange( 0.0f, 0.0f );
	float			m_impulseOnCollide		= 0.0f;
	bool			m_simulated				= false;
	bool			m_flying				= false;
	float			m_walkSpeed				= 0.0f;
	float			m_runSpeed				= 0.0f;
	float			m_drag					= 0.0f;
	float			m_turnSpeed				= 0.0f;
	float			m_eyeHeight				= 0.0f;
	float			m_cameraFOVDegrees		= 60.0f;
	bool			m_aiEnabled				= false;
	float			m_sightRadius			= 0.0f;
	float			m_sightAngle			= 0.0f;
	std::string		m_name					= "UNINTIALIZED_ACTOR";
	bool			m_dieOnSpawn			= false;
	
	// Visuals variables
	Vec2			m_size					= Vec2::ZERO;
	Vec2			m_pivot					= Vec2::ZERO;
	std::string		m_billboardType			= "InvalidBillboardType";
	bool			m_renderLit				= false;
	bool			m_renderRounded			= false;
	std::string		m_shaderName			= "InvalidShader";
	Shader*			m_shader				= nullptr;
	std::string		m_spriteSheetName		= "invalidSpriteSheet";
//	SpriteSheet		m_spriteSheet			= SpriteSheet();
	SpriteSheet*	m_spriteSheet			= nullptr;
	Vec2			m_cellCount				= Vec2::ZERO;

	// Animation variables
	std::vector<SpriteAnimationGroupDefinition*>	m_spriteAnimGroupDefList;	
	Texture* m_texture = nullptr;

	// Weapon variables
	std::vector<std::string>						m_weaponNameList;

	// Mouse Missile
	bool m_slidesAlongWalls			= false;
	bool m_isProjectileActor		= false;

	// Ranged Demon
	float m_chasePlayerRange = -1.0f;
};