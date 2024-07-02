#include "Game/Scorpio.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

Scorpio::Scorpio(  Map* currentMap, Vec2 position, float orientationDegrees, EntityType type )
	: Entity( currentMap, position, orientationDegrees, type )
{
	m_isPushedByWalls		= true;
	m_isPushedByEntities	= false;
	m_doesPushEntities		= true;
	m_hitByBullet			= true;

	m_physicsRadius = 0.3f;
	m_cosmeticRadius = 1.4f;

	m_scorpioTankTexture	= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTurretBase.png" );
	m_scorpioTurretTexture	= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyCannon.png" );
}

// #ToDo Oak u need to add the logic for this function, it's currently incomplete
void Scorpio::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	// test if physics work
	// if yes, move code to leo, change scorpio functionality
	
	// randomize orientation
	// float randNum = g_theRNG->RollRandomFloatInRange( 0.5f, 1.0f );
	// m_orientationDegrees *= randNum;
}

void Scorpio::Render() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render Scorpio Base
	std::vector<Vertex_PCU> verts;
	AABB2 bounds = AABB2( Vec2( -0.5f, -0.5f ), Vec2(  0.5f, 0.5f ) );

	SpriteSheet spriteSheet( *m_scorpioTankTexture, IntVec2( 1, 1 ) );
	Rgba8 tint = Rgba8( 255, 0, 0, 255 );
	AABB2 UVs = spriteSheet.GetSpriteUVs( 0 );

	AddVertsForAABB2D( verts, bounds, tint, UVs );
	TransformVertexArrayXY3D( (int)verts.size(), verts.data(), 1.0f, m_orientationDegrees, m_position );
	

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( m_scorpioTankTexture );
	g_theRenderer->DrawVertexArray( (int)verts.size(), verts.data() );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Scorpio Turret
	AABB2 turretBounds = AABB2( Vec2( -0.5f, -0.5f ), Vec2( 0.5f, 0.5f ) );
	
	std::vector<Vertex_PCU> verts2;

	SpriteSheet spriteSheet2( *m_scorpioTurretTexture, IntVec2( 1, 1 ) );
	
	Vec2 playerPos = m_map->GetPlayerPosition();
	if ( m_map->HasLineOfSight( m_position, playerPos ) )
	{
		// green color
		tint = Rgba8( 0, 0, 0, 255 );
	}

	AddVertsForAABB2D( verts2, bounds, tint, UVs );
	TransformVertexArrayXY3D( (int)verts2.size(), verts2.data(), 1.0f, m_orientationDegrees, m_position );

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( m_scorpioTurretTexture );
	g_theRenderer->DrawVertexArray( (int)verts2.size(), verts2.data() );
}