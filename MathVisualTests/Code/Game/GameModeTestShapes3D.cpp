#include "Game/GameModeTestShapes3D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeTestShapes3D::GameModeTestShapes3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameModeTestShapes3D::~GameModeTestShapes3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeTestShapes3D::Startup()
{
	// Initializing textures;
	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeTestShapes3D::Update( float deltaSeconds )
{
	deltaSeconds = m_clock.GetDeltaSeconds();

	// Implement camera disp to mousePos
	Vec2 cursorClientDelta	= g_theInput->GetCursorClientDelta();
	float mouseSpeed		= 0.05f;
	float yaw				= cursorClientDelta.x * mouseSpeed;
	float pitch				= cursorClientDelta.y * mouseSpeed;

	m_angularVelocity.m_yawDegrees	 -= yaw;
	m_angularVelocity.m_pitchDegrees += pitch;

	UpdateGameCameraTestShapes3D();
	UpdateInputForCameraMovement( deltaSeconds );
	UpdatePauseQuitAndSlowMo();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeTestShapes3D::Render() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin Camera
	g_theRenderer->BeginCamera( m_testShapesWorldCamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Create verts
	std::vector<Vertex_PCU> verts;

	//----------------------------------------------------------------------------------------------------------------------
	// Render 2 Spheres
	AddVertsForSphere3D( verts, m_centerPos,  m_radius, m_numSlices, m_numStacks );
	AddVertsForSphere3D( verts, m_centerPos2, m_radius, m_numSlices, m_numStacks );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Quad
	AddVertsForAABB3D( verts, m_bounds );

//	//----------------------------------------------------------------------------------------------------------------------
//	// Render stationary world compass
	std::vector<Vertex_PCU> compassVerts;
	AddVertsForCompass( compassVerts, Vec3::ZERO, 1.0f, 0.08f );

	//----------------------------------------------------------------------------------------------------------------------
	// Render player compass
	Mat44 playerMatrix  = m_testShapesWorldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 playerForward	= playerMatrix.GetIBasis3D();
	float distFromCam	= 0.2f;

	Vec3 playerCenter		  = m_testShapesWorldCamera.m_position + ( playerForward * distFromCam );
	float playerCompassLength = 0.01f;
	float playerAxisThickness = 0.001f;
	AddVertsForCompass( compassVerts, playerCenter, playerCompassLength, playerAxisThickness );

	//----------------------------------------------------------------------------------------------------------------------
	// Adding wireFrame Object
	std::vector<Vertex_PCU> wireframeVerts;

	//----------------------------------------------------------------------------------------------------------------------
	// Render 2 wireframe Spheres
	AddVertsForSphere3D( wireframeVerts, m_wireframeCenterPos,  m_radius, m_numSlices, m_numStacks );
	AddVertsForSphere3D( wireframeVerts, m_wireframeCenterPos2, m_radius, m_numSlices, m_numStacks );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Quad
	AddVertsForAABB3D( wireframeVerts, m_wireframeBounds );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Cylinder
	AddVertsForCylinderZ3D( verts, m_centerXY, m_minMaxZ, m_cylinderRadius, m_cylinderNumSlices );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Wireframe Cylinder
	AddVertsForCylinderZ3D( wireframeVerts, m_wireFrameCenterXY, m_wireFrameMinMaxZ, m_wireFrameCylinderRadius,	m_wireFrameCylinderNumSlices );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for World	// Rendering spheres with test texture
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( m_testTexture );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );
	g_theRenderer->BindTexture( nullptr );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for world Compass
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( compassVerts.size() ), compassVerts.data() );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for wireFrame Objects (Sphere, AABB3, Cylinder)
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_NONE );
	g_theRenderer->DrawVertexArray( static_cast<int>( wireframeVerts.size() ), wireframeVerts.data() );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );

	//----------------------------------------------------------------------------------------------------------------------
	// End Camera
	g_theRenderer->EndCamera( m_testShapesWorldCamera );

	RenderUIStuff();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeTestShapes3D::Reshuffle() 
{
	//----------------------------------------------------------------------------------------------------------------------
	// Object variables
	// Sphere
	m_centerPos	 = Vec3( g_theRNG->RollRandomFloatInRange( 0.0f, 10.0f ), g_theRNG->RollRandomFloatInRange( 0.0f, 10.0f ), g_theRNG->RollRandomFloatInRange( 0.0f, 10.0f ) );
	m_centerPos2 = Vec3( g_theRNG->RollRandomFloatInRange( 0.0f, 10.0f ), g_theRNG->RollRandomFloatInRange( 0.0f, 10.0f ), g_theRNG->RollRandomFloatInRange( 0.0f, 10.0f ) );
	m_radius	 = g_theRNG->RollRandomFloatInRange( 0.0f, 4.0f );

	// Quad
	m_bounds = AABB3( Vec3( g_theRNG->RollRandomFloatInRange( 5.0f, 30.0f ), g_theRNG->RollRandomFloatInRange( 5.0f, 10.0f ), g_theRNG->RollRandomFloatInRange( 5.0f, 10.0f ) ), 
					  Vec3( g_theRNG->RollRandomFloatInRange( 30.0f, 45.0f ), g_theRNG->RollRandomFloatInRange( 20.0f, 25.0f ), g_theRNG->RollRandomFloatInRange( 20.0f, 25.0f ) ) );

	// Wireframe Spheres
	m_wireframeCenterPos  = Vec3( 5.0f,   g_theRNG->RollRandomFloatInRange( 30.0f, 35.0f ), g_theRNG->RollRandomFloatInRange( -5.0f, 10.0f ) );
	m_wireframeCenterPos2 = Vec3( g_theRNG->RollRandomFloatInRange( 35.0f, 45.0f ), g_theRNG->RollRandomFloatInRange( -10.0f, 10.0f ), g_theRNG->RollRandomFloatInRange( -10.0f, 10.0f ));

	// Wireframe Quad
	m_wireframeBounds = AABB3( Vec3( g_theRNG->RollRandomFloatInRange( 40.0f, 45.0f ), g_theRNG->RollRandomFloatInRange( 40.0f, 45.0f ), g_theRNG->RollRandomFloatInRange( -15.0f, 20.0f ) ),
							   Vec3( g_theRNG->RollRandomFloatInRange( 50.0f, 55.0f ), g_theRNG->RollRandomFloatInRange( 70.0f, 85.0f ), g_theRNG->RollRandomFloatInRange( -15.0f, -5.0f ) ) );	

	// Cylinder
	m_centerXY = Vec2( g_theRNG->RollRandomFloatInRange( -10.0f, 10.0f ), g_theRNG->RollRandomFloatInRange( -10.0f, 10.0f ) );

	// Wireframe Cylinder
	m_wireFrameCenterXY	= Vec2( g_theRNG->RollRandomFloatInRange( 40.0f, 45.0f ), g_theRNG->RollRandomFloatInRange( 50.0f, 55.0f ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeTestShapes3D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeTestShapes3D::UpdateGameCameraTestShapes3D()
{
	m_testShapesWorldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 100.0f  );
//	m_player->m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f ), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) );
	m_testShapesWorldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f ), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) );
	m_testShapesUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( SCREEN_SIZE_X, SCREEN_SIZE_Y ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeTestShapes3D::UpdateInputForCameraMovement( float deltaSeconds)
{
	EulerAngles orientationPerFrame = EulerAngles( m_turnRate, m_turnRate, m_turnRate );

	Vec3 forward;
	Vec3 left;
	Vec3 up;

	m_testShapesWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( forward, left, up );

	//----------------------------------------------------------------------------------------------------------------------
	// Forward
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
		m_testShapesWorldCamera.m_position += ( m_speed * forward * deltaSeconds );
	}
	// Back
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_testShapesWorldCamera.m_position -= ( m_speed * forward * deltaSeconds );
	}
	// Left
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_testShapesWorldCamera.m_position += ( m_speed * left * deltaSeconds );
	}
	// Right
	if ( g_theInput->IsKeyDown( 'F' ) )
	{
		m_testShapesWorldCamera.m_position -= ( m_speed * left * deltaSeconds );
	}
	// Pitch up
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) )
	{
//		m_testShapesWorldCamera.m_orientation.m_pitchDegrees -= ( m_angularVelocity.m_pitchDegrees * deltaSeconds );
		m_angularVelocity.m_pitchDegrees -= ( orientationPerFrame.m_pitchDegrees * deltaSeconds );
	}

	// Pitch down
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) )
	{
//		m_testShapesWorldCamera.m_orientation.m_pitchDegrees += ( m_angularVelocity.m_pitchDegrees * deltaSeconds );
		m_angularVelocity.m_pitchDegrees += ( orientationPerFrame.m_pitchDegrees * deltaSeconds );
	}

	// Yaw left
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) )
	{
//		m_testShapesWorldCamera.m_orientation.m_yawDegrees += ( m_angularVelocity.m_yawDegrees * deltaSeconds );
		m_angularVelocity.m_yawDegrees += ( orientationPerFrame.m_yawDegrees * deltaSeconds );
	}

	// Yaw right 
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
	{
//		m_testShapesWorldCamera.m_orientation.m_yawDegrees -= ( m_angularVelocity.m_yawDegrees * deltaSeconds );
		m_angularVelocity.m_yawDegrees -= ( orientationPerFrame.m_yawDegrees * deltaSeconds );
	}

	// Elevate
	if ( g_theInput->IsKeyDown( 'R' ) || g_theInput->IsKeyDown( 'Q' ) || g_theInput->IsKeyDown( 'A' ))
	{
		m_testShapesWorldCamera.m_position.z += ( m_speed * deltaSeconds );
	}

	// De-Elevate
	if ( g_theInput->IsKeyDown( 'W' ) || g_theInput->IsKeyDown( 'Z' ) )
	{
		m_testShapesWorldCamera.m_position.z -= ( m_speed * deltaSeconds );
	}

	// Speed up speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_speed = m_doublespeed;
	}
	// Speed up speed variable
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_speed = m_defaultSpeed;
	}

	// Slow down speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_CONTROL ) )
	{
		m_speed = m_halfspeed;
	}

	// Slow down speed variable
	if ( g_theInput->WasKeyJustReleased( KEYCODE_CONTROL ) )
	{
		m_speed = m_defaultSpeed;
	}

	// Slow-Mo
	if ( g_theInput->IsKeyDown( 'T' ) )
	{
		m_testShapesWorldCamera.m_position.z -= ( m_elevateSpeed * deltaSeconds );
	}

//	m_orientationDegrees = m_angularVelocity;
//	m_orientationDegrees.m_pitchDegrees = GetClamped( m_orientationDegrees.m_pitchDegrees, -89.9f, 89.9f );

	m_testShapesWorldCamera.m_orientation = m_angularVelocity;
	m_testShapesWorldCamera.m_orientation.m_pitchDegrees = GetClamped( m_testShapesWorldCamera.m_orientation.m_pitchDegrees, -89.9f, 89.9f );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeTestShapes3D::RenderUIStuff() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_testShapesUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;

	float cellHeight = 2.0f;
	AABB2 textbox1 = AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	AABB2 textbox2 = AABB2( textbox1.m_mins, Vec2( textbox1.m_maxs.x, textbox1.m_maxs.y - cellHeight ) );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, "Mode (F6/F7 for prev/next): Test Shapes (3D)", Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, "F8 to randomize; ESDF = fly horizontal, WR/QZ/AZ = fly vertical, hold T slow", Rgba8::GREEN, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	//----------------------------------------------------------------------------------------------------------------------
	// Render playerCompass
	std::vector<Vertex_PCU> playerCompassVerts;

//	Vec3  minsX				= Vec3( SCREEN_CENTER_X - 1.0f,		SCREEN_CENTER_Y - 1.0f,		0.0f );
//	Vec3  maxsX				= Vec3( SCREEN_CENTER_X + 1.0f,		SCREEN_CENTER_Y + 1.0f,		0.0f );
//	AABB3 compassBounds_X	= AABB3( minsX, maxsX );
//	AddVertsForAABB3D( playerCompassVerts, compassBounds_X, Rgba8::RED );
	
	Vec3 forward;// = Vec3( 1.0f, 0.0f, 0.0f );
	Vec3 left	;// = Vec3( 0.0f, 1.0f, 0.0f );
	Vec3 up		;// = Vec3( 0.0f, 0.0f, 1.0f );

	m_testShapesWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( forward, left, up );	

//	AddVertsForAABB3D( playerCompassVerts, AABB3( Vec3( SCREEN_CENTER_X - 1.0f, SCREEN_CENTER_Y - 1.0f, 0.0f ), forward ), Rgba8::RED );
//	AddVertsForAABB3D( playerCompassVerts, AABB3( Vec3( 0.0f, 0.0f, 0.0f ), left	), Rgba8::RED );
//	AddVertsForAABB3D( playerCompassVerts, AABB3( Vec3( 0.0f, 0.0f, 0.0f ), up		), Rgba8::RED );


	// Y
//	Vec3  minsY = Vec3( 0.0f, 0.0f, 0.0f );
//	Vec3  maxsY = Vec3( 0.05f, 1.0f, 0.05f );
//	AABB3 compassBounds_Y = AABB3( minsY, maxsY );
//	AddVertsForAABB3D( playerCompassVerts, compassBounds_Y, Rgba8::GREEN );
//	// Z
//	Vec3  minsZ = Vec3( 0.0f, 0.0f, 0.0f );
//	Vec3  maxsZ = Vec3( 0.05f, 0.05f, 1.0f );
//	AABB3 compassBounds_Z = AABB3( minsZ, maxsZ );
//	AddVertsForAABB3D( playerCompassVerts, compassBounds_Z, Rgba8::BLUE );

	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_testShapesUICamera  );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for UI camera // Rendering texts
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for playerCompass 
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( playerCompassVerts.size() ), playerCompassVerts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeTestShapes3D::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if ( g_theInput->WasKeyJustPressed( 'P' ) || g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::BUTTON_START ) )
	{
		SoundID testSound = g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
		g_theAudio->StartSound( testSound );			// Comment out this line of code to remove pause sound playing

		m_clock.TogglePause();
	}

	// Slow-Mo functionality
	m_isSlowMo = g_theInput->IsKeyDown( 'T' );
	if ( m_isSlowMo )
	{
		m_clock.SetTimeScale( 0.1f );
	}
	if ( g_theInput->WasKeyJustReleased( 'T' ) )
	{
		m_clock.SetTimeScale( 1.0f );
	}

	// Fast-Mo functionality
	m_isSlowMo = g_theInput->IsKeyDown( 'Y' );
	if ( m_isSlowMo )
	{
		m_clock.SetTimeScale( 2.0f );
	}
	if ( g_theInput->WasKeyJustReleased( 'Y' ) )
	{
		m_clock.SetTimeScale( 1.0f );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeTestShapes3D::AddVertsForCompass( std::vector<Vertex_PCU>& compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render stationary world compass
	// X
	Vec3 endPosX		 = startPosition + Vec3( axisLength, 0.0f, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosX, axisThickness, Rgba8::RED );
	// Y
	Vec3 endPosY		 = startPosition + Vec3( 0.0f, axisLength, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosY, axisThickness, Rgba8::GREEN );
	// Z
	Vec3 endPosZ		 = startPosition + Vec3( 0.0f, 0.0f, axisLength );
	AddVertsForArrow3D( compassVerts, startPosition, endPosZ, axisThickness, Rgba8::BLUE );
}
 
//----------------------------------------------------------------------------------------------------------------------
// code example for TGP squashing fish while boosting
//	float timeToMaxSquash = 3.f;
//	float driftTime = 0.f; //0-length of time you've been holding drift
//	
//	float currentSquashAmount = RangeMapClamped(driftTime, 0.f, timeToMaxSquash, -1.f, 1.f);
//	SetSquash(currentSquashAmount);
//	
//	// Squash amount is [-1.f, 1.f]
//	void SetSquash(float squashAmount)
//	{
//		m_fishSkeleton.SetMorphTarget("Squash", squashAmount);
//	} 