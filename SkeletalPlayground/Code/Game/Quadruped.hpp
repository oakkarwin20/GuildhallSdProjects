#pragma once

#include "Engine/SkeletalSystem/CreatureBase.hpp"


//----------------------------------------------------------------------------------------------------------------------
class  GameMode3D;
class  GameMode_Quadruped_3D;
struct Block;


//----------------------------------------------------------------------------------------------------------------------
// struct Raycast
// {
// 	RaycastResult3D		m_raycastResult			= RaycastResult3D();
// 	Vec3				m_rayEndPos				= Vec3::ZERO;
// };


// class DisplacerBeast : public Quadruped
//----------------------------------------------------------------------------------------------------------------------
class Quadruped : public CreatureBase
{
public:
	Quadruped( GameMode3D* game,					  Vec3 const& rootStartPos, float limbLength = 1.0f );
	Quadruped( GameMode_Quadruped_3D* gameMode_Biped,  Vec3 const& rootStartPos, float limbLength = 1.0f );
	~Quadruped();

	void RenderQuadruped(	std::vector<Vertex_PCU>&	verts, 
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
	void ClampModelRootToFeetAvgPos( float deltaSeconds );
	// Debug draw functions
	void DebugDrawBezier  ( std::vector<Vertex_PCU>& verts, CubicBezierCurve3D const& bezierCurve, Stopwatch const& timer );
	void DebugDrawRaycasts( std::vector<Vertex_PCU>& verts ) const;
	// Generic refactor for gait management
	void ComputeIdealStepPosition( IK_Chain3D* chainToEdit, EulerAngles eulerAngles, Vec3 const& anchorPos, float fwdScalar, float leftScalar, float upScalar );
	bool TryFreeAnchor			 ( IK_Chain3D* chainToEdit, IK_Chain3D* anchorChain );
	bool ShouldChainStep		 ( IK_Chain3D* chainToEdit, bool canMove, float maxDistFromIdealPos_XY, float maxDistFromIdealPos_Z );
	void TryInitBezierTimer		 ( bool canMove, bool shouldStep, Stopwatch& bezierTimer, IK_Chain3D* chainToEdit, IK_Chain3D* anchorChain );
	bool DoesRayHitFloor_CurPos	 ( IK_Chain3D* chainToEdit, Raycast& raycast );
	void UpdateIdealStepPosition ( bool doesRaycastHitFloor_CurPos, IK_Chain3D* chainToEdit, IK_Chain3D* anchorChain, Stopwatch& curChainTimer );


public:
	// GameMode pointers
	GameMode3D*				m_gameMode3D		= nullptr;
	GameMode_Quadruped_3D*	m_gameMode_Creature	= nullptr;

	// Core variables
	float m_distPelvisToRoot 			=  5.0f;
	float m_defaultHeightZ				= 10.0f;
	float m_distHipToRoot				= 15.0f;
	float m_desiredRootHeightAboveLimbs	=  0.0f;			// Offset height from avg limbEE positions
	float m_numArms						=  2.0f;
	float m_numSpineSegments			=  6.0f;
	float m_limbLength					= 10.0f;
	float m_halfLimbLength				= m_limbLength * 0.5f;
	float m_maxDistFromIdealPosXY		=  7.5f;
	float m_maxDistFromIdealPosZ		=  2.0f;
	float m_maxAngle_turnBody			= 70.0f;
	float m_maxDist_comeCloser			= 55.0f;
	float m_minDist_moveAway			= 40.0f;

	// Limb configurable data
	float m_armLength	= 10.0f;
	float m_legLength	= 10.0f;
	float m_spineLength = 5.0f;
	float m_tailLength  = 4.0f;
	float m_neckLength  = 10.0f;
	float m_headLength  = 3.0f;

	// Ideal step pos
	float m_fwdNess		= m_distHipToRoot;
	float m_leftNess	= (m_distPelvisToRoot * 2.0f);


	// Skeleton System pointers
	// Create array of chains
	IK_Chain3D* m_spine_CCD			= nullptr;
	IK_Chain3D* m_leftArm_2BIK		= nullptr;
	IK_Chain3D* m_rightArm_2BIK		= nullptr;
	IK_Chain3D* m_leftFoot_2BIK		= nullptr;
	IK_Chain3D* m_rightFoot_2BIK	= nullptr;
	IK_Chain3D* m_neck_FABRIK 		= nullptr;
	IK_Chain3D* m_head_FABRIK		= nullptr;
	IK_Chain3D* m_tail_CCD			= nullptr;
	IK_Chain3D* m_tentacleLeft_CCD  = nullptr;
	IK_Chain3D* m_tentacleRight_CCD	= nullptr;
	
	// Bezier curve and timer
	float stepTimer = 0.45f;
	// Left Arm
	CubicBezierCurve3D  m_bezier_leftArm		= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_bezierTimer_leftArm	= Stopwatch( &g_theApp->m_gameClock, 0.45f );
	// Right Arm
	CubicBezierCurve3D  m_bezier_rightArm		= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_bezierTimer_rightArm	= Stopwatch( &g_theApp->m_gameClock, 0.47f );
	// Left Foot
	CubicBezierCurve3D  m_bezier_leftFoot		= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_bezierTimer_leftFoot	= Stopwatch( &g_theApp->m_gameClock, 0.46f );
	// Right Foot
	CubicBezierCurve3D  m_bezier_rightFoot		= CubicBezierCurve3D( Vec3( -40.0f, 0.0f, 0.0f ), Vec3( -20.0f, 0.0f, 25.0f ), Vec3( 20.0f, 0.0f, 25.0f ), Vec3( 40.0f, 0.0f, 0.0f ) );
	Stopwatch			m_bezierTimer_rightFoot	= Stopwatch( &g_theApp->m_gameClock, 0.44f );

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
	Raycast m_raycast_IdealPos_LeftArm;
	Raycast m_raycast_IdealPos_RightArm;
	Raycast m_raycast_IdealPos_LeftFoot;
	Raycast m_raycast_IdealPos_RightFoot;
	// Current positions
	Raycast m_raycast_CurPos_LeftArm;
	Raycast m_raycast_CurPos_RightArm;
	Raycast m_raycast_CurPos_LeftFoot;
	Raycast m_raycast_CurPos_RightFoot;

	// Clamping model root near avgFeetPos
	float m_maxDistToClampRootFromAvgFeetPos = 2.0f;
	float m_clampedDistModelRootToAvgFeetPos = 0.0f;

	// Handling palm lerping
	EulerAngles m_yprLastFrame_leftFootPalm = EulerAngles();
};
