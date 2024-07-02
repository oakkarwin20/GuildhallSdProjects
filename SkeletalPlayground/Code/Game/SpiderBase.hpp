#pragma once

#include "Engine/SkeletalSystem/CreatureBase.hpp"


//----------------------------------------------------------------------------------------------------------------------
class  GameMode3D;
class  GameMode_Spider_3D;
struct Block;


// class DisplacerBeast : public SpiderBase
//----------------------------------------------------------------------------------------------------------------------
class SpiderBase : public CreatureBase
{
public:
	SpiderBase( GameMode3D* game,					   Vec3 const& rootStartPos, float limbLength = 1.0f );
	SpiderBase( GameMode_Spider_3D*	  gameMode_Spider, Vec3 const& rootStartPos, float limbLength = 1.0f );
	~SpiderBase();

	void RenderSpiderBase(	std::vector<Vertex_PCU>&	verts, 
							std::vector<Vertex_PCU>&	verts_textured,
							std::vector<Vertex_PCU>&	textVerts, 
							std::vector<Vertex_PCU>&	verts_BackFace, 
							std::vector<Vertex_PCU>&	verts_chainBody, 
							Camera const&				worldCamera, 
							Texture*					creatureTexture 
						 ) const;

	// Chain setup functions
	void InitJointMesh();
	void InitLimbs();
	void SetChainConstraints( IK_Chain3D* const currentChain );
	// Walk logic functions
	void PreUpdateChains ( float deltaSeconds );
	void PostUpdateChains( float deltaSeconds );
	bool IsIdealPosTooFar( Vec3 const& idealPos, Vec3 posToCompare, float maxDistXY, float maxDistZ );
	// Raycast Queries
	bool DidRaycastHitWalkableBlock	( RaycastResult3D& raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal, std::vector<Block*>	   blockList );
	bool DidRaycastHitTriangle		( RaycastResult3D& raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal, std::vector<unsigned int> triIndexList, std::vector<Vertex_PCU> planeVertList );
	bool DoesRaycastHitFloor		( Vec3& refPosition, Raycast& raycast );
	// Bezier Curve & Timer functions
	void UpdateBezierCurvePositions	( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const skeleton, Vec3 const& refUpDir, Stopwatch& bezierTimer );
	void UpdateBezierTimer			( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const curChain, IK_Chain3D* const anchorLinkedChain, Stopwatch& bezierTimer );
	void UpdatePoleVector();
	// Utils
	void ClampModelRootToFeetAvgPos();
	// Debug draw functions
	void DebugDrawBezier  ( std::vector<Vertex_PCU>& verts, CubicBezierCurve3D const& bezierCurve, Stopwatch const& timer );
	void DebugDrawRaycasts( std::vector<Vertex_PCU>& verts ) const;
	// Generic refactor for gait management
	void ComputeIdealStepPosition( IK_Chain3D* chainToEdit, EulerAngles eulerAngles, Vec3 const& anchorPos, float fwdScalar, float leftScalar, float upScalar );
	bool TryFreeAnchor( IK_Chain3D* chainToEdit, IK_Chain3D* anchorChain );
	bool ShouldChainStep( IK_Chain3D* chainToEdit, bool canMove, float maxDistFromIdealPos_XY, float maxDistFromIdealPos_Z );
	void TryInitBezierTimer( bool canMove, bool shouldStep, Stopwatch& bezierTimer, IK_Chain3D* chainToEdit, IK_Chain3D* anchorChain );
	bool DoesRayHitFloor_CurPos( IK_Chain3D* chainToEdit, Raycast& raycast );
	void UpdateIdealStepPosition( bool doesRaycastHitFloor_CurPos, IK_Chain3D* chainToEdit, IK_Chain3D* anchorChain, Stopwatch& curChainTimer );

public:
	// GameMode pointers
	GameMode3D*			m_gameMode3D		= nullptr;
	GameMode_Spider_3D*	m_gameMode_Spider	= nullptr;

	// Core variables
	float m_leg4Start_xOffset			=  3.0f;
	float m_distBetweenLegs				=  2.0f;
	float m_distPelvisToRoot 			=  5.0f;
	float m_defaultHeightZ				=  5.0f;
	float m_distHipToRoot				= 10.0f;
	float m_desiredRootHeightAboveLimbs	=  0.0f;			// Offset height from avg limbEE positions
	float m_numArms						=  2.0f;
	float m_numSpineSegments			=  6.0f;
	float m_limbLength					= 10.0f;
	float m_halfLimbLength				= m_limbLength * 0.5f;
	// Ideal step pos
	float m_fwdNess1  =  22.0f;
	float m_fwdNess2  =  10.0f;
	float m_fwdNess3  = - 5.0f;
	float m_fwdNess4  = -20.0f;
	float m_leftNess1 =  15.0f;
	float m_leftNess2 =  25.0f;
	float m_leftNess3 =  15.0f;
	float m_leftNess4 =  16.0f;

	// Limb configurable data
	float m_leg1_segment1 =  8.0f;
	float m_leg1_segment2 = 12.0f;
	float m_leg1_segment3 = 10.0f;
	float m_leg1_segment4 = 4.0f;

	float m_legLength1	= 18.0f;
	float m_legLength2	= 16.0f;
	float m_legLength3  = 12.0f;
	float m_legLength4  = 14.0f;
	float m_spineLength = 5.0f;
	float m_tailLength  = 4.0f;
	float m_neckLength  = 10.0f;

	// AI variables
	float m_maxDistFromIdealPos_XY		=  7.5f;
	float m_maxDistFromIdealPos_Z		=  2.0f;
	float m_maxAngle_turnBody			= 70.0f;
	float m_maxDist_comeCloser			= 55.0f;
	float m_minDist_moveAway			= 16.0f;

	// Skeleton System pointers
	// Create array of chains
	IK_Chain3D* m_spine_CCD			= nullptr;
	IK_Chain3D* m_leftLeg1_2BIK		= nullptr;
	IK_Chain3D* m_leftLeg2_2BIK		= nullptr;
	IK_Chain3D* m_leftLeg3_2BIK		= nullptr;
	IK_Chain3D* m_leftLeg4_2BIK		= nullptr;
	IK_Chain3D* m_rightLeg1_2BIK	= nullptr;
	IK_Chain3D* m_rightLeg2_2BIK	= nullptr;
	IK_Chain3D* m_rightLeg3_2BIK	= nullptr;
	IK_Chain3D* m_rightLeg4_2BIK	= nullptr;
	IK_Chain3D* m_neck_FABRIK 		= nullptr;
	IK_Chain3D* m_tail_CCD			= nullptr;
	
	// Bezier curve and timer
	// Left legs
	float stepTimer1_L								= 0.3f;
	CubicBezierCurve3D  m_bezier_leftFoot1			= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );		// Left Foot 1
	Stopwatch			m_bezierTimer_leftFoot1		= Stopwatch( &g_theApp->m_gameClock, stepTimer1_L );
	float stepTimer2_L								= 0.35f;
	CubicBezierCurve3D  m_bezier_leftFoot2			= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );		// Left Foot 2
	Stopwatch			m_bezierTimer_leftFoot2		= Stopwatch( &g_theApp->m_gameClock, stepTimer2_L );
	float stepTimer3_L								= 0.3f;
	CubicBezierCurve3D  m_bezier_leftFoot3			= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );		// Left Foot 3
	Stopwatch			m_bezierTimer_leftFoot3		= Stopwatch( &g_theApp->m_gameClock, stepTimer3_L );
	float stepTimer4_L								= 0.36f;
	CubicBezierCurve3D  m_bezier_leftFoot4			= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );		// Left Foot 4
	Stopwatch			m_bezierTimer_leftFoot4		= Stopwatch( &g_theApp->m_gameClock, stepTimer4_L );
	// Right legs
	float stepTimer1_R								= 0.3f;
	CubicBezierCurve3D  m_bezier_rightFoot1			= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );		// Right Arm 1
	Stopwatch			m_bezierTimer_rightFoot1	= Stopwatch( &g_theApp->m_gameClock, stepTimer1_R );
	float stepTimer2_R								= 0.35f;
	CubicBezierCurve3D  m_bezier_rightFoot2			= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );		// Right Arm 2
	Stopwatch			m_bezierTimer_rightFoot2	= Stopwatch( &g_theApp->m_gameClock, stepTimer2_R );
	float stepTimer3_R								= 0.3f;
	CubicBezierCurve3D  m_bezier_rightFoot3			= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );		// Right Arm 3
	Stopwatch			m_bezierTimer_rightFoot3	= Stopwatch( &g_theApp->m_gameClock, stepTimer3_R );
	float stepTimer4_R								= 0.36f;
	CubicBezierCurve3D  m_bezier_rightFoot4			= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );		// Right Arm 4
	Stopwatch			m_bezierTimer_rightFoot4	= Stopwatch( &g_theApp->m_gameClock, stepTimer4_R );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug Variables
	//----------------------------------------------------------------------------------------------------------------------
	RaycastResult3D m_rayResult_Blocks;
	RaycastResult3D m_rayResult_Tri;
	Vec3			m_debugVector_RotationAxis		= Vec3::ZERO;
	Vec3			m_debugVec_RotationAxis_WS_Palm	= Vec3::ZERO;
	Vec3			m_debugVec_ImpactNormal_WS_Palm = Vec3::ZERO;

	//----------------------------------------------------------------------------------------------------------------------
	// Raycasts
	//----------------------------------------------------------------------------------------------------------------------
	// Ideal positions
	Raycast m_raycast_IdealPos_LeftFoot1;
	Raycast m_raycast_IdealPos_LeftFoot2;
	Raycast m_raycast_IdealPos_LeftFoot3;
	Raycast m_raycast_IdealPos_LeftFoot4;
	Raycast m_raycast_IdealPos_RightFoot1;
	Raycast m_raycast_IdealPos_RightFoot2;
	Raycast m_raycast_IdealPos_RightFoot3;
	Raycast m_raycast_IdealPos_RightFoot4;
	// Current positions
	Raycast m_raycast_CurPos_LeftFoot1;
	Raycast m_raycast_CurPos_LeftFoot2;
	Raycast m_raycast_CurPos_LeftFoot3;
	Raycast m_raycast_CurPos_LeftFoot4;
	Raycast m_raycast_CurPos_RightFoot1;
	Raycast m_raycast_CurPos_RightFoot2;
	Raycast m_raycast_CurPos_RightFoot3;
	Raycast m_raycast_CurPos_RightFoot4;

	// Clamping model root relative to avgFeetPos
	float m_maxDistToClampRootFromAvgFeetPos = 4.0f;
	float m_clampedDistModelRootToAvgFeetPos = 0.0f;
};
