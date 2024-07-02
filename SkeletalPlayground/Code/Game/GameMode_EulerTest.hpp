#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/SkeletalSystem/IK_Joint3D.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/CubicBezierCurve3D.hpp"
#include "Engine/Core/Stopwatch.hpp"


//----------------------------------------------------------------------------------------------------------------------
class Texture;
class IK_Chain3D;
class IK_Joint3D;
class CreatureBase;


//----------------------------------------------------------------------------------------------------------------------
class GameMode_EulerTest : public GameModeBase
{
public:
	GameMode_EulerTest();
	virtual ~GameMode_EulerTest();
	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Shutdown();

	// Camera and Render Functions
	void UpdateDebugKeys();
	void UpdateCameraInput( float deltaSeconds );
	void UpdateGameMode3DCamera();
	void RenderWorldObjects()	const;
	void RenderUIObjects()		const;

	// Debug Functions
	void AddVertsForWorldBasis( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness, Rgba8 fwdColor = Rgba8::RED, Rgba8 leftColor = Rgba8::GREEN, Rgba8 upColor = Rgba8::BLUE ) const;
	void RenderControlledBasis( std::vector<Vertex_PCU>& verts, Vec3 const& basisStart, float axisLength, float axisThickness ) const; 
	void RenderComputedBasis_MatrixToEuler( std::vector<Vertex_PCU>& verts, EulerAngles const& eulerToUse, Vec3 const& basisStart, float axisLength, float axisThickness ) const; 
	void UpdateInputDebugPosEE( float deltaSeconds );


public:
	//----------------------------------------------------------------------------------------------------------------------
	// Core Variables
	//----------------------------------------------------------------------------------------------------------------------
	float m_sine						= 0.0f;
	float m_currentTime					= 0.0f;
	float m_walkLerpSpeed				= 4.0f;
	float m_sprintLerpSpeed				= m_walkLerpSpeed * 4.0f;
	float m_goalWalkLerpSpeed			= m_walkLerpSpeed;
	float m_currentWalkLerpSpeed		= m_goalWalkLerpSpeed;

	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	//----------------------------------------------------------------------------------------------------------------------
	Camera	m_gameMode3DWorldCamera;
	Camera	m_gameMode3DUICamera;
	// Camera movement Variables
	float	m_defaultSpeed	= 60.0f;
	float	m_currentSpeed	= m_defaultSpeed;
	float	m_fasterSpeed	= m_defaultSpeed * 8.0f;
	float	m_elevateSpeed  = 6.0f;
	float	m_turnRate		= 90.0f;


	//----------------------------------------------------------------------------------------------------------------------
	// Tree Variables
	//----------------------------------------------------------------------------------------------------------------------
	IK_Chain3D* m_treeBranch1			= nullptr;
	float		m_numTreeSegments		= 3.0f;
	float		m_limbLength			= 10.0f;
	float		m_halfLimbLength		= m_limbLength * 0.5f;

	//----------------------------------------------------------------------------------------------------------------------
	// Debug variables
	//----------------------------------------------------------------------------------------------------------------------
	float m_distCamAwayFromPlayer			= 180.0f;
	Vec3  m_debugTargetPos					= Vec3::ZERO;
	float m_debugTargetAngle_PolarCoords	= 0.0f;
	float m_debugTargetLength_PolarCoords	= 0.0f;
	bool  m_debugStepOneFrame				= false;

	//----------------------------------------------------------------------------------------------------------------------
	// Creature variables
	//----------------------------------------------------------------------------------------------------------------------
	CreatureBase*				m_creature				= nullptr;
	std::vector<IK_Chain3D*>	m_creatureSkeletalSystemsList;
	IK_Chain3D*					m_leftFoot				= nullptr;
	IK_Chain3D*					m_rightFoot				= nullptr;

	float	m_maxLength				= 0.0f;

	float	m_numFeet				=  5.0f;
	float	m_rootDefaultHeightZ	= 20.0f;


	//----------------------------------------------------------------------------------------------------------------------
	// Bezier Curves
	//----------------------------------------------------------------------------------------------------------------------
	CubicBezierCurve3D  m_bezierCurve	= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_timer			= Stopwatch( &g_theApp->m_gameClock, 1.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Hand
	//----------------------------------------------------------------------------------------------------------------------
	IK_Chain3D*		m_hand			= nullptr;
	IK_Chain3D*		m_thumb			= nullptr;
	IK_Chain3D*		m_indexFinger	= nullptr;
	IK_Chain3D*		m_middleFinger	= nullptr;
	IK_Chain3D*		m_ringFinger	= nullptr;
	IK_Chain3D*		m_pinkyFinger	= nullptr;

	// Euler Angles for yaw and pitch preferred orientations
//	EulerAngles m_eulerControlled								= EulerAngles( 135, 45, 0 );
	EulerAngles m_eulerControlled								= EulerAngles( 45, -20, 0 );
	EulerAngles m_jointEuler_FwdTemporal				= EulerAngles(   0, 0, 0 );
	EulerAngles m_jointEuler_FwdAndLeft					= EulerAngles(   0, 0, 0 );
	EulerAngles m_jointEuler_FwdAndLeftTemporal			= EulerAngles(   0, 0, 0 );
	EulerAngles m_jointEuler_FwdAndLeftAlternate		= EulerAngles(   0, 0, 0 );
	EulerAngles m_jointEuler_FwdAndLeftCustom			= EulerAngles(   0, 0, 0 );
	EulerAngles m_jointEuler_FwdOnlyYawPrefered			= EulerAngles(	 0, 0, 0 );
	EulerAngles m_jointEuler_FwdOnlyPitchPrefered		= EulerAngles(	 0, 0, 0 );
	// Last frame's euler angles. Have to make copies in game code since joint typically only has one copy per joint
	EulerAngles m_lastFrameEuler_FwdTemporal			= EulerAngles(   0, 0, 0 );
	EulerAngles m_lastFrameEuler_FwdAndLeft				= EulerAngles(   0, 0, 0 );
	EulerAngles m_lastFrameEuler_FwdAndLeftTemporal		= EulerAngles(   0, 0, 0 );
	EulerAngles m_lastFrameEuler_FwdAndLeftAlternate	= EulerAngles(   0, 0, 0 );
	EulerAngles m_lastFrameEuler_FwdAndLeftCusom		= EulerAngles(   0, 0, 0 );
	EulerAngles m_lastFrameEuler_FwdOnlyYawPrefered		= EulerAngles(	 0, 0, 0 );
	EulerAngles m_lastFrameEuler_FwdOnlyPitchPrefered	= EulerAngles(	 0, 0, 0 );
	IK_Joint3D  m_joint									= IK_Joint3D( 0, Vec3::ZERO, 100.0f );
};
