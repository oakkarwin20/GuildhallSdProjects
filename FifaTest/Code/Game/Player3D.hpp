#pragma once

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class Football3D;
class GameModeFifaTest3D;

//----------------------------------------------------------------------------------------------------------------------
class Player3D
{
public:
	Player3D( Football3D* football, GameModeFifaTest3D* gameMode3D );
	~Player3D();
	virtual void Update( float deltaSeconds );
	virtual void Render( std::vector<Vertex_PCU>& outVerts );

	// Physics
	void UpdatePhysics( float deltaSeconds );
	void AddForce( Vec2 forceAmount );
	void MoveInDirection( Vec2 directionToMove, float speed );
	void ApplyForceRandAndClampedInDir( Vec3 direction, float speed );

	// Football Functions
	void UpdateFootballPossessionAndInput();
	
	// Input
	void UpdatePlayerInput( float deltaSeconds );
	
	// Clamp to world bounds
	void UpdateClampToWorldBounds();

public:
	// Player possession variables
	bool m_playerIsControlled = false;

	// Core Variables
	Vec3	m_position				= Vec3::ZERO;
	Vec3	m_velocity				= Vec3::ZERO;
	Vec3	m_forwardDir			= Vec3::ZERO;
	Vec3	m_acceleration			= Vec3::ZERO;
	
	// Orientation
	float	m_playerOrientation			= 0.0f;
	float	m_goalOrientationDegrees	= 0.0f;
	float	m_turnRate					= 2880.0f;
	
	// Speed
	float	m_defaultSpeed				= 5.0f;
	float	m_currentSpeed				= m_defaultSpeed;
	float	m_slowSpeed					= m_defaultSpeed * 0.5f;
	float	m_maxSpeed					= m_defaultSpeed * 2.0f;
	float	m_drag						= 1.0f;
	float	m_sprintingAcceleration		= 50.0f;

	// Physics 
	float	m_physicsRadius				= 0.41f;
	float	m_playerIsControlledRadius	= m_physicsRadius * 3.5f;

	// Player physicality variables
	float	m_height	= 1.8f;

	// Ball Possession Variables
	bool		m_hasTheBall						= false;
	bool		m_ballIsInPossessionRange		= false;
	float		m_ballManipulateRadius			= m_physicsRadius * 4.0f;
	Rgba8		m_ballPossessionRangeColor		= Rgba8::BLACK;
	Football3D*	m_football						= nullptr;
	bool		m_isDribblingTheBall			= false;
	bool		m_isReceivingTheBall			= false;
	bool		m_shouldTakeATouch				= false;
	float		m_footballDefaultDribbleSpeed	= 0.35f;
	float		m_footballCurrentDribbleSpeed	= 0.35f;
	float		m_footballSprintSpeed			= m_footballCurrentDribbleSpeed * 2.5f;
	float		m_footballSlowSpeed				= m_footballCurrentDribbleSpeed * 0.5f;
	float		m_footballShootSpeed			= 600.0f;
	float		m_footballGroundPassSpeed		= 15.0f;
	float		m_footballAirPassSpeed			= 15.0f;

	// Debug variables
	float	m_debugRadius			= m_physicsRadius * 2.0f;
	float	m_debugLineThickness	= 1.0f;
	float	m_debugArrowSize		= 5.0f;
	Rgba8	m_color					= Rgba8::MAGENTA;

private:
	Rgba8	m_physicsRadiusColor	= Rgba8::DARK_GRAY;
	float	m_arrowSize				= 2.0f;
	float	m_lineThickness			= 0.5f;
	Camera	m_worldCamera;
	GameModeFifaTest3D* m_gameMode3D = nullptr;
};