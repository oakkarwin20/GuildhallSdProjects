 #pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/SkeletalSystem/IK_Joint3D.hpp"

#include <string>
#include <map>
#include <vector>


//----------------------------------------------------------------------------------------------------------------------
class  CreatureBase;
class  BitmapFont;
class  Renderer;
class  Texture;
struct Shader;


//----------------------------------------------------------------------------------------------------------------------
enum DeadlockSolverType
{
	DEADLOCK_SOLVER_DEBT_ANGLE,
	DEADLOCK_SOLVER_RESET,						// Reset all joint euler angles back to zero
	DEADLOCK_SOLVER_RESET_AND_POSE_ROOT_TO_Z,	// Pose the chain +Z up
	DEADLOCK_SOLVER_POSE_Z,
	DEADLOCK_SOLVER_POSE_X,
};


//----------------------------------------------------------------------------------------------------------------------
enum ChainSolveType
{
	CHAIN_SOLVER_INVALID = -1,
	CHAIN_SOLVER_FABRIK,
	CHAIN_SOLVER_FABRIK_FWD_ONLY,
	CHAIN_SOLVER_CCD,
	CHAIN_SOLVER_2BONE,
};


//----------------------------------------------------------------------------------------------------------------------
enum AnchorStates
{
	ANCHOR_STATE_LOCKED,		//  Is grounded, not moving,  is locked.			// Limb is placed on a valid position and is currently locked, it cannot be moved.
	ANCHOR_STATE_MOVING,		//  Not grounded,  is moving, not locked.			// Limb is moving to a valid position and is NOT locked. 
	ANCHOR_STATE_FREE,			//  Is grounded, not moving, not locked.			// Limb finished moving, is grounded, and is placed in a valid position, but it is NOT locked.
	ANCHOR_STATE_NUM,
};


//----------------------------------------------------------------------------------------------------------------------
struct JointInfo
{
public:
	// Constructor for debug drawing, info is passed in world space
	JointInfo( Vec3 const& jointPos_WS, Vec3 const& fwdDir_WS, Vec3 const& leftDir_WS, Vec3 const& upDir_WS )
		: m_jointPos_WS( jointPos_WS )
		, m_fwdDir_WS  ( fwdDir_WS	 )
		, m_leftDir_WS ( leftDir_WS  )
		, m_upDir_WS   ( upDir_WS	 )
	{
	}	
	
	// Constructor for disregarding bad solutions computed this frame, info is passed in MS and LS
	JointInfo( Vec3 const& jointPos_MS, EulerAngles const& eulerAngles_MS )
		: m_jointPos_MS( jointPos_MS )
		, m_eulerAngles_MS( eulerAngles_MS )
	{
	}

	// Constructor for disregarding bad solutions computed this frame, info is ONLY passed in LS
	JointInfo( EulerAngles const& eulerAngles_LS )
		: m_eulerAngles_LS( eulerAngles_LS )
	{
	}


public:
	// Debug draw info 
	Vec3		m_jointPos_WS	 = Vec3::ZERO;
	Vec3		m_fwdDir_WS		 = Vec3::ZERO;
	Vec3		m_leftDir_WS	 = Vec3::ZERO;
	Vec3		m_upDir_WS		 = Vec3::ZERO;
	// Preserving last frame's "previous best" solution 
	Vec3		m_jointPos_MS	 = Vec3::ZERO;
	EulerAngles m_eulerAngles_MS = EulerAngles();
	EulerAngles m_eulerAngles_LS = EulerAngles();
};


//----------------------------------------------------------------------------------------------------------------------
struct ChainInfo
{
public:
	ChainInfo( std::vector<JointInfo> jointInfoList )
		: m_jointInfoList( jointInfoList )
	{
	}

public:
	std::vector<JointInfo> m_jointInfoList;		// Per iteration
};


//----------------------------------------------------------------------------------------------------------------------
struct Target
{
public:
	Target() {};
	Target( Vec3 currentPos_WS, Vec3 goalPos_WS, Vec3 fwdDir = Vec3( 1.0f, 0.0f, 0.0f ), Vec3 leftDir = Vec3( 0.0f, 1.0f, 0.0f ), Vec3 upDir = Vec3( 0.0f, 0.0f, 1.0f ), Rgba8 color = Rgba8::WHITE )
	{
		m_currentPos_WS	= currentPos_WS;
		m_goalPos_WS	= goalPos_WS;
		m_fwdDir		= fwdDir;
		m_leftDir		= leftDir;
		m_upDir			= upDir;
		m_color			= color;
	};
	~Target() {};

public:
	Vec3	m_currentPos_WS	= Vec3::ZERO;
	Vec3	m_goalPos_WS	= Vec3::ZERO;
	Vec3	m_fwdDir		= Vec3( 1.0f, 0.0f, 0.0f );
	Vec3	m_leftDir		= Vec3( 0.0f, 1.0f, 0.0f );
	Vec3	m_upDir			= Vec3( 0.0f, 0.0f, 1.0f );
	Rgba8	m_color			= Rgba8::WHITE;
	std::vector<Vertex_PCU> m_meshList;
};


//----------------------------------------------------------------------------------------------------------------------
class IK_Chain3D
{
public:
	IK_Chain3D( std::string name, Vec3 position_WS, IK_Joint3D* ownerSkeletonFirstJoint = nullptr, CreatureBase* const creatureOwner = nullptr, bool shouldReachInsteadOfDrag = true );
	~IK_Chain3D();

	void	Startup();
	void	Shutdown();
	void	Update( float deltaSeconds );
	void	RenderTargetMesh( Renderer* g_theRenderer, Texture* texture = nullptr ) const;
	void	RenderMesh( Renderer* g_theRenderer, Texture* texture = nullptr, Shader* const shader = nullptr, Rgba8 const& color = Rgba8::WHITE ) const;
	void	Render( std::vector<Vertex_PCU>&	verts, 
					Rgba8 const&				limbColor, 
					Rgba8 const&				jointColor, 
					bool						renderDebugJointBasis		= false, 
					bool						renderDebugCurrentPos_EE	= false )	const;

	void AddNewJointToChain(	Vec3		const&	position_localSpace, 
						 		EulerAngles			orientation_localSpace	= EulerAngles(), 
						 		FloatRange			yawConstraints			= FloatRange( -180.0f, 180.0f ), 
						 		FloatRange			pitchConstraints		= FloatRange( -180.0f, 180.0f ), 
						 		FloatRange			rollConstraints 		= FloatRange( -180.0f, 180.0f ) 
						   );

	// Render Functions
	void RenderTarget_IJK				( std::vector<Vertex_PCU>& verts, float endPosLength )											const;
	void RenderTarget_CurrentPos		( std::vector<Vertex_PCU>& verts, float radius = 2.0f, Rgba8 const& color = Rgba8::WHITE )		const;
	void RenderTarget_GoalPos			( std::vector<Vertex_PCU>& verts, float radius = 2.0f, Rgba8 const& color = Rgba8::WHITE )		const;
	void DebugDrawJoints_IJK			( std::vector<Vertex_PCU>& verts, float arrowThickness = 0.1f, float endPosLength = 3.0f )		const;
	void DebugDrawConstraints_YPR		( std::vector<Vertex_PCU>& verts, float length )												const;
	void DebugDrawParentBasis			( std::vector<Vertex_PCU>& verts, float length = 4.0f )											const;
	void DebugDrawPoleVectorInfo		( std::vector<Vertex_PCU>& verts, bool renderDebugAxis = true )									const;
	void DebugDrawAnchorStates			( std::vector<Vertex_PCU>& verts, bool renderAtGoalOrientation, float radius )					const;
	void DebugDrawTarget				( std::vector<Vertex_PCU>& verts, float			radius,		
																		  Rgba8 const&  color			= Rgba8::WHITE, 
																		  bool			renderBasis		= false,
																		  float			basisLength		= 1.0f )						const;

	void DebugTextJointPos_WorldSpace	( std::vector<Vertex_PCU>& verts, float			textHeight,
									 									  float			heightOffset, 
									 									  Vec3 const&	camera_jBasis, 
									 									  Vec3 const&	camera_kBasis, 
									 									  BitmapFont*	textFont,
									 									  Rgba8 const&  color				= Rgba8::WHITE )			const;	

	void DebugTextJointPos_LocalSpace	( std::vector<Vertex_PCU>& verts, float			textHeight,
																		  float			heightOffset,	
																		  Vec3 const&	camera_jBasis, 
																		  Vec3 const&	camera_kBasis, 
																		  BitmapFont*	textFont,
																		  Rgba8 const&	color				= Rgba8::WHITE,
																		  bool			renderTargetPos		= true )					const;

	void DebugTextConstraints_YPR		( std::vector<Vertex_PCU>& verts, float			textHeight, 
																		  float			lengthToTextPosFromStart, 
																		  float			heightOffset, 
																		  Vec3 const&	jBasis, 
																		  Vec3 const&	kBasis, 
																		  BitmapFont*	textFont )										const;

	//----------------------------------------------------------------------------------------------------------------------
	// Iterative Solver Functions
	//----------------------------------------------------------------------------------------------------------------------
	// CCD
	void	Solve_CCD	( Target target );
	void	CCD_Forward ( Target target );
	// FABRIK
	void	FABRIK_Forward		( Target target );
	void	Solve_FABRIK		( Target target );
	void	FABRIK_Backward		( Target target );
	// FABRIK FWD ONLY TEST
	void	Solve_FABRIK_fwdOnly( Target target );
	void	FABRIK_fwdOnly		( Target target );


	//----------------------------------------------------------------------------------------------------------------------
	// Analytical Solver Functions
	//----------------------------------------------------------------------------------------------------------------------
	void	SolveTwoBoneIK_TriangulationMethod( Target target );
	void	ComputeBendAngle_Cos_Sine( Target target );

	//----------------------------------------------------------------------------------------------------------------------
	// Deadlock Solvers
	//----------------------------------------------------------------------------------------------------------------------
	void	DeadlockSolver_DebtAngle			( IK_Joint3D* curJoint, Target target );
	void	DeadlockSolver_Reset				();
	void	DeadlockSolver_ResetAndPoseRootZUp	();

	//----------------------------------------------------------------------------------------------------------------------
	// Queries
	//----------------------------------------------------------------------------------------------------------------------
	bool		CanMove();
	bool		TryFreeAnchor( IK_Chain3D* const otherChain );		
	bool		TryFreeAnchor( IK_Chain3D* const otherChainA, IK_Chain3D* const otherChainB );		
	bool		IsAnyJointBentToMaxConstraints();

	//----------------------------------------------------------------------------------------------------------------------
	// Util Functions
	//----------------------------------------------------------------------------------------------------------------------
	float		GetMaxChainLength();
	void		UpdateTargetOrientationToRef( Vec3 const& fwd, Vec3 const& left, Vec3 const& up );
	void		UpdateJointsPositions_MS();
//	EulerAngles	GetEulerFromFwdDir( IK_Joint3D* curJoint, Vec3 const& fwdDir );
	void		ResetEulerForAllJoints();
	float		GetDistEeToTarget  ( Target target );
	float		GetDistRootToTarget( Target target );
	void		ResetAllJointConstraints();
	void		SetSolverType( ChainSolveType solverType = CHAIN_SOLVER_INVALID );
	void		ResetBendMoreAngles();
	void		SetChainPreference_MatrixToEuler( MatrixToEulerPreference rotationPreference );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug Visuals
	//----------------------------------------------------------------------------------------------------------------------
	void SaveDebugPose_curIter();
	void SaveDebugPose_curIter( IK_Joint3D* const& curJoint );

	//----------------------------------------------------------------------------------------------------------------------
	// Anchor states
	//----------------------------------------------------------------------------------------------------------------------
	// Setters 
	void SetAnchor_Locked();
	void SetAnchor_Moving();
	void SetAnchor_Free();
	// Getters
	bool IsAnchor_Locked();
	bool IsAnchor_Moving();
	bool IsAnchor_Free();
	std::string GetAnchorAsString();

public:
	// Core variables
	std::string					m_name								= std::string( "Un-Named" );
	Vec3						m_position_WS						= Vec3::ZERO;
	EulerAngles					m_eulerAngles_WS					= EulerAngles();
	std::vector<IK_Joint3D*>	m_jointList;
	IK_Joint3D*					m_finalJoint						= nullptr;		// The limb at the end of the limbList
	IK_Joint3D*					m_firstJoint						= nullptr;		// The limb at the start of the limbList. Aka, parent
	CreatureBase*				m_creatureOwner						= nullptr;
	IK_Joint3D*					m_ownerSkeletonJoint				= nullptr;			
	int							m_numIterations						= 1;
//	std::vector<IK_Chain3D*>	m_childChainList;									// Unused? Delete?
//	IK_Chain3D*					m_parentChain						= nullptr;		// Unused? Delete?
	bool						m_attemptDeadlockSolutions			= true;			// IK solvers will NOT consider Deadlock solutions if this bool is false. Used for demo-ing IK solvers w/wo deadlock solutions
	bool						m_shouldRender						= true;
	float						m_distEeToTarget					= 99999.9f;
	bool						m_shouldReachInsteadOfDrag			= true;
	Vec3						m_chainRootPosBeforeFwdFABRIK_MS	= Vec3::ZERO;
	bool						m_solveRoll							= false;

	// Pole Vector
	Vec3 m_poleVector_MS = Vec3::ZERO;

	// End Effector
	Target		m_target;
	float		m_prevFrameDistToTarget	= 0.0f;
	float		m_prevIterDistToTarget	= 0.0f;
	Vec3		m_targetPos_LastFrame	= Vec3::ZERO;
	// Anchored
	AnchorStates m_anchorState = ANCHOR_STATE_LOCKED;

	//----------------------------------------------------------------------------------------------------------------------
	// Debug variables for screen printing
	//----------------------------------------------------------------------------------------------------------------------
	float m_degrees						= 0.0f;
	float m_clampedDegrees				= 0.0f;
	Vec3  m_debugVector_RotationAxis_MS	= Vec3::ZERO;
	Vec3  m_debugVector_rootToTarget_MS	= Vec3::ZERO;
	Vec3  m_debugVector_rootToPV_MS		= Vec3::ZERO;
	std::vector<std::string> m_deadlockDebugInfoList;
	//----------------------------------------------------------------------------------------------------------------------
	// Debug render variables
	Vec3 m_idealStepPos_WS = Vec3::ZERO;
	Vec3 m_prevIdealPos_WS = Vec3::ZERO;

	// Debug single stepping
	bool m_isSingleStep_Debug	= false;
	bool m_breakFABRIK			= false;
	// Chain solver type
	ChainSolveType m_solverType = CHAIN_SOLVER_INVALID;
	// Deadlock solutions
	float m_bestDistSolvedThisFrame = 0.0f;
	bool  m_isDeadlock				= false;
	bool  m_isTargetTooFar			= false;
	float m_debtAngle				= 0.0f;
	std::vector<ChainInfo> m_solutionsThisFrameList;
	std::vector<JointInfo> m_jointInfosThisIterList;
	int  m_iterCount		 = 0;
	int  m_curDeadlockCount  = 0;
	int  m_prevDeadlockCount = 0;
	Vec3 m_crossRotationAxis = Vec3::ZERO;
	DeadlockSolverType m_deadlockSolverType = DEADLOCK_SOLVER_DEBT_ANGLE;

	//----------------------------------------------------------------------------------------------------------------------
	// Lerping
	//----------------------------------------------------------------------------------------------------------------------
//	bool  m_lerpEulerToGoal				 = true; 
	float m_turnRate					 = 312.0f;		// Specified "degrees per second". Set to "22500.0f" for instantaneous snapping, no lerping.
	bool  m_lerpEulerToGoal				 = true;
//	bool  m_renderEulerAnglesGoal_notCur = false;

	// Unused variables?
};