#include "Game/App.hpp"
#include "Game/GameMode_EulerTest.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/SkeletalSystem/CreatureBase.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameMode_EulerTest::GameMode_EulerTest()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Bezier Curve
	//----------------------------------------------------------------------------------------------------------------------
	m_timer.Restart();
}

//----------------------------------------------------------------------------------------------------------------------
GameMode_EulerTest::~GameMode_EulerTest()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_EulerTest::Startup()
{
	g_debugFreeFly_F1 = true;

//	InitializeCreature();

	Vec3 fwdDir, leftDir, upDir;
	m_eulerControlled.GetAsVectors_XFwd_YLeft_ZUp( fwdDir, leftDir, upDir );
	m_joint.m_leftDir_lastFrame	= leftDir;
	m_joint.m_upDir_lastFrame	= upDir;
	m_joint.m_eulerLS_LastFrame	= m_eulerControlled;


	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Player cam pos and orientation
	//----------------------------------------------------------------------------------------------------------------------
//	m_gameMode3DWorldCamera.m_position	  = Vec3( -568.0f,  -15.0f, 220.0f );
	m_gameMode3DWorldCamera.m_position	  = Vec3( -795.0f, -250.0f, 220.0f );
	m_gameMode3DWorldCamera.m_orientation = EulerAngles( 0.0f, 17.0f, 0.0f );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_EulerTest::Update( float deltaSeconds )
{	
	// Update core systems
	UpdatePauseQuitAndSlowMo();
	UpdateDebugKeys();
	UpdateCameraInput( deltaSeconds );
	UpdateInputDebugPosEE( deltaSeconds );

	// Update joint euler ONLY from euler1's fwdDir
	Vec3 fwdDir, leftDir, upDir;
	m_eulerControlled.GetAsVectors_XFwd_YLeft_ZUp( fwdDir, leftDir, upDir );
// 	if ( m_eulerControlled.m_pitchDegrees > 90.0f )
// 	{
// 		int x = 0;
// 	}

	// Compute different cases of EulerAngles from matrix
	MatrixToEulerPreference prevPreference	= m_joint.m_matrixToEulerPreference;

	m_joint.m_matrixToEulerPreference		= FWD_TEMPORAL;
	m_joint.m_eulerAnglesGoal_LS			= m_lastFrameEuler_FwdTemporal;						// Set last frame euler for reference
	m_jointEuler_FwdTemporal				= m_joint.GetEulerFromFwdDir(fwdDir);
	m_joint.m_eulerAnglesGoal_LS			= m_jointEuler_FwdTemporal;

	m_joint.m_matrixToEulerPreference		= FWD_LEFT_DEFAULT;
	m_joint.m_eulerAnglesGoal_LS			= m_lastFrameEuler_FwdAndLeft;
	m_jointEuler_FwdAndLeft					= m_joint.GetEulerFromFwdAndLeft( fwdDir, leftDir );
	m_lastFrameEuler_FwdAndLeft				= m_jointEuler_FwdAndLeft;

	m_joint.m_matrixToEulerPreference		= PITCH_PREFERRED;
	m_joint.m_eulerAnglesGoal_LS			= m_lastFrameEuler_FwdOnlyPitchPrefered;
	m_jointEuler_FwdOnlyPitchPrefered		= m_joint.GetEulerFromFwdDir(fwdDir);
	m_lastFrameEuler_FwdOnlyPitchPrefered	= m_jointEuler_FwdOnlyPitchPrefered;

	m_joint.m_eulerAnglesGoal_LS			= m_lastFrameEuler_FwdOnlyYawPrefered;
	m_joint.m_matrixToEulerPreference		= YAW_PREFERRED;
	m_jointEuler_FwdOnlyYawPrefered			= m_joint.GetEulerFromFwdDir(fwdDir);
	m_lastFrameEuler_FwdOnlyYawPrefered		= m_jointEuler_FwdOnlyYawPrefered;

	m_joint.m_matrixToEulerPreference		= FWD_LEFT_TEMPORAL;
	m_joint.m_eulerAnglesGoal_LS			= m_lastFrameEuler_FwdAndLeftTemporal;						// Set last frame euler for reference
	m_jointEuler_FwdAndLeftTemporal			= m_joint.GetEulerFromFwdAndLeft( fwdDir, leftDir );		// Compute euler
	m_lastFrameEuler_FwdAndLeftTemporal		= m_jointEuler_FwdAndLeftTemporal;							// Update euler for next frame

	m_joint.m_matrixToEulerPreference		= FWD_LEFT_ALTERNATE;
	m_joint.m_eulerAnglesGoal_LS			= m_lastFrameEuler_FwdAndLeftAlternate;				
	m_jointEuler_FwdAndLeftAlternate		= m_joint.GetEulerFromFwdAndLeft( fwdDir, leftDir );
	m_lastFrameEuler_FwdAndLeftAlternate	= m_jointEuler_FwdAndLeftAlternate;					

	m_joint.m_matrixToEulerPreference		= FWD_LEFT_CUSTOM;
	m_joint.m_eulerAnglesGoal_LS			= m_lastFrameEuler_FwdAndLeftCusom;				
	m_jointEuler_FwdAndLeftCustom			= m_joint.GetEulerFromFwdAndLeft( fwdDir, leftDir );
	m_lastFrameEuler_FwdAndLeftCusom		= m_jointEuler_FwdAndLeftCustom;					

	m_joint.m_matrixToEulerPreference		= prevPreference;

	// Update Camera
	UpdateGameMode3DCamera();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_EulerTest::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_EulerTest::Shutdown()
{
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_EulerTest::UpdateDebugKeys()
{
	// Toggle common debug bools
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		g_debugFreeFly_F1 = !g_debugFreeFly_F1;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F2 ) )
	{
		g_debugRenderRaycast_F2 = !g_debugRenderRaycast_F2;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F3 ) )
	{
		g_debugBasis_F3 = !g_debugBasis_F3;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F4 ) )
	{
		g_debugText_F4 = !g_debugText_F4;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F5 ) )
	{
		g_debugAngles_F5 = !g_debugAngles_F5;
	}

	// Control camera dist from player
	if ( g_theInput->IsKeyDown( '9' ) )
	{
		m_distCamAwayFromPlayer += 10.0f;
	}
	if ( g_theInput->IsKeyDown( '0' ) )
	{
		m_distCamAwayFromPlayer -= 10.0f;
	}

	// Preset camera positions
//	float distFromOrigin = 150.0f;
	if ( g_theInput->WasKeyJustPressed( 'X' ) )
	{
		// Look down X-axis (YZ plane)
		m_gameMode3DWorldCamera.m_position	  = Vec3( 591.0f, 0.0f, 135.0f );
		m_gameMode3DWorldCamera.m_orientation = EulerAngles( 180.0f, 5.0f, 0.0f );
	}
	if ( g_theInput->WasKeyJustPressed( 'Y' ) )
	{
		// Look down Y-axis (XZ plane)
		m_gameMode3DWorldCamera.m_position	  = Vec3( 261.0f, 658.0f, 120.0f );
		m_gameMode3DWorldCamera.m_orientation = EulerAngles( -131.0f, 12.0f, 0.0f );
	}
	if ( g_theInput->WasKeyJustPressed( 'Z' ) )
	{
		// Look down Z-axis (XY plane)
		m_gameMode3DWorldCamera.m_position	  = Vec3( 2.0f, 0.0f, 575.0f );
		m_gameMode3DWorldCamera.m_orientation = EulerAngles( 0.0f, 85.0f, 0.0f );
	}
	if ( g_theInput->WasKeyJustPressed( 'C' ) )
	{
		// Look down negative X-axis (XY plane)
		m_gameMode3DWorldCamera.m_position	  = Vec3( -795.0f,  -250.0f, 220.0f );
		m_gameMode3DWorldCamera.m_orientation = EulerAngles( 0.0f, 17.0f, 0.0f );
	}
	if ( g_theInput->WasKeyJustPressed( 'V' ) )
	{
		// Look down negative X-axis (XY plane)
		m_gameMode3DWorldCamera.m_position	  = Vec3( -428.0f, -464.0f, 170.0f );
		m_gameMode3DWorldCamera.m_orientation = EulerAngles( 26.5f, 13.0f, 0.0f );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_EulerTest::UpdateCameraInput( float deltaSeconds )
{
	if ( g_debugFreeFly_F1 == false )
	{
		return;
	}

	Vec3 iBasis, jBasis, kBasis;
	m_gameMode3DWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
	iBasis.z = 0.0f;
	jBasis.z = 0.0f;
	iBasis	 = iBasis.GetNormalized(); 
	jBasis	 = jBasis.GetNormalized(); 
	kBasis   = kBasis.GetNormalized();

	//----------------------------------------------------------------------------------------------------------------------
	// All directions are local
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_currentSpeed = m_fasterSpeed;
	}
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_currentSpeed = m_defaultSpeed;
	}
	// Forward
	if ( g_theInput->IsKeyDown( 'W' ) )
	{
		m_gameMode3DWorldCamera.m_position += ( iBasis * m_currentSpeed ) * deltaSeconds;
	}
	// Left
	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		m_gameMode3DWorldCamera.m_position += ( jBasis * m_currentSpeed ) * deltaSeconds;
	}
	// Backwards
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_gameMode3DWorldCamera.m_position -= ( iBasis * m_currentSpeed ) * deltaSeconds;
	}
	// Right
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_gameMode3DWorldCamera.m_position -= ( jBasis * m_currentSpeed ) * deltaSeconds;
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
//		m_gameMode3DWorldCamera.m_position += ( kBasis * m_currentSpeed ) * deltaSeconds;
		m_gameMode3DWorldCamera.m_position += ( Vec3( 0.0f, 0.0f, 1.0f ) * m_currentSpeed ) * deltaSeconds;
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'Q' ) )
	{
//		m_gameMode3DWorldCamera.m_position -= ( kBasis * m_currentSpeed ) * deltaSeconds;
		m_gameMode3DWorldCamera.m_position -= ( Vec3( 0.0f, 0.0f, 1.0f ) * m_currentSpeed ) * deltaSeconds;
	}

	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees = GetClamped( m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees, -85.0f, 85.0f );
	m_gameMode3DWorldCamera.m_orientation.m_rollDegrees  = GetClamped(  m_gameMode3DWorldCamera.m_orientation.m_rollDegrees, -45.0f, 45.0f );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_EulerTest::UpdateGameMode3DCamera()
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
	
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees = GetClamped( m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees, -85.0f, 85.0f );

	if ( g_debugFreeFly_F1 )
	{
		// Set cameraPos roam "freely"
		m_gameMode3DWorldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 10000.0f );
		m_gameMode3DWorldCamera.SetRenderBasis( Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) );
	}

	// Update UI camera
	m_gameMode3DUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );		
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_EulerTest::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->BeginCamera( m_gameMode3DWorldCamera );

	// Initialize common render variables
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> compassVerts;
	std::vector<Vertex_PCU> verts_Backface;

	//----------------------------------------------------------------------------------------------------------------------
	// Rings for each basis
	//----------------------------------------------------------------------------------------------------------------------
	// Render world compass 1 (world origin, matrix to euler fwd only)
	Vec3  arrowStart	= Vec3::ZERO;
	float axisLength	= 100.0f;
	float axisThickness =   1.0f;
	float ringThickness =	5.0f;
	Rgba8 ringColor		= Rgba8::MAGENTA;
	AddVertsForWorldBasis( compassVerts, arrowStart, axisLength, axisThickness );
	AddVertsForRing2D ( verts_Backface, Vec2::ZERO, axisLength, ringThickness, ringColor );
	// Render world compass 2 (-Y 250, matrix to Euler fwd AND left)
	arrowStart			= Vec3( 0.0f, -250.0f, 0.0f );
	AddVertsForWorldBasis( compassVerts, arrowStart, axisLength, axisThickness );
	AddVertsForRing2D ( verts_Backface, Vec2( arrowStart.x, arrowStart.y ), axisLength, ringThickness, ringColor );
	// Render world compass 3 (-Y 500, matrix to Euler fwd AND left, UNROLLED)
	arrowStart			= Vec3( 0.0f, -500.0f, 0.0f );
	AddVertsForWorldBasis( compassVerts, arrowStart, axisLength, axisThickness );
	AddVertsForRing2D ( verts_Backface, Vec2( arrowStart.x, arrowStart.y ), axisLength, ringThickness, ringColor );
	// Render world compass 4 (-Y 750, matrix to Euler fwd AND left, ALTERNATE)
	arrowStart			= Vec3( 0.0f, -750.0f, 0.0f );
	AddVertsForWorldBasis( compassVerts, arrowStart, axisLength, axisThickness );
	AddVertsForRing2D ( verts_Backface, Vec2( arrowStart.x, arrowStart.y ), axisLength, ringThickness, ringColor );
	// Render world compass 4 (-Y 750, matrix to Euler fwd AND left, CUSTOM)
	arrowStart			= Vec3( 0.0f, -1000.0f, 0.0f );
	AddVertsForWorldBasis( compassVerts, arrowStart, axisLength, axisThickness );
	AddVertsForRing2D ( verts_Backface, Vec2( arrowStart.x, arrowStart.y ), axisLength, ringThickness, ringColor );
	// Render world compass 6 (+Y 250, don't negate negative sign for ATAN2)
	arrowStart			= Vec3(0.0f, 250.0f, 0.0f);
	AddVertsForWorldBasis( compassVerts, arrowStart, axisLength, axisThickness );
	AddVertsForRing2D ( verts_Backface, Vec2( arrowStart.x, arrowStart.y ), axisLength, ringThickness, ringColor );
	// Render world compass 7 (+Y 500, negate negative sign for ATAN2)
	arrowStart			= Vec3(0.0f, 500.0f, 0.0f);
	AddVertsForWorldBasis( compassVerts, arrowStart, axisLength, axisThickness );
	AddVertsForRing2D ( verts_Backface, Vec2( arrowStart.x, arrowStart.y ), axisLength, ringThickness, ringColor );


	//----------------------------------------------------------------------------------------------------------------------
	// (World origin) matrix to euler FWD only
	//----------------------------------------------------------------------------------------------------------------------
	// Debug render ambiguous matrix to euler conversions 
	arrowStart							= Vec3::ZERO;		// Reset debug arrow start pos
	axisLength							= 75.0f;
	axisThickness						= 2.0f;
	float axisLength_computedEuler		= 50.0f;
	float axisThickness_computedEuler	= 3.0f;
	RenderControlledBasis( compassVerts, arrowStart, axisLength, axisThickness );
	// Joint euler computed ONLY from forward
	RenderComputedBasis_MatrixToEuler( compassVerts, m_jointEuler_FwdTemporal, arrowStart, axisLength_computedEuler, axisThickness_computedEuler );

	//----------------------------------------------------------------------------------------------------------------------
	// -Y offset from world origin, matrix to euler FWD AND LEFT
	//----------------------------------------------------------------------------------------------------------------------
	// Render controlled Euler basis
	arrowStart = Vec3( 0.0f, -250.0f, 0.0f );
	// Debug render ambiguous matrix to euler conversions 
	RenderControlledBasis( compassVerts, arrowStart, axisLength, axisThickness );
	// Joint euler computed fwd AND left
	RenderComputedBasis_MatrixToEuler( compassVerts, m_jointEuler_FwdAndLeft, arrowStart, axisLength_computedEuler, axisThickness_computedEuler );
	
	//----------------------------------------------------------------------------------------------------------------------
	// -Y offset from world origin, matrix to euler FWD AND LEFT (TEMPORAL)
	//----------------------------------------------------------------------------------------------------------------------
	// Render controlled Euler basis
	arrowStart = Vec3( 0.0f, -500.0f, 0.0f );
	// Debug render ambiguous matrix to euler conversions 
	RenderControlledBasis( compassVerts, arrowStart, axisLength, axisThickness );
	// Joint euler computed fwd AND left
	RenderComputedBasis_MatrixToEuler( compassVerts, m_jointEuler_FwdAndLeftTemporal, arrowStart, axisLength_computedEuler, axisThickness_computedEuler );

	//----------------------------------------------------------------------------------------------------------------------
	// -Y offset from world origin, matrix to euler FWD AND LEFT (ALTERNATE)
	//----------------------------------------------------------------------------------------------------------------------
	// Render controlled Euler basis
	arrowStart = Vec3( 0.0f, -750.0f, 0.0f );
	// Debug render ambiguous matrix to euler conversions 
	RenderControlledBasis( compassVerts, arrowStart, axisLength, axisThickness );
	// Joint euler computed fwd AND left
	RenderComputedBasis_MatrixToEuler( compassVerts, m_jointEuler_FwdAndLeftAlternate, arrowStart, axisLength_computedEuler, axisThickness_computedEuler );

	//----------------------------------------------------------------------------------------------------------------------
	// -Y offset from world origin, matrix to euler FWD AND LEFT (CUSTOM)
	//----------------------------------------------------------------------------------------------------------------------
	// Render controlled Euler basis
	arrowStart = Vec3( 0.0f, -1000.0f, 0.0f );
	// Debug render ambiguous matrix to euler conversions 
	RenderControlledBasis( compassVerts, arrowStart, axisLength, axisThickness );
	// Joint euler computed fwd AND left
	RenderComputedBasis_MatrixToEuler( compassVerts, m_jointEuler_FwdAndLeftCustom, arrowStart, axisLength_computedEuler, axisThickness_computedEuler );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Yaw preferred matrix to Euler conversion
	//----------------------------------------------------------------------------------------------------------------------
	// Render controlled Euler basis
	arrowStart = Vec3( 0.0f, 250.0f, 0.0f );	// Reset debug arrow start pos
	RenderControlledBasis( compassVerts, arrowStart, axisLength, axisThickness );
	// Joint euler computed, yaw preferred
	RenderComputedBasis_MatrixToEuler( compassVerts, m_jointEuler_FwdOnlyYawPrefered, arrowStart, axisLength_computedEuler, axisThickness_computedEuler );

	//----------------------------------------------------------------------------------------------------------------------
	// Pitch preferred matrix to Euler conversion
	//----------------------------------------------------------------------------------------------------------------------
	// Render controlled Euler basis
	arrowStart		= Vec3( 0.0f, 500.0f, 0.0f );	// Reset debug arrow start pos
	RenderControlledBasis( compassVerts, arrowStart, axisLength, axisThickness );
	// Joint euler computed, yaw preferred
	RenderComputedBasis_MatrixToEuler( compassVerts, m_jointEuler_FwdOnlyPitchPrefered, arrowStart, axisLength_computedEuler, axisThickness_computedEuler );
	

	//----------------------------------------------------------------------------------------------------------------------
	// "ProtoMode" title
	//----------------------------------------------------------------------------------------------------------------------
	Rgba8 color_darkRed		= Rgba8::GREEN;
	Rgba8 color_darkGreen	= Rgba8::WHITE;
	Vec3  iBasis			= Vec3(	  0.0f,  -1.0f,  0.0f );
	Vec3  jBasis			= Vec3(	  0.0f,   0.0f,  1.0f );
	float textZ_ypr			= 130.0f;
	float textZ_type		= 120.0f;
	float textZ_mode		= 110.0f;
	g_theApp->m_textFont->AddVertsForText3D( textVerts,	Vec3(100.0f, 300.0f, 150.0f), iBasis, jBasis, 45.0f, "Euler from Vector Visual Test 3D!",		color_darkRed, 0.8f );

	// Euler from fwd ONLY, pitch preferred
	std::string euler_fwdOnlyPitchPref = Stringf( "Y:%0.2f, P:%0.2f, R:%0.2f", m_jointEuler_FwdOnlyPitchPrefered.m_yawDegrees, m_jointEuler_FwdOnlyPitchPrefered.m_pitchDegrees, m_jointEuler_FwdOnlyPitchPrefered.m_rollDegrees );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f,  620.0f, textZ_ypr  ), iBasis, jBasis, 10.0f,  euler_fwdOnlyPitchPref,				color_darkRed,	 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f,  560.0f, textZ_type ), iBasis, jBasis, 10.0f, "Euler from Fwd",						color_darkGreen, 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f,  560.0f, textZ_mode	), iBasis, jBasis, 10.0f, "PitchPreferred",						color_darkGreen, 0.8f );

	// Euler from fwd ONLY, yaw preferred
	std::string euler_fwdOnlyYawPref = Stringf( "Y:%0.2f, P:%0.2f, R:%0.2f", m_jointEuler_FwdOnlyYawPrefered.m_yawDegrees, m_jointEuler_FwdOnlyYawPrefered.m_pitchDegrees, m_jointEuler_FwdOnlyYawPrefered.m_rollDegrees );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, 350.0f, textZ_ypr  ), iBasis, jBasis, 10.0f, euler_fwdOnlyYawPref, color_darkRed, 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, 310.0f, textZ_type ), iBasis, jBasis, 10.0f, "Euler from Fwd",						color_darkGreen, 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, 300.0f, textZ_mode ), iBasis, jBasis, 10.0f, "YawPreferred",						color_darkGreen, 0.8f );

	// Euler from fwd ONLY, TEMPORAL
	std::string euler_fwdOnly = Stringf( "Y:%0.2f, P:%0.2f, R:%0.2f", m_jointEuler_FwdTemporal.m_yawDegrees, m_jointEuler_FwdTemporal.m_pitchDegrees, m_jointEuler_FwdTemporal.m_rollDegrees );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f,  95.0f, textZ_ypr  ), iBasis, jBasis, 10.0f,  euler_fwdOnly,						color_darkRed,	 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f,  55.0f, textZ_type ), iBasis, jBasis, 10.0f, "Euler from Fwd",						color_darkGreen, 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f,  35.0f, textZ_mode ), iBasis, jBasis, 10.0f, "Temporal",							color_darkGreen, 0.8f );

	// Euler from fwd AND left DEFAULT (yaw preferred)
	std::string euler_fwdAndLeft = Stringf( "Y:%0.2f, P:%0.2f, R:%0.2f", m_jointEuler_FwdAndLeft.m_yawDegrees, m_jointEuler_FwdAndLeft.m_pitchDegrees, m_jointEuler_FwdAndLeft.m_rollDegrees );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, -150.0f, textZ_ypr  ), iBasis, jBasis, 10.0f,  euler_fwdAndLeft,					color_darkRed,	 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, -160.0f, textZ_type ), iBasis, jBasis, 10.0f, "Euler from Fwd & Left",				color_darkGreen, 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, -220.0f, textZ_mode	), iBasis, jBasis, 10.0f, "Default",							color_darkGreen, 0.8f );

	// Euler from fwd AND left (TEMPORAL)
	std::string euler_fwdAndLeftUnrolled = Stringf( "Y:%0.2f, P:%0.2f, R:%0.2f", m_jointEuler_FwdAndLeftTemporal.m_yawDegrees, m_jointEuler_FwdAndLeftTemporal.m_pitchDegrees, m_jointEuler_FwdAndLeftTemporal.m_rollDegrees );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, -390.0f, textZ_ypr  ), iBasis, jBasis, 10.0f,  euler_fwdAndLeftUnrolled,			color_darkRed,	 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, -410.0f, textZ_type ), iBasis, jBasis, 10.0f, "Euler from Fwd & Left",				color_darkGreen, 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, -470.0f, textZ_mode	), iBasis, jBasis, 10.0f, "Temporal",							color_darkGreen, 0.8f );

	// Euler from fwd AND left (ALTERNATE)
	std::string euler_fwdAndLeftAlternate = Stringf( "Y:%0.2f, P:%0.2f, R:%0.2f", m_jointEuler_FwdAndLeftAlternate.m_yawDegrees, m_jointEuler_FwdAndLeftAlternate.m_pitchDegrees, m_jointEuler_FwdAndLeftAlternate.m_rollDegrees );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, -625.0f, textZ_ypr  ), iBasis, jBasis, 10.0f, euler_fwdAndLeftAlternate,			color_darkRed, 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, -675.0f, textZ_type ), iBasis, jBasis, 10.0f, "Euler from Fwd & Left",				color_darkGreen, 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, -710.0f, textZ_mode ), iBasis, jBasis, 10.0f, "Alternate",							color_darkGreen, 0.8f );

	// Euler from fwd AND left (CUSTOM)
	std::string euler_fwdAndLeftCustom = Stringf( "Y:%0.2f, P:%0.2f, R:%0.2f", m_jointEuler_FwdAndLeftCustom.m_yawDegrees, m_jointEuler_FwdAndLeftCustom.m_pitchDegrees, m_jointEuler_FwdAndLeftCustom.m_rollDegrees );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, -900.0f, textZ_ypr  ), iBasis, jBasis, 10.0f,  euler_fwdAndLeftCustom,				color_darkRed,	 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, -920.0f, textZ_type ), iBasis, jBasis, 10.0f, "Euler from Fwd & Left",				color_darkGreen, 0.8f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, Vec3( 0.0f, -980.0f, textZ_mode ), iBasis, jBasis, 10.0f, "Custom",								color_darkGreen, 0.8f );


	//----------------------------------------------------------------------------------------------------------------------
	// Render skybox
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> verts_skybox;
	AddVertsForSphere3D( verts_skybox, Vec3( 0.0f, 0.0f, 100.0f ), 2000.0f, 32.0f, 32.0f );
	//	AddVertsForQuad3D( verts_skybox, Vec3(1000.0f, -1000.0f, -1000.0f), Vec3(-1000.0f, -1000.0f, -1000.0f), Vec3(-1000.0f, -1000.0f, 1000.0f), Vec3(1000.0f, -1000.0f, 1000.0f), Rgba8::WHITE, AABB2::ZERO_TO_ONE );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->SetModelConstants( Mat44(), Rgba8::DARKER_GRAY );
	g_theRenderer->BindTexture( nullptr );
//	g_theRenderer->BindTexture( g_theApp->m_texture_Skybox );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_skybox.size() ), verts_skybox.data() );
	// Reset bindings
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );

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
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );

	// Backface objects objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_Backface.size() ), verts_Backface.data() );
	
	// Text objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	// Reset binded texture
	g_theRenderer->BindTexture( nullptr );

	// Compass 
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( compassVerts.size() ), compassVerts.data() );

	DebugRenderWorld( m_gameMode3DWorldCamera );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_EulerTest::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_gameMode3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	Rgba8 color_lightGreen	= Rgba8::GREEN;
	Rgba8 color_darkGreen	= Rgba8::DARK_GREEN;
	float cellHeight		= 2.0f;
	AABB2 textbox1			= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	Vec2 alignment			= Vec2( 0.0f, 1.0f );
	
	std::string gameInfo;
	if ( g_debugFreeFly_F1 )
	{
		gameInfo = Stringf( "F1 (Toggle camera/player) Camera || Mode (F6/F7 for prev/next) || Euler Visual Test (3D)" );
	}						
	else
	{
		gameInfo = Stringf( "F1 (Toggle camera/player) Player || Mode (F6/F7 for prev/next) || Euler Visual Test (3D)" );
	}
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, gameInfo, color_lightGreen, 0.75f, alignment, TextDrawMode::SHRINK_TO_FIT );

	// Initialize and set UI variables
	Vec2 timeAlignment					= Vec2( 1.0f, 1.0f  );

	Vec2 cameraPosAlignment				= Vec2( 0.0f, 0.97f );
	Vec2 cameraOrientationAlignment		= Vec2( 0.0f, 0.94f );
	Vec2 treeOrientationAlignment		= Vec2( 0.0f, 0.91f );

	Vec2 yprAlignment					= Vec2( 0.0f, 0.88f );

	Vec2 limb1StartPosAlignment			= Vec2( 0.0f, 0.86f );
	Vec2 limb1EndPosAlignment			= Vec2( 0.0f, 0.83f );
	Vec2 limb2StartPosAlignment			= Vec2( 0.0f, 0.80f );
	Vec2 limb2EndPosAlignment			= Vec2( 0.0f, 0.77f );
	Vec2 debugTreeTargetPosAlignment	= Vec2( 0.0f, 0.74f );
	Vec2 Limb1FwdAlignment				= Vec2( 0.0f, 0.71f );
	Vec2 Limb1LeftAlignment				= Vec2( 0.0f, 0.68f );
	Vec2 Limb1UpAlignment				= Vec2( 0.0f, 0.65f );

	// Core Values
	float fps							= 1.0f / g_theApp->m_gameClock.GetDeltaSeconds();
	float scale							= g_theApp->m_gameClock.GetTimeScale();
	std::string cameraPosText			= Stringf( "Cam position:           %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_position.x,					m_gameMode3DWorldCamera.m_position.y,						m_gameMode3DWorldCamera.m_position.z );
	std::string cameraOrientationText	= Stringf( "Cam Orientation (YPR):  %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees,		m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees,		m_gameMode3DWorldCamera.m_orientation.m_rollDegrees );
	std::string timeText				= Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.",			g_theApp->m_gameClock.GetTotalSeconds(), fps, scale );
	std::string euler1					= Stringf( "Euler (controlled): %0.2f, %0.2f, %0.2f", m_eulerControlled.m_yawDegrees, m_eulerControlled.m_pitchDegrees, m_eulerControlled.m_rollDegrees );
//	std::string euler1					= Stringf( "Euler (controlled):        %0.2f, %0.2f, %0.2f",			 			   m_euler1.m_yawDegrees,							m_euler1.m_pitchDegrees,						  m_euler1.m_rollDegrees );
// 	std::string eulerFromFwdLeft		= Stringf( "Euler fwd left:            %0.2f, %0.2f, %0.2f",			 m_jointEuler_FwdAndLeft.m_yawDegrees,			 m_jointEuler_FwdAndLeft.m_pitchDegrees,		   m_jointEuler_FwdAndLeft.m_rollDegrees );
// 	std::string eulerFromFwdOnly		= Stringf( "Euler fwd only:            %0.2f, %0.2f, %0.2f",			    m_jointEuler_FwdOnly.m_yawDegrees,				m_jointEuler_FwdOnly.m_pitchDegrees,			  m_jointEuler_FwdOnly.m_rollDegrees );
// 	std::string eulerFromFwdOnly_yaw	= Stringf( "Euler fwd only, yawPref:   %0.2f, %0.2f, %0.2f",	 m_jointEuler_FwdOnlyYawPrefered.m_yawDegrees,	 m_jointEuler_FwdOnlyYawPrefered.m_pitchDegrees,   m_jointEuler_FwdOnlyYawPrefered.m_rollDegrees );
// 	std::string eulerFromFwdOnly_pitch	= Stringf( "Euler fwd only, pitchPref: %0.2f, %0.2f, %0.2f",   m_jointEuler_FwdOnlyPitchPrefered.m_yawDegrees, m_jointEuler_FwdOnlyPitchPrefered.m_pitchDegrees, m_jointEuler_FwdOnlyPitchPrefered.m_rollDegrees );
	std::string eulerPreference			= Stringf( "EulerPreference: %s", m_joint.GetEulerPreference_EnumAsString().c_str() ).c_str();

	float textAlignmentY	= 1.0f;
	float textHeight		= 0.03f;
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			 	   timeText, color_lightGreen, 0.75f, Vec2( 1.0f, textAlignmentY ), TextDrawMode::SHRINK_TO_FIT );	
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			  cameraPosText, color_lightGreen, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,    cameraOrientationText, color_lightGreen, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
//	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,					 euler1, color_lightGreen, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
// 	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,		   eulerFromFwdLeft, color_lightGreen, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
// 	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,		   eulerFromFwdOnly, color_lightGreen, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
// 	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,	   eulerFromFwdOnly_yaw, color_lightGreen, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
// 	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,	 eulerFromFwdOnly_pitch, color_lightGreen, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			eulerPreference, color_lightGreen, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	// controlled orientation
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, 5.0f,						euler1, color_lightGreen, 0.75f, Vec2( 0.5f, 0.1f ), TextDrawMode::SHRINK_TO_FIT );


	if ( g_debugText_F4 )
	{
	}	

	
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
void GameMode_EulerTest::AddVertsForWorldBasis( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness, Rgba8 fwdColor, Rgba8 leftColor, Rgba8 upColor ) const
{
	// Render stationary world compass
	// X
	Vec3 endPosX = startPosition + Vec3( axisLength, 0.0f, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosX, axisThickness, fwdColor );
	// Y
	Vec3 endPosY = startPosition + Vec3( 0.0f, axisLength, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosY, axisThickness, leftColor );
	// Z
	Vec3 endPosZ = startPosition + Vec3( 0.0f, 0.0f, axisLength );
	AddVertsForArrow3D( compassVerts, startPosition, endPosZ, axisThickness, upColor );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_EulerTest::RenderControlledBasis( std::vector<Vertex_PCU>& verts, Vec3 const& basisStart, float axisLength, float axisThickness ) const
{
	Vec3 fwdDir, leftDir, upDir;
	m_eulerControlled.GetAsVectors_XFwd_YLeft_ZUp( fwdDir, leftDir, upDir );
	Vec3 endPosX = basisStart + fwdDir * axisLength;
	AddVertsForArrow3D( verts, basisStart, endPosX, axisThickness, Rgba8(100, 0, 0) );		// X
	Vec3 endPosY = basisStart + leftDir * axisLength;
	AddVertsForArrow3D( verts, basisStart, endPosY, axisThickness, Rgba8(0, 100, 0) );		// Y
	Vec3 endPosZ = basisStart + upDir * axisLength;
	AddVertsForArrow3D( verts, basisStart, endPosZ, axisThickness, Rgba8(0, 0, 100) );		// Z
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_EulerTest::RenderComputedBasis_MatrixToEuler( std::vector<Vertex_PCU>& verts, EulerAngles const& eulerToUse, Vec3 const& basisStart, float axisLength, float axisThickness ) const
{
	unsigned char alpha		 = 200;
	unsigned char brightness = 180;
	Vec3 fwdDir, leftDir, upDir;
	eulerToUse.GetAsVectors_XFwd_YLeft_ZUp( fwdDir, leftDir, upDir );
	Vec3 endPosX = basisStart + fwdDir * axisLength;
	AddVertsForArrow3D( verts, basisStart, endPosX, axisThickness, Rgba8(255, brightness, brightness, alpha) );		// X
	Vec3 endPosY = basisStart + leftDir * axisLength;
	AddVertsForArrow3D( verts, basisStart, endPosY, axisThickness, Rgba8(brightness, 255, brightness, alpha) );		// Y
	Vec3 endPosZ = basisStart + upDir * axisLength;
	AddVertsForArrow3D( verts, basisStart, endPosZ, axisThickness, Rgba8(brightness, brightness, 255, alpha) );		// Z
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_EulerTest::UpdateInputDebugPosEE( float deltaSeconds )
{
	float stepAmount = 40.0f;
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		stepAmount *= 4.0f;
	}

	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		m_eulerControlled = EulerAngles();
	}
	else if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		m_eulerControlled = EulerAngles( -180.0f, 0.0f, 0.0f );
	}
	if ( g_theInput->WasKeyJustPressed( '3' ) )
	{
		m_eulerControlled = EulerAngles( 0.0f, 90.0f, 0.0f );
	}
	if ( g_theInput->WasKeyJustPressed( '4' ) )
	{
		m_eulerControlled = EulerAngles( -180.0f, 135.0f, 0.0f );
	}
	if ( g_theInput->WasKeyJustPressed( '5' ) )
	{
		float randYaw   = g_theRNG->RollRandomFloatInRange( -360.0f, 360.0f );
		float randPitch = g_theRNG->RollRandomFloatInRange( -360.0f, 360.0f );
		m_eulerControlled		= EulerAngles( randYaw, randPitch, 0.0f );
	}

	// East (+X)
	if ( g_theInput->IsKeyDown( 'I' ) )
	{
		m_eulerControlled.m_pitchDegrees += stepAmount * deltaSeconds;
	}
	// West (-X)
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		m_eulerControlled.m_pitchDegrees -= stepAmount * deltaSeconds;
	}
	// North (+Y)
	if ( g_theInput->IsKeyDown( 'J' ) )
	{	
		m_eulerControlled.m_yawDegrees += stepAmount * deltaSeconds;
	}
	// South (-Y)
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		m_eulerControlled.m_yawDegrees -= stepAmount * deltaSeconds;
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'O' ) )
	{
		m_eulerControlled.m_rollDegrees += stepAmount * deltaSeconds;
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'U' ) )
	{
		m_eulerControlled.m_rollDegrees -= stepAmount * deltaSeconds;
	}

	if ( g_theInput->WasKeyJustPressed( KEYCODE_HOME ) )
	{
		m_joint.m_matrixToEulerPreference = FWD_LEFT_TEMPORAL;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_END ) )
	{
		m_joint.m_matrixToEulerPreference = YAW_PREFERRED;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_INSERT ) )
	{
		m_joint.m_matrixToEulerPreference = PITCH_PREFERRED ;
	}
}