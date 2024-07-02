#pragma once

#include "Game/GameModeBase.hpp"

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
class GameModeRaycastVsAABB3D : public GameModeBase
{
public:
	GameModeRaycastVsAABB3D();
	virtual ~GameModeRaycastVsAABB3D();

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

	// Raycasting Functions
	void UpdateRaycastResult3D();
	void MoveRaycastInput( float deltaSeconds );

public:
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
	// Quad
	AABB3* m_bounds = new AABB3( Vec3( 10.0f, 10.0f, 10.0f ), Vec3( 20.0f, 20.0f, 20.0f ) );

	// Wireframe Quad
	int	  m_numBoxes		= 10;
	AABB3 m_wireframeBounds = AABB3( Vec3( 40.0f, 40.0f, -15.0f ), Vec3( 50.0f, 70.0f, -5.0f ) );

	// Raycasting Variables
	RaycastResult3D		m_raycastVsAABB2Result3D;
	Vec3				m_updatedImpactPos		= Vec3::ZERO;
	Vec3				m_updatedImpactNormal	= Vec3::ZERO;
	Vec3				m_rayStartPos			= Vec3::ZERO;
	Vec3				m_rayEndPos				= Vec3::ZERO;
	bool				m_didAABB3Impact		= false;
	float				m_arrowSize				= 1.5f;
	float				m_arrowThickness		= 0.1f;
	int					m_currentLine			= -1;
	std::vector<AABB3*>	m_aabb3List;

	Rgba8				m_rayDefaultColor		= Rgba8::MAGENTA;
	Rgba8				m_rayImpactDistColor	= Rgba8::RED;
	Rgba8				m_rayAfterImpactColor	= Rgba8::GRAY;
	Rgba8				m_rayImpactDiscColor	= Rgba8::WHITE;
	Rgba8				m_rayImpactNormalColor	= Rgba8::DARK_BLUE;


public:
	Clock		m_clock;
	bool		m_isSlowMo = false;
};