#include "Game/Player2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Football2D.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
Player2D::Player2D( Football2D* football )
{
	m_football = football;
}

//----------------------------------------------------------------------------------------------------------------------
Player2D::~Player2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Player2D::Update( float deltaSeconds )
{
	UpdatePhysics( deltaSeconds );
	UpdatePlayerInput( deltaSeconds );
	UpdateFootballPhysics();
	UpdateClampToWorldBounds();
}

//----------------------------------------------------------------------------------------------------------------------
void Player2D::Render( std::vector<Vertex_PCU>& outVerts )
{
	// Render Physics radius
	AddVertsForRing2D( outVerts, m_playerPosition, m_playerPhysicsRadius, m_lineThickness, m_playerPhysicsRadiusColor );

	// Render Possession radius
	AddVertsForRing2D( outVerts, m_playerPosition, m_playerPossessionRadius, m_lineThickness, m_playerPhysicsRadiusColor );

	// Render Player
	m_playerForwardDir = m_playerPosition.MakeFromPolarDegrees( m_playerOrientation, 2.0f );
	m_playerForwardDir = m_playerForwardDir.GetNormalized();
	AddVertsForArrow2D( outVerts, m_playerPosition, (m_playerPosition + m_playerForwardDir), m_arrowSize, m_lineThickness, m_playerColor );
}

//----------------------------------------------------------------------------------------------------------------------
void Player2D::UpdatePhysics( float deltaSeconds )
{
	// Decelerate every frame with drag
	Vec2 decelerationForce = -m_playerDrag * m_playerVelocity;
	AddForce( decelerationForce );


	// Apply physics movement
	m_playerVelocity += m_playerAcceleration * deltaSeconds;
	m_playerPosition += m_playerVelocity * deltaSeconds;

	// Reset acceleration
	m_playerAcceleration = Vec2::ZERO;
}

//----------------------------------------------------------------------------------------------------------------------
void Player2D::AddForce( Vec2 forceAmount )
{
	m_playerAcceleration = m_playerAcceleration + forceAmount;
}

//----------------------------------------------------------------------------------------------------------------------
void Player2D::MoveInDirection( Vec2 directionToMove, float speed )
{
	// Add Force in direction of target
	Vec2 forceAmount = directionToMove * m_playerDrag * speed;			// Direction should be normalized
	AddForce( forceAmount );
//	DebuggerPrintf( "forceAmount X = %0.2f, Y = %0.2f, m_playerAcceleration X = %0.2f, Y = %0.2f \n", forceAmount.x, forceAmount.y, m_playerAcceleration.x, m_playerAcceleration.y );
}

//----------------------------------------------------------------------------------------------------------------------
void Player2D::UpdateFootballPhysics()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Update the ball's physics is player currently possesses the ball
	//----------------------------------------------------------------------------------------------------------------------
//	if ( m_ballIsPossessed && m_ballIsInPossessionRange )
	if ( m_ballIsInPossessionRange )
	{
		if ( m_shootTheBall )
		{
		 	float shootVelocity = m_football->m_footballVelocity.GetLength() * m_footballShootSpeed;
			m_football->MoveInDirection( m_playerForwardDir, shootVelocity );
		}
		else if ( m_passTheBall )
		{
		 	float passVelocity = m_football->m_footballVelocity.GetLength() * m_footballPassSpeed;
			m_football->MoveInDirection( m_playerForwardDir, passVelocity );
		}
		else // If dribbling
		{
			m_football->MoveInDirection( m_playerForwardDir, m_footballDribbleSpeed );
		}
		
		// Reset player interaction with football variables
		m_shootTheBall = false;
		m_passTheBall  = false;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Player2D::UpdatePlayerInput( float deltaSeconds )
{
	XboxController const& controller = g_theInput->GetController(0);

	//----------------------------------------------------------------------------------------------------------------------
	// Update Player to Football controls
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->WasKeyJustPressed( ' ' ) || controller.WasButtonJustPressed(BUTTON_B) )
	{
		// Shoot 
		m_shootTheBall = true;
	}
	if ( g_theInput->WasKeyJustPressed( 'J') || controller.WasButtonJustPressed(BUTTON_A) )
	{
		// Shoot 
		m_passTheBall = true;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Update player speed
	//----------------------------------------------------------------------------------------------------------------------
	// Increase speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) || (controller.GetRightTrigger() > 0.0f) )
	{
		m_playerCurrentSpeed = m_playerMaxSpeed;
	}
	// Reset speed variable to default
	else if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) || (controller.GetRightTrigger() < 0.0f) )
	{
		m_playerCurrentSpeed = m_playerDefaultSpeed;
	}
	// Decrease speed variable
	else if ( g_theInput->IsKeyDown( KEYCODE_CONTROL ) || (controller.IsButtonDown( LEFT_SHOULDER ) ) )
	{
		m_playerCurrentSpeed = m_playerSlowSpeed;
	}
	// Reset speed variable to default
	else 
	{
		m_playerCurrentSpeed = m_playerDefaultSpeed;
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
		m_playerGoalOrientationDegrees	= playerMoveIntention.GetOrientationDegrees();
		float newOrientation			= GetTurnedTowardDegrees( m_playerOrientation, m_playerGoalOrientationDegrees, (m_playerTurnRate * deltaSeconds) );
		m_playerOrientation				= newOrientation;
//		Vec2 fowardNormal				= m_playerPosition.GetFowardNormal( m_playerOrientation );
//		m_playerVelocity				= fowardNormal * ( m_playerCurrentSpeed * playerMoveIntention.GetLength() );
	}

//	DebuggerPrintf( "playerVelocity X = %0.2f, Y = %0.2f \n", playerVelocity.x, playerVelocity.y );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Apply updated position
	//----------------------------------------------------------------------------------------------------------------------
	MoveInDirection( playerMoveIntention, m_playerCurrentSpeed );
//	DebuggerPrintf( "moveIntention X = %0.2f, Y = %0.2f, currentSpeed %0.2f\n", playerMoveIntention.x, playerMoveIntention.y, m_playerCurrentSpeed );
}

//----------------------------------------------------------------------------------------------------------------------
void Player2D::UpdateClampToWorldBounds()
{
	if ( m_playerPosition.x <= 0.0f )
	{
		m_playerPosition.x *= -1.0f;
	}
	else if ( m_playerPosition.x >= WORLD_SIZE_X )
	{
		m_playerPosition.x *= -1.0f; 
	}
	else if ( m_playerPosition.y <= 0.0f )
	{
		m_playerPosition.y *= -1.0f;
	}
	else if ( m_playerPosition.y >= WORLD_SIZE_Y )
	{
		m_playerPosition.y *= -1.0f;
	}
}
