#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Jellyfish.hpp"

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
class Quadruped;
class SpiderBase;
class Map_GameMode3D;

//----------------------------------------------------------------------------------------------------------------------
enum class ControlledCreatureType
{
	CONTROLLED_QUADRUPED,
	CONTROLLED_SPIDER,
	CONTROLLED_CAMERA_AI,		// Creatures follow the jellyfish
//	CONTROLLED_CAMERA_FOLLOW,	// Creatures follow the camera
};


//----------------------------------------------------------------------------------------------------------------------
struct Raycast_old
{
	RaycastResult3D		m_raycastResult			= RaycastResult3D();
	Vec3				m_updatedImpactPos		= Vec3::ZERO;
	Vec3				m_updatedImpactNormal	= Vec3::ZERO;
	Vec3				m_rayStartPos			= Vec3::ZERO;
	Vec3				m_rayEndPos				= Vec3::ZERO;
	bool				m_didRayImpact			= false;
};


//----------------------------------------------------------------------------------------------------------------------
class GameMode3D : public GameModeBase
{
public:
	GameMode3D();
	virtual ~GameMode3D();
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
	void AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const;
	void UpdateDebugTreePosInput();

	// Creature Functions
	void InitializeCreatures();
	void RenderCreature( std::vector<Vertex_PCU>& verts, 
						 std::vector<Vertex_PCU>& verts_texturedCreature, 
						 std::vector<Vertex_PCU>& textVerts, 
						 std::vector<Vertex_PCU>& verts_BackFace,
						 std::vector<Vertex_PCU>& verts_ikChainBody 
						) const;
	void UpdateCameraAndCreatureInput( float deltaSeconds );
	void TurnCreatureTowardsCameraDir();

	// Environment
	void InitializeEnvironment();

	// Raycast Functions
	void RenderRaycasts( std::vector<Vertex_PCU>& verts ) const;
//	void UpdateRaycastResult3D();
	void MoveRaycastInput( float deltaSeconds );
	bool DidRaycastHitTriangle( RaycastResult3D& raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal );

	//----------------------------------------------------------------------------------------------------------------------
	// Jellyfish functions
	//----------------------------------------------------------------------------------------------------------------------
	void InitJellyfish();


public:
	// Creature pointers
	Jellyfish*  m_jellyfish  = nullptr;
	Quadruped*  m_quadruped  = nullptr;
	SpiderBase* m_spiderBase = nullptr;
	float m_rootDefaultHeightZ_spider = 6.0f;
	ControlledCreatureType m_controlledCreatureType = ControlledCreatureType::CONTROLLED_CAMERA_AI;

	// Debug rayVsTri
	RaycastResult3D m_rayVsTri;

	//----------------------------------------------------------------------------------------------------------------------
	// Core Variables
	//----------------------------------------------------------------------------------------------------------------------
	Map_GameMode3D* m_map					= nullptr;
	float			m_sine					= 0.0f;
	float			m_currentTime			= 0.0f;
	float			m_walkLerpSpeed			= 6.0f;
	float			m_sprintLerpSpeed		= m_walkLerpSpeed * 20.0f;
	float			m_goalWalkLerpSpeed		= m_walkLerpSpeed;
	float			m_currentWalkLerpSpeed	= m_goalWalkLerpSpeed;

	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	//----------------------------------------------------------------------------------------------------------------------
	Camera	m_gameMode3DWorldCamera;
	Camera	m_gameMode3DUICamera;
	// Camera movement Variables
	float	m_defaultSpeed  = 10.0f;
	float	m_currentSpeed  = 10.0f;
	float	m_fasterSpeed	= m_defaultSpeed * 2.0f;
	float	m_elevateSpeed  = 6.0f;
	float	m_turnRate		= 90.0f;

	// Limb length/count/segment Variables
 	float m_limbLength_OldCreature	= 10.0f;
	float m_numArms					=  4.0f;
	float m_numFeet					=  4.0f;
	float m_numHips					=  3.0f;
	float m_numTailSegments			=  5.0f;
	float m_numNeckSegments			=  1.0f;
	float m_rootDefaultHeightZ		= 15.0f;
	float m_maxArmLength			= m_limbLength_OldCreature * m_numArms;
	float m_maxFeetLength			= m_limbLength_OldCreature * m_numFeet;

	//----------------------------------------------------------------------------------------------------------------------
	// Creature Limb placement variables
	//----------------------------------------------------------------------------------------------------------------------
	bool m_isSprinting		= false;

	Vec3 m_hipFwdDir		= Vec3::ZERO;;
	Vec3 m_hipLeftDir		= Vec3::ZERO;;
	Vec3 m_loweredHipPos	= Vec3::ZERO;;

	//----------------------------------------------------------------------------------------------------------------------
	// Anchor variables
	//----------------------------------------------------------------------------------------------------------------------
	bool m_rightArmAnchored	= false;
	bool m_leftArmAnchored	= true;

	//----------------------------------------------------------------------------------------------------------------------
	// Creature Movement mechanic
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 m_moveFwdDir = Vec3::ZERO;
	bool m_isClimbing = false;

	//----------------------------------------------------------------------------------------------------------------------
	// Tree Variables
	//----------------------------------------------------------------------------------------------------------------------
	CreatureBase*	  m_treeCreature		= nullptr;
	IK_Chain3D*		  m_treeBranch1			= nullptr;
	IK_Chain3D*		  m_treeBranch2			= nullptr;
	IK_Chain3D*		  m_treeBranch3			= nullptr;
	Vec3			  m_treeDebugTargetPos	= Vec3::ZERO;
	float			  m_numTreeSegments		= 2.0f;


	//----------------------------------------------------------------------------------------------------------------------
	// Raycast Result
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// Mount Height Ray
	Raycast_old m_raycast_Mount;
	Raycast_old m_raycast_FWD;
	//----------------------------------------------------------------------------------------------------------------------
	// Movement 
	// Fwd Ray
	RaycastResult3D		m_raycastResult_FWD					= RaycastResult3D();		
	Vec3				m_updatedImpactPos_FWD				= Vec3::ZERO;
	Vec3				m_updatedImpactNormal_FWD			= Vec3::ZERO;
	Vec3				m_rayStartPos_FWD					= Vec3::ZERO;
	Vec3				m_rayEndPos_FWD						= Vec3::ZERO;
	bool				m_didRayImpact_FWD					= false;
	bool				m_didRayImpactClimbableObject_FWD	= false;
	//----------------------------------------------------------------------------------------------------------------------
	// Left arm. 
	Raycast_old m_raycast_LeftArmDown;		
	// "LAF" means "Left arm forward"
	Raycast_old m_raycast_LeftArmFwd;
	// "NLAD" means "Next Left arm down"
	Raycast_old m_raycast_NextLeftArmDown;
	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm. 
	// "RAD" means "right arm down"
	Raycast_old m_raycast_rightArmDown;
	// "RAF" means "right arm forward"
	Raycast_old m_raycast_rightArmFwd;
	// NRAD (Next Right Arm Down)
	Raycast_old m_raycast_NextRightArmDown;	
	//----------------------------------------------------------------------------------------------------------------------
	// Right Feet
	// "RFD" means "right foot down"
	Raycast_old m_raycast_rightFootDown;	
	// "RFF" means "right foot fwd"
	Raycast_old m_raycast_rightFootFwd;		
	//----------------------------------------------------------------------------------------------------------------------
	// Left Foot
	// "LFD" means "left Foot down"
	Raycast_old m_raycast_leftFootDown;	
	// "LFF" means "left foot fwd"
	Raycast_old m_raycast_LeftFootFwd;		
	//----------------------------------------------------------------------------------------------------------------------
	// Common raycast variables
	float				m_raylength_Long			= 100.0f;
	float				m_raylength_Short			= 25.0f;
	float				m_arrowThickness			=  0.5f;
	Rgba8				m_rayDefaultColor			= Rgba8::MAGENTA;
	Rgba8				m_rayImpactDistColor		= Rgba8::RED;
	Rgba8				m_rayImpactDistColor2		= Rgba8::DARKER_GREEN;
	Rgba8				m_rayImpactDiscColor		= Rgba8::WHITE;
	Rgba8				m_rayImpactNormalColor		= Rgba8::DARK_BLUE;

	//----------------------------------------------------------------------------------------------------------------------
	// Debug variables
	//----------------------------------------------------------------------------------------------------------------------
	float m_distCamAwayFromPlayer = 50.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Bezier Curves
	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm
	CubicBezierCurve3D  m_bezierCurve_RightArm	= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_timer_RightArm		= Stopwatch( &g_theApp->m_gameClock, 1.0f );
	// Left Arm
	CubicBezierCurve3D  m_bezierCurve_LeftArm	= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_timer_LeftArm			= Stopwatch( &g_theApp->m_gameClock, 1.0f );
	// Right Foot
	CubicBezierCurve3D  m_bezierCurve_RightFoot = CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_timer_RightFoot		= Stopwatch( &g_theApp->m_gameClock, 1.0f );
	// Left Foot
	CubicBezierCurve3D  m_bezierCurve_LeftFoot  = CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_timer_LeftFoot		= Stopwatch( &g_theApp->m_gameClock, 1.0f );

	// Lighting Variables
	Vec3	m_sunDirection				= Vec3::ZERO;
	float   m_sunIntensity				= 0.85f;
	float   m_ambientIntensity			= 0.35f;
	float	m_specularIntensity			= 0.75f;
	float	m_specularPower				= 18.0f;
	float	m_specularIntensity_spider	= 2.0f;
	float	m_specularPower_spider		= 54.0f;
};