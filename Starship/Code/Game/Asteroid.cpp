#include "Game/GameCommon.hpp"
#include "Game/Asteroid.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------------------
Asteroid::Asteroid( Game* game, Vec2 position ) : Entity ( game, position )
{
	m_health = 3;

	m_ASTERIOD_COSMETIC_RADIUS = ASTERIOD_COSMETIC_RADIUS;
	m_ASTERIOD_PHYSICS_RADIUS = ASTERIOD_PHYSICS_RADIUS;

	m_angularVelocity = g_theRNG->RollRandomFloatInRange( -200, 200 );
	
	float randX = g_theRNG->RollRandomFloatInRange( -1.f, 1.f );
	float randY = g_theRNG->RollRandomFloatInRange( -1.f, 1.f );
	m_velocity = Vec2( randX, randY );
	m_velocity *= ASTERIOD_SPEED;
	
	initializelocalVerts();
}

//----------------------------------------------------------------------------------------------------------------------------------
Asteroid::~Asteroid()
{
}
 
//----------------------------------------------------------------------------------------------------------------------------------
void Asteroid::Update(float deltaSeconds)
{
	//add movement
	m_position += ( m_velocity * deltaSeconds );

	//add rotation
	m_orientationDegrees += ( m_angularVelocity * deltaSeconds );
}
  
//----------------------------------------------------------------------------------------------------------------------------------
void Asteroid::Render() const
{
	//add logic for isDead
	if (m_isDead) 
	{
		return;
	}

	Vertex_PCU tempWorldVerts[Num_Asteroid_Verts];

	for (int vertIndex = 0; vertIndex < Num_Asteroid_Verts; vertIndex++)
	{
		tempWorldVerts[vertIndex] = m_localVerts[vertIndex];
	}

	TransformVertexArrayXY3D( Num_Asteroid_Verts, tempWorldVerts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray( Num_Asteroid_Verts, tempWorldVerts );
}

//----------------------------------------------------------------------------------------------------------------------------------
void Asteroid::initializelocalVerts()
{
	// randNum for Asteroid radii
	float randomAsteroidRadii[Num_Asteroid_Verts] = {};

	for (int i = 0; i < Num_Asteroid_Verts; i++)
	{
		randomAsteroidRadii[i] = g_theRNG->RollRandomFloatInRange( m_ASTERIOD_PHYSICS_RADIUS, m_ASTERIOD_COSMETIC_RADIUS);
	}
	 
	// Calculate 2D vertex changes
	constexpr float degreesPerAsteroidSlice = 360.f / Num_Asteroid_Triangles_Slices;	// 22.5 degrees per "slice", 16 slices in 1 Asteroid
	Vec2 asteroidLocalVertPositions[Num_Asteroid_Triangles_Slices] = {};				// Stores position of 16 triangles & 48 random radii coords locally

	for (int slicesNum = 0; slicesNum < Num_Asteroid_Triangles_Slices; slicesNum++)
	{
		float degrees = degreesPerAsteroidSlice * (float) slicesNum;
		float radius = randomAsteroidRadii[slicesNum];

		asteroidLocalVertPositions[slicesNum].x = radius * CosDegrees( degrees );		// x = Radius * Cos(theta)
		asteroidLocalVertPositions[slicesNum].y = radius * SinDegrees( degrees );		// y = Radius * Sin(theta)
	}

	// Render triangles while < Num_Asteroid_Verts
	for (int triNum = 0; triNum < Num_Asteroid_Triangles_Slices; triNum++)
	{
		// counting "this" and "next" triangle radii
		int startRadiusIndex = triNum;
		int	endRadiusIndex	= (triNum + 1) % Num_Asteroid_Triangles_Slices;				// % makes vert 16 = position 0

		// counting increasing triangle verts
		int firstVertIndex	= (triNum * 3) + 0;
		int secondVertIndex = (triNum * 3) + 1;
		int thirdVertIndex	= (triNum * 3) + 2;

		// retrieves verts position
		Vec2 secondVertOfs	= asteroidLocalVertPositions[ startRadiusIndex ];
		Vec2 thirdVertOfs	= asteroidLocalVertPositions[ endRadiusIndex ];

		// assign coords to verts in each triangle
		m_localVerts[firstVertIndex].m_position  = Vec3( 0.f, 0.f, 0.f );
		m_localVerts[secondVertIndex].m_position = Vec3( secondVertOfs.x, secondVertOfs.y, 0.f );
		m_localVerts[thirdVertIndex].m_position	 = Vec3( thirdVertOfs.x, thirdVertOfs.y, 0.f );
	}

	// Set colors for each triangle
	for (int vertIndex = 0; vertIndex < Num_Asteroid_Verts; vertIndex++)
	{
		m_localVerts[vertIndex].m_color = Rgba8( 100, 100, 100 );
	}
 }