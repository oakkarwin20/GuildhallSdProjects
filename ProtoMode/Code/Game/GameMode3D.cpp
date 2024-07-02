#include "Game/App.hpp"
#include "Game/GameMode3D.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameMode3D::GameMode3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameMode3D::~GameMode3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::Update( float deltaSeconds )
{		
	UNUSED( deltaSeconds );
	UpdatePauseQuitAndSlowMo();
	UpdateCameraInput();
	UpdateGameMode3DCamera();
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateCameraInput()
{
	float deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();

	XboxController const& controller = g_theInput->GetController(0);
	EulerAngles orientationPerFrame = EulerAngles( m_turnRate, m_turnRate, m_turnRate );

	Vec3 forward;
	Vec3 left;
	Vec3 up;

	m_gameMode3DWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( forward, left, up );
	forward.z	= 0.0f;
	left.z		= 0.0f;
	forward		= forward.GetNormalized();
	left		= left.GetNormalized();

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
		m_gameMode3DWorldCamera.m_position += ( m_currentSpeed * forward * deltaSeconds );
	}

	// Back
	if ( g_theInput->IsKeyDown( 'S' ) ||  controller.GetLeftJoyStick().GetPosition().y < 0.0f )
	{
		m_gameMode3DWorldCamera.m_position -= ( m_currentSpeed * forward * deltaSeconds );
	}

	// Left
	if ( g_theInput->IsKeyDown( 'A' ) ||  controller.GetLeftJoyStick().GetPosition().x < 0.0f )
	{
		m_gameMode3DWorldCamera.m_position += ( m_currentSpeed * left * deltaSeconds);
	}

	// Right
	if ( g_theInput->IsKeyDown( 'D' ) ||  controller.GetLeftJoyStick().GetPosition().x > 0.0f )
	{
		m_gameMode3DWorldCamera.m_position -= ( m_currentSpeed * left * deltaSeconds );
	}

	// Pitch up
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) || controller.GetRightJoyStick().GetPosition().y > 0.0f )
	{
		m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees -= orientationPerFrame.m_pitchDegrees * deltaSeconds;
	}

	// Pitch down
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) || controller.GetRightJoyStick().GetPosition().y < 0.0f )
	{
		m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees += orientationPerFrame.m_pitchDegrees * deltaSeconds;

	}
	// Yaw left
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) || controller.GetRightJoyStick().GetPosition().x < 0.0f )
	{
		m_gameMode3DWorldCamera.m_orientation.m_yawDegrees += orientationPerFrame.m_yawDegrees * deltaSeconds;
	}

	// Yaw right 
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW || controller.GetRightJoyStick().GetPosition().x > 0.0f ) )
	{
		m_gameMode3DWorldCamera.m_orientation.m_yawDegrees -= orientationPerFrame.m_yawDegrees * deltaSeconds;
	}

	// Elevate
	if ( g_theInput->IsKeyDown( 'Z' )	|| 
		 g_theInput->IsKeyDown( 'Q' )	|| 
		controller.GetButton(LEFT_SHOULDER).m_isPressed  )
	{
		m_gameMode3DWorldCamera.m_position.z += ( m_currentSpeed * deltaSeconds );
	}

	// De-Elevate
	if ( g_theInput->IsKeyDown( 'C' ) || 
		 g_theInput->IsKeyDown( 'E' ) || 
		 controller.GetButton(RIGHT_SHOULDER).m_isPressed )
	{
		m_gameMode3DWorldCamera.m_position.z -= ( m_currentSpeed * deltaSeconds );
	}
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees = GetClamped( m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees, -85.0f, 85.0f );
	m_gameMode3DWorldCamera.m_orientation.m_rollDegrees  = GetClamped(  m_gameMode3DWorldCamera.m_orientation.m_rollDegrees, -45.0f, 45.0f );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateGameMode3DCamera()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Camera follows mouse
	//----------------------------------------------------------------------------------------------------------------------
	// Implement displacement.x to yaw and displacement.y to pitch
	//----------------------------------------------------------------------------------------------------------------------

	// Update WORLD camera as perspective
	Vec2 cursorClientDelta								  = g_theInput->GetCursorClientDelta();
	float mouseSpeed									  = 0.05f;
	float yaw											  = cursorClientDelta.x * mouseSpeed;
	float pitch											  = cursorClientDelta.y * mouseSpeed;
	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees	 -= yaw;
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees += pitch;
	m_gameMode3DWorldCamera.SetTransform( m_gameMode3DWorldCamera.m_position, m_gameMode3DWorldCamera.m_orientation );

	m_gameMode3DWorldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 10000.0f );
	m_gameMode3DWorldCamera.SetRenderBasis( Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) );

	// Update UI camera
	m_gameMode3DUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );		
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->BeginCamera( m_gameMode3DWorldCamera );

	// Initialize common render variables
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> compassVerts;

	// Render world compass
	AddVertsForCompass( compassVerts, Vec3::ZERO, 20.0f, 0.5f );

	//----------------------------------------------------------------------------------------------------------------------
	// "ProtoMode" title
	Vec3 textOrigin = Vec3( 200.0f, 130.0f, 30.0f );
	Vec3 iBasis		= Vec3(	  0.0f,  -1.0f,  0.0f );
	Vec3 jBasis		= Vec3(	  0.0f,   0.0f,  1.0f );
	g_theApp->m_textFont->AddVertsForText3D( verts, textOrigin, iBasis, jBasis, 25.0f, "ProtoMode3D!", Rgba8::GREEN );

	//----------------------------------------------------------------------------------------------------------------------
	// End world camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->EndCamera( m_gameMode3DWorldCamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw calls 
	//----------------------------------------------------------------------------------------------------------------------
	// World objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );
	
	// Compass 
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( compassVerts.size() ), compassVerts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_gameMode3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	float cellHeight	= 2.0f;
	AABB2 textbox1		= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	Vec2 alignment		= Vec2( 0.0f, 1.0f );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, "F1 (Toggle camera/player controls) || Mode (F6/F7 for prev/next) || GameMode (3D)", Rgba8::YELLOW, 0.75f, alignment, TextDrawMode::SHRINK_TO_FIT );

	// Initialize and set UI variables
	Vec2 cameraPosAlignment				= Vec2( 0.0f, 0.97f );
	Vec2 cameraOrientationAlignment		= Vec2( 0.0f, 0.94f );
	Vec2 timeAlignment					= Vec2( 1.0f, 1.0f );
	float fps							= 1.0f / g_theApp->m_gameClock.GetDeltaSeconds();
	float scale							= g_theApp->m_gameClock.GetTimeScale();
	std::string cameraPosText			= Stringf( "Camera position:        %0.2f, %0.2f, %0.2f",		m_gameMode3DWorldCamera.m_position.x,				m_gameMode3DWorldCamera.m_position.y,					m_gameMode3DWorldCamera.m_position.z );
	std::string cameraOrientationText	= Stringf( "Orientation (YPR):      %0.2f, %0.2f, %0.2f",		m_gameMode3DWorldCamera.m_orientation.m_yawDegrees, m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees,	m_gameMode3DWorldCamera.m_orientation.m_rollDegrees );
	std::string timeText				= Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.", g_theApp->m_gameClock.GetTotalSeconds(), fps, scale );
	
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			cameraPosText, Rgba8::YELLOW, 0.75f,		 cameraPosAlignment, TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			 	 timeText, Rgba8::YELLOW, 0.75f,  			  timeAlignment, TextDrawMode::SHRINK_TO_FIT );	
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,  cameraOrientationText, Rgba8::YELLOW, 0.75f, cameraOrientationAlignment, TextDrawMode::SHRINK_TO_FIT );

	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_gameMode3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw for UI camera text
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
{
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