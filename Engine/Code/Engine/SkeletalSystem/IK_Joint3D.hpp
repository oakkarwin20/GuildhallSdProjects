 #pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/VertexUtils.hpp"

#include <string>
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class IK_Chain3D;
struct Target;

//----------------------------------------------------------------------------------------------------------------------
enum MatrixToEulerPreference
{
	YAW_PREFERRED,
	PITCH_PREFERRED,
	FWD_TEMPORAL,		// Neither yaw nor pitch preferred, just compute matrix to euler, then compare with last frame to get most similar/consistent results
	FWD_LEFT_TEMPORAL,	// Temporal coherence
	FWD_LEFT_ALTERNATE,	// Attempting alternative versions of the base euler (yaw preferred)
	FWD_LEFT_CUSTOM,	// ONLY modify the base euler using our custom solution
	FWD_LEFT_DEFAULT,	// Default 
	UNMODIFIED,
};


//----------------------------------------------------------------------------------------------------------------------
// Bend towards or away from target
enum DeadlockBendDirection
{
	DEADLOCK_BEND_AWAY,
	DEADLOCK_BEND_TOWARD,
	DEADLOCK_BEND_NUM,
};


//----------------------------------------------------------------------------------------------------------------------
enum JointConstraintType
{
// 	JOINT_CONSTRAINT_TYPE_DISTANCE,					// Free range of motion (orientation/rotates freely), only bound by length of current limb
// 	JOINT_CONSTRAINT_TYPE_LINEAR,					// Zero range of motion, shares the same orientation as the parent, can only "extend" and "retract"
// 	JOINT_CONSTRAINT_TYPE_HINGE,					// 1 degree of motion, constrained in distance
// 	JOINT_CONSTRAINT_TYPE_HINGE_KNEE,
// 	JOINT_CONSTRAINT_TYPE_ROTATION,					// Free range of motion
// 	JOINT_CONSTRAINT_TYPE_BALL_AND_SOCKET,
	JOINT_CONSTRAINT_TYPE_EULER,
	JOINT_CONSTRAINT_TYPE_NUM,
};

/*
* Note: Position and direction data for each joint is stored in local space
* Variable naming conventions:
*	1. 'LS' means 'local space' (relative to parent)
*	2. 'WS' means 'world space' (relative to world)
*	2. 'MS' means 'model space' (relative to the model root joint)
*/

//----------------------------------------------------------------------------------------------------------------------
class IK_Joint3D
{
public:
	IK_Joint3D(		int					jointIndex, 
					Vec3				jointPosition, 
					float				length, 
					IK_Chain3D*			skeletalSystem		= nullptr,
					JointConstraintType jointConstraintType = JOINT_CONSTRAINT_TYPE_EULER, 
					EulerAngles			eulerAngles_LS		= EulerAngles(), 
					FloatRange			yawConstraints		= FloatRange(), 
					FloatRange			pitchConstraints	= FloatRange(), 
					FloatRange			rollConstraints		= FloatRange(), 
					IK_Joint3D*			parent				= nullptr 
				);

	~IK_Joint3D();
	
	// Init mesh shapes
	void InitMesh_Cylinder			   ( Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE );
	void InitMesh_CylinderTapered	   ( Vec3 const& start, Vec3 const& end, float startRadius, float endRadius, Rgba8 const& color = Rgba8::WHITE );
	void InitMesh_CapsuleTaperedEllipse( Vec3 const& start, Vec3 const& end, float startRadius, float endRadius, Vec2 const& startRadiusScalarXY = Vec2(1.0f, 1.0f), Vec2 const& endRadiusScalarXY = Vec2(1.0f, 1.0f), Rgba8 const& color = Rgba8::WHITE, float numSlices = 32.0f, float numStacks = 32.0f );
	void InitMesh_Sphere( Vec3 const& center, float radius, Rgba8 const& color = Rgba8::WHITE );
//	void InitMesh_Arrow( Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE );

	// Debug Render Functions
	void RenderIJK_MS( std::vector<Vertex_PCU>& verts, float length, float arrowThickness = 1.0f ) const;

	// Mutators & Modifiers (Getters & setters)
	void		SetConstraints_YPR(  FloatRange    yawConstraints = FloatRange( -360.0f, 360.0f ), 
									 FloatRange	 pitchConstraints = FloatRange( -360.0f, 360.0f ), 
									 FloatRange	 rollConstraints  = FloatRange( -360.0f, 360.0f ) );
	EulerAngles	GetEulerFromFwdDir	  ( Vec3 const& fwdDir );
	EulerAngles	GetEulerFromFwdAndLeft( Vec3 const& fwdDir, Vec3 const& leftDir );
	void		ToggleSingleStep_Forwards();
	void		ToggleSingleStep_Backwards();
	void		ClampYPR();
	void		ToggleDeadlockDir_Pitch();
	void		ToggleDeadlockDir_Yaw();

	//----------------------------------------------------------------------------------------------------------------------
	// Angle lerping
	//----------------------------------------------------------------------------------------------------------------------
	void		LerpChainEuler_CurToGoal( float deltaSeconds, float turnRateYaw, float turnRatePitch, float turnRateRoll );
	float		TurnTowardsGoal( float currentValue, float goalValue, float maxDelta, FloatRange angleConstraints );

	//----------------------------------------------------------------------------------------------------------------------
	// Matrix functions to jump between spaces
	//----------------------------------------------------------------------------------------------------------------------
	Mat44 GetMatrix_LocalToParent( bool useCurEulerAngles_LS = false )											const;
	Mat44 GetMatrix_LocalToModel ( Mat44 localToModelMatrix = Mat44(), bool useCurEulerAngles_LS = false )		const;
	Mat44 GetMatrix_LocalToWorld ( bool useCurEulerAngles_LS = false )											const;
	Mat44 GetMatrix_LocalToWorld ( Mat44 const& parentModelToWorldMatrix, bool useCurEulerAngles_LS = false )	const;
	// Matrix Util functions
	Mat44 GetIkChainMatrix_ModelToWorld() const;
	Mat44 GetModelRootMatrix_ModelToWorld() const;
	
	//----------------------------------------------------------------------------------------------------------------------
	// Enum Util Functions
	//----------------------------------------------------------------------------------------------------------------------
	std::string GetEulerPreference_EnumAsString() const;
	
public:
	// Core variables
	int					m_jointIndex			= 0;
	Vec3				m_jointPos_LS			= Vec3::ZERO;
	Vec3				m_jointPos_MS			= Vec3::ZERO;
	EulerAngles			m_eulerAnglesGoal_LS	= EulerAngles();		// Expressed in local space (relative to parent)	
	EulerAngles			m_eulerAnglesCurrent_LS	= EulerAngles();		// Expressed in local space (relative to parent)	
	// Chain management variables
	IK_Chain3D*			m_ikChain				= nullptr;				// ikChain refers to the chain this joint belongs to
	float				m_distToChild			= 0.0f;
	IK_Joint3D*			m_parent				= nullptr;
	IK_Joint3D*			m_child					= nullptr;
	// Constraints variables (Note: local space is relative to parent)
	JointConstraintType	  m_jointConstraintType		= JOINT_CONSTRAINT_TYPE_EULER;
	FloatRange			  m_yawConstraints_LS		= FloatRange( -360.0f, 360.0f );
	FloatRange			  m_pitchConstraints_LS		= FloatRange( -360.0f, 360.0f );
	FloatRange			  m_rollConstraints_LS		= FloatRange( -360.0f, 360.0f );
	Vec3				  m_jointPoleVector_MS		= Vec3::ZERO;	
	// Debug single step
	bool m_solveSingleStep_Forwards  = false;
	bool m_solveSingleStep_Backwards = false;
	// Basis flipping
	Vec3		m_leftDir_lastFrame		= Vec3::Y_LEFT;
	Vec3		m_upDir_lastFrame		= Vec3::Z_UP;
	EulerAngles m_eulerLS_LastFrame		= EulerAngles();
	EulerAngles m_eulerCloserToTarget	= EulerAngles();
	// Deadlock: bend more
	float m_bendMore_pitch  = 0.0f;
	float m_bendMore_yaw	= 0.0f;
	DeadlockBendDirection m_deadlockDir_Pitch = DEADLOCK_BEND_AWAY;
	DeadlockBendDirection m_deadlockDir_Yaw	  = DEADLOCK_BEND_AWAY;
	// Mesh
//	std::vector<Vertex_PCU>		m_meshList;
	std::vector<Vertex_PCUTBN>	m_meshList_TBN;
	bool						m_renderMeshWithTexture = true;
	
	// MatrixToEulerPreference
	MatrixToEulerPreference m_matrixToEulerPreference = FWD_TEMPORAL;

	// To delete?
	Vec3 m_refVector = Vec3( 0.0f, 0.0f, -1.0f );		// Ball and socket 
	bool m_isSubBase = false;							// Multi end effectors
};