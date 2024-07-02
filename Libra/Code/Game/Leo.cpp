#include "Game/Leo.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"

Leo::Leo( Map* currentMap, Vec2 position, float orientation, EntityType type )
	: Entity( currentMap, position, orientation, type  )
{
	m_isPushedByWalls		= true;
	m_isPushedByEntities	= true;
	m_doesPushEntities		= true;
	m_hitByBullet			= true;

	m_physicsRadius = 0.3f;
	m_cosmeticRadius = 1.4f;

	m_leoTankTexture	= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyTank4.png" );
}

void Leo::Update( float deltaSeconds )
{
	Vec2 m_leoMoveIntention = Vec2( -1.0f, -1.0f );

	m_orientationDegrees = -120.0f;

	float m_leoGoalOrientationDegrees = m_leoMoveIntention.GetLength();
	m_orientationDegrees = GetTurnedTowardDegrees( m_orientationDegrees, m_leoGoalOrientationDegrees, ( PLAYER_TANK_TURN_SPEED * deltaSeconds ) );

	m_velocity = GetFowardNormal() * ( PLAYER_TANK_MAX_SPEED * m_leoMoveIntention.GetLength() );

	m_position += m_velocity * deltaSeconds;
}

void Leo::Render() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render Leo Base
	std::vector<Vertex_PCU> verts;
	AABB2 bounds = AABB2( Vec2( -0.5f, -0.5f ), Vec2( 0.5f, 0.5f ) );

	SpriteSheet spriteSheet( *m_leoTankTexture, IntVec2( 1, 1 ) );
	Rgba8 whiteColor = Rgba8( 255, 255, 255, 255 );
	AABB2 UVs = spriteSheet.GetSpriteUVs( 0 );

	AddVertsForAABB2D( verts, bounds, whiteColor, UVs );
	TransformVertexArrayXY3D( (int)verts.size(), verts.data(), 1.0f, m_orientationDegrees, m_position );
	g_theRenderer->BindTexture( m_leoTankTexture );
	g_theRenderer->DrawVertexArray( (int)verts.size(), verts.data() );
}