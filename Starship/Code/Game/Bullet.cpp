#include "Game/Game.hpp"
#include "Game/Bullet.hpp"
#include "Game/App.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

Bullet::Bullet( Game* game, Vec2 spawnPosition ) 
	: Entity( game, spawnPosition ) 
{
	/*float numX = 
	m_velocity = Vec2( PlayerShip::m_position.x, PlayerShip::m_position.y );*/

	m_BULLET_COSMETIC_RADIUS = BULLET_COSMETIC_RADIUS;
	m_BULLET_PHYSICS_RADIUS = BULLET_PHYSICS_RADIUS;
	m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees, 1.0f ) * BULLET_SPEED;

	InitializeLocalVerts();
}

Bullet::~Bullet()
{
}

void Bullet::Update(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed(' '))
	{
		Vec2 fowardDirection = Vec2( 1.0f, 0.0f );
		fowardDirection.SetOrientationDegrees( m_orientationDegrees );
		m_velocity += (fowardDirection * BULLET_SPEED * deltaSeconds);
	}
		m_position += (m_velocity * deltaSeconds);
}

void Bullet::Render() const
{
	Vertex_PCU tempWorldVerts [Num_Bullet_Verts] = {};

	for (int vertIndex = 0; vertIndex < Num_Bullet_Verts; vertIndex++)
	{
		tempWorldVerts[vertIndex] = m_localVerts[vertIndex];
	}

	TransformVertexArrayXY3D( Num_Bullet_Verts, tempWorldVerts, 1.f, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray( Num_Bullet_Verts, tempWorldVerts );
}

void Bullet::InitializeLocalVerts()
{
	// Nose cone
	m_localVerts[0].m_position	= Vec3(  0.f, -0.5f, 0.f);		 //Triangle A, point A
	m_localVerts[0].m_color		= Rgba8( 255, 0, 0, 255);
	m_localVerts[1].m_position	= Vec3(  0.5f, 0.f,  0.f);		 //Triangle A, point B
	m_localVerts[1].m_color		= Rgba8( 255, 0, 0, 255);
	m_localVerts[2].m_position	= Vec3(  0.f,  0.5f, 0.f);		 //Triangle A, point C
	m_localVerts[2].m_color		= Rgba8( 255, 0, 0, 0);

	// Tail
	m_localVerts[3].m_position	= Vec3( -2.f,  0.f,  0.f );		//Triangle B, point A
	m_localVerts[3].m_color		= Rgba8( 255, 255, 255, 255);		
	m_localVerts[4].m_position	= Vec3(  0.f, -0.5f, 0.f );		//Triangle B, point B
	m_localVerts[4].m_color		= Rgba8( 255, 255, 0, 255);		
	m_localVerts[5].m_position	= Vec3(  0.f,  0.5f, 0.f );		//Triangle B, point C
	m_localVerts[5].m_color		= Rgba8( 255, 255, 0, 255);		
}

