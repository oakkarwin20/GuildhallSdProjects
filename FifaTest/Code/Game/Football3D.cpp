#include "Game/Football3D.hpp"
#include "Game/Player3D.hpp"
#include "Game/GameModeFifaTest3D.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
Football3D::Football3D( GameModeFifaTest3D* gameModeFifaTest3D )
{
	m_gameModeFifaTest3D = gameModeFifaTest3D;
}

//----------------------------------------------------------------------------------------------------------------------
Football3D::~Football3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::Update( float deltaSeconds )
{
	// Get ball Forward
	m_footballFwdDir = m_footballOrientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
	m_footballFwdDir = m_footballFwdDir.GetNormalized();

	m_debugFootballFwdDirStartPos = m_footballPosition + ( m_footballFwdDir * FOOTBALL_RADIUS );
	m_debugFootballFwdDirEndPos	  = m_debugFootballFwdDirStartPos + ( m_footballFwdDir * m_debugFootballFwdDirLength );

	if ( g_debugBallCanMoveIndependently )
	{
		UpdateFootballInput();
	}

	ApplyGravity( deltaSeconds );
	ApplyCounterGravityForce( deltaSeconds );
	UpdateMagnusEffect( deltaSeconds );
	UpdatePhysics( deltaSeconds );
	UpdateBallToFloorCollision( deltaSeconds ); 
	UpdateBallOrientation( deltaSeconds );
	UpdateClampToWorldBounds();
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::Render( std::vector<Vertex_PCU>& outVerts ) const
{
	// Render ball with fwd dir
	AddVertsForSphere3D( outVerts, m_footballPosition, m_footballRadius, m_numSlices, m_numStacks, m_footballColor );
//	AddVertsForArrow3D(  outVerts, m_debugFootballFwdDirStartPos, m_debugFootballFwdDirEndPos, FOOTBALL_RADIUS, m_debugFootballFwdDirColor, AABB2::ZERO_TO_ONE );
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::UpdatePhysics( float deltaSeconds )
{
	// Apply physics movement
	m_footballVelocity += m_footballAcceleration * m_footballMass * deltaSeconds;
	m_footballPosition += m_footballVelocity * deltaSeconds;
//	m_footballAngularVelocity += m_intertiaTensor * deltaSeconds;

	// Reset acceleration
	m_footballAcceleration = Vec3::ZERO;
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::UpdateMagnusEffect( float deltaSeconds )
{
	Vec3 sky				= Vec3( 0.0f, 0.0f, 1.0f );
	Mat44 footballMatrix	= m_footballOrientation.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 footballUp			= footballMatrix.TransformPosition3D( sky );

	// Multiply cross product result by spin direction to determine which direction force should be applied
//	Vec3 magnusForceToApply =  * CrossProduct3D( m_footballVelocity, footballUp );

	Vec3 magnusForceToApply = CrossProduct3D( m_footballVelocity, sky );		// Result is "right"

	// If the ball is spinning +yaw, (counter-clockwise) apply a "left-ward" force
	if ( m_footballAngularVelocity.x > 0 )
	{
		m_footballVelocity += (0.25f * -magnusForceToApply) * deltaSeconds;
		DebuggerPrintf("+YAW, magnusForceToApply %0.2f\n", magnusForceToApply );
	}
	else if ( m_footballAngularVelocity.x < 0 )
	{
		// Ball is spinning -yaw (clockwise), apply a "right-ward" force
		m_footballVelocity += (0.25f * magnusForceToApply) * deltaSeconds;
		DebuggerPrintf("-YAW, magnusForceToApply %0.2f\n", magnusForceToApply );
	}

//	DebuggerPrintf( Stringf( "magnusForceToApply X: %0.2f, Y: %0.2f, Z: %0.2f || footballOrientation yaw: %0.2f, pitch: %0.2f, roll: %0.2f \n",
//		magnusForceToApply.x, magnusForceToApply.y, magnusForceToApply.z,
//		m_footballOrientation.m_yawDegrees, m_footballOrientation.m_pitchDegrees, m_footballOrientation.m_rollDegrees ).c_str() );
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::UpdateBallOrientation( float deltaSeconds )
{
	m_footballOrientation.m_yawDegrees		+= m_footballAngularVelocity.x * deltaSeconds;
	m_footballOrientation.m_pitchDegrees	+= m_footballAngularVelocity.y * deltaSeconds;
	m_footballOrientation.m_rollDegrees		+= m_footballAngularVelocity.z * deltaSeconds;
}

//----------------------------------------------------------------------------------------------------------------------
// void Football3D::MoveInDirection( Vec3 directionToMove, float speed )
// {
// 	// Add Force in direction of target
// //	Vec3 forceAmount = directionToMove * m_footballDrag * speed;			// Direction should be normalized
// //	AddAcceleration( forceAmount );
// //	DebuggerPrintf( "forceAmount X = %0.2f, Y = %0.2f, m_playerAcceleration X = %0.2f, Y = %0.2f \n", forceAmount.x, forceAmount.y, m_playerAcceleration.x, m_playerAcceleration.y );
// }

//----------------------------------------------------------------------------------------------------------------------
void Football3D::ApplyAcceleration( Vec3 acceleration )
{
	m_footballAcceleration += acceleration;
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::ApplyVelocity( Vec3 forceAmount )
{
	m_footballVelocity += forceAmount;
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::UpdateBallToFloorCollision( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	// Initialize common variables
	float ballBottomPosZ				 = m_footballPosition.z - m_footballRadius;
	float negativeVelocityZWithTolerance = -0.25f;
	//----------------------------------------------------------------------------------------------------------------------
	// Apply relevant logic for ball's first contact with the ground IF falling 
	//----------------------------------------------------------------------------------------------------------------------
	if ( ( ballBottomPosZ <= 0.0f ) && ( m_footballVelocity.z < negativeVelocityZWithTolerance ) )	// 0 == minZThreshold		// if the ball is moving downwards (negative Z) AND it is on the floor
	{		
		// Apply restitution 
		m_footballVelocity.z *= ( -FLOOR_ELASTICITY );
//		DebuggerPrintf( "Ball on floor and falling \n " );
	}
	//----------------------------------------------------------------------------------------------------------------------
	// if the ball is grounded (on the floor) AND NOT moving ground wards (-Z)
	//----------------------------------------------------------------------------------------------------------------------
	else if ( ballBottomPosZ <= 0.0f ) 
	{
		// Mark is grounded
		m_footballIsGrounded = true;

		//----------------------------------------------------------------------------------------------------------------------
		// Apply spin
		//----------------------------------------------------------------------------------------------------------------------
//		m_footballAngularVelocity.x = -ComputeGarwinAngularVelocity( m_footballVelocity.GetLength(), m_footballAngularVelocity.GetLength(), m_footballRadius );
//		m_footballAngularVelocity.y = -ComputeGarwinAngularVelocity( m_footballVelocity.GetLength(), m_footballAngularVelocity.GetLength(), m_footballRadius );
//		m_footballAngularVelocity.z = -ComputeGarwinAngularVelocity( m_footballVelocity.GetLength(), m_footballAngularVelocity.GetLength(), m_footballRadius );
		m_footballAngularVelocity *= FLOOR_FRICTION;

//		DebuggerPrintf( "BallAV2 X: %0.2f, Y: %0.2f, Z: %0.2f\n", m_footballAngularVelocity.x, m_footballAngularVelocity.y, m_footballAngularVelocity.z );
//		Player3D* currentPlayer = nullptr;
//		for ( int i = 0; i < m_gameModeFifaTest3D->m_playerList.size(); i++ )
//		{
//			if ( m_gameModeFifaTest3D->m_playerList[i]->m_playerIsControlled )
//			{
//				currentPlayer = m_gameModeFifaTest3D->m_playerList[i];
//			}
//		}
//		if ( currentPlayer != nullptr )
//		{
//			// Test code 1
//			if ( currentPlayer->m_velocity.x >= 0 )
//			{
//				m_footballAngularVelocity.y = ComputeGarwinAngularVelocity( m_footballVelocity.z, m_footballAngularVelocity.z, m_footballRadius );
//				DebuggerPrintf("OAK\n");
//			}
//			else
//			{
//				DebuggerPrintf("WIN\n");
//				m_footballAngularVelocity.y = -ComputeGarwinAngularVelocity( m_footballVelocity.z, m_footballAngularVelocity.z, m_footballRadius );
//			}
//			m_footballAngularVelocity.y = -ComputeGarwinAngularVelocity( m_footballVelocity.z, m_footballAngularVelocity.z, m_footballRadius );
//			m_footballAngularVelocity.x = ComputeGarwinAngularVelocity( m_footballVelocity.y, m_footballAngularVelocity.y, m_footballRadius );
//			m_footballAngularVelocity.z = -ComputeGarwinAngularVelocity( m_footballVelocity.x, m_footballAngularVelocity.x, m_footballRadius );
//		}

		// Test code 1
//		m_footballAngularVelocity.x = -ComputeGarwinAngularVelocity( m_footballVelocity.y, m_footballAngularVelocity.y, m_footballRadius );
//		m_footballAngularVelocity.y = -ComputeGarwinAngularVelocity( m_footballVelocity.z, m_footballAngularVelocity.z, m_footballRadius );
//		m_footballAngularVelocity.z = -ComputeGarwinAngularVelocity( m_footballVelocity.x, m_footballAngularVelocity.x, m_footballRadius );
//		// Test code 2
//		if ( m_footballAngularVelocity.GetLength() > 0.05f )
//		{
//			m_footballAngularVelocity.x = -ComputeGarwinAngularVelocity( m_footballVelocity.x, m_footballAngularVelocity.x, m_footballRadius );
//			m_footballAngularVelocity.y = -ComputeGarwinAngularVelocity( m_footballVelocity.y, m_footballAngularVelocity.y, m_footballRadius );
//			m_footballAngularVelocity.z = -ComputeGarwinAngularVelocity( m_footballVelocity.z, m_footballAngularVelocity.z, m_footballRadius );
//		}
		// Test code 3
//		if ( m_footballAngularVelocity.GetLength() > 0.05f )
//		{
			// Loop through player list
			// Get current player
			// Get player forward
//		Player3D* currentPlayer = nullptr;
//		for ( int i = 0; i < m_gameModeFifaTest3D->m_playerList.size(); i++ )
//		{
//			if ( m_gameModeFifaTest3D->m_playerList[i]->m_playerIsControlled )
//			{
//				currentPlayer = m_gameModeFifaTest3D->m_playerList[i];
//			}
//		}
//		float yaw = currentPlayer->m_playerOrientation;

//			m_footballAngularVelocity.x = -ComputeGarwinAngularVelocity( m_footballVelocity.x, m_footballAngularVelocity.x, m_footballRadius );
//			m_footballAngularVelocity.y = -ComputeGarwinAngularVelocity( m_footballVelocity.y, m_footballAngularVelocity.y, m_footballRadius );
//			m_footballAngularVelocity.z = -ComputeGarwinAngularVelocity( m_footballVelocity.z, m_footballAngularVelocity.z, m_footballRadius );
//		}

//		DebuggerPrintf( "Ball is on floor\n " );

		// Flatten velocity if grounded and velocity's magnitude is low
		m_footballPosition.z		= m_footballRadius;
		float magnitude				= m_footballVelocity.GetLength();
		float minMagnitudeTolerance = 0.25f;
		if ( magnitude <= minMagnitudeTolerance )
		{
			// Set ball to "rest" state if barely moving (velocity's magnitude is close to ZERO)
			m_footballVelocity		= Vec3::ZERO;
			m_footballAcceleration  = Vec3::ZERO;

//			DebuggerPrintf( "Ball on floor BUT stopped \n " );
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Execute logic while ball is grounded AND moving 
		//----------------------------------------------------------------------------------------------------------------------
		else	
		{
			//----------------------------------------------------------------------------------------------------------------------
			// Apply floor friction
			//----------------------------------------------------------------------------------------------------------------------
			m_footballVelocity.x *= FLOOR_FRICTION;
			m_footballVelocity.y *= FLOOR_FRICTION;

			m_footballAngularVelocity *= FLOOR_FRICTION;

			//----------------------------------------------------------------------------------------------------------------------
			// Apply spin
			//----------------------------------------------------------------------------------------------------------------------
			// Test code 1
//			m_footballAngularVelocity.x = -ComputeGarwinAngularVelocity( m_footballVelocity.y, m_footballAngularVelocity.y, m_footballRadius );
//			m_footballAngularVelocity.y = -ComputeGarwinAngularVelocity( m_footballVelocity.z, m_footballAngularVelocity.z, m_footballRadius );
//			m_footballAngularVelocity.z = -ComputeGarwinAngularVelocity( m_footballVelocity.x, m_footballAngularVelocity.x, m_footballRadius );
			// Test code 2
//			m_footballAngularVelocity.x = -ComputeGarwinAngularVelocity( m_footballVelocity.x, m_footballAngularVelocity.x, m_footballRadius );
//			m_footballAngularVelocity.y = -ComputeGarwinAngularVelocity( m_footballVelocity.y, m_footballAngularVelocity.y, m_footballRadius );
//			m_footballAngularVelocity.z = -ComputeGarwinAngularVelocity( m_footballVelocity.z, m_footballAngularVelocity.z, m_footballRadius );

			// Mark is NOT grounded
			m_footballIsGrounded = false;

//			DebuggerPrintf( "Ball on floor AND moving. FrictionApplied! \n " );
		}
	}
	//----------------------------------------------------------------------------------------------------------------------
	// Execute relevant logic while Ball is in air
	//----------------------------------------------------------------------------------------------------------------------
	else if ( ballBottomPosZ > ( 0.0f + FOOTBALL_RADIUS + 0.1f ) )		// 0.0f == floorZ, 0.1f == tolerance value 
	{
		// Only apply Air Drag IF ball is not on the floor 
		ApplyAirDrag();

		// Mark is NOT grounded
		m_footballIsGrounded = false;

//		DebuggerPrintf( "Ball is in air \n " );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::UpdateClampToWorldBounds()
{
	if ( 
		 ( ( m_footballPosition.x - m_footballRadius ) < 0.0f			  )  ||
		 ( ( m_footballPosition.x + m_footballRadius ) > PITCH_SIZE_X	  )  ||
		 ( ( m_footballPosition.y - m_footballRadius ) < 0.0f			  )  ||
		 ( ( m_footballPosition.y + m_footballRadius ) > PITCH_SIZE_Y	  )	 ||
		 ( ( m_footballPosition.z + m_footballRadius ) > MAX_PITCH_HEIGHT ) 
	   )
	{
		ResetBallToPitchCenter();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::ApplyAirDrag()
{
	m_footballVelocity			*= AIR_DRAG;
	m_footballAngularVelocity	*= AIR_DRAG;
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::ApplyGravity( float deltaSeconds )
{
	ApplyAcceleration(  Vec3(0.0f, 0.0f, -GRAVITY) * deltaSeconds );
	ApplyVelocity(		Vec3(0.0f, 0.0f, -GRAVITY) * deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::ApplyCounterGravityForce( float deltaSeconds )
{
	if ( m_footballIsGrounded )
	{
//		DebuggerPrintf("COUNTER-GRAVITY!\n");
		ApplyAcceleration( Vec3( 0.0f, 0.0f, GRAVITY ) * deltaSeconds );
		ApplyAcceleration( Vec3( 0.0f, 0.0f, GRAVITY ) * deltaSeconds );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::ResetBallToPitchCenter()
{
	m_footballPosition						= Vec3( PITCH_CENTER_X, PITCH_CENTER_Y, m_footballRadius );
	m_footballVelocity						= Vec3::ZERO;
	m_footballAngularVelocity				= Vec3::ZERO;
	m_footballOrientation.m_yawDegrees		= 0.0f;
	m_footballOrientation.m_pitchDegrees	= 0.0f;
	m_footballOrientation.m_rollDegrees		= 0.0f;
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::UpdateFootballInput()
{
	// Get Xbox Controller
	XboxController const& controller = g_theInput->GetController( 0 );

	//----------------------------------------------------------------------------------------------------------------------
	// Turn Ball orientation
	//----------------------------------------------------------------------------------------------------------------------
	// North
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) || controller.GetRightJoyStick().GetPosition().y > 0.0f )
	{
		m_footballOrientation.m_pitchDegrees += 5.0f;
	}
	// South
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) ||  controller.GetRightJoyStick().GetPosition().y < 0.0f )
	{
		m_footballOrientation.m_pitchDegrees -= 5.0f;
	}
	// East
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) ||  controller.GetRightJoyStick().GetPosition().x < 0.0f )
	{
		m_footballOrientation.m_yawDegrees += 5.0f;
	}
	// West
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) ||  controller.GetRightJoyStick().GetPosition().x > 0.0f )
	{
		m_footballOrientation.m_yawDegrees -= 5.0f;
	}

	//	DebuggerPrintf( "playerVelocity X = %0.2f, Y = %0.2f \n", playerVelocity.x, playerVelocity.y );

	//----------------------------------------------------------------------------------------------------------------------
	// Apply updated position ONLY if no actor has the ball AND debugMode is on
	//----------------------------------------------------------------------------------------------------------------------
	if ( !m_isCurrentlyPossessed && g_debugBallCanMoveIndependently )
	{
		if ( g_theInput->WasKeyJustPressed( ' ' ) || controller.GetButton( BUTTON_A ).m_isPressed )
		{
			m_footballAngularVelocity  += m_angularVelocityToAddBasedOnPOC;
			m_footballVelocity			= m_footballFwdDir * m_passSpeed;
			m_isCurrentlyPossessed		= false;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Football3D::CalculateInertialTensor()
{
	// For a cube
	// 1/6 * (size * size) * mass;
	float momentOfIntertiaTensor = GARWIN_ALPHA * ( FOOTBALL_MASS ) * (FOOTBALL_RADIUS * FOOTBALL_RADIUS );
	
	Vec3 iBasis = Vec3( momentOfIntertiaTensor,					  0.0f,					  0.0f );
	Vec3 jBasis = Vec3(					  0.0f, momentOfIntertiaTensor,					  0.0f );
	Vec3 kBasis = Vec3(					  0.0f,					  0.0f, momentOfIntertiaTensor );
	m_intertiaTensor.SetIJKT3D( iBasis, jBasis, kBasis, m_footballPosition );
}

//----------------------------------------------------------------------------------------------------------------------
float Football3D::ComputeGarwinAngularVelocity( float velocity, float angularVelocity, float radius )
{
	float velocityScaledByElasticity									= (1.0f + m_footballElasticity) * ( velocity * 10.0f );
	float garwinAlphaMinusElasticityScaledByRadiusTimesAngularVelocity	= (GARWIN_ALPHA - m_footballElasticity) * radius * angularVelocity;
	float radiusScaledByOnePlusGarwinAlpha								= radius * (1.0f + GARWIN_ALPHA);
	float newAngularVelocity											= ( velocityScaledByElasticity + garwinAlphaMinusElasticityScaledByRadiusTimesAngularVelocity ) / radiusScaledByOnePlusGarwinAlpha;
	return newAngularVelocity;
}
