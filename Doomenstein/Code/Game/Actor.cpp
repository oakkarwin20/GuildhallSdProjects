#include "Game/Map.hpp"
#include "Game/SpawnInfo.hpp"
#include "Game/Actor.hpp"
#include "Game/GameCommon.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/AIController.hpp"
#include "Game/App.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"	
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <Engine/Core/ErrorWarningAssert.hpp>

//----------------------------------------------------------------------------------------------------------------------
Actor::Actor( Map* currentMap, SpawnInfo const& currentSpawnInfo )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Initialize member variables
	m_currentMap = currentMap;

	m_actorDef		= ActorDefinition::GetActorDefByName( currentSpawnInfo.m_actorName );
	m_position		= currentSpawnInfo.m_actorPosition;
	m_orientation	= currentSpawnInfo.m_actorOrientation;

	m_physicsRadius  = m_actorDef->m_physicsRadius;
	m_physicsHeight  = m_actorDef->m_physicsHeight;
	m_solidColor	 = m_actorDef->m_solidColor;
	m_wireframeColor = m_actorDef->m_wireframeColor;
	
	m_currentHealth = m_actorDef->m_health;

	//----------------------------------------------------------------------------------------------------------------------
	// Create and equip current weapon 
	if ( m_actorDef->m_weaponNameList.size() > 0 )
	{
		// Create weapons
		for ( int i = 0; i < m_actorDef->m_weaponNameList.size(); i++ )
		{
			WeaponDefinition const* currentWeaponDef = WeaponDefinition::GetWeaponDefByName( m_actorDef->m_weaponNameList[i] );

			Weapon* currentWeapon = new Weapon( currentWeaponDef );
			currentWeapon->m_currentMap = currentMap;
			m_weaponList.push_back( currentWeapon );
		}

		EquipWeapon();
//		m_currentWeapon = m_weaponList[ m_currentWeaponIndex ];
	}
	
	// Create rounded quad 
//	// Calculate quad positions using size
//	Vec3 bottomLeft  = Vec3( 0.0f,				   0.0f,				 0.0f );
//	Vec3 bottomRight = Vec3( 0.0f, m_actorDef->m_size.x,				 0.0f );
//	Vec3 topLeft	 = Vec3( 0.0f,				   0.0f, m_actorDef->m_size.y );
//	Vec3 topRight	 = Vec3( 0.0f, m_actorDef->m_size.x, m_actorDef->m_size.y );
//
//	// Calculate quad offset using pivot
//	float offsetX = m_actorDef->m_size.x * m_actorDef->m_pivot.x;
//	float offsetY = m_actorDef->m_size.y * m_actorDef->m_pivot.y;
//
//	// Apply offset
//	bottomLeft  = Vec3(  bottomLeft.x,	 bottomLeft.y - offsetX,  bottomLeft.z - offsetY );
//	bottomRight = Vec3( bottomRight.x,	bottomRight.y - offsetX, bottomRight.z - offsetY );
//	topLeft		= Vec3(		topLeft.x,		topLeft.y - offsetX,	 topLeft.z - offsetY );
//	topRight	= Vec3(	   topRight.x,	   topRight.y - offsetX,	topRight.z - offsetY );
//
//	// Create quad
//	AddVertsForRoundedQuad3D( m_vertexList, bottomLeft, bottomRight, topRight, topLeft, m_solidColor );

	// Set gameClock as animClock's parent 
	m_animClock = new Clock( g_theApp->m_gameClock );

	//----------------------------------------------------------------------------------------------------------------------
	// Mark actor for dead if dieOnSpawn == true
	if ( m_actorDef->m_dieOnSpawn )
	{
		m_isDead = true;
	}

	if ( m_actorDef->m_spriteAnimGroupDefList.size() > 0.0f )
	{
		std::string animName = m_actorDef->m_spriteAnimGroupDefList[0]->m_name;
		PlayAnimationsByName( animName );
	}

//	AddVertsForCylinderZ3D( m_vertexList, Vec2::ZERO, FloatRange( 0.0f, m_physicsHeight ), m_physicsRadius, 8.0f );
//	AddVertsForCone3D( m_vertexList, Vec3::ZERO, Vec3(0.0f, 0.0f, m_physicsHeight), 0.2f );

//	Mat44 coneMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
//	coneMatrix.AppendYRotation( 90.0f );
//	Vec3 forward	= coneMatrix.GetIBasis3D();

//	AddVertsForCone3D( m_vertexList, Vec3::ZERO, forward, 1.0f );

	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Audio
	bool is3D = true;
	m_missileMiceExplodeSID		= g_theAudio->CreateOrGetSound( "Data/Audio/MissileMiceExplode.wav" , is3D );
	m_demonPoopExplodeSID		= g_theAudio->CreateOrGetSound( "Data/Audio/WilhelmScream.wav"		, is3D );
	m_demonAttackSID			= g_theAudio->CreateOrGetSound( "Data/Audio/DemonAttack.wav"		, is3D );
	m_demonHurtSID				= g_theAudio->CreateOrGetSound( "Data/Audio/DemonHurt.wav"			, is3D );
	m_demonDeathSID				= g_theAudio->CreateOrGetSound( "Data/Audio/DemonDeath.wav"			, is3D );
	m_marineHurtSID				= g_theAudio->CreateOrGetSound( "Data/Audio/PlayerHurt.wav"			, is3D );
	m_marineDeathSID			= g_theAudio->CreateOrGetSound( "Data/Audio/PlayerDeath1.wav"		, is3D );
}

//----------------------------------------------------------------------------------------------------------------------
Actor::Actor( Vec3 position, EulerAngles orientation, Rgba8 color, float radius, float height, bool isMovable )
{
	m_position		= position;
	m_orientation	= orientation;
	m_solidColor	= color;
	m_physicsRadius = radius;
	m_physicsHeight = height;
	m_isMovable		= isMovable;
	
	// Add verts for cylinder in local space
	Vec3 cylinderStartLocalPos = Vec3::ZERO;
	Vec3 cylinderEndLocalPos   = cylinderStartLocalPos + Vec3( 0.0f, 0.0f, height );
//	AddVertsForCylinder3D( m_vertexList, cylinderStartLocalPos, cylinderEndLocalPos, m_physicsRadius, Rgba8::WHITE );

	//----------------------------------------------------------------------------------------------------------------------
	 // Calculate quad positions using size
	Vec3 bottomLeft		= Vec3( 0.0f, 0.0f, 0.0f );
	Vec3 bottomRight	= Vec3( 0.0f, m_actorDef->m_size.x, 0.0f );
	Vec3 topleft		= Vec3( 0.0f, 0.0f, m_actorDef->m_size.y );
	Vec3 topRight		= Vec3( 0.0f, m_actorDef->m_size.x, m_actorDef->m_size.y );

	// Calculate quad offset using pivot

	// Create quad
	AddVertsForRoundedQuad3D( m_vertexListPNCU, bottomLeft, bottomRight, topleft, topRight );
}

//----------------------------------------------------------------------------------------------------------------------
Actor::~Actor()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::Update( float deltaSeconds )
{
//	if ( m_actorDef->m_name == "PlasmaProjectile" )
//	{
//		bool durationElapsed = m_owner->m_currentWeapon->m_weaponRefireStopwatch.HasDurationElapsed();
//		DebuggerPrintf( "elapsedTime = %0.2f, hasDurationElapsed = %d, isDead %d \n", m_owner->m_currentWeapon->m_weaponRefireStopwatch.GetElapsedTime(), ( durationElapsed ) ? 1:0, (m_isDead) ? 1:0 );
//	}

	// Don't update current actor if dead
	if ( m_isDead )
	{
//		m_actorDef->m_collidesWithActors = false;

		// Start timer
		if ( !m_shouldGarbageStopwatchStart )
		{
			float duration			= m_actorDef->m_corpseLifetime;
			m_isGarbageStopwatch	= Stopwatch( &g_theApp->m_gameClock, duration );
			m_isGarbageStopwatch.Start();
			m_shouldGarbageStopwatchStart = true;
		}

		// Timer is elapsed? Set 2nd bool = true
		if ( m_isGarbageStopwatch.HasDurationElapsed() )
		{
			m_isGarbage = true;

			// Stop rendering drugs camera effect if player's possessed actor isDead && isGarbage
			if ( m_currentMap->m_player->m_currentActor->m_isDead && m_currentMap->m_player->m_currentActor->m_isGarbage )
			{
				m_currentMap->m_player->m_drugEffectIsActive = false;
			}
		}

		return;
	}

	UpdatePhysics( deltaSeconds );
	if ( m_currentWeapon != nullptr )
	{
		m_currentWeapon->Update();

		bool durationElapsed = m_currentMap->m_player->m_currentActor->m_drugsStopwatch.HasDurationElapsed();
		if ( durationElapsed )
		{
			m_currentMap->m_player->m_currentActor->m_isOnDrugs = false;
			m_currentMap->m_player->m_currentActor->m_drugsStopwatch.DecrementDurationIfElapsed();
			m_currentMap->m_player->m_currentActor->m_drugsStopwatch.Stop();
			m_currentMap->m_player->m_drugEffectIsActive = false;
		}
//		UpdateWeaponProjectileActors();
	}

	// Check if currentAnim is done playing
	m_animIsDonePlaying = IsAnimDonePlaying();
	
	// If currentAnimGroup is not Null
	if ( m_currentAnimGroup )
	{
		// And currentAnimGroup should scale by speed
		if ( m_currentAnimGroup->m_scaleBySpeed )
		{
			// Set the timeScale based on actor's movement speed
			float magnitude = m_velocity.GetLength();
			float timeScale = magnitude / m_actorDef->m_runSpeed;
			m_animClock->SetTimeScale( timeScale );
		}
		else
		{
			m_animClock->SetTimeScale( 1.0f );
		}
	}
	else
	{
		m_animClock->SetTimeScale( 1.0f );
	}

	// Play hurt sound for Demon
	if ( m_actorDef->m_faction == "Demon" )
	{
		if ( g_theAudio->IsPlaying( m_demonHurtSPBID ) )
		{
			// Play pistol fire sound
			bool is3D = true;
			m_demonHurtSID = g_theAudio->CreateOrGetSound( "Data/Audio/DemonHurt.wav", is3D );
			g_theAudio->SetSoundPosition( m_demonHurtSPBID, m_position );
//			DebuggerPrintf( Stringf( "\nDemonPos setPos= %f, %f, %f\n\n", m_position.x, m_position.y, m_position.z ).c_str() );
		}
	}
	// Play hurt sound Actor
	if ( m_actorDef->m_faction == "Marine" )
	{
		if ( g_theAudio->IsPlaying( m_marineHurtSPBID ) )
		{
			// Play pistol fire sound
			bool is3D = true;
			m_marineHurtSID = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerHurt.wav", is3D );
			g_theAudio->SetSoundPosition( m_marineHurtSPBID, m_position );
			//			DebuggerPrintf( Stringf( "\nDemonPos setPos= %f, %f, %f\n\n", m_position.x, m_position.y, m_position.z ).c_str() );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::Render()
{
	// Drugs Debug Message code
//	if ( m_actorDef->m_name == "Marine" )
//	{
//		if ( m_currentMap->m_player->m_currentActor != nullptr )
//		{
//			float duration = m_currentMap->m_player->m_currentActor->m_drugsStopwatch.GetElapsedTime();
//			if ( m_isOnDrugs ) 
//			{
//				DebuggerPrintf( Stringf( "isOnDrugs, duration %f\n", duration ).c_str() );
//			}
//			else
//			{
//				DebuggerPrintf( Stringf( "NotOnDrugs, duration %f\n", duration).c_str() );
//			}
//		}
//	}

	if ( m_currentMap->m_player->m_currentActor != nullptr )
	{
//		if ( m_actorDef->m_faction == "Marine" && m_currentMap->m_player->m_currentActor == this )
		if ( !m_currentMap->m_player->m_isFreeFlyOn && m_currentMap->m_player->m_currentActor == this )
		{
			return;
		}
	}

	// If possessing a player and this player is not a demon
	if ( !m_currentMap->m_player->m_isFreeFlyOn && m_actorDef->m_visible && m_actorDef->m_faction != "Demon" )
	{
		if ( m_isPossessed || m_isGarbage )
		{
			return;
		}
	}

	// Set model matrix to Billboard matrix 
	Mat44 cameraMatrix = m_currentMap->m_player->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	cameraMatrix.SetTranslation3D( m_currentMap->m_player->m_position );
	Mat44 modelMatrix = GetModelMatrix( m_position, m_orientation );

	if ( m_actorDef->m_billboardType == "WorldUpFacing" )
	{
		modelMatrix = GetBillboardMatrix( BillboardType::WORLD_UP_CAMERA_FACING, cameraMatrix, m_position );
	}
	if ( m_actorDef->m_billboardType == "WorldUpOpposing" )
	{
		modelMatrix = GetBillboardMatrix( BillboardType::WORLD_UP_CAMERA_OPPOSING, cameraMatrix, m_position );
	}
	if ( m_actorDef->m_billboardType == "FullOpposing" )
	{
		modelMatrix = GetBillboardMatrix( BillboardType::FULL_CAMERA_OPPOSING, cameraMatrix, m_position );
	}

	// Debug drawCall
	// Draw call Solid
//	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
//	g_theRenderer->BindTexture( nullptr );
//	g_theRenderer->BindShader( m_actorDef->m_shader );
//	g_theRenderer->SetModelConstants( modelMatrix, solidColor );
//	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
//	g_theRenderer->DrawVertexArray( static_cast<int>( m_vertexList.size() ), m_vertexList.data() );
//	g_theRenderer->BindShader( nullptr );
//
//	// Draw call Wireframe
//	g_theRenderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_BACK );
//	modelMatrix = GetModelMatrix( m_position, m_orientation );
//	g_theRenderer->SetModelConstants( modelMatrix, m_wireframeColor );
//	g_theRenderer->DrawVertexArray( static_cast<int>( m_vertexList.size() ), m_vertexList.data() );

//----------------------------------------------------------------------------------------------------------------------
//	if ( (m_actorDef->m_name == "Marine") || (m_actorDef->m_name == "Demon") || (m_actorDef->m_name == "PlasmaProjectile") || (m_actorDef->m_name == "BloodSplatter") || (m_actorDef->m_name == "BulletHit") )
//	{
//		SpriteDefinition spriteDef = GetCurrentSpriteDef();
//		Vec2 uvMins = Vec2::ZERO; 
//		Vec2 uvMaxs = Vec2::ZERO;
//		spriteDef.GetUVs( uvMins, uvMaxs );
//		
//		// Calculate quad positions using size
//		Vec3 bottomLeft  = Vec3( 0.0f,				   0.0f,				 0.0f );
//		Vec3 bottomRight = Vec3( 0.0f, m_actorDef->m_size.x,				 0.0f );
//		Vec3 topLeft	 = Vec3( 0.0f,				   0.0f, m_actorDef->m_size.y );
//		Vec3 topRight	 = Vec3( 0.0f, m_actorDef->m_size.x, m_actorDef->m_size.y );
//
//		// Calculate quad offset using pivot
//		float offsetX = m_actorDef->m_size.x * m_actorDef->m_pivot.x;
//		float offsetY = m_actorDef->m_size.y * m_actorDef->m_pivot.y;
//
//		// Apply offset
//		bottomLeft  = Vec3(  bottomLeft.x,	 bottomLeft.y - offsetX,  bottomLeft.z - offsetY );
//		bottomRight = Vec3( bottomRight.x,	bottomRight.y - offsetX, bottomRight.z - offsetY );
//		topLeft		= Vec3(		topLeft.x,		topLeft.y - offsetX,	 topLeft.z - offsetY );
//		topRight	= Vec3(	   topRight.x,	   topRight.y - offsetX,	topRight.z - offsetY );
//
//		// Create quad
//		m_vertexList.clear();
//		AddVertsForRoundedQuad3D( m_vertexList, bottomLeft, bottomRight, topRight, topLeft, m_solidColor, AABB2( uvMins, uvMaxs ) );
//	}
//
//	// Draw call Solid
//	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
//	g_theRenderer->BindTexture( m_actorDef->m_texture );
//	g_theRenderer->BindShader( m_actorDef->m_shader );
//	g_theRenderer->SetModelConstants( modelMatrix, solidColor );
//	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
//	g_theRenderer->DrawVertexArray( static_cast<int>( m_vertexList.size() ), m_vertexList.data() );
//	g_theRenderer->BindShader( nullptr );
	// Draw call Wireframe
//	g_theRenderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_BACK );
//	modelMatrix = GetModelMatrix( m_position, m_orientation );
//	g_theRenderer->SetModelConstants( modelMatrix, m_wireframeColor );
//	g_theRenderer->DrawVertexArray( static_cast<int>( m_vertexList.size() ), m_vertexList.data() );

	// Render using PNCU AddVertsForQuad			// renderLit and renderRounded
	if ( m_actorDef->m_renderLit )
	{
		SpriteDefinition spriteDef = GetCurrentSpriteDef();
		Vec2 uvMins = Vec2::ZERO; 
		Vec2 uvMaxs = Vec2::ZERO;
		spriteDef.GetUVs( uvMins, uvMaxs );

		// Calculate quad positions using size
		Vec3 bottomLeft  = Vec3( 0.0f,				   0.0f,				 0.0f );
		Vec3 bottomRight = Vec3( 0.0f, m_actorDef->m_size.x,				 0.0f );
		Vec3 topLeft	 = Vec3( 0.0f,				   0.0f, m_actorDef->m_size.y );
		Vec3 topRight	 = Vec3( 0.0f, m_actorDef->m_size.x, m_actorDef->m_size.y );

		// Calculate quad offset using pivot
		float offsetX = m_actorDef->m_size.x * m_actorDef->m_pivot.x;
		float offsetY = m_actorDef->m_size.y * m_actorDef->m_pivot.y;

		// Apply offset
		bottomLeft  = Vec3(  bottomLeft.x,	 bottomLeft.y - offsetX,  bottomLeft.z - offsetY );
		bottomRight = Vec3( bottomRight.x,	bottomRight.y - offsetX, bottomRight.z - offsetY );
		topLeft		= Vec3(		topLeft.x,		topLeft.y - offsetX,	 topLeft.z - offsetY );
		topRight	= Vec3(	   topRight.x,	   topRight.y - offsetX,	topRight.z - offsetY );

		// Create quad
		m_vertexListPNCU.clear();
		AddVertsForQuad3D( m_vertexListPNCU, bottomLeft, bottomRight, topRight, topLeft, m_solidColor, AABB2( uvMins, uvMaxs ) );

		// Draw call Solid
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindTexture( m_actorDef->m_texture );
		g_theRenderer->BindShader( m_actorDef->m_shader );
		g_theRenderer->SetModelConstants( modelMatrix, m_solidColor );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->DrawVertexArray( static_cast<int>( m_vertexListPNCU.size() ), m_vertexListPNCU.data() );
		g_theRenderer->BindShader( nullptr );
	}
	
	// Render using PNCU AddVertsForRoundedQuad		// renderLit
	else if ( m_actorDef->m_renderLit && m_actorDef->m_renderRounded )
	{
		SpriteDefinition spriteDef = GetCurrentSpriteDef();
		Vec2 uvMins = Vec2::ZERO; 
		Vec2 uvMaxs = Vec2::ZERO;
		spriteDef.GetUVs( uvMins, uvMaxs );

		// Calculate quad positions using size
		Vec3 bottomLeft  = Vec3( 0.0f,				   0.0f,				 0.0f );
		Vec3 bottomRight = Vec3( 0.0f, m_actorDef->m_size.x,				 0.0f );
		Vec3 topLeft	 = Vec3( 0.0f,				   0.0f, m_actorDef->m_size.y );
		Vec3 topRight	 = Vec3( 0.0f, m_actorDef->m_size.x, m_actorDef->m_size.y );

		// Calculate quad offset using pivot
		float offsetX = m_actorDef->m_size.x * m_actorDef->m_pivot.x;
		float offsetY = m_actorDef->m_size.y * m_actorDef->m_pivot.y;

		// Apply offset
		bottomLeft  = Vec3(  bottomLeft.x,	 bottomLeft.y - offsetX,  bottomLeft.z - offsetY );
		bottomRight = Vec3( bottomRight.x,	bottomRight.y - offsetX, bottomRight.z - offsetY );
		topLeft		= Vec3(		topLeft.x,		topLeft.y - offsetX,	 topLeft.z - offsetY );
		topRight	= Vec3(	   topRight.x,	   topRight.y - offsetX,	topRight.z - offsetY );

		// Create quad
		m_vertexListPNCU.clear();
		AddVertsForRoundedQuad3D( m_vertexListPNCU, bottomLeft, bottomRight, topRight, topLeft, m_solidColor, AABB2( uvMins, uvMaxs ) );

		// Draw call Solid 
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindTexture( m_actorDef->m_texture );
		g_theRenderer->BindShader( m_actorDef->m_shader );
		g_theRenderer->SetModelConstants( modelMatrix, m_solidColor );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->DrawVertexArray( static_cast<int>( m_vertexListPNCU.size() ), m_vertexListPNCU.data() );
		g_theRenderer->BindShader( nullptr );
	}
	// Render using AddVertsForPCU					// not lit or rounded
	else
	{
		// Don't render if currentActor is a spawnPoint
		if ( m_actorDef->m_name == "SpawnPoint" )
		{
			return;
		}

		SpriteDefinition spriteDef	= GetCurrentSpriteDef();
		Vec2 uvMins					= Vec2::ZERO; 
		Vec2 uvMaxs					= Vec2::ZERO;
		spriteDef.GetUVs( uvMins, uvMaxs );

		// Calculate quad positions using size
		Vec3 bottomLeft  = Vec3( 0.0f,				   0.0f,				 0.0f );
		Vec3 bottomRight = Vec3( 0.0f, m_actorDef->m_size.x,				 0.0f );
		Vec3 topLeft	 = Vec3( 0.0f,				   0.0f, m_actorDef->m_size.y );
		Vec3 topRight	 = Vec3( 0.0f, m_actorDef->m_size.x, m_actorDef->m_size.y );

		// Calculate quad offset using pivot
		float offsetX = m_actorDef->m_size.x * m_actorDef->m_pivot.x;
		float offsetY = m_actorDef->m_size.y * m_actorDef->m_pivot.y;

		// Apply offset
		bottomLeft  = Vec3(  bottomLeft.x,	 bottomLeft.y - offsetX,  bottomLeft.z - offsetY );
		bottomRight = Vec3( bottomRight.x,	bottomRight.y - offsetX, bottomRight.z - offsetY );
		topLeft		= Vec3(		topLeft.x,		topLeft.y - offsetX,	 topLeft.z - offsetY );
		topRight	= Vec3(	   topRight.x,	   topRight.y - offsetX,	topRight.z - offsetY );

		// Create quad
		m_vertexListPCU.clear();
		AddVertsForQuad3D( m_vertexListPCU, bottomLeft, bottomRight, topRight, topLeft, m_solidColor, AABB2( uvMins, uvMaxs ) );

		// Draw call Solid 
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindTexture( m_actorDef->m_texture );
		g_theRenderer->BindShader( m_actorDef->m_shader );
		g_theRenderer->SetModelConstants( modelMatrix, m_solidColor );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->DrawVertexArray( static_cast<int>( m_vertexListPCU.size() ), m_vertexListPCU.data() );
		g_theRenderer->BindShader( nullptr );
	}

	// Animation Debug Message code
//	if ( m_actorDef->m_name == "RangedDemon" )
//	{
//		DebuggerPrintf( Stringf( "CurrentAnimGroup %s\n", m_currentAnimGroup->m_name.c_str() ).c_str() );
//	}

//	// Draw call Solid 
//	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
//	g_theRenderer->BindTexture( m_actorDef->m_texture );
//	g_theRenderer->BindShader( m_actorDef->m_shader );
//	g_theRenderer->SetModelConstants( modelMatrix, solidColor );
//	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
//	g_theRenderer->DrawVertexArray( static_cast<int>( m_vertexList.size() ), m_vertexList.data() );
//	g_theRenderer->BindShader( nullptr );
//
//	// Draw call Solid
//	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
//	g_theRenderer->BindTexture( m_actorDef->m_texture );
//	g_theRenderer->BindShader( m_actorDef->m_shader );
//	g_theRenderer->SetModelConstants( modelMatrix, solidColor );
//	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
//	g_theRenderer->DrawVertexArray( static_cast<int>( m_vertexListPCU.size() ), m_vertexListPCU.data() );
//	g_theRenderer->BindShader( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 Actor::GetModelMatrix( Vec3 position, EulerAngles orientation ) const
{
	Mat44 modelMatrix;
	modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.SetTranslation3D( position );
	return modelMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::UpdatePhysics( float deltaSeconds )
{
	Vec3 decelerationForce = -m_actorDef->m_drag * m_velocity;
	AddForce( decelerationForce );									// Decelerate every frame by applying negative vector 

	if ( m_actorDef->m_name == "MiceProjectile" && m_position.z <= 0.1f )
	{
		m_velocity.z = 0.0f;
	}

	m_velocity += m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;

//	if ( m_actorDef->m_name == "MiceProjectile" )
//	{
//		g_theDevConsole->AddLine( Rgba8::WHITE, Stringf("velocity: %0.02f, %0.02f, %0.02f accel: %0.02f, %0.02f, %0.02f", m_velocity.x, m_velocity.y, m_velocity.z, m_acceleration.x, m_acceleration.y, m_acceleration.z ) );
//		g_theDevConsole->AddLine( Rgba8::WHITE, Stringf("velocity: %0.02f, %0.02f, %0.02f", m_velocity.x, m_velocity.y, m_velocity.z) );
//		g_theDevConsole->AddLine( Rgba8::WHITE, Stringf("speed: %0.02f", m_velocity.GetLength() ) );
//	}

	// Reset acceleration
	m_acceleration = Vec3::ZERO;
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::TakeDamage( float damageAmount, Actor* actorWhoDamagedMe )
{
	if ( m_isDead )
	{
//		// Debug Message code
//		if ( m_actorDef->m_name == "RangedDemon" )
//		{
//			DebuggerPrintf( Stringf( "Enemy dead\n").c_str() );
//		}
		return;
	}

	// Debug Message code
//	if ( m_actorDef->m_name == "RangedDemon" )
//	{
//		DebuggerPrintf( Stringf( "Health before %f, damage %f\n", m_currentHealth, damageAmount).c_str() );
//	}

	// If still alive, take damage
	m_currentHealth -= damageAmount; 
	PlayAnimationsByName( "Hurt" );

	if ( m_actorDef->m_name == "Demon" || m_actorDef->m_name == "RangedDemon" )
	{
		// Play pistol fire sound
		bool is3D = true;
		m_demonHurtSID = g_theAudio->CreateOrGetSound( "Data/Audio/DemonHurt.wav", is3D );
		m_demonHurtSPBID = g_theAudio->StartSoundAt( m_demonHurtSID, m_position );
//		DebuggerPrintf( Stringf( "\nDemonPos Hurt = %f, %f, %f\n\n", m_position.x, m_position.y, m_position.z ).c_str() );
	}
	
	if ( m_actorDef->m_name == "Marine" )
	{
		// Play pistol fire sound
		bool is3D = true;
		m_marineHurtSID   = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerHurt.wav", is3D );
		m_marineHurtSPBID = g_theAudio->StartSoundAt( m_marineHurtSID, m_position );
		//		DebuggerPrintf( Stringf( "\nDemonPos Hurt = %f, %f, %f\n\n", m_position.x, m_position.y, m_position.z ).c_str() );
	}


	// Debug Message code
//	if ( m_actorDef->m_name == "RangedDemon" )
//	{
//		DebuggerPrintf( Stringf( "Health after %f\n", m_currentHealth ).c_str() );
//	}

	// Debug Message code
//	if ( m_actorDef->m_name == "RangedDemon" )
//	{
//		int start = m_currentSpriteAnimDef->m_startSpriteIndex;
//		int end   = m_currentSpriteAnimDef->m_endSpriteIndex;
//		DebuggerPrintf( Stringf( "%s, SF %d, EF %d\n", m_currentAnimGroup->m_name.c_str(), start, end ).c_str() );
//	}

	// Handle dying if health is depleted
	if ( m_currentHealth <= 0.0f )
	{
		m_isDead = true;
		m_currentHealth = 0.0f;
		PlayAnimationsByName( "Death" );

		bool is3D = true;
		// Increment death counter
		if ( m_actorDef->m_name == "Marine" )
		{
			m_currentMap->m_player->m_deathCounter++;
			m_marineDeathSID   = g_theAudio->CreateOrGetSound( "Data/Audio/PlayerDeath1.wav", is3D );
			m_marineDeathSPBID = g_theAudio->StartSoundAt( m_marineDeathSID, m_position );
		}

		if ( m_actorDef->m_name == "Demon" || m_actorDef->m_name == "RangedDemon" )
		{
			// Play pistol fire sound
			m_demonDeathSID		= g_theAudio->CreateOrGetSound( "Data/Audio/DemonDeath.wav", is3D );
			m_demonDeathSPBID	= g_theAudio->StartSoundAt( m_demonDeathSID, m_position );
//			DebuggerPrintf( Stringf( "\nDemonPos = %f, %f, %f\n\n", m_position.x, m_position.y, m_position.z ).c_str() );
		}
	}

	if ( m_actorDef->m_aiEnabled )
	{
		// Keep track of actorWhoDamagedMe
//		m_currentController
		m_AIController->DamagedBy( actorWhoDamagedMe );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::AddForce( Vec3 forceAmount )
{
	// Add force to our acceleration every frame
	m_acceleration = m_acceleration + forceAmount;
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::AddImpulse( Vec3 const& impulseForce )
{
	m_velocity += impulseForce;
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::OnCollide( Actor* actorToCollide )
{	
	// Do damage
	float randDamage	= g_theRNG->RollRandomFloatInRange( m_actorDef->m_damageOnCollide.m_min, m_actorDef->m_damageOnCollide.m_max );
	Actor* playerActor	= m_currentMap->m_player->GetActor();

	// Tell actors to die
	if ( m_actorDef->m_dieOnCollide )
	{
		PlayAnimationsByName( "Death" );
		m_isDead = true;
		m_velocity = Vec3::ZERO;

		// If plasma rifle hit, render ImpactPoint and ImpactNormal
//		m_orientation.GetAsVectors_XFwd_YLeft_ZUp( m_forward, m_left, m_up );
//		Vec3 tinyImpactNormalEndPos = ( m_position + m_forward * 0.2f );
//		float duration				= 10.0f;
//		DebugAddWorldPoint( m_position, 0.05f, duration );
//		Vec3 dir	 = m_velocity.GetNormalized();
//		Vec3 endPos	 = ( m_position + ( 0.3f * (-dir).GetNormalized() ) );
//		DebugAddWorldArrow( m_position, endPos, 0.02f, duration, Rgba8::BLUE, Rgba8::BLUE );
	}
	
	// If currentActor is MissileMice
	if ( m_actorDef->m_slidesAlongWalls )
	{
		ExplodeMissileMice();
		PlayAnimationsByName( "Death" );
		m_isDead = true;
		m_velocity = Vec3::ZERO;
	}
	else	// If currentActor is player
	{
		if ( actorToCollide == nullptr )
		{
			return;
		}
	
		actorToCollide->TakeDamage( randDamage, playerActor );
	}
	
	// Add impulse on collision
//	Vec3 dispMyselfToTarget = actorToCollide->m_position - m_position;
//	dispMyselfToTarget.z	= 0.0f;
//	dispMyselfToTarget		= dispMyselfToTarget.GetNormalized();
//	AddImpulse( dispMyselfToTarget * m_actorDef->m_impulseOnCollide );
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::MoveInDirection( Vec3 directionToMove, float speed )
{
	// Calculate forceAmount
//	Vec3 forwardNormal = directionToMove.GetNormalized();
//	Vec3 forceAmount = directionToMove.GetLength() * speed * forwardNormal;

	// Add Force in direction of target
	Vec3 forceAmount = directionToMove * m_actorDef->m_drag * speed;			// Direction should be normalized
	AddForce( forceAmount );
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::TurnInDirection( Vec3 directionToMove, float deltaDegrees )
{
	// Direction to move
	float goalAngleDegrees = directionToMove.GetAngleAboutZDegrees();

	// GetTurnedTowardsDegrees
	float newYawDegrees = GetTurnedTowardDegrees( m_orientation.m_yawDegrees, goalAngleDegrees, deltaDegrees );
	
	// apply to current actor's orientation
	m_orientation.m_yawDegrees = newYawDegrees;
	
//	//----------------------------------------------------------------------------------------------------------------------
//	Vec3 marinePos;
//	for ( int i = 0; i < m_currentMap->m_actorList.size(); i++ )
//	{
//		if ( m_currentMap->m_actorList[i]->m_actorDef->m_name == "Marine" )
//		{
//			marinePos = m_currentMap->m_actorList[i]->m_position;
////			return;
//		}
//	}
//
//	directionToMove.GetAngleAboutZDegrees();
//
//	// Calculate displacement from currentActor to Marine	
//	Vec3 dispCurrentActorToMarine			= marinePos - m_position;																// Current actor's (Demon) pos to Marine
//	Vec2 dispActorToMarineV2				= Vec2(dispCurrentActorToMarine.x, dispCurrentActorToMarine.y).GetNormalized();
//
//	// Calculate angle to turn
//	m_orientation.GetAsVectors_XFwd_YLeft_ZUp( m_forward, m_left, m_up );
//	Vec2 forwardV2							= Vec2(m_forward.x, m_forward.y).GetNormalized();
//	float angleBetweenActorAndMarine		= GetAngleDegreesBetweenVectors2D( dispActorToMarineV2, forwardV2 );
//	float yawAfterSmallRotation				=  GetTurnedTowardDegrees( m_orientation.m_yawDegrees, angleBetweenActorAndMarine, m_actorDef->m_turnSpeed );
//
//	// Rotate from Demon's forward towards goalOrientation by specified turn rate
//	m_orientation.m_yawDegrees = yawAfterSmallRotation;
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::Attack()
{
	// Check if weapon is != NULL
	m_currentWeapon->Fire( this );
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::EquipWeapon()
{
	m_currentWeapon = m_weaponList[m_currentWeaponIndex];
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::NextWeapon()
{
	m_currentWeaponIndex++;
	if ( m_currentWeaponIndex > ( m_weaponList.size() - 1 ) )
	{
		m_currentWeaponIndex = 0;
	}
	EquipWeapon();
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::PreviousWeapon()
{
	if ( m_currentWeaponIndex <= 0 )
	{
		m_currentWeaponIndex = (int)m_weaponList.size() - 1;
	}
	else
	{
		m_currentWeaponIndex--;
	}
	
	EquipWeapon();
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::AIPossessCurrentActor( ActorUID actorUID )
{
	if ( m_actorDef->m_aiEnabled )
	{
		if ( m_AIController == nullptr )
		{
			m_AIController = new AIController();
		}

		m_AIController->m_currentMap = m_currentMap;
		m_AIController->m_currentlyPossessedActorUID = m_currentActorUID;
		m_AIController->Possess( m_currentActorUID );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::PlayAnimationsByName( std::string name )
{
	if ( !m_currentAnimGroup )
	{
		return;
	}

	// Check if currentName is name passed in parameter
//	if ( m_currentAnimGroupName == name && !m_animIsDonePlaying )
	if ( m_currentAnimGroup->m_name == name && !m_animIsDonePlaying )
	{
		return;
	}

	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < m_actorDef->m_spriteAnimGroupDefList.size(); i++ )
	{
		if ( m_actorDef->m_spriteAnimGroupDefList[i]->m_name == m_currentAnimGroupName )
		{
			m_currentAnimGroup = m_actorDef->m_spriteAnimGroupDefList[i];
			break;
		}
	}

	// set currentName to name passed in the parameter
	m_currentAnimGroupName = name;
	m_animStartTime		   = m_animClock->GetTotalSeconds();
	m_animClock->SetTimeScale( 1.0f );
}

//----------------------------------------------------------------------------------------------------------------------
bool Actor::IsAnimDonePlaying()
{
	// Check for nullptr
	if ( !m_currentSpriteAnimDef )
	{
		return false;
	}

	// If animation duration has elapsed, m_animIsDonePlaying = true;
	int	  animFrameCount  = abs( m_currentSpriteAnimDef->m_endSpriteIndex - m_currentSpriteAnimDef->m_startSpriteIndex ) + 1;
	float animMaxDuration = animFrameCount * m_currentSpriteAnimDef->m_secondsPerFrame;

	float currentTime			 = m_animClock->GetTotalSeconds();
	float animCurrentTimeElasped = currentTime - m_animStartTime;

	if ( animCurrentTimeElasped >= animMaxDuration )
	{
		m_animIsDonePlaying		= true;
		PlayAnimationsByName( "Walk" );
	}
	else
	{
		m_animIsDonePlaying = false;
	}
	return m_animIsDonePlaying;
}

//----------------------------------------------------------------------------------------------------------------------
SpriteDefinition Actor::GetCurrentSpriteDef()
{
	for ( int i = 0; i < m_actorDef->m_spriteAnimGroupDefList.size(); i++ )
	{
		if ( m_actorDef->m_spriteAnimGroupDefList[i]->m_name == m_currentAnimGroupName )
		{
			m_currentAnimGroup = m_actorDef->m_spriteAnimGroupDefList[i];
		}
	}

	// If actor is idle
	if ( m_currentAnimGroup == nullptr )
	{
		SpriteDefinition spriteDef = m_actorDef->m_spriteSheet->GetSpriteDef( 0 );
		return spriteDef;
	}	
		
	// Get disp from playerCamera to Actor
	Vec3 viewDirection	= m_position - m_currentMap->m_player->m_worldCamera.m_position;
	viewDirection		= Vec3( viewDirection.x, viewDirection.y, 0.0f );
	viewDirection		= viewDirection.GetNormalized();
	
	// Transform viewDirection vector from world space to actor's local space
	Mat44 actorModelMatrix			= GetModelMatrix( m_position, m_orientation );
	Mat44 actorModelMatrixInverse	= actorModelMatrix.GetOrthoNormalInverse();
	Vec3 localViewDirection			= actorModelMatrixInverse.TransformVectorQuantity3D( viewDirection );

	// Loop through SpriteAnimGroups and dotProduct viewingDirection with SpriteAnimDirection	
	float closestDotProductResult	= -1.0f;
	for ( int j = 0; j < m_currentAnimGroup->m_directionalAnimList.size(); j++ )
	{
//		Vec3 actorDirV3				  = currentAnimGroup->m_directionalAnimList[j].m_direction;
		Vec3 actorDirV3				  = m_currentAnimGroup->m_directionalAnimList[j].m_direction;
		float currentDotProductResult = DotProduct3D( localViewDirection, actorDirV3 );
		if ( currentDotProductResult > closestDotProductResult )
		{
			closestDotProductResult = currentDotProductResult;
			m_currentSpriteAnimDef	= m_currentAnimGroup->m_directionalAnimList[j].m_spriteAnimDef;
		}
	}

	float seconds						= m_animClock->GetTotalSeconds();
// 	SpriteDefinition const& spriteDef	= m_currentSpriteAnimDef->GetSpriteDefAtTime( seconds );

	float currentTime					= seconds - m_animStartTime;
 	SpriteDefinition const& spriteDef	= m_currentSpriteAnimDef->GetSpriteDefAtTime( currentTime );

	return spriteDef;
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::UpdateExplosiveProjectileActors()
{
	// Check for early exits
	if ( !m_actorDef->m_isProjectileActor )		// If the weapon can explode
	{
		return;
	}
	if ( m_actorDef->m_name == "PlasmaProjectile" )	 
	{
		return;
	}
	if ( m_isDead )
	{
		return;
	}
	if ( m_owner->m_isDead )
	{
		return;
	}
	if ( m_owner->m_currentWeapon->m_weaponDef->m_projectileAttackRange < 0.1f )		// If the weapon can explode
	{
		return;
	}
	
	// Check if refire time has elapsed
	bool durationElapsed = false;
	if ( m_owner->m_currentWeapon->m_weaponRefireStopwatch.GetElapsedTime() >= ( m_owner->m_currentWeapon->m_weaponRefireStopwatch.m_duration - 0.1f ) )
	{
		durationElapsed = true;
	}

//	DebuggerPrintf( "Actor = %s, Duration = %f \n", m_actorDef->m_name.c_str(), m_currentMap->m_player->m_currentActor->m_currentWeapon->m_weaponRefireStopwatch.m_duration );
//	DebuggerPrintf( "Actor = %s, elapsedTime = %0.2f hasDurationElapsed = %d \n", m_actorDef->m_name.c_str(), m_owner->m_currentWeapon->m_weaponRefireStopwatch.GetElapsedTime(), (durationElapsed) ? 1 : 0 );
	//if ( m_actorDef->m_name == "MiceProjectile" && durationElapsed )
	//{
	//	int x = 5;
	//}
	if ( durationElapsed )
	{
		// Tell projectileActor to die since duration elapsed
		PlayAnimationsByName( "Death" );
		m_isDead = true;

		if ( m_owner->m_currentWeapon->m_weaponDef->m_name == "MissileMice" )
		{
			ExplodeMissileMice();

//			// Render explosion
//			DebugAddWorldWireSphere( m_position, m_actorDef->m_explosionRadius, 0.0f, Rgba8::MAGENTA, Rgba8::MAGENTA );

			// Play MissileMice explode sound
			m_missileMiceExplodeSPBID = g_theAudio->StartSound( m_missileMiceExplodeSID );
		}

//		if ( m_owner->m_currentWeapon->m_weaponDef->m_name == "DemonPoop" )
//		{
//			m_isOnDrugs = true;
//			m_drugsStopwatch = Stopwatch( m_durationOnDrugs );
//		}
	
		if ( m_owner->m_currentWeapon->m_weaponDef->m_name == "DemonPoop" )
		{
			ExplodeDemonPoop();

			// Play demon poop explode sound
			m_demonPoopExplodeSPBID = g_theAudio->StartSound( m_demonPoopExplodeSID );
			g_theAudio->SetSoundPosition( m_demonPoopExplodeSPBID, m_position );
		}
	}
//	else if ( m_currentMap->m_player->m_currentActor->m_currentWeapon->m_weaponDef->m_name == "MissileMice" )
	else if ( m_owner->m_currentWeapon->m_weaponDef->m_name == "MissileMice" )
	{
		ExplodeMissileMice();
	}
	else if ( m_owner->m_currentWeapon->m_weaponDef->m_name == "DemonPoop" )
	{
		ExplodeDemonPoop();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::ExplodeMissileMice()
{
	// Loop through all actors
	for ( int i = 0; i < m_currentMap->m_actorList.size(); i++ )
	{
		// Check if this actor exists && ignore itself, the player, and spawn points
		if ( m_currentMap->m_actorList[i] == nullptr ||	
			m_currentMap->m_actorList[i]->m_actorDef->m_name == "Marine" ||
			m_currentMap->m_actorList[i]->m_actorDef->m_name == "MiceProjectile" ||
			m_currentMap->m_actorList[i]->m_actorDef->m_name == "SpawnPoint" )
		{
			continue;
		}

		// Check if enemies are within explosion radius
		float distance = GetDistance2D( Vec2( m_position.x, m_position.y ), Vec2( m_currentMap->m_actorList[i]->m_position.x, m_currentMap->m_actorList[i]->m_position.y) );
		if ( distance < m_actorDef->m_explosionRadius )
		{
			// Deal damage if enemies are within range of MissileMice		
			float randDamage = g_theRNG->RollRandomFloatInRange( m_actorDef->m_damageOnCollide.m_min, m_actorDef->m_damageOnCollide.m_max );
			m_currentMap->m_actorList[i]->TakeDamage( randDamage, this );
//			m_currentMap->m_actorList[i]->TakeDamage( randDamage, m_currentMap->m_player );
			
			// Render explosion
//			DebugAddWorldWireSphere( m_position, m_actorDef->m_explosionRadius, 0.0f, Rgba8::MAGENTA, Rgba8::MAGENTA );

			// Tell MissileMice projectile to die since duration elapsed
			PlayAnimationsByName( "Death" );
			m_isDead = true;

			// Play MissileMice explode sound
			m_missileMiceExplodeSPBID = g_theAudio->StartSound( m_missileMiceExplodeSID );
		}
	}

//	float randDamage = g_theRNG->RollRandomFloatInRange( m_actorDef->m_damageOnCollide.m_min, m_actorDef->m_damageOnCollide.m_max );
//	DebuggerPrintf( "Actor = %s, Damage = %f \n", m_actorDef->m_name.c_str(), randDamage );
}

//----------------------------------------------------------------------------------------------------------------------
void Actor::ExplodeDemonPoop()
{
	// Loop through all actors
	for ( int i = 0; i < m_currentMap->m_actorList.size(); i++ )
	{
		// Check if this actor exists
		if ( m_currentMap->m_actorList[i] == nullptr ||
			m_currentMap->m_actorList[i]->m_actorDef->m_name == "DemonPoopProjectile" || 
			m_currentMap->m_actorList[i]->m_actorDef->m_name == "RangedDemon" )
		{
			continue;
		}

		// Check if enemies are within explosion radius
		float distance = GetDistance2D( Vec2( m_position.x, m_position.y ), Vec2( m_currentMap->m_actorList[i]->m_position.x, m_currentMap->m_actorList[i]->m_position.y) );
		if ( distance < m_actorDef->m_explosionRadius )
		{
			// Deal damage if enemies are within range of Demon Poop		
			float randDamage = g_theRNG->RollRandomFloatInRange( m_actorDef->m_damageOnCollide.m_min, m_actorDef->m_damageOnCollide.m_max );
			m_currentMap->m_actorList[i]->TakeDamage( randDamage, this );
//			m_currentMap->m_actorList[i]->TakeDamage( randDamage, m_currentMap->m_player );

			// Render explosion
//			DebugAddWorldWireSphere( m_position, m_actorDef->m_explosionRadius, 0.0f, Rgba8::MAGENTA, Rgba8::MAGENTA );
	
			// Tell Demon Poop projectile to die since duration elapsed
			PlayAnimationsByName( "Death" );
			m_isDead = true;

			// Play demon poop explode sound
			m_demonPoopExplodeSPBID = g_theAudio->StartSound( m_demonPoopExplodeSID );


			if ( m_currentMap->m_actorList[i]->m_actorDef->m_name == "Marine" )
			{
				// Turn on Vertigo effect since player got hit by demon poop bomb
				m_currentMap->m_player->m_currentActor->m_drugsStopwatch = Stopwatch( m_durationOnDrugs );
				m_currentMap->m_player->m_currentActor->m_drugsStopwatch.Start();
				m_currentMap->m_player->m_currentActor->m_isOnDrugs		= true;
				m_currentMap->m_player->m_drugEffectIsActive			= true;
//				m_currentMap->m_player->m_worldCamera
			}
		}
	}

	//	float randDamage = g_theRNG->RollRandomFloatInRange( m_actorDef->m_damageOnCollide.m_min, m_actorDef->m_damageOnCollide.m_max );
	//	DebuggerPrintf( "Actor = %s, Damage = %f \n", m_actorDef->m_name.c_str(), randDamage );
}
