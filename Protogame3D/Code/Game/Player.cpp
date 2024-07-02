#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
Player::Player(Game* game)
	: Entity( game )
{
}

//----------------------------------------------------------------------------------------------------------------------
Player::~Player()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Player::Update( float deltaSeconds )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Camera follows mouse
	//----------------------------------------------------------------------------------------------------------------------
	// Implement displacement.x to yaw and displacement.y to pitch
	//----------------------------------------------------------------------------------------------------------------------
	
	Vec2 cursorClientDelta = g_theInput->GetCursorClientDelta();
	float mouseSpeed = 0.05f;
	float yaw	= cursorClientDelta.x * mouseSpeed;
	float pitch = cursorClientDelta.y * mouseSpeed;

	m_angularVelocity.m_yawDegrees	 -= yaw;
	m_angularVelocity.m_pitchDegrees += pitch;

	//if ( g_theInput->WasKeyJustPressed( 'L' ) )
	//{
//	DebuggerPrintf( "Delta %f, %f ", cursorClientDelta.x, cursorClientDelta.y );
//	DebuggerPrintf( "Yaw %f ", yaw );
//	DebuggerPrintf( "Pitch %f\n", pitch );
//	DebuggerPrintf( "Yaw is %f", yaw );
//	DebuggerPrintf( "\n" );
//	DebuggerPrintf( "Pitch is %f", pitch );
//	DebuggerPrintf( "\n" );
//	DebuggerPrintf( "cursorClientDeltaX is %f", cursorClientDelta.x );
//	DebuggerPrintf( "\n" );
//	DebuggerPrintf( "cursorClientDeltaY is %f", cursorClientDelta.y );
//	DebuggerPrintf( "\n" );

//	}

	//----------------------------------------------------------------------------------------------------------------------
	m_worldCamera.SetTransform( m_position, m_orientationDegrees );
	UpdatePlayerInput( deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void Player::Render() const
{
}

//----------------------------------------------------------------------------------------------------------------------
void Player::UpdatePlayerInput( float deltaSeconds )
{
	deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();

	XboxController const& controller = g_theInput->GetController(0);
	EulerAngles orientationPerFrame = EulerAngles( m_turnRate, m_turnRate, m_turnRate );

	Vec3 forward;
	Vec3 left;
	Vec3 up;

	m_orientationDegrees.GetAsVectors_XFwd_YLeft_ZUp( forward, left, up );

	// Speed up speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) || controller.GetButton(BUTTON_A).m_isPressed )
	{
		m_speed = 20.0f;
	}
	// Speed up speed variable
//	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) || (controller.GetButton(BUTTON_A).m_wasPressedLastFrame && !controller.GetButton(BUTTON_A).m_wasPressedLastFrame) )
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) || (controller.GetButton(BUTTON_A).m_wasPressedLastFrame) )
	{
		m_speed = 2.0f;
	}

	// Slow down speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_CONTROL ) )
	{
		m_speed = 0.5f;
	}

	// Slow down speed variable
	if ( g_theInput->WasKeyJustReleased( KEYCODE_CONTROL ) )
	{
		m_speed = 2.0f;
	}
	
	// Forward
	if ( g_theInput->IsKeyDown('W') || controller.GetLeftJoyStick().GetPosition().y > 0.0f )
	{
		m_position += ( m_speed * forward * deltaSeconds );
	}

	// Back
	if ( g_theInput->IsKeyDown( 'S' ) ||  controller.GetLeftJoyStick().GetPosition().y < 0.0f )
	{
		m_position -= ( m_speed * forward * deltaSeconds );
	}

	// Left
	if ( g_theInput->IsKeyDown( 'A' ) ||  controller.GetLeftJoyStick().GetPosition().x < 0.0f )
	{
		m_position += ( m_speed * left * deltaSeconds);
	}

	// Right
	if ( g_theInput->IsKeyDown( 'D' ) ||  controller.GetLeftJoyStick().GetPosition().x > 0.0f )
	{
		m_position -= ( m_speed * left * deltaSeconds );
	}

	// Pitch up
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) || controller.GetRightJoyStick().GetPosition().y > 0.0f )
	{
		m_angularVelocity.m_pitchDegrees -= orientationPerFrame.m_pitchDegrees * deltaSeconds;
	}

	// Pitch down
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) || controller.GetRightJoyStick().GetPosition().y < 0.0f )
	{
		m_angularVelocity.m_pitchDegrees += orientationPerFrame.m_pitchDegrees * deltaSeconds;

	}
	// Yaw left
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) || controller.GetRightJoyStick().GetPosition().x < 0.0f )
	{
		m_angularVelocity.m_yawDegrees += orientationPerFrame.m_yawDegrees * deltaSeconds;
	}

	// Yaw right 
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW || controller.GetRightJoyStick().GetPosition().x > 0.0f ) )
	{
		m_angularVelocity.m_yawDegrees -= orientationPerFrame.m_yawDegrees * deltaSeconds;
	}

	// Elevate
	if ( g_theInput->IsKeyDown( 'E' ) || controller.GetRightTrigger() )
	{
		m_position.z += ( m_speed * deltaSeconds );
	}

	// De-Elevate
	if ( g_theInput->IsKeyDown( 'Q') || controller.GetLeftTrigger() )
	{
		m_position.z -= ( m_speed * deltaSeconds );
	}

	m_orientationDegrees = m_angularVelocity;
	m_orientationDegrees.m_pitchDegrees = GetClamped( m_orientationDegrees.m_pitchDegrees, -85.0f, 85.0f );
	m_orientationDegrees.m_rollDegrees  = GetClamped(  m_orientationDegrees.m_rollDegrees, -45.0f, 45.0f );

	// Reset to world center
	if ( g_theInput->WasKeyJustPressed( 'H' ) || controller.GetButton(BUTTON_START).m_isPressed )
	{
		// Reset Position
		m_position	= Vec3( 0.0f, 0.0f, 0.0f );

		forward		= Vec3( 1.0f, 0.0f, 0.0f );
		left		= Vec3( 0.0f, 1.0f, 0.0f );
		up			= Vec3( 0.0f, 0.0f, 1.0f );

		// Reset Orientation
		m_orientationDegrees.m_yawDegrees	= 0.0f;
		m_orientationDegrees.m_pitchDegrees = 0.0f;
		m_orientationDegrees.m_rollDegrees	= 0.0f;
	}
		m_angularVelocity = m_orientationDegrees;
}
