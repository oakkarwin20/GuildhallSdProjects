#pragma once

#include "Game/GameCommon.hpp"

#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class GameModeFifaTest3D;

//----------------------------------------------------------------------------------------------------------------------
class Football3D
{
public:
	Football3D( GameModeFifaTest3D* gameModeFifaTest3D );
	~Football3D();

	void Update( float deltaSeconds );
	void Render( std::vector<Vertex_PCU>& outVerts ) const;
	
	// Physics Functions
	void UpdatePhysics( float deltaSeconds );
	void UpdateMagnusEffect( float deltaSeconds );
	void UpdateBallOrientation( float deltaSeconds );
	void MoveInDirection( Vec3 directionToMove, float speed );
	void ApplyAcceleration( Vec3 forceAmount );
	void ApplyVelocity( Vec3 forceAmount );
	void UpdateBallToFloorCollision( float deltaSeconds );
	void UpdateClampToWorldBounds();
	void ApplyAirDrag();
	void ApplyGravity( float deltaSeconds );
	void ApplyCounterGravityForce( float deltaSeconds );
	void ResetBallToPitchCenter();

	// Input functions
	void UpdateFootballInput();

	// Rotations
	void  CalculateInertialTensor();
	float ComputeGarwinAngularVelocity( float velocity, float angularVelocity, float radius );

public:
	// GameMode
	GameModeFifaTest3D* m_gameModeFifaTest3D = nullptr;

	// Core Physics variables
	float		m_footballMass				= FOOTBALL_MASS;			// 450cm == 0.45 meters (assuming ball radius is 22cm)
	float		m_footballRadius			= FOOTBALL_RADIUS;
	float		m_footballElasticity		= 0.88f;
	Vec3		m_footballPosition			= Vec3::ZERO;
	Vec3		m_footballVelocity			= Vec3::ZERO;				// Linear Velocity
	Vec3		m_footballAngularVelocity	= Vec3::ZERO;
	EulerAngles m_footballOrientation		= EulerAngles();
	Vec3		m_footballFwdDir			= Vec3::ZERO;
	Rgba8		m_footballColor				= Rgba8::ORANGE;
	bool		m_footballIsGrounded		= true;

	// Ball Input variables
	float m_passSpeed = 10.0f;

	// Possession variables
	bool m_isCurrentlyPossessed = false;

	// Ball Trajectory Variables
	std::vector<Vec3>		m_ballTrajectoryPositionList;
	int						m_numFramesToCalculateTrajectory	= 180;
	float					m_ballTrajectoryRadius				= FOOTBALL_RADIUS * 0.5f;
	Rgba8					m_ballTrajectoryColor				= Rgba8::PURPLE;

//	float m_footballDrag			= 1.0f;
	Vec3  m_footballAcceleration	= Vec3::ZERO;
	float m_numSlices				= 10.0f;
	float m_numStacks				= 20.0f;

	// UI POC variables	// POC = point of contact 
	Vec3 m_angularVelocityToAddBasedOnPOC = Vec3::ZERO;

	// Debug variables
	Rgba8 m_debugFootballFwdDirColor	= Rgba8::CYAN;
	float m_debugFootballShadowRadius	= FOOTBALL_RADIUS;
	float m_debugFootballFwdDirLength	= FOOTBALL_RADIUS * 4.0f;
	Vec3  m_debugFootballFwdDirStartPos = Vec3::ZERO;
	Vec3  m_debugFootballFwdDirEndPos	= Vec3::ZERO;

	// Inertia Tensor
	Mat44 m_intertiaTensor;
};