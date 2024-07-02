#pragma once

#include "Game/WeaponDefinition.hpp"
//#include "Game/Map.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Stopwatch.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Actor;
class Map;

//----------------------------------------------------------------------------------------------------------------------
class Weapon
{
public:
	Weapon( WeaponDefinition const* weaponDef );
	~Weapon();

	void Fire( Actor* weaponOwner );
// GetRandomDirectionInCone();

	void Update();
	void Render()	const;
	
	// UI Functions
	void RenderUI() const;
	void RenderHud() const;
	void RenderWeaponUI() const;
	void RenderReticleUI() const;
	void RenderTextUI() const;

	// Anim Functions
	void PlayAnimationsByName( std::string name );
	void IsAnimDonePlaying();
	
public:
	WeaponDefinition const* m_weaponDef;

	Actor* m_actorIHit	= nullptr;

	Vec3  m_weaponOwnerForward	= Vec3( -1.0f, -1.0f, -1.0f );
	Vec3  m_weaponOwnerleft		= Vec3( -1.0f, -1.0f, -1.0f );
	Vec3  m_weaponOwnerup		= Vec3( -1.0f, -1.0f, -1.0f );

	Stopwatch m_weaponRefireStopwatch = Stopwatch();
	bool	  m_canAttack		 = true; 
	bool	  m_startWeaponTimer = true;

	Map*     m_currentMap		= nullptr;
//	Texture* m_weaponTexture	= nullptr;

	// Weapon anim variables
	Clock*				m_weaponAnimClock				= nullptr;
	std::string			m_currentWeaponAnimGroupName	= "Idle";
	WeaponAnimGroupDef  m_currentWeaponGroupDef; 
	bool				m_weaponAnimIsDonePlaying		= false;
	float				m_weaponAnimStartTime			= 1.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Audio variables
	SoundID m_pistolFireSID			= SoundID( -1 );
	SoundID m_plasmaFireSID			= SoundID( -1 );
	SoundID m_plasmaHitSID			= SoundID( -1 );
	SoundID m_demonAttackSID		= SoundID( -1 );
	SoundID m_missileMiceFireSID	= SoundID( -1 );
	SoundID m_demonPoopFireSID		= SoundID( -1 );

	SoundPlaybackID m_pistolFireSPBID			= SoundID( -1 );
	SoundPlaybackID m_plasmaFireSPBID			= SoundID( -1 );
	SoundPlaybackID m_plasmaHitSPBID			= SoundID( -1 );
	SoundPlaybackID m_demonAttackSPBID			= SoundID( -1 );
	SoundPlaybackID m_missileMiceFireSPBID		= SoundID( -1 );
	SoundPlaybackID m_demonPoopFireSPBID		= SoundID( -1 );

	Vec3 m_missileMiceProjectileActorPos	= Vec3::NEGATIVE_ONE;
	Vec3 m_pistolPosition					= Vec3::NEGATIVE_ONE;


};

