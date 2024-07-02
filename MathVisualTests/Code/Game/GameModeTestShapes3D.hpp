#pragma once

#include "Game/GameModeBase.hpp"

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/FloatRange.hpp"

//----------------------------------------------------------------------------------------------------------------------
class GameModeTestShapes3D : public GameModeBase
{
public:
	GameModeTestShapes3D();
	virtual ~GameModeTestShapes3D();

	virtual void Startup()						override;
	virtual void Update( float deltaSeconds )	override;
	virtual void Render() const					override;
	virtual void Reshuffle()					override;
	virtual void Shutdown()						override;

	// Functions
	void UpdateGameCameraTestShapes3D();
	void UpdateInputForCameraMovement( float deltaSeconds );
	void RenderUIStuff() const;
	void UpdatePauseQuitAndSlowMo();
	void AddVertsForCompass( std::vector<Vertex_PCU>& compassVerts, Vec3 position, float axisLength, float axisThickness ) const;

	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	Camera m_testShapesWorldCamera;
	Camera m_testShapesUICamera;

	// Camera / Player movement variables
	float			m_turnRate				= 90.0f;
	float			m_defaultSpeed			= 4.0f;
	float			m_speed					= 4.0f;
	float			m_doublespeed			= m_speed * 4.0f;
	float			m_halfspeed				= m_speed * 0.5f;
	float			m_elevateSpeed			= 6.0f;
	EulerAngles		m_angularVelocity;
	EulerAngles		m_orientationDegrees;
	Texture*		m_testTexture			= nullptr;

	//----------------------------------------------------------------------------------------------------------------------
	// Object variables
	// Sphere
	Vec3 m_centerPos	= Vec3( 10.0f,  1.0f, 0.0f );
	Vec3 m_centerPos2	= Vec3( 20.0f, 10.0f, 0.0f );
	float m_radius		= 2.0f;
	float m_numSlices	= 16.0f;
	float m_numStacks	= 8.0f;

	// Quad
	AABB3 m_bounds = AABB3( Vec3( 10.0f, 10.0f, 10.0f ), Vec3( 20.0f, 20.0f, 20.0f ) );

	// Wireframe Spheres
	Vec3 m_wireframeCenterPos	= Vec3( 5.0f,   30.0f, 10.0f );
	Vec3 m_wireframeCenterPos2	= Vec3( 40.0f, -10.0f, 0.0f );

	// Wireframe Quad
	AABB3 m_wireframeBounds = AABB3( Vec3( 40.0f, 40.0f, -15.0f ), Vec3( 50.0f, 70.0f, -5.0f ) );

	// Cylinder
	Vec2		m_centerXY			= Vec2( 10.0f, -10.0f );
	FloatRange	m_minMaxZ			= FloatRange( 0.0f, 10.0f );
	float		m_cylinderRadius	= 5.0f;
	float		m_cylinderNumSlices	= 18.0f;

	// Wireframe Cylinder
	Vec2		m_wireFrameCenterXY			 = Vec2( 40.0f, 10.0f );
	FloatRange	m_wireFrameMinMaxZ			 = FloatRange( 0.0f, 10.0f );
	float		m_wireFrameCylinderRadius	 = 5.0f;
	float		m_wireFrameCylinderNumSlices = 18.0f;

public:
	Clock		m_clock;
	bool		m_isSlowMo = false;
};