#pragma once

#include "Engine/Renderer/Camera.hpp"

#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class Football2D;

//----------------------------------------------------------------------------------------------------------------------
class Player2D
{
public:
	Player2D( Football2D* football );
	~Player2D();
	virtual void Update( float deltaSeconds );
	virtual void Render( std::vector<Vertex_PCU>& outVerts );

	// Physics
	void UpdatePhysics( float deltaSeconds );
	void AddForce( Vec2 forceAmount );
	void MoveInDirection( Vec2 directionToMove, float speed );

	// Football Functions
	void UpdateFootballPhysics();
	
	// Input
	void UpdatePlayerInput( float deltaSeconds );
	
	// Clamp to world bounds
	void UpdateClampToWorldBounds();
	

public:
	// Core Variables
//	Vec3	m_playerPosition				= Vec3::ZERO;
	Vec2	m_playerPosition				= Vec2::ZERO;
	Vec2	m_playerVelocity				= Vec2::ZERO;
	Vec2	m_playerForwardDir				= Vec2::ZERO;
	Vec2	m_playerAcceleration			= Vec2::ZERO;
	
	// Orientation
	float	m_playerOrientation				= 0.0f;
	float	m_playerGoalOrientationDegrees	= 0.0f;
	float	m_playerTurnRate				= 1440.0f;
	
	// Speed
	float	m_playerDefaultSpeed			= 5.0f;
	float	m_playerCurrentSpeed			= m_playerDefaultSpeed;
	float	m_playerSlowSpeed				= 2.0f;
	float	m_playerMaxSpeed				= 20.0f;
	float	m_playerDrag					= 0.5f;

	// Physics 
	float	m_playerPhysicsRadius			= 2.0f;
	float	m_playerPossessionRadius		= m_playerPhysicsRadius * 2.5f;

	// Ball Possession Variables
	bool		m_ballIsPossessed			= false;
	bool		m_ballIsInPossessionRange	= false;
	Football2D*	m_football					= nullptr;
	bool		m_shootTheBall				= false;
	bool		m_passTheBall				= false;
	float		m_footballDribbleSpeed		= m_playerCurrentSpeed * 2.0f;
	float		m_footballShootSpeed		= 600.0f;
	float		m_footballPassSpeed			= 300.0f;

private:
	Rgba8	m_playerColor				= Rgba8::MAGENTA;
	Rgba8	m_playerPhysicsRadiusColor	= Rgba8::DARK_GRAY;
	float	m_arrowSize					= 2.0f;
	float	m_lineThickness				= 0.5f;
	Camera	m_worldCamera;
};