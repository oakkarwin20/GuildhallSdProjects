#include "Game/Weapon.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/AIController.hpp"
#include "Game/App.hpp"
#include "Game/WeaponDefinition.hpp"

#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
Weapon::Weapon( WeaponDefinition const* weaponDef )
{
	m_weaponDef			= weaponDef;
	m_weaponAnimClock	= new Clock( g_theApp->m_gameClock );

	// Initialize Audio 
	bool is3D = true;
	m_pistolFireSID				= g_theAudio->CreateOrGetSound( "Data/Audio/PistolFire.wav",		is3D );
	m_plasmaFireSID				= g_theAudio->CreateOrGetSound( "Data/Audio/PlasmaFire.wav",		is3D );
	m_plasmaHitSID				= g_theAudio->CreateOrGetSound( "Data/Audio/PlasmaHit.wav",			is3D );
	m_demonAttackSID			= g_theAudio->CreateOrGetSound( "Data/Audio/DemonAttack.wav",		is3D );
	m_missileMiceFireSID		= g_theAudio->CreateOrGetSound( "Data/Audio/MissileMiceFire.wav",	is3D );
	m_demonPoopFireSID			= g_theAudio->CreateOrGetSound( "Data/Audio/RangedDemonFart.wav",	is3D );
}

//----------------------------------------------------------------------------------------------------------------------
Weapon::~Weapon()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Weapon::Fire( Actor* weaponOwner )
{
	bool is3D = true;
	
	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	// Do plasma logic
	if ( m_weaponDef->m_name == "PlasmaRifle" )
	{		
		// Check if timer has elapsed
		if ( !m_canAttack )
		{
			// Check if weapon is ready to fire	// != null
			if ( m_weaponRefireStopwatch.HasDurationElapsed() )
			{
				// If refire time has elapsed, 
				m_startWeaponTimer	= true;			// Restart timer
				m_canAttack			= true;			// attack is now available	
			}
			else
			{
				return;
			}
		}

		// Play attack animation
		PlayAnimationsByName( "Attack" );

		// Play plasma fire sound
		m_plasmaFireSPBID = g_theAudio->CreateOrGetSound( "Data/Audio/PlasmaFire.wav", is3D );
		m_plasmaFireSPBID = g_theAudio->StartSound( m_plasmaFireSID );

		// Spawn projectile actor in front of possessed actor
		SpawnInfo plasmaSpawnInfo			= SpawnInfo();
		plasmaSpawnInfo.m_actorName			= m_weaponDef->m_projectileActor;

		Vec3 plasmaProjectileStartPos		= Vec3( weaponOwner->m_position );
		weaponOwner->m_currentMap->m_player->m_worldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( m_weaponOwnerForward, m_weaponOwnerleft, m_weaponOwnerup );
		plasmaProjectileStartPos			+= m_weaponOwnerForward * weaponOwner->m_physicsRadius;
		plasmaProjectileStartPos.z			= weaponOwner->m_position.z + ( weaponOwner->m_actorDef->m_eyeHeight * 0.8f );

		plasmaSpawnInfo.m_actorPosition		= plasmaProjectileStartPos;
		plasmaSpawnInfo.m_actorOrientation	= weaponOwner->m_orientation;
		Actor* plasmaActor					= weaponOwner->m_currentMap->SpawnActor( plasmaSpawnInfo );
		plasmaActor->m_owner				= weaponOwner;

		// addForce to move it forward 
 		plasmaActor->m_velocity = m_weaponOwnerForward * m_weaponDef->m_projectileSpeed;
		
		// randomizing projectile trajectory in cone

		// Add impulse 
//		weaponOwner->m_orientation.GetAsVectors_XFwd_YLeft_ZUp( weaponOwner->m_forward, weaponOwner->m_left, weaponOwner->m_up );
//		Vec3 impulseForce = weaponOwner->m_forward.GetNormalized() * weaponOwner->m_currentWeapon->m_weaponDef->m_meleeImpulse;
//		m_actorIHit->AddImpulse( impulseForce );
//		return;

		// Start timer
		if ( m_startWeaponTimer )
		{
			m_weaponRefireStopwatch = Stopwatch( m_weaponAnimClock, m_weaponDef->m_refireTime );
			m_weaponRefireStopwatch.Start();
			m_startWeaponTimer	= false;
			m_canAttack			= false;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_weaponDef->m_name == "Pistol" )
	{
		// Check if timer has elapsed
		if ( !m_canAttack )
		{
			// Check if weapon is ready to fire	// != null
			if ( m_weaponRefireStopwatch.HasDurationElapsed() )
			{
				// If refire time has elapsed, 
				m_startWeaponTimer	= true;			// Restart timer
				m_canAttack			= true;			// attack is now available	
			}
			else
			{
				return;
			}
		}

		// Play attack animation
		PlayAnimationsByName( "Attack" );

		// Play pistol fire sound
		m_pistolFireSID = g_theAudio->CreateOrGetSound( "Data/Audio/PistolFire.wav", is3D );
		m_pistolPosition = weaponOwner->m_position;
		m_pistolFireSPBID = g_theAudio->StartSoundAt( m_pistolFireSID, m_pistolPosition );


		// Shoot Raycast
		weaponOwner->m_currentMap->m_player->m_worldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( m_weaponOwnerForward, m_weaponOwnerleft, m_weaponOwnerup );
		float rayLength = m_weaponDef->m_rayRange ;	
		Vec3  rayStart  = Vec3( weaponOwner->m_position ); 
		rayStart.z		= weaponOwner->m_position.z + weaponOwner->m_actorDef->m_eyeHeight;
		Vec3  rayEnd	= rayStart + ( m_weaponOwnerForward * rayLength );
	
		// Draw debug raycast 
//		float radius	= 0.01f;
//		float duration	= 10.0f;
//		DebugAddWorldArrow( rayStart, rayEnd, radius, duration, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY );

		// Raycast logic
		RaycastResult3D raycastAll = weaponOwner->m_currentMap->RaycastAll( rayStart, m_weaponOwnerForward, rayLength, m_actorIHit, weaponOwner );
		if ( raycastAll.m_didImpact )
		{
			// If an actor exists
			if ( m_actorIHit != nullptr ) // && m_actorIHit->m_currentActorUID.m_saltAndIndexData != weaponOwner->m_currentActorUID.m_saltAndIndexData )
			{
				// Tell actor that got hit to take damage
				float randRayDamage = g_theRNG->RollRandomFloatInRange( weaponOwner->m_currentWeapon->m_weaponDef->m_rayDamage.m_min, weaponOwner->m_currentWeapon->m_weaponDef->m_rayDamage.m_max );
				m_actorIHit->TakeDamage( randRayDamage, weaponOwner );

				// Add impulse 
				weaponOwner->m_orientation.GetAsVectors_XFwd_YLeft_ZUp( weaponOwner->m_forward, weaponOwner->m_left, weaponOwner->m_up );
				Vec3 impulseForce = weaponOwner->m_forward * weaponOwner->m_currentWeapon->m_weaponDef->m_meleeImpulse;
				m_actorIHit->AddImpulse( impulseForce );

				// Play blood splatter animation
				SpawnInfo spawnInfo			 = SpawnInfo();
				spawnInfo.m_actorName		 = "BloodSplatter";
				spawnInfo.m_actorPosition	 = raycastAll.m_impactPos;

				m_currentMap->SpawnActor( spawnInfo );
				m_actorIHit = nullptr;
			}

			// If raycast hit, render ImpactPoint and ImpactNormal
//			Vec3 tinyImpactNormalPos = ( raycastAll.m_impactPos + raycastAll.m_impactNormal * 0.2f );
//			DebugAddWorldPoint( raycastAll.m_impactPos, 0.05f, duration );
//			DebugAddWorldArrow( raycastAll.m_impactPos, tinyImpactNormalPos, 0.02f, duration, Rgba8::BLUE, Rgba8::BLUE );

			// Play bullet hit animation
			SpawnInfo spawnInfo			 = SpawnInfo();
			spawnInfo.m_actorName		 = "BulletHit";
			spawnInfo.m_actorPosition	 = raycastAll.m_impactPos;
			m_currentMap->SpawnActor( spawnInfo );
		}

		// Start timer
		if ( m_startWeaponTimer )
		{
			m_weaponRefireStopwatch = Stopwatch( &g_theApp->m_gameClock, m_weaponDef->m_refireTime );
			m_weaponRefireStopwatch.Start();
			m_startWeaponTimer	= false;
			m_canAttack			= false;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	// Do MissileMice logic
	if ( m_weaponDef->m_name == "MissileMice" )
	{		
		// Check if timer has elapsed
		if ( !m_canAttack )
		{
			// Check if weapon is ready to fire	// != null
			if ( m_weaponRefireStopwatch.HasDurationElapsed() )
			{
				// If refire time has elapsed, 
				m_startWeaponTimer	= true;			// Restart timer
				m_canAttack			= true;			// attack is now available	
			}
			else
			{
				return;
			}
		}

		// Play attack animation
		PlayAnimationsByName( "Attack" );

		// Play MissileMice fire sound
		m_missileMiceFireSPBID = g_theAudio->CreateOrGetSound( "Data/Audio/MissileMiceFire.wav", is3D );
		if ( !g_theAudio->IsPlaying( m_missileMiceFireSPBID ) )
		{
			m_missileMiceFireSPBID = g_theAudio->StartSound( m_missileMiceFireSID );
		}

		// Spawn projectile actor in front of possessed actor
		SpawnInfo missileMiceSpawnInfo			= SpawnInfo();
		missileMiceSpawnInfo.m_actorName		= m_weaponDef->m_projectileActor;

		Vec3 missileMiceStartPos				= Vec3( weaponOwner->m_position );
		m_currentMap->m_player->m_worldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( m_weaponOwnerForward, m_weaponOwnerleft, m_weaponOwnerup );
		missileMiceStartPos						+= m_weaponOwnerForward * weaponOwner->m_physicsRadius;
		missileMiceStartPos.z					= weaponOwner->m_position.z + ( weaponOwner->m_actorDef->m_eyeHeight * 1.15f );

		missileMiceSpawnInfo.m_actorPosition	= missileMiceStartPos;
		missileMiceSpawnInfo.m_actorOrientation	= weaponOwner->m_orientation;
		Actor* missileMiceActor					= m_currentMap->SpawnActor( missileMiceSpawnInfo );
		missileMiceActor->m_owner				= weaponOwner;

		m_missileMiceProjectileActorPos = missileMiceActor->m_position;

		// addForce to move it forward 
		missileMiceActor->m_velocity = m_weaponOwnerForward * m_weaponDef->m_projectileSpeed;

		// Start timer
		if ( m_startWeaponTimer )
		{
			m_weaponRefireStopwatch = Stopwatch( m_weaponAnimClock, m_weaponDef->m_refireTime );
			m_weaponRefireStopwatch.Start();
			m_startWeaponTimer	= false;
			m_canAttack			= false;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	// Do Demon Poop logic
	if ( m_weaponDef->m_name == "DemonPoop" )
	{		
		// Check if timer has elapsed
		if ( !m_canAttack )
		{
			// Check if weapon is ready to fire	// != null
			if ( m_weaponRefireStopwatch.HasDurationElapsed() )
			{
				// If refire time has elapsed, 
				m_startWeaponTimer	= true;			// Restart timer
				m_canAttack			= true;			// attack is now available	
			}
			else
			{
				return;
			}
		}

		// Play attack animation
		PlayAnimationsByName( "Attack" );

		// Play demonPoop fire sound
		m_demonPoopFireSPBID = g_theAudio->StartSound( m_demonPoopFireSID );

		// Spawn projectile actor in front of possessed actor
		SpawnInfo demonPoopSpawnInfo			= SpawnInfo();	
		demonPoopSpawnInfo.m_actorName			= m_weaponDef->m_projectileActor;

		Vec3 demonPoopStartPos					= Vec3( weaponOwner->m_position );
		m_currentMap->m_player->m_worldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( m_weaponOwnerForward, m_weaponOwnerleft, m_weaponOwnerup );
		demonPoopStartPos						+= m_weaponOwnerForward * weaponOwner->m_physicsRadius * -4.0f;
		demonPoopStartPos.z						= 0.0f;

		demonPoopSpawnInfo.m_actorPosition		= demonPoopStartPos;
		demonPoopSpawnInfo.m_actorOrientation	= weaponOwner->m_orientation;
		Actor* demonPoopActor					= m_currentMap->SpawnActor( demonPoopSpawnInfo );
		demonPoopActor->m_owner					= weaponOwner;

		// Start timer
		if ( m_startWeaponTimer )
		{
			m_weaponRefireStopwatch = Stopwatch( m_weaponAnimClock, m_weaponDef->m_refireTime );
			m_weaponRefireStopwatch.Start();
			m_startWeaponTimer	= false;
			m_canAttack			= false;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_weaponDef->m_name == "DemonMelee" )
	{
		// Check if timer has elapsed
		if ( !m_canAttack )
		{
			// Check if weapon is ready to fire	// != null
			if ( m_weaponRefireStopwatch.HasDurationElapsed() )
			{
				// If refire time has elapsed, 
				m_startWeaponTimer = true;			// Restart timer
				m_canAttack = true;			// attack is now available
			}
			else
			{
				return;
			}
		}

		// Check if actorIHit exists
		m_actorIHit = weaponOwner->m_currentMap->GetActorByUID( weaponOwner->m_AIController->m_targetUID ); 
		if ( m_actorIHit == nullptr )
		{			
			m_actorIHit = weaponOwner->m_currentMap->GetActorByUID( weaponOwner->m_AIController->m_targetUID ); 
		}

		if ( m_actorIHit == nullptr )
		{
			return;
		}

		// Check if actorIHit is in range		
		// Demon to player direction
		Vec2 targetPosV2		= Vec2( m_actorIHit->m_position.x, m_actorIHit->m_position.y );
		Vec2 weaponOwnerV2		= Vec2( weaponOwner->m_position.x, weaponOwner->m_position.y);
		Vec2 dirOwnerToTarget	= ( targetPosV2 -weaponOwnerV2 ).GetNormalized();
		float attackRange		= weaponOwner->m_currentWeapon->m_weaponDef->m_meleeRange;
		float aperature			= (float)weaponOwner->m_currentWeapon->m_weaponDef->m_meleeArc;
		if ( !IsPointInsideDirectedSector2D(targetPosV2, weaponOwnerV2, dirOwnerToTarget, aperature, attackRange ) )
		{
			return;
		}

		// Play attack animation
		weaponOwner->PlayAnimationsByName( "Attack" );

		// Play demon melee sound
		m_demonAttackSID = g_theAudio->CreateOrGetSound( "Data/Audio/DemonAttack.wav", is3D );
		if ( !g_theAudio->IsPlaying(m_demonAttackSPBID) )
		{
			m_demonAttackSPBID = g_theAudio->StartSound( m_demonAttackSID );
		}

		// Get random damage value
		float randMeleeDamage = g_theRNG->RollRandomFloatInRange( weaponOwner->m_currentWeapon->m_weaponDef->m_meleeDamage.m_min, weaponOwner->m_currentWeapon->m_weaponDef->m_meleeDamage.m_max );
		m_actorIHit->TakeDamage( randMeleeDamage, weaponOwner );
//		m_actorIHit->TakeDamage( 40, weaponOwner );				// Test code

		// Add impulse 
		weaponOwner->m_orientation.GetAsVectors_XFwd_YLeft_ZUp( weaponOwner->m_forward, weaponOwner->m_left, weaponOwner->m_up );
		Vec3 impulseForce = weaponOwner->m_forward.GetNormalized() * weaponOwner->m_currentWeapon->m_weaponDef->m_meleeImpulse;
		m_actorIHit->AddImpulse( impulseForce );

		// Start timer
		if ( m_startWeaponTimer )
		{
			m_weaponRefireStopwatch = Stopwatch( &g_theApp->m_gameClock, m_weaponDef->m_refireTime );
			m_weaponRefireStopwatch.Start();
			m_startWeaponTimer	= false;
			m_canAttack			= false;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Weapon::Update()
{
	// Check for nullptr
	if ( !m_weaponDef )
	{
		return;
	}

	// Check if anim finished playing
	IsAnimDonePlaying();

	// Update currentWeaponDef
	for ( int i = 0; i < m_weaponDef->m_weaponAnimGroupDefList.size(); i++ )
	{
		if ( m_currentWeaponAnimGroupName == m_weaponDef->m_weaponAnimGroupDefList[i].m_name )
		{
			m_currentWeaponGroupDef = m_weaponDef->m_weaponAnimGroupDefList[i];
			break;
		}
	}

	// Update sound position
	if ( g_theAudio->IsPlaying(m_pistolFireSPBID) )
	{
		m_pistolFireSID = g_theAudio->CreateOrGetSound( "Data/Audio/PistolFire.wav", true );
		g_theAudio->SetSoundPosition( m_pistolFireSPBID, m_pistolPosition );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Weapon::RenderUI() const
{
	if ( m_currentMap->m_player->m_isFreeFlyOn )
	{
		return;
	}

	RenderHud();
	RenderWeaponUI();
	RenderReticleUI();
	RenderTextUI();
}

//----------------------------------------------------------------------------------------------------------------------
void Weapon::RenderHud() const
{
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> hudVerts;

	// Hud bounds
	Vec2 mins		= Vec2::ZERO;
	Vec2 maxs		= Vec2( SCREEN_SIZE_X, SCREEN_SIZE_Y * 0.15 );
	AABB2 hudBounds = AABB2( mins, maxs );
	AddVertsForAABB2D( hudVerts, hudBounds );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw call for Hud 
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( m_weaponDef->m_baseTexture );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader( m_weaponDef->m_shader );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->DrawVertexArray( (int)hudVerts.size(), hudVerts.data() );

	// Unbind texture and shader
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
void Weapon::RenderReticleUI() const
{
	std::vector<Vertex_PCU> reticleVerts;

	//----------------------------------------------------------------------------------------------------------------------
	// Reticle bounds
	Vec2 reticleMins		= Vec2( SCREEN_CENTER_X * 0.98f, SCREEN_CENTER_Y * 0.98f );
	Vec2 reticleMaxs		= Vec2( reticleMins.x + SCREEN_SIZE_X * 0.01f, reticleMins.y + SCREEN_SIZE_X * 0.01f);
	AABB2 reticleBounds = AABB2( reticleMins, reticleMaxs );
	AddVertsForAABB2D( reticleVerts, reticleBounds );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw call for Reticle
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( m_weaponDef->m_reticleTexture );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader( m_weaponDef->m_shader );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->DrawVertexArray( (int)reticleVerts.size(), reticleVerts.data() );

	// Unbind texture and shader
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );

}

//----------------------------------------------------------------------------------------------------------------------
void Weapon::RenderTextUI() const
{
	std::vector<Vertex_PCU> textVerts;

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	BitmapFont* bitmapFont	= nullptr;
	bitmapFont				= g_theRenderer->CreateOrGetBitmapFontFromFile( std::string( "Data/Fonts/SquirrelFixedFont" ).c_str() );
	Texture& bitmapTexture	= bitmapFont->GetTexture();

	//----------------------------------------------------------------------------------------------------------------------
	// Render player currentHealth count
	Vec2 healthMins				= Vec2::ZERO; 
	Vec2 healthMaxs				= Vec2( 20.0f, 20.0f );
	AABB2 healthBounds			= AABB2( healthMins, healthMaxs );
	healthBounds.SetCenter( Vec2( SCREEN_SIZE_X * 0.31f, SCREEN_SIZE_Y * 0.08f ) ); 
	float cellHeight			= 6.0f;
	std::string healthString	= Stringf( "%.0f", m_currentMap->m_player->m_currentActor->m_currentHealth );
	bitmapFont->AddVertsForTextInBox2D( textVerts, healthBounds, cellHeight, healthString );

	//----------------------------------------------------------------------------------------------------------------------
	// Render player death count
	Vec2 deathMins				= Vec2::ZERO; 
	Vec2 deathMaxs				= Vec2( 20.0f, 20.0f );
	AABB2 deathBounds			= AABB2( deathMins, deathMaxs );
	deathBounds.SetCenter( Vec2( SCREEN_SIZE_X * 0.94f, SCREEN_SIZE_Y * 0.08f ) ); 
	float deathCellHeight		= 6.0f;
//	std::string deathString		= Stringf( "%i", m_currentMap->m_player->m_deathCounter );
//	std::string deathString		= Stringf( "%i", m_currentMap->m_player->m_deathCounter );
	std::string deathString		= Stringf( "%i", 0 );
	bitmapFont->AddVertsForTextInBox2D( textVerts, deathBounds, deathCellHeight, deathString );

	//----------------------------------------------------------------------------------------------------------------------
	// Render player death count
	Vec2 killsMins				= Vec2::ZERO; 
	Vec2 killsMaxs				= Vec2( 20.0f, 20.0f );
	AABB2 killsBounds			= AABB2( killsMins, killsMaxs );
	killsBounds.SetCenter( Vec2( SCREEN_SIZE_X * 0.065f, SCREEN_SIZE_Y * 0.08f ) ); 
	float killsCellHeight		= 6.0f;
//	std::string deathString		= Stringf( "%i", m_currentMap->m_player->m_deathCounter );
	std::string killsString		= Stringf( "%i", 0 );
	bitmapFont->AddVertsForTextInBox2D( textVerts, killsBounds, killsCellHeight, killsString );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw call for UI text
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( &bitmapTexture );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() );

	// Unbind texture and shader
	g_theRenderer->BindTexture( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
void Weapon::RenderWeaponUI() const
{
	std::vector<Vertex_PCU> weaponVerts;

	// Calculate WeaponAnim bounds	
	Vec2 weaponMins		= Vec2::ZERO;
	Vec2 weaponMaxs		= m_weaponDef->m_spriteSize;
	
	// Set WeaponAnim bounds	
	AABB2 weaponBounds	= AABB2( weaponMins, weaponMaxs );
	Vec2 newCenter		= Vec2(SCREEN_CENTER_X, SCREEN_SIZE_Y * 0.31f) ;
	weaponBounds.SetCenter( newCenter ); 

	// Calculate quad offset using pivot
	float offsetX = m_weaponDef->m_spriteSize.x * m_weaponDef->m_spritePivot.x;
	float offsetY = m_weaponDef->m_spriteSize.y * m_weaponDef->m_spritePivot.y;
	
	// Apply offset
	weaponBounds.m_mins.x		+= offsetX;
	weaponBounds.m_mins.y		+= offsetY;
	weaponBounds.m_maxs.x		+= offsetX;
	weaponBounds.m_maxs.y		+= offsetY; 

	Texture* weaponTexture = nullptr;

	//----------------------------------------------------------------------------------------------------------------------
	if ( m_currentWeaponAnimGroupName == "Idle" )
	{
		weaponTexture				= m_currentWeaponGroupDef.m_texture;
		float totalSeconds			= m_currentMap->m_player->m_currentActor->m_animClock->GetTotalSeconds();
		float currentTime			= totalSeconds - m_weaponAnimStartTime;
		SpriteDefinition spriteDef	= m_currentWeaponGroupDef.m_spriteAnimDef->GetSpriteDefAtTime( currentTime );			
	
		Vec2 uvMins = Vec2::ZERO;
		Vec2 uvMaxs = Vec2::ZERO;
		spriteDef.GetUVs( uvMins, uvMaxs );
		AddVertsForAABB2D( weaponVerts, weaponBounds, Rgba8::WHITE, AABB2( uvMins, uvMaxs ) );
		
//		DebuggerPrintf( "SpriteDef Idle %d \n", spriteDef.m_spriteIndex );
	}
	else if ( m_currentWeaponAnimGroupName == "Attack" )
	{
		weaponTexture				= m_currentWeaponGroupDef.m_texture;
		float totalSeconds			= m_currentMap->m_player->m_currentActor->m_animClock->GetTotalSeconds();
		float currentTime			= totalSeconds - m_weaponAnimStartTime;
		SpriteDefinition spriteDef	= m_currentWeaponGroupDef.m_spriteAnimDef->GetSpriteDefAtTime( currentTime );

		Vec2 uvMins = Vec2::ZERO;
		Vec2 uvMaxs = Vec2::ZERO;
		spriteDef.GetUVs( uvMins, uvMaxs );
		AddVertsForAABB2D( weaponVerts, weaponBounds, Rgba8::WHITE, AABB2( uvMins, uvMaxs ) );
		
//		DebuggerPrintf( "SpriteDef Attack %d \n", spriteDef.m_spriteIndex );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Draw call for weaponAnim
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( weaponTexture );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader( m_weaponDef->m_shader );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->DrawVertexArray( (int)weaponVerts.size(), weaponVerts.data() );

	// Unbind texture and shader
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
void Weapon::PlayAnimationsByName( std::string name )
{
	// Check if currentName is name passed in parameter
	if ( m_currentWeaponAnimGroupName == name && !m_weaponAnimIsDonePlaying )
	{
		return;
	}

	// set currentName to name passed in the parameter
	m_currentWeaponAnimGroupName	= name;
	m_weaponAnimStartTime			= m_weaponAnimClock->GetTotalSeconds();
	m_weaponAnimClock->SetTimeScale( 1.0f );
}

//----------------------------------------------------------------------------------------------------------------------
void Weapon::IsAnimDonePlaying()
{	
	int	  animFrameCount  = abs( m_currentWeaponGroupDef.m_endFrame - m_currentWeaponGroupDef.m_startFrame ) + 1;
	float animMaxDuration = animFrameCount * m_currentWeaponGroupDef.m_secondsPerFrame;

	if ( animMaxDuration == 0.0f )
	{
		return;
	}

	float currentTime				= m_weaponAnimClock->GetTotalSeconds();
	float animCurrentTimeElasped	= currentTime - m_weaponAnimStartTime;
	if ( animCurrentTimeElasped >= animMaxDuration )
	{
		m_weaponAnimIsDonePlaying = true;
		PlayAnimationsByName( "Idle" );
	}
	else
	{
		m_weaponAnimIsDonePlaying= false;
	}

//	DebuggerPrintf( "endF %d - startF %d  = anim %d, animMaxDuration %f, SPF %f, currentTime%f \n", m_currentWeaponGroupDef.m_endFrame, m_currentWeaponGroupDef.m_startFrame, animFrameCount, animMaxDuration, m_currentWeaponGroupDef.m_secondsPerFrame, currentTime );
}
