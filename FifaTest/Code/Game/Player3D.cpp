#include "Game/Player3D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Football3D.hpp"
#include "Game/GameModeFifaTest3D.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
Player3D::Player3D( Football3D* football, GameModeFifaTest3D* gameMode3D )
{
	m_football	 = football;
	m_gameMode3D = gameMode3D;
}

//----------------------------------------------------------------------------------------------------------------------
Player3D::~Player3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Player3D::Update( float deltaSeconds )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Update pass "force" based on distance between 2 actors
	Vec3 dispBetweenBothActors   = m_gameMode3D->m_player2->m_position - m_gameMode3D->m_player1->m_position;
	float distBetweenBothActors  = dispBetweenBothActors.GetLength();
	distBetweenBothActors		*= 0.8f;
	m_footballGroundPassSpeed    = distBetweenBothActors;
	m_footballAirPassSpeed		 = distBetweenBothActors;

	UpdatePhysics( deltaSeconds );
	if ( m_playerIsControlled )
	{
		UpdatePlayerInput( deltaSeconds );
	}
	UpdateFootballPossessionAndInput();			// This function checks if the player has the ball, then executes relevant player-ball logic
	UpdateClampToWorldBounds();

	//----------------------------------------------------------------------------------------------------------------------
	// Update fwd dir variables
	//----------------------------------------------------------------------------------------------------------------------
	Vec2 positionV2 = Vec2( m_position.x, m_position.y );
	Vec2 fwdV2		= positionV2.GetFowardNormal( m_playerOrientation );
	m_forwardDir.x	= fwdV2.x;
	m_forwardDir.y	= fwdV2.y;
}

//----------------------------------------------------------------------------------------------------------------------
void Player3D::Render( std::vector<Vertex_PCU>& outVerts )
{		
	Vec2 positionV2	= Vec2( m_position.x, m_position.y );

	// Render Physics radius
	if ( m_playerIsControlled )
	{
		Vec2 forwardDirV2 = Vec2( m_forwardDir.x, m_forwardDir.y );
		// Render Actor Possession radius
//		AddVertsForRing2D( outVerts,  positionV2, m_debugRadius, m_debugLineThickness, m_physicsRadiusColor );
		Vec2 fwdStartPos = positionV2 + ( forwardDirV2 * m_debugArrowSize );
		Vec2 fwdEndPos	 = fwdStartPos  + ( forwardDirV2 * m_debugArrowSize );
		// Render Fwd dir
		AddVertsForArrow2D( outVerts, fwdStartPos, fwdEndPos, m_debugArrowSize, m_debugLineThickness, m_physicsRadiusColor );
	}

	// Render Ball Possession radius
	AddVertsForRing2D( outVerts, positionV2, m_ballManipulateRadius, m_debugLineThickness, m_ballPossessionRangeColor );

	// Render Player
	FloatRange floatRange = FloatRange( 0.0f, m_height );
	AddVertsForCylinderZ3D( outVerts, Vec2( m_position.x, m_position.y ), floatRange, m_physicsRadius, 40.0f, m_color );

	// Remove Z fighting 
	Mat44 transform;
	transform.SetTranslation3D( Vec3( 0.0f, 0.0f, 0.1f ) );
	TransformVertexArray3D( outVerts, transform );
}

//----------------------------------------------------------------------------------------------------------------------
void Player3D::UpdatePhysics( float deltaSeconds )
{
	// Decelerate every frame with drag
	Vec2 velocityXY			= Vec2( m_velocity.x, m_velocity.y );
	Vec2 decelerationForce	= -m_drag * velocityXY;
	AddForce( decelerationForce );

	// Apply physics movement
	m_velocity += m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;

	// Reset acceleration
	m_acceleration = Vec3::ZERO;
}

//----------------------------------------------------------------------------------------------------------------------
void Player3D::AddForce( Vec2 forceAmount )
{
	Vec3 forceAmountXYZ = Vec3( forceAmount.x, forceAmount.y, 0.0f );
	m_acceleration = m_acceleration + forceAmountXYZ;
}

//----------------------------------------------------------------------------------------------------------------------
void Player3D::MoveInDirection( Vec2 directionToMove, float speed )
{
	// Add Force in direction of target
	Vec2 forceAmount = directionToMove * m_drag * speed;			// Direction should be normalized
	AddForce( forceAmount );
//	DebuggerPrintf( "forceAmount X = %0.2f, Y = %0.2f, m_playerAcceleration X = %0.2f, Y = %0.2f \n", forceAmount.x, forceAmount.y, m_playerAcceleration.x, m_playerAcceleration.y );
}

//----------------------------------------------------------------------------------------------------------------------
void Player3D::ApplyForceRandAndClampedInDir( Vec3 direction, float forceMagnitude )
{
	direction		= direction.GetNormalized();
	Vec3 fwdForce	= direction * forceMagnitude;
	fwdForce		= fwdForce.GetClamped( forceMagnitude );

	// Calculate forceToApply = passForce - absf( currentVelocity );
	// Only apply force if forceToApply is NOT negative
//	float ballAbsMagnitude	= fabsf( m_football->m_footballVelocity.GetLength() );
	float ballAbsMagnitude	= m_football->m_footballVelocity.GetLength();
	float netForceToApply	= fwdForce.GetLength() - ballAbsMagnitude;

	RandomNumberGenerator rng;
	float minClampedForce = rng.RollRandomFloatInRange(  -8.0f, -6.0f );
	float maxClampedForce = rng.RollRandomFloatInRange(   4.0f, 8.0f );
	maxClampedForce		  += forceMagnitude;
	minClampedForce		  -= forceMagnitude;
	if ( ( netForceToApply >= minClampedForce ) && ( netForceToApply <= maxClampedForce ) )
	{
		DebuggerPrintf( "forceToApply: %0.2f\n", netForceToApply );
		m_football->ApplyVelocity( fwdForce );
	}
	else
	{
		DebuggerPrintf( "force NOT applied: %0.2f\n", netForceToApply  );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Player3D::UpdateFootballPossessionAndInput()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Tell player to possess the ball if ball is NOT possessed and is within manipulate radius
	//----------------------------------------------------------------------------------------------------------------------
	// Check if the ball is within "manipulation" radius
	float distBallToPlayer = GetDistance3D( m_football->m_footballPosition, m_position );
	if ( distBallToPlayer <= m_ballManipulateRadius )
	{
		// Check if player does NOT have the ball
		if ( !m_football->m_isCurrentlyPossessed )
		{
			// If the ball is within manipulation radius 
			// AND it is NOT possessed 
			// AND none of the players currently possesses the ball, then this player should possess the ball
			if ( !m_gameMode3D->m_player1->m_hasTheBall && !m_gameMode3D->m_player2->m_hasTheBall )
			{
				m_football->m_isCurrentlyPossessed	= true;
				m_hasTheBall						= true;
//				m_isReceivingTheBall				= true;
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Receiving the ball
		//----------------------------------------------------------------------------------------------------------------------
		// Player should "take a touch" and reduce ball velocity IF (ball is moving too fast)
//		if ( m_football->m_footballVelocity.GetLength() > 200.0f && m_isReceivingTheBall )
//		{
//			m_isReceivingTheBall = false;
//			m_football->m_footballVelocity *= 0.3f;
//		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Execute ball manipulation logic IF ball IS possessed
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_hasTheBall )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Update Player pos, "catch up to the ball"
		//----------------------------------------------------------------------------------------------------------------------
		// Get disp playerToBall
		if ( distBallToPlayer > m_playerIsControlledRadius )
		{
			Vec2 nearestPointToBall = GetNearestPointOnDisc2D( Vec2( m_football->m_footballPosition.x, m_football->m_footballPosition.y ), Vec2( m_position.x, m_position.y ), m_football->m_footballRadius );

//			Vec3 dispPlayerToBall	= m_football->m_footballPosition - m_position;
//			Vec2 dispPlayerToBallV2 = Vec2( dispPlayerToBall.x, dispPlayerToBall.y );
			Vec2 dispPlayerToBallV2 = nearestPointToBall - Vec2( m_position.x, m_position.y );
			MoveInDirection( dispPlayerToBallV2.GetNormalized(), m_currentSpeed );
		}

		XboxController const& controller = g_theInput->GetController( 0 );
		//----------------------------------------------------------------------------------------------------------------------
		// If player is close enough to the ball (within manipulation radius), manipulate the ball
		//----------------------------------------------------------------------------------------------------------------------
		if ( distBallToPlayer <= m_playerIsControlledRadius )
		{
			//----------------------------------------------------------------------------------------------------------------------
			// Dribbling
			//----------------------------------------------------------------------------------------------------------------------
			m_forwardDir = m_forwardDir.GetNormalized();
			if (	
					g_theInput->IsKeyDown( 'W' ) || controller.GetLeftJoyStick().GetPosition().y > 0.0f  ||
					g_theInput->IsKeyDown( 'S' ) || controller.GetLeftJoyStick().GetPosition().y < 0.0f  ||
					g_theInput->IsKeyDown( 'A' ) || controller.GetLeftJoyStick().GetPosition().x < 0.0f	 ||
					g_theInput->IsKeyDown( 'D' ) || controller.GetLeftJoyStick().GetPosition().x > 0.0f 
				)
			{
				// Apply clamped dribble force to ball in fwd dir
//				Vec3 fwdForce			= m_forwardDir * m_footballCurrentDribbleSpeed;
//				fwdForce				= fwdForce.GetClamped( m_footballCurrentDribbleSpeed );
				ApplyForceRandAndClampedInDir( m_forwardDir, m_footballCurrentDribbleSpeed );

//				// Calculate forceToApply = passForce - absf( currentVelocity );
//				// Only apply force if forceToApply is NOT negative
//				float ballAbsMagnitude	= fabsf( m_football->m_footballVelocity.GetLength() );
//				float forceToApply		= fwdForce.GetLength() - ballAbsMagnitude;
//
//				RandomNumberGenerator rng;
//				float minClampedForce = rng.RollRandomFloatInRange( -10.0f, -5.0f );
//				float maxClampedForce = rng.RollRandomFloatInRange(   5.0f, 10.0f );
//				if ( ( forceToApply >= minClampedForce ) && ( forceToApply <= maxClampedForce ) )
//				{
//					DebuggerPrintf( "forceToApply: %0.2f\n", forceToApply );
//					m_football->ApplyVelocity( fwdForce );
//				}
//				else
//				{
//					DebuggerPrintf( "force NOT applied: %0.2f\n", forceToApply  );
//				}
			}
	
			//----------------------------------------------------------------------------------------------------------------------
			// Ground-Pass the ball 
			//----------------------------------------------------------------------------------------------------------------------
			if ( g_theInput->WasKeyJustPressed( ' ' ) || controller.GetButton( BUTTON_A ).m_wasPressedLastFrame )
			{
				// Apply impulse to pass
//				Vec3 fwdForce			= m_forwardDir * m_footballGroundPassSpeed;
//				fwdForce				= fwdForce.GetClamped( m_footballGroundPassSpeed );
//				m_football->ApplyVelocity( fwdForce );
				ApplyForceRandAndClampedInDir( m_forwardDir, m_footballGroundPassSpeed );
				m_football->m_footballAngularVelocity += m_football->m_angularVelocityToAddBasedOnPOC;		// Add spin to the ball which will result in the magnus effect depending on the "direction" of the spin (+yaw = counter-clockwise) (-yaw = clockwise)

				// Set ball NO longer possessed
				m_football->m_isCurrentlyPossessed = false;

				// Toggle actor possession bool
				for ( int i = 0; i < m_gameMode3D->m_playerList.size(); i++ )
				{
					m_gameMode3D->m_playerList[i]->m_hasTheBall			= !m_gameMode3D->m_playerList[i]->m_hasTheBall; 
//					m_gameMode3D->m_playerList[i]->m_isReceivingTheBall = !m_gameMode3D->m_playerList[i]->m_isReceivingTheBall;
				}
				m_gameMode3D->PossessNextActor();		// Possess the actor about to receive the ball
			}
			//----------------------------------------------------------------------------------------------------------------------
			// "Air-Pass" the ball ( lower arc ) 
			//----------------------------------------------------------------------------------------------------------------------
			if ( g_theInput->WasKeyJustPressed( 'N' ) || controller.GetButton( BUTTON_X ).m_wasPressedLastFrame )
			{
				Vec3 airFwdDir	 = Vec3( m_forwardDir.x, m_forwardDir.y, 1.0f );
				airFwdDir		 = airFwdDir.GetNormalized();

				ApplyForceRandAndClampedInDir( airFwdDir, m_footballAirPassSpeed * 0.4f  );
				m_football->m_footballAngularVelocity += m_football->m_angularVelocityToAddBasedOnPOC;		// Add spin to the ball which will result in the magnus effect depending on the "direction" of the spin (+yaw = counter-clockwise) (-yaw = clockwise)
//				airFwdDir		*= m_footballAirPassSpeed;
//				airFwdDir		 = airFwdDir.GetClamped( m_footballAirPassSpeed );
//				m_football->ApplyVelocity( airFwdDir );	
				m_football->m_isCurrentlyPossessed  = false;
				for ( int i = 0; i < m_gameMode3D->m_playerList.size(); i++ )
				{
					m_gameMode3D->m_playerList[i]->m_hasTheBall  = !m_gameMode3D->m_playerList[i]->m_hasTheBall; 
					m_gameMode3D->m_playerList[i]->m_shouldTakeATouch			= !m_gameMode3D->m_playerList[i]->m_shouldTakeATouch; 
				}
				m_gameMode3D->PossessNextActor();
			}
			//----------------------------------------------------------------------------------------------------------------------
			// "Air-Pass" the ball ( higher arc )
			//----------------------------------------------------------------------------------------------------------------------
			if ( g_theInput->WasKeyJustPressed( 'M' ) )
			{
				Vec3 airFwdDir						= Vec3( m_forwardDir.x, m_forwardDir.y, 3.0f );
				airFwdDir							= airFwdDir.GetNormalized();
				airFwdDir						   *= m_footballAirPassSpeed;
				airFwdDir							= airFwdDir.GetClamped( m_footballAirPassSpeed );
				ApplyForceRandAndClampedInDir( airFwdDir, m_footballAirPassSpeed * 0.6f );
				m_football->m_footballAngularVelocity += m_football->m_angularVelocityToAddBasedOnPOC;		// Add spin to the ball which will result in the magnus effect depending on the "direction" of the spin (+yaw = counter-clockwise) (-yaw = clockwise)

//				m_football->ApplyVelocity( airFwdDir );	
				m_football->m_isCurrentlyPossessed  = false;
				for ( int i = 0; i < m_gameMode3D->m_playerList.size(); i++ )
				{
					m_gameMode3D->m_playerList[i]->m_hasTheBall			= !m_gameMode3D->m_playerList[i]->m_hasTheBall; 
					m_gameMode3D->m_playerList[i]->m_shouldTakeATouch	= !m_gameMode3D->m_playerList[i]->m_shouldTakeATouch; 
				}
				m_gameMode3D->PossessNextActor();
			}

//			// Set ball forward to THIS player's forward
// 			m_football->m_footballOrientation.m_yawDegrees = m_playerOrientation;
					
			// Reset player interaction with football variables
//			m_isDribblingTheBall = false;
//			m_ballWasJustPassed  = false;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Player3D::UpdatePlayerInput( float deltaSeconds )
{
	XboxController const& controller = g_theInput->GetController(0);
	//----------------------------------------------------------------------------------------------------------------------
	// Update player speed
	//----------------------------------------------------------------------------------------------------------------------
	// Increase speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) || (controller.GetRightTrigger() > 0.0f) )
	{
		m_currentSpeed = m_maxSpeed;
		m_footballCurrentDribbleSpeed = m_footballSprintSpeed;
	}
	// Reset speed variable to default
	else if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) || (controller.GetRightTrigger() < 0.0f) )
	{
		m_currentSpeed = m_defaultSpeed;
		m_footballCurrentDribbleSpeed = m_footballDefaultDribbleSpeed;
	}
	// Decrease speed variable
	else if ( g_theInput->IsKeyDown( KEYCODE_CONTROL ) || (controller.IsButtonDown( LEFT_SHOULDER ) ) )
	{
		m_currentSpeed = m_slowSpeed;
		m_footballCurrentDribbleSpeed = m_footballSlowSpeed;
	}
	// Reset speed variable to default
	else 
	{
		m_currentSpeed = m_defaultSpeed;
		m_footballCurrentDribbleSpeed = m_footballDefaultDribbleSpeed;
	}
	
	//----------------------------------------------------------------------------------------------------------------------
	// Calculate Player Movement
	//----------------------------------------------------------------------------------------------------------------------
	Vec2 playerMoveIntention	= Vec2::ZERO;
	Vec2 north					= Vec2(  0.0f,  1.0f );
	Vec2 south					= Vec2(  0.0f, -1.0f );
	Vec2 east					= Vec2(  1.0f,  0.0f );
	Vec2 west					= Vec2( -1.0f,  0.0f );
	// Forward
	if ( g_theInput->IsKeyDown('W') || controller.GetLeftJoyStick().GetPosition().y > 0.0f )
	{
		playerMoveIntention += north;
	}
	// Back
	if ( g_theInput->IsKeyDown( 'S' ) ||  controller.GetLeftJoyStick().GetPosition().y < 0.0f )
	{
		playerMoveIntention += south;
	}
	// Left
	if ( g_theInput->IsKeyDown( 'A' ) ||  controller.GetLeftJoyStick().GetPosition().x < 0.0f )
	{
		playerMoveIntention += west;
	}
	// Right
	if ( g_theInput->IsKeyDown( 'D' ) ||  controller.GetLeftJoyStick().GetPosition().x > 0.0f )
	{
		playerMoveIntention += east;
	}
	// Clamp playerMoveIntention
	playerMoveIntention.ClampLength( 1.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Update player orientation
	//----------------------------------------------------------------------------------------------------------------------
	if ( playerMoveIntention.GetLength() > 0.0f )
	{
//		DebuggerPrintf( "goal = %0.2f, orientation = %0.2f\n", m_playerGoalOrientationDegrees, m_playerOrientation );
		m_goalOrientationDegrees	= playerMoveIntention.GetOrientationDegrees();
		float newOrientation		= GetTurnedTowardDegrees( m_playerOrientation, m_goalOrientationDegrees, (m_turnRate * deltaSeconds) );
		m_playerOrientation			= newOrientation;
	}

//	DebuggerPrintf( "playerVelocity X = %0.2f, Y = %0.2f \n", playerVelocity.x, playerVelocity.y );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Apply updated position
	//----------------------------------------------------------------------------------------------------------------------
	if ( !m_hasTheBall )
	{
		MoveInDirection( playerMoveIntention, m_currentSpeed );
	}
//	DebuggerPrintf( "moveIntention X = %0.2f, Y = %0.2f, currentSpeed %0.2f\n", playerMoveIntention.x, playerMoveIntention.y, m_playerCurrentSpeed );
}

//----------------------------------------------------------------------------------------------------------------------
void Player3D::UpdateClampToWorldBounds()
{
	float tolerance = 0.5f;
	if ( m_position.x <= 0.0f )
	{
		m_position.x = PITCH_SIZE_X + tolerance;
	}
	else if ( m_position.x >= PITCH_SIZE_X )
	{
		m_position.x = tolerance;
	}
	else if ( m_position.y <= 0.0f )
	{
		m_position.y = PITCH_SIZE_Y + tolerance;

	}
	else if ( m_position.y >= PITCH_SIZE_Y )
	{
		m_position.y = tolerance;
	}
	else if ( m_position.z < 0.0f )
	{
		m_position.z *= -1.0f;
	}
	else if ( m_position.z >= MAX_PITCH_HEIGHT )
	{
		m_position.z *= -1.0f;
	}
}
