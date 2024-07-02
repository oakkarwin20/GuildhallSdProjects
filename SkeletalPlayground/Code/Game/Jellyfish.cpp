#include "Game/GameModeBase.hpp"
#include "Game/GameMode_Quadruped_3D.hpp"
#include "Game/GameMode3D.hpp"
#include "Game/Jellyfish.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/ThirdParty/Squirrel/Noise/SmoothNoise.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"


//----------------------------------------------------------------------------------------------------------------------
Jellyfish::Jellyfish( GameMode3D* game, Vec3 const& rootStartPos, float limbLength ) : CreatureBase( rootStartPos, limbLength )
{
	m_game = game;
}


//----------------------------------------------------------------------------------------------------------------------
Jellyfish::~Jellyfish()
{
}


//----------------------------------------------------------------------------------------------------------------------
void Jellyfish::InitJoints()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Tentacles coming out from the center
	//----------------------------------------------------------------------------------------------------------------------
	// Tentacle 1
	m_tentacle1					= CreateChildChain( "m_tentacle1" );
	m_tentacle1->AddNewJointToChain( Vec3::ZERO );						// 0 
	m_tentacle1->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );		// 1 
	m_tentacle1->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );		// 2 
	m_tentacle1->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );		// 3 
	m_tentacle1->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );		// 4 
	// Tentacle 2
	m_tentacle2					= CreateChildChain( "m_tentacle2" );
	m_tentacle2->AddNewJointToChain( Vec3::ZERO );
	m_tentacle2->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle2->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle2->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle2->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	// Tentacle 3
	m_tentacle3					= CreateChildChain( "m_tentacle3" );
	m_tentacle3->AddNewJointToChain( Vec3::ZERO );
	m_tentacle3->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle3->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle3->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle3->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	// Tentacle 4
	m_tentacle4					= CreateChildChain( "m_tentacle4" );
	m_tentacle4->AddNewJointToChain( Vec3::ZERO );
	m_tentacle4->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle4->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle4->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle4->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Tentacles coming out from the edges of the head/body sphere
	//----------------------------------------------------------------------------------------------------------------------
	// Tentacle 5
	m_tentacle5					= CreateChildChain( "m_tentacle5" );
	m_tentacle5->AddNewJointToChain( Vec3( 0.0f, m_yOffset_tentacleStart, 0.0f ) );
	m_tentacle5->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle5->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle5->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle5->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	// Tentacle 6
	m_tentacle6					= CreateChildChain( "m_tentacle6" );
	m_tentacle6->AddNewJointToChain( Vec3( 0.0f, -m_yOffset_tentacleStart, 0.0f ) );
	m_tentacle6->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle6->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle6->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );
	m_tentacle6->AddNewJointToChain( Vec3( m_tentacleLength, 0.0f, 0.0f ) );

	// Set chain solver types
	m_tentacle1->m_solverType = CHAIN_SOLVER_FABRIK;
	m_tentacle2->m_solverType = CHAIN_SOLVER_FABRIK;
	m_tentacle3->m_solverType = CHAIN_SOLVER_FABRIK;
	m_tentacle4->m_solverType = CHAIN_SOLVER_FABRIK;
	m_tentacle5->m_solverType = CHAIN_SOLVER_FABRIK;
	m_tentacle6->m_solverType = CHAIN_SOLVER_FABRIK;


	//----------------------------------------------------------------------------------------------------------------------
	// Hack to include FABRIK fwd only
	//----------------------------------------------------------------------------------------------------------------------
	// Create creature root and arm
	m_creature_FABRIK_FWD_ONLY	= new CreatureBase( Vec3( 0.0f, 0.0f, 0.0f ) );
	m_ikChain_FABRIK_FWD_ONLY_1	= m_creature_FABRIK_FWD_ONLY->CreateChildChain( "m_ikChainFwd1", Vec3( 0.0f, 10.0f, 0.0f ), nullptr, true );
	m_ikChain_FABRIK_FWD_ONLY_1->AddNewJointToChain( Vec3( 0.0f, 0.0f, 0.0f ), EulerAngles() );		// 1
	m_ikChain_FABRIK_FWD_ONLY_1->AddNewJointToChain( Vec3(10.0f, 0.0f, 0.0f ), EulerAngles() );		// 2
	m_ikChain_FABRIK_FWD_ONLY_1->AddNewJointToChain( Vec3(10.0f, 0.0f, 0.0f ), EulerAngles() );		// 3
	m_ikChain_FABRIK_FWD_ONLY_1->AddNewJointToChain( Vec3(10.0f, 0.0f, 0.0f ), EulerAngles() );		// 4

	m_ikChain_FABRIK_FWD_ONLY_2	= m_creature_FABRIK_FWD_ONLY->CreateChildChain( "m_ikChainFwd2", Vec3( 0.0f, -10.0f, 0.0f ), nullptr, true );
	m_ikChain_FABRIK_FWD_ONLY_2->AddNewJointToChain( Vec3( 0.0f, 0.0f, 0.0f ), EulerAngles() );		// 1
	m_ikChain_FABRIK_FWD_ONLY_2->AddNewJointToChain( Vec3(10.0f, 0.0f, 0.0f ), EulerAngles() );		// 2
	m_ikChain_FABRIK_FWD_ONLY_2->AddNewJointToChain( Vec3(10.0f, 0.0f, 0.0f ), EulerAngles() );		// 3
	m_ikChain_FABRIK_FWD_ONLY_2->AddNewJointToChain( Vec3(10.0f, 0.0f, 0.0f ), EulerAngles() );		// 4

	m_ikChain_FABRIK_FWD_ONLY_3	= m_creature_FABRIK_FWD_ONLY->CreateChildChain( "m_ikChainFwd3", Vec3( 10.0f, 0.0f, 0.0f ), nullptr, true );
	m_ikChain_FABRIK_FWD_ONLY_3->AddNewJointToChain( Vec3( 0.0f, 0.0f, 0.0f ), EulerAngles() );		// 1
	m_ikChain_FABRIK_FWD_ONLY_3->AddNewJointToChain( Vec3(10.0f, 0.0f, 0.0f ), EulerAngles() );		// 2
	m_ikChain_FABRIK_FWD_ONLY_3->AddNewJointToChain( Vec3(10.0f, 0.0f, 0.0f ), EulerAngles() );		// 3
	m_ikChain_FABRIK_FWD_ONLY_3->AddNewJointToChain( Vec3(10.0f, 0.0f, 0.0f ), EulerAngles() );		// 4

	m_ikChain_FABRIK_FWD_ONLY_4 = m_creature_FABRIK_FWD_ONLY->CreateChildChain( "m_ikChainFwd", Vec3( -10.0f, 0.0f, 0.0f ), nullptr, true );
	m_ikChain_FABRIK_FWD_ONLY_4->AddNewJointToChain( Vec3( 0.0f, 0.0f, 0.0f ), EulerAngles() );		// 1
	m_ikChain_FABRIK_FWD_ONLY_4->AddNewJointToChain( Vec3(10.0f, 0.0f, 0.0f ), EulerAngles() );		// 2
	m_ikChain_FABRIK_FWD_ONLY_4->AddNewJointToChain( Vec3(10.0f, 0.0f, 0.0f ), EulerAngles() );		// 3
	m_ikChain_FABRIK_FWD_ONLY_4->AddNewJointToChain( Vec3(10.0f, 0.0f, 0.0f ), EulerAngles() );		// 4

	// Set once to remember starting position, rootPos_MS will change in forwards pass, but need to remember prevPos for the backwards pass
	m_ikChain_FABRIK_FWD_ONLY_1->m_chainRootPosBeforeFwdFABRIK_MS = Vec3(  0.0f, 0.0f, 0.0f );		
	m_ikChain_FABRIK_FWD_ONLY_1->UpdateJointsPositions_MS();								// Only needs to be called once on startup? 	
	m_ikChain_FABRIK_FWD_ONLY_1->SetSolverType( CHAIN_SOLVER_FABRIK_FWD_ONLY );				// Set solverTypes
	m_ikChain_FABRIK_FWD_ONLY_1->m_deadlockSolverType		= DEADLOCK_SOLVER_DEBT_ANGLE;	// Deadlock solver type
	m_ikChain_FABRIK_FWD_ONLY_1->m_attemptDeadlockSolutions = true;							// Disable deadlock solutions
	m_ikChain_FABRIK_FWD_ONLY_1->SetChainPreference_MatrixToEuler( FWD_TEMPORAL );			// Set chain matrix to euler preference

	m_ikChain_FABRIK_FWD_ONLY_2->m_chainRootPosBeforeFwdFABRIK_MS = Vec3(  0.0f, 0.0f, 0.0f );		
	m_ikChain_FABRIK_FWD_ONLY_2->UpdateJointsPositions_MS();								// Only needs to be called once on startup? 	
	m_ikChain_FABRIK_FWD_ONLY_2->SetSolverType( CHAIN_SOLVER_FABRIK_FWD_ONLY );				// Set solverTypes
	m_ikChain_FABRIK_FWD_ONLY_2->m_deadlockSolverType		= DEADLOCK_SOLVER_DEBT_ANGLE;	// Deadlock solver type
	m_ikChain_FABRIK_FWD_ONLY_2->m_attemptDeadlockSolutions = true;							// Disable deadlock solutions
	m_ikChain_FABRIK_FWD_ONLY_2->SetChainPreference_MatrixToEuler( FWD_TEMPORAL );			// Set chain matrix to euler preference

	m_ikChain_FABRIK_FWD_ONLY_3->m_chainRootPosBeforeFwdFABRIK_MS = Vec3(  0.0f, 0.0f, 0.0f );		
	m_ikChain_FABRIK_FWD_ONLY_3->UpdateJointsPositions_MS();								// Only needs to be called once on startup? 	
	m_ikChain_FABRIK_FWD_ONLY_3->SetSolverType( CHAIN_SOLVER_FABRIK_FWD_ONLY );				// Set solverTypes
	m_ikChain_FABRIK_FWD_ONLY_3->m_deadlockSolverType		= DEADLOCK_SOLVER_DEBT_ANGLE;	// Deadlock solver type
	m_ikChain_FABRIK_FWD_ONLY_3->m_attemptDeadlockSolutions = true;							// Disable deadlock solutions
	m_ikChain_FABRIK_FWD_ONLY_3->SetChainPreference_MatrixToEuler( FWD_TEMPORAL );			// Set chain matrix to euler preference

	m_ikChain_FABRIK_FWD_ONLY_4->m_chainRootPosBeforeFwdFABRIK_MS = Vec3(  0.0f, 0.0f, 0.0f );		
	m_ikChain_FABRIK_FWD_ONLY_4->UpdateJointsPositions_MS();								// Only needs to be called once on startup? 	
	m_ikChain_FABRIK_FWD_ONLY_4->SetSolverType( CHAIN_SOLVER_FABRIK_FWD_ONLY );				// Set solverTypes
	m_ikChain_FABRIK_FWD_ONLY_4->m_deadlockSolverType		= DEADLOCK_SOLVER_DEBT_ANGLE;	// Deadlock solver type
	m_ikChain_FABRIK_FWD_ONLY_4->m_attemptDeadlockSolutions = true;							// Disable deadlock solutions
	m_ikChain_FABRIK_FWD_ONLY_4->SetChainPreference_MatrixToEuler( FWD_TEMPORAL );			// Set chain matrix to euler preference

}


//----------------------------------------------------------------------------------------------------------------------
void Jellyfish::InitMesh()
{
	Rgba8 meshTint = Rgba8::WHITE;
	//----------------------------------------------------------------------------------------------------------------------
	// Tentacles coming out from the center
	//----------------------------------------------------------------------------------------------------------------------
	// Tentacle 1
	float tentacleThicknessStart = 1.0f;
	float tentacleDecrement		 = 0.15f;
	float tentacleThicknessEnd   = tentacleThicknessStart - tentacleDecrement;
	for ( int i = 0; i < m_tentacle1->m_jointList.size(); i++ )
	{
		m_tentacle1->m_jointList[ i ]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3( m_tentacleLength, 0.0f, 0.0f ), tentacleThicknessStart, tentacleThicknessEnd, Vec2( 1.0f, 1.0f ), Vec2( 1.0f, 1.0f ), meshTint );	// Hip to knee																																			// Forearm
		tentacleThicknessEnd -= tentacleDecrement;
	}
	// Tentacle 2
	tentacleThicknessStart = 1.0f;
	tentacleThicknessEnd   = tentacleThicknessStart - tentacleDecrement;
	for ( int i = 0; i < m_tentacle2->m_jointList.size(); i++ )
	{
		m_tentacle2->m_jointList[ i ]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3( m_tentacleLength, 0.0f, 0.0f ), tentacleThicknessStart, tentacleThicknessEnd, Vec2( 1.0f, 1.0f ), Vec2( 1.0f, 1.0f ), meshTint );	// Hip to knee																																			// Forearm
		tentacleThicknessEnd -= tentacleDecrement;
	}
	// Tentacle 3
	tentacleThicknessStart = 1.0f;
	tentacleThicknessEnd = tentacleThicknessStart - tentacleDecrement;
	for ( int i = 0; i < m_tentacle3->m_jointList.size(); i++ )
	{
		m_tentacle3->m_jointList[ i ]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3( m_tentacleLength, 0.0f, 0.0f ), tentacleThicknessStart, tentacleThicknessEnd, Vec2( 1.0f, 1.0f ), Vec2( 1.0f, 1.0f ), meshTint );	// Hip to knee																																			// Forearm
		tentacleThicknessEnd -= tentacleDecrement;
	}
	// Tentacle 4
	tentacleThicknessStart = 1.0f;
	tentacleThicknessEnd = tentacleThicknessStart - tentacleDecrement;
	for ( int i = 0; i < m_tentacle4->m_jointList.size(); i++ )
	{
		m_tentacle4->m_jointList[ i ]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3( m_tentacleLength, 0.0f, 0.0f ), tentacleThicknessStart, tentacleThicknessEnd, Vec2( 1.0f, 1.0f ), Vec2( 1.0f, 1.0f ), meshTint );	// Hip to knee																																			// Forearm
		tentacleThicknessEnd -= tentacleDecrement;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Tentacles coming out from the sides of the head
	//----------------------------------------------------------------------------------------------------------------------
	// Tentacle 5
	tentacleThicknessStart = 1.0f;
	tentacleThicknessEnd = tentacleThicknessStart - tentacleDecrement;
	for ( int i = 0; i < m_tentacle5->m_jointList.size(); i++ )
	{
		m_tentacle5->m_jointList[ i ]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3( m_tentacleLength, 0.0f, 0.0f ), tentacleThicknessStart, tentacleThicknessEnd, Vec2( 1.0f, 1.0f ), Vec2( 1.0f, 1.0f ), meshTint );	// Hip to knee																																			// Forearm
		tentacleThicknessStart = tentacleThicknessEnd;
		tentacleThicknessEnd  -= tentacleDecrement;
	}
	// Tentacle 6
	tentacleThicknessStart = 1.0f;
	tentacleThicknessEnd = tentacleThicknessStart - tentacleDecrement;
	for ( int i = 0; i < m_tentacle6->m_jointList.size(); i++ )
	{
		m_tentacle6->m_jointList[ i ]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3( m_tentacleLength, 0.0f, 0.0f ), tentacleThicknessStart, tentacleThicknessEnd, Vec2( 1.0f, 1.0f ), Vec2( 1.0f, 1.0f ), meshTint );	// Hip to knee																																			// Forearm
		tentacleThicknessStart = tentacleThicknessEnd;
		tentacleThicknessEnd  -= tentacleDecrement;
	}

	// Head (umbrella)
	Rgba8 headTint = Rgba8( 255, 255, 255, 245 );
	m_tentacle1->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3( -0.5f, 0.0f, 0.0f ), Vec3( 1.0f, 0.0f, 0.0f ), m_headRadiusStart, m_headRadiusEnd, Vec2( 0.5f, 1.0f ), Vec2( 1.0f, 1.0f ), headTint );
}


//----------------------------------------------------------------------------------------------------------------------
void Jellyfish::UpdateJoints( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	//----------------------------------------------------------------------------------------------------------------------
	// Update Jellyfish body
	//----------------------------------------------------------------------------------------------------------------------
	// Keep track of position last frame
	m_prevPosLastFrame_WS = m_modelRoot->m_jointPos_MS;
	// Compute new position this frame
 	float age					= g_theApp->m_gameClock.GetTotalSeconds();
 	float sine					= SinDegrees( age * 15.0f ) * 150.0f;
 	float cos					= CosDegrees( age * 15.0f ) * 150.0f;
 	float perlinNoise			= Compute2dPerlinNoise( age * 0.5f, 0.0f ) * 65.0f;
 	float baseHeight			= 120.0f;
	m_modelRoot->m_jointPos_MS	= Vec3( 0.0f + cos, 0.0f + sine, baseHeight + perlinNoise );
// 	m_modelRoot->m_jointPos_MS	= Vec3( 0.0f, 0.0f, 50.0f );

	float fwdOffset  = SinDegrees( age * 100.0f ) * 5.0f;
	float leftOffset = SinDegrees( age * 100.0f ) * 8.0f;
	float upOffset   = SinDegrees( age * 100.0f ) * 8.0f;

	Vec3 jellyfishFwdDir = ( m_modelRoot->m_jointPos_MS - m_prevPosLastFrame_WS ).GetNormalized();
	float velocity = jellyfishFwdDir.GetLength();
	if ( CompareIfFloatsAreEqual( velocity, 0.0f, 0.01f ) )
	{
		jellyfishFwdDir = Vec3::X_FWD;
	}

	Vec3 rootPos_WS			= m_modelRoot->m_jointPos_MS;
	EulerAngles newEuler	= m_modelRoot->GetEulerFromFwdDir( jellyfishFwdDir );
	Mat44 newRootMatrix		= newEuler.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 leftDir			= newRootMatrix.GetJBasis3D();
	Vec3 upDir				= newRootMatrix.GetKBasis3D();

 	// Old approach to updating tentacles using noise
 	// Update tentacles
//	m_tentacle1->m_target.m_currentPos_WS = rootPos_WS - ( jellyfishFwdDir * ( 30.0f - fwdOffset ) ) + ( leftDir * (0.0f + leftOffset) ) + ( upDir * (0.0f + upOffset) );
//	m_tentacle2->m_target.m_currentPos_WS = rootPos_WS - ( jellyfishFwdDir * ( 35.0f + fwdOffset ) ) + ( leftDir * (0.5f - leftOffset) ) + ( upDir * (0.0f + upOffset) );
//	m_tentacle3->m_target.m_currentPos_WS = rootPos_WS - ( jellyfishFwdDir * ( 35.0f - fwdOffset ) ) + ( leftDir * (0.0f + leftOffset) ) + ( upDir * (0.5f - upOffset) );
//	m_tentacle4->m_target.m_currentPos_WS = rootPos_WS - ( jellyfishFwdDir * ( 30.0f + fwdOffset ) ) + ( leftDir * (0.5f - leftOffset) ) + ( upDir * (0.4f - upOffset) );
//	m_tentacle5->m_target.m_currentPos_WS = rootPos_WS - ( jellyfishFwdDir * ( 28.0f + fwdOffset ) ) + ( leftDir * (0.3f - leftOffset) ) + ( upDir * (0.2f - upOffset) );
//	m_tentacle6->m_target.m_currentPos_WS = rootPos_WS - ( jellyfishFwdDir * ( 26.0f + fwdOffset ) ) + ( leftDir * (0.2f + leftOffset) ) + ( upDir * (0.4f + upOffset) );
	
	// Update FABRIK FWD ONLY chains
	m_ikChain_FABRIK_FWD_ONLY_1->m_target.m_currentPos_WS	= m_prevPosLastFrame_WS - upDir   * 2.0f; // ( 0.01f  + fwdOffset  );
	m_ikChain_FABRIK_FWD_ONLY_1->m_target.m_goalPos_WS		= m_prevPosLastFrame_WS - upDir   * 2.0f; // ( 0.01f  + fwdOffset  );
	m_ikChain_FABRIK_FWD_ONLY_2->m_target.m_currentPos_WS	= m_prevPosLastFrame_WS + leftDir * 4.0f; // ( 0.01f  - fwdOffset  );
	m_ikChain_FABRIK_FWD_ONLY_2->m_target.m_goalPos_WS		= m_prevPosLastFrame_WS + leftDir * 4.0f; // ( 0.01f  - fwdOffset  );
	m_ikChain_FABRIK_FWD_ONLY_3->m_target.m_currentPos_WS	= m_prevPosLastFrame_WS - leftDir * 4.0f; // ( 0.01f + leftOffset );
	m_ikChain_FABRIK_FWD_ONLY_3->m_target.m_goalPos_WS		= m_prevPosLastFrame_WS - leftDir * 4.0f; // ( 0.01f + leftOffset );
	m_ikChain_FABRIK_FWD_ONLY_4->m_target.m_currentPos_WS	= m_prevPosLastFrame_WS + upDir   * 2.0f; // ( 0.01f - leftOffset );
	m_ikChain_FABRIK_FWD_ONLY_4->m_target.m_goalPos_WS		= m_prevPosLastFrame_WS + upDir   * 2.0f; // ( 0.01f - leftOffset );

	// New approach, update tentacles relative to FABRIK fwd final joints + noise
 	m_tentacle1->m_target.m_currentPos_WS = m_ikChain_FABRIK_FWD_ONLY_1->m_firstJoint->m_jointPos_MS + ( jellyfishFwdDir * (  0.0f - fwdOffset ) ) + ( leftDir * (0.0f + leftOffset) ) + ( upDir * (4.0f + upOffset) );
 	m_tentacle2->m_target.m_currentPos_WS = m_ikChain_FABRIK_FWD_ONLY_1->m_firstJoint->m_jointPos_MS + ( jellyfishFwdDir * (  5.0f + fwdOffset ) ) + ( leftDir * (1.5f - leftOffset) ) + ( upDir * (3.0f + upOffset) );
 	m_tentacle3->m_target.m_currentPos_WS = m_ikChain_FABRIK_FWD_ONLY_2->m_firstJoint->m_jointPos_MS + ( jellyfishFwdDir * (  2.0f - fwdOffset ) ) + ( leftDir * (3.0f + leftOffset) ) + ( upDir * (3.5f - upOffset) );
 	m_tentacle4->m_target.m_currentPos_WS = m_ikChain_FABRIK_FWD_ONLY_3->m_firstJoint->m_jointPos_MS + ( jellyfishFwdDir * (  0.0f + fwdOffset ) ) + ( leftDir * (5.5f - leftOffset) ) + ( upDir * (3.4f - upOffset) );
 	m_tentacle5->m_target.m_currentPos_WS = m_ikChain_FABRIK_FWD_ONLY_4->m_firstJoint->m_jointPos_MS + ( jellyfishFwdDir * (  4.0f + fwdOffset ) ) + ( leftDir * (2.3f - leftOffset) ) + ( upDir * (2.2f - upOffset) );
 	m_tentacle6->m_target.m_currentPos_WS = m_ikChain_FABRIK_FWD_ONLY_4->m_firstJoint->m_jointPos_MS + ( jellyfishFwdDir * (  6.0f + fwdOffset ) ) + ( leftDir * (0.5f + leftOffset) ) + ( upDir * (1.4f + upOffset) );

	m_creature_FABRIK_FWD_ONLY->Update( deltaSeconds );
	Update( deltaSeconds );
}


//----------------------------------------------------------------------------------------------------------------------
void Jellyfish::ComputeAndRenderRootIJK_WS( std::vector<Vertex_PCU>& verts ) const
{
	Vec3 rootPos_WS			= m_modelRoot->m_jointPos_MS;
	Vec3 fwdDir				= ( rootPos_WS - m_prevPosLastFrame_WS ).GetNormalized();
	EulerAngles newEuler	= m_modelRoot->GetEulerFromFwdDir( fwdDir );
	Mat44 newRootMatrix		= newEuler.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 leftDir			= newRootMatrix.GetJBasis3D();
	Vec3 upDir				= newRootMatrix.GetKBasis3D();
	float length			= 50.0f;
	Vec3 fwdPos				= rootPos_WS + ( fwdDir	 * length );
	Vec3 leftPos			= rootPos_WS + ( leftDir * length );
	Vec3 upPos				= rootPos_WS + ( upDir	 * length );
	AddVertsForArrow3D( verts, rootPos_WS,  fwdPos, 0.5f, Rgba8::RED	);		// Fwd
	AddVertsForArrow3D( verts, rootPos_WS, leftPos, 0.5f, Rgba8::GREEN	);		// Left
	AddVertsForArrow3D( verts, rootPos_WS, 	 upPos, 0.5f, Rgba8::BLUE	);		// Up
}


//----------------------------------------------------------------------------------------------------------------------
void Jellyfish::RenderJellyfish( std::vector<Vertex_PCU>& verts ) const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render Jellyfish
	//----------------------------------------------------------------------------------------------------------------------
//	AddVertsForSphere3D( verts, m_modelRoot->m_jointPos_MS, m_headRadius, 16.0f, 16.0f, Rgba8( 255, 255, 255, 245 ) );	// model root pos, current frame
//	AddVertsForSphere3D( verts, m_prevPosLastFrame_WS, 3.0f, 16.0f, 16.0f, Rgba8::CYAN );		// model root pos, prev frame 

	// Compute root world IJK
//	ComputeAndRenderRootIJK_WS( verts );

	// Render Tentacles
	if ( g_debugToggleMesh_Backspace )
	{
		RenderMesh( g_theRenderer, g_theApp->m_texture_Galaxy, g_litShader_PCUTBN );

// 		for ( int i = 0; i < m_skeletalList.size(); i++ )
// 		{
// 			IK_Chain3D* curChain = m_skeletalList[i];
// 			// New version, rendering mesh at current euler lerping positions instead of joint positions to remove discontinuity between lerp and cur positions
// 			Vec3 parentPos_WS = Vec3::ZERO;
// 			for ( int jointIndex = 0; jointIndex <  curChain->m_jointList.size(); jointIndex++ )
// 			{
// 				IK_Joint3D* curJoint = curChain->m_jointList[ jointIndex ];
// 				if ( curJoint->m_parent == nullptr )
// 				{
// 					Mat44 curJointMat = curJoint->GetMatrix_LocalToWorld( curChain->m_renderEulerAnglesGoal_notCur );
// 					g_theRenderer->SetModelConstants( curJointMat );
// 					g_theRenderer->BindTexture( nullptr );
// 					g_theRenderer->BindShader ( g_litShader_PCUTBN );
// 					g_theRenderer->DrawVertexArray( int( curJoint->m_meshList_TBN.size() ), curJoint->m_meshList_TBN.data() );
// 					// Reset bindings and constants
// 					g_theRenderer->SetModelConstants();
// 					g_theRenderer->BindTexture( nullptr );
// 					g_theRenderer->BindShader ( nullptr );
// 					// Update parent pos
// 					Vec3 curJointFwd = curJointMat.GetIBasis3D();
// 					Vec3 curPos_WS	 = curJointMat.GetTranslation3D();
// 					parentPos_WS	 = curPos_WS + ( curJointFwd * curJoint->m_distToChild );		// Parent pos for child to use next iter
// 				}
// 				else	// if parent exists, use parent pos calculated last iteration
// 				{			
// 					// Get curJoint Matrix
// 					Mat44 curJointMat = curJoint->GetMatrix_LocalToWorld( curChain->m_renderEulerAnglesGoal_notCur );
// 					curJointMat.SetTranslation3D( parentPos_WS );
// 					// Transform mesh
// 					g_theRenderer->SetModelConstants( curJointMat );
// 					g_theRenderer->BindTexture( nullptr );
// 					g_theRenderer->BindShader ( g_litShader_PCUTBN );
// 					g_theRenderer->DrawVertexArray( int( curJoint->m_meshList_TBN.size() ), curJoint->m_meshList_TBN.data() );
// 					// Reset bindings and constants
// 					g_theRenderer->SetModelConstants();
// 					g_theRenderer->BindShader ( nullptr );
// 					// Update parent pos
// 					Vec3 curJointFwd = curJointMat.GetIBasis3D();
// 					Vec3 curPos_WS	 = curJointMat.GetTranslation3D();
// 					parentPos_WS	 = curPos_WS + ( curJointFwd * curJoint->m_distToChild );		// Parent pos for child to use next iter
// 				}
// 			}
// 		}

		m_creature_FABRIK_FWD_ONLY->Render( verts, Rgba8::PURPLE, Rgba8::PURPLE );
	}
	else
	{
		Render( verts, Rgba8::WHITE, Rgba8::CYAN );
		m_creature_FABRIK_FWD_ONLY->Render( verts, Rgba8::PURPLE, Rgba8::PURPLE );
	}
}