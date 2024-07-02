#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class Football2D
{
public:
	Football2D();
	~Football2D();

	void Update( float deltaSeconds );
	void Render( std::vector<Vertex_PCU>& outVerts ) const;
	
	// Physics Functions
	void UpdatePhysics( float deltaSeconds );
	void MoveInDirection( Vec2 directionToMove, float speed );
	void AddForce( Vec2 forceAmount );
	void UpdateClampToWorldBounds();

public:
	Vec2  m_footballPosition	= Vec2( -1.0f, -1.0f );
	Vec2  m_footballVelocity	= Vec2(	 0.0f,  0.0f );
	float m_footballRadius		= 1.0f;
	float m_footballElasticity	= 0.9f;
	Rgba8 m_footballColor		= Rgba8::BROWN;

private:
	float m_footballDrag			= 2.0f;
	Vec2  m_footballAcceleration	= Vec2::ZERO;
};