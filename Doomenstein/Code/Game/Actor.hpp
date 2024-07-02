#pragma once

#include "Game/ActorUID.hpp"
#include "Game/Weapon.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/SpriteAnimationGroupDefinition.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class Map;
class SpawnInfo;
class ActorDefinition;
class Controller;
class AIController;
class Stopwatch;

//----------------------------------------------------------------------------------------------------------------------
class Actor
{
public:
	Actor( Map* currentMap, SpawnInfo const& currentSpawnInfo );
	Actor( Vec3 position, EulerAngles orientation, Rgba8 color, float radius, float height, bool isMovable );
	~Actor();

	void Update( float deltaSeconds );
	void Render();
	Mat44 GetModelMatrix( Vec3 position, EulerAngles orientation ) const;

	void UpdatePhysics( float deltaSeconds );
	void TakeDamage( float damageAmount, Actor* actorWhoDamagedMe );
	void AddForce( Vec3 forceAmount );
	void AddImpulse( Vec3 const& impulseForce );
	void OnCollide( Actor* actorToCollide );
//	void OnPossessed();
//	void OnUnpossessed();
	void MoveInDirection( Vec3 directionToMove, float speed );
	void TurnInDirection( Vec3 directionToMove, float deltaDegrees );
	void Attack();
	void EquipWeapon();
	void NextWeapon();
	void PreviousWeapon();

	void AIPossessCurrentActor( ActorUID actorUID );

	void PlayAnimationsByName( std::string name );
	bool IsAnimDonePlaying();
	SpriteDefinition GetCurrentSpriteDef();

	// Mouse Missile Functions
	void UpdateExplosiveProjectileActors();
	void ExplodeMissileMice();

	// Demon Poop Functions
	void ExplodeDemonPoop();
public:
	// Actor Variables
	Vec3							m_position				= Vec3::ZERO;
	EulerAngles						m_orientation;
	Rgba8							m_solidColor			= Rgba8::WHITE;
	Rgba8							m_wireframeColor;
	float							m_physicsRadius			= -1.0f;
	float							m_physicsHeight			= -1.0f;
	bool							m_isMovable				= false;
	std::vector<Vertex_PCU>			m_vertexListPCU;
	std::vector<Vertex_PCUTBN>		m_vertexListPNCU;

	ActorUID						m_currentActorUID;
	ActorDefinition const* 			m_actorDef				= nullptr;
	Map*							m_currentMap			= nullptr;
	Vec3							m_velocity				= Vec3( 0.0f, 0.0f, 0.0f );
	Vec3							m_acceleration			= Vec3( 0.0f, 0.0f, 0.0f );
	
	Actor const*					m_owner					= nullptr;
	bool							m_isDead				= false;
	bool							m_isGarbage				= false;
	float							m_currentHealth			= 0.0f;
	bool							m_isPossessed			= false;
	Controller*						m_currentController		= nullptr;
	AIController*					m_AIController			= nullptr;

	Vec3 m_forward	= Vec3::NEGATIVE_ONE;
	Vec3 m_left		= Vec3::NEGATIVE_ONE;
	Vec3 m_up		= Vec3::NEGATIVE_ONE;

	// Weapon variables
	std::vector<Weapon*>	m_weaponList;
	unsigned int			m_currentWeaponIndex	= 0;
	Weapon*					m_currentWeapon			= nullptr;

	// Stopwatch variables
	Stopwatch	m_isGarbageStopwatch			= Stopwatch();
	bool		m_shouldGarbageStopwatchStart	= false;

	// Animation variables
	std::string							m_currentAnimGroupName		= "Walk";
	SpriteAnimationGroupDefinition*		m_currentAnimGroup			= nullptr;
	SpriteAnimDefinition*				m_currentSpriteAnimDef		= nullptr;
	Clock*								m_animClock					= nullptr;
	bool								m_animIsDonePlaying			= true;
	float								m_animStartTime				= 1.0f;

	// Vertigo effect variables
	bool		m_isOnDrugs			= false;
	Stopwatch	m_drugsStopwatch	= Stopwatch();
	float		m_durationOnDrugs   = 5.0f;
	
	// Audio Variables
	SoundPlaybackID m_missileMiceExplodeSPBID	= SoundPlaybackID( -1 );
	SoundPlaybackID m_demonPoopExplodeSPBID		= SoundPlaybackID( -1 );
	SoundPlaybackID m_demonAttackSPBID			= SoundPlaybackID( -1 );
	SoundPlaybackID m_demonHurtSPBID			= SoundPlaybackID( -1 );
	SoundPlaybackID m_demonDeathSPBID			= SoundPlaybackID( -1 );
	SoundPlaybackID m_marineHurtSPBID			= SoundPlaybackID( -1 );
	SoundPlaybackID m_marineDeathSPBID			= SoundPlaybackID( -1 );

	SoundID			m_marineDeathSID			= SoundID( -1 );
	SoundID			m_marineHurtSID				= SoundID( -1 );
	SoundID			m_missileMiceExplodeSID		= SoundID( -1 );
	SoundID			m_demonPoopExplodeSID		= SoundID( -1 );
	SoundID			m_demonAttackSID			= SoundID( -1 );
	SoundID			m_demonHurtSID				= SoundID( -1 );
	SoundID			m_demonDeathSID				= SoundID( -1 );
};