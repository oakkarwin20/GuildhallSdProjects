#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"

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

	//----------------------------------------------------------------------------------------------------------------------
	m_worldCamera.SetTransform( m_position, m_orientationDegrees );
	UpdatePlayerInput( deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void Player::Render() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render player compass
	std::vector<Vertex_PCU> compassVerts;
	int estimatedNumVerts	= 120; // Estimated numVerts of Compass
	compassVerts.reserve( estimatedNumVerts );
	Mat44 playerMatrix		= m_worldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 playerForward		= playerMatrix.GetIBasis3D();
	float distFromCam		= 0.2f;

	Vec3 playerCenter			= m_worldCamera.m_position + ( playerForward * distFromCam );
	float playerCompassLength	= 0.01f;
	float playerAxisThickness	= 0.001f;
	AddVertsForCompass( compassVerts, playerCenter, playerCompassLength, playerAxisThickness );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for Compass
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( compassVerts.size() ), compassVerts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void Player::AddVertsForCompass( std::vector<Vertex_PCU>& compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render stationary world compass
	// X
	Vec3 endPosX = startPosition + Vec3( axisLength, 0.0f, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosX, axisThickness, Rgba8::RED );
	// Y
	Vec3 endPosY = startPosition + Vec3( 0.0f, axisLength, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosY, axisThickness, Rgba8::GREEN );
	// Z
	Vec3 endPosZ = startPosition + Vec3( 0.0f, 0.0f, axisLength );
	AddVertsForArrow3D( compassVerts, startPosition, endPosZ, axisThickness, Rgba8::BLUE );
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
	forward.z	= 0.0f;
	left.z		= 0.0f;
	forward		= forward.GetNormalized();
	left		= left.GetNormalized();

	// Remove block directly under player
//	if ( g_theInput->IsKeyDown( KEYCODE_LEFT_MOUSE ) )
//	{
//		g_theGame->m_currentWorld->ChangeBlockTypeAtOrBelowPlayerToAir();
//	}
//	// Add block directly under player + IntVec3( 0, 0, 1 )
//	if ( g_theInput->IsKeyDown( KEYCODE_RIGHT_MOUSE ) )
//	{
//
//	}

	// Increase speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) || g_theInput->IsKeyDown( ' ' ) || controller.GetButton(BUTTON_A).m_isPressed )
	{
		m_currentSpeed = m_fasterSpeed;
	}
	// Reset speed variable to default
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) ||  g_theInput->WasKeyJustReleased( ' ' ) || ( controller.GetButton(BUTTON_A).m_wasPressedLastFrame ) )
	{
		m_currentSpeed = m_defaultSpeed;
	}

	// Decrease speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_CONTROL ) )
	{
		m_currentSpeed = m_slowerSpeed;
	}

	// Reset speed variable to default
	if ( g_theInput->WasKeyJustReleased( KEYCODE_CONTROL ) )
	{
		m_currentSpeed = m_defaultSpeed;
	}
	
	// Forward
	if ( g_theInput->IsKeyDown('W') || controller.GetLeftJoyStick().GetPosition().y > 0.0f )
	{
		m_position += ( m_currentSpeed * forward * deltaSeconds );
	}

	// Back
	if ( g_theInput->IsKeyDown( 'S' ) ||  controller.GetLeftJoyStick().GetPosition().y < 0.0f )
	{
		m_position -= ( m_currentSpeed * forward * deltaSeconds );
	}

	// Left
	if ( g_theInput->IsKeyDown( 'A' ) ||  controller.GetLeftJoyStick().GetPosition().x < 0.0f )
	{
		m_position += ( m_currentSpeed * left * deltaSeconds);
	}

	// Right
	if ( g_theInput->IsKeyDown( 'D' ) ||  controller.GetLeftJoyStick().GetPosition().x > 0.0f )
	{
		m_position -= ( m_currentSpeed * left * deltaSeconds );
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
	if ( g_theInput->IsKeyDown( 'Z' )	|| 
		 g_theInput->IsKeyDown( 'E' )	|| 
		controller.GetButton(LEFT_SHOULDER).m_isPressed  )
	{
		m_position.z += ( m_currentSpeed * deltaSeconds );
	}

	// De-Elevate
	if ( g_theInput->IsKeyDown( 'C' ) || 
		 g_theInput->IsKeyDown( 'Q' ) || 
		 controller.GetButton(RIGHT_SHOULDER).m_isPressed )
	{
		m_position.z -= ( m_currentSpeed * deltaSeconds );
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
