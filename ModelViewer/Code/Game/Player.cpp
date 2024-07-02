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
	
	Vec2 cursorClientDelta	= g_theInput->GetCursorClientDelta();
	float mouseSpeed		= 0.05f;
	float yaw				= cursorClientDelta.x * mouseSpeed;
	float pitch				= cursorClientDelta.y * mouseSpeed;
	m_orientationDegrees.m_yawDegrees   -= yaw;
	m_orientationDegrees.m_pitchDegrees += pitch;

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

	Vec3 iBasis, jBasis, kBasis;
	m_worldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
	iBasis.z = 0.0f;
	jBasis.z = 0.0f;
	iBasis = iBasis.GetNormalized();
	jBasis = jBasis.GetNormalized();
	kBasis = kBasis.GetNormalized();

	// Speed up speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_speed = m_sprintSpeed;
	}
	// Speed up speed variable
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_speed = m_defaultSpeed;
	}

	// Slow down speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_CONTROL ) )
	{
		m_speed = m_slowSpeed;
	}

	// Slow down speed variable
	if ( g_theInput->WasKeyJustReleased( KEYCODE_CONTROL ) )
	{
		m_speed = m_defaultSpeed;
	}

	// Forward
	if ( g_theInput->IsKeyDown( 'W' ) )
	{
		m_position += ( m_speed * iBasis * deltaSeconds );
	}

	// Back
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_position -= ( m_speed * iBasis * deltaSeconds );
	}

	// Left
	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		m_position += ( m_speed * jBasis * deltaSeconds );
	}

	// Right
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_position -= ( m_speed * jBasis * deltaSeconds );
	}

	// Elevate
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
		m_position.z += ( m_speed * deltaSeconds );
	}

	// De-Elevate
	if ( g_theInput->IsKeyDown( 'Q' ) )
	{
		m_position.z -= ( m_speed * deltaSeconds );
	}
	m_orientationDegrees.m_pitchDegrees = GetClamped( m_worldCamera.m_orientation.m_pitchDegrees, -89.0f, 89.0f );

	// Reset to world center
	if ( g_theInput->WasKeyJustPressed( 'O' ) )
	{
		// Reset Position
		m_position = Vec3( 0.0f, 0.0f, 5.0f );

		iBasis = Vec3( 1.0f, 0.0f, 0.0f );
		jBasis = Vec3( 0.0f, 1.0f, 0.0f );
		kBasis = Vec3( 0.0f, 0.0f, 1.0f );

		// Reset Orientation
		m_orientationDegrees.m_yawDegrees   = 90.0f;
		m_orientationDegrees.m_pitchDegrees = 60.0f;
		m_orientationDegrees.m_rollDegrees  = 0.0f;
	}
}
