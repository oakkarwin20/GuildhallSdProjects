#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/EngineCommon.hpp"

//----------------------------------------------------------------------------------------------------------------------
Bullet::Bullet( Map* currentMap, Vec2 position, float orientation, EntityType type )
	: Entity( currentMap, position, orientation, type)
{
	m_isPushedByWalls		= false;
	m_isPushedByEntities	= false;
	m_doesPushEntities		= false;
	m_hitByBullet			= false;

	m_physicsRadius = 0.0f;
	m_cosmeticRadius = 1.4f;

	m_bulletTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyBullet.png" );
}

//----------------------------------------------------------------------------------------------------------------------
void Bullet::Update( float deltaSeconds )
{
	m_velocity = GetFowardNormal() * BULLET_MAX_SPEED * deltaSeconds;
	// check if bullet hits walls

	// Check bullet collision with walls
	//	1. do a for loop, check for all entities of type bullet, if their point is inside a wall, if so bounce 3 times then die, if not, return

	Vec2 nextPos = m_position + ( m_velocity * deltaSeconds );

	if ( m_map->IsPointInSolid( nextPos ) )
	{
		IntVec2 nextTile	= m_map->GetTileCoordsForWorldPos( nextPos );
		IntVec2 currentTIle = m_map->GetTileCoordsForWorldPos( m_position );

		Vec2 normalizedBounce = Vec2(nextTile - currentTIle).GetNormalized();
		
		Bounce( normalizedBounce );
	}
	else
	{
		m_position = nextPos;
	}

	Entity::Update( deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void Bullet::Render() const
{
	std::vector<Vertex_PCU> verts;
	AABB2 bounds = AABB2( Vec2( -0.15f, -0.05f ), Vec2( 0.15f, 0.05f ) );

	SpriteSheet spriteSheet( *m_bulletTexture, IntVec2( 1, 1 ) );
	Rgba8 whiteColor = Rgba8( 255, 255, 255, 255 );
	AABB2 UVs = spriteSheet.GetSpriteUVs( 0 );

	AddVertsForAABB2D( verts, bounds, whiteColor, UVs );
	TransformVertexArrayXY3D( (int)verts.size(), verts.data(), 1.0f, m_orientationDegrees, m_position );

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( m_bulletTexture );
	g_theRenderer->DrawVertexArray( (int)verts.size(), verts.data() );
}

//#ToDo Oak, you need to finish adding the reflect function then implement that into bullet bounce.
// also check w Prof is everything related to this function was written correctly
//----------------------------------------------------------------------------------------------------------------------
void Bullet::Bounce( Vec2& normalized )
{
	UNUSED( normalized );
//	m_position = nextPos - currentPos;
//	normalized

//	normalized.Reflect( );
}


