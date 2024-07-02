#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

//----------------------------------------------------------------------------------------------------------------------
Game::Game()
{
	// Initializing textures;
	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TestUV.png" );
}

//----------------------------------------------------------------------------------------------------------------------
Game::~Game()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Game::StartUp()
{
	if (m_AttractModeIsOn)
	{
		// Get main menu visuals verts
		InitializeAttractModeVerts();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Adding player to scene
	m_player = new Player(this);
	m_entityList.push_back(m_player);

	// Adding Prop(s) and Sphere to scene
	Prop* m_propRotatingCube = new Prop( this );
	Prop* m_propBlinkingCube = new Prop( this );
	Prop* m_propSphere		 = new Prop( this );
	Prop* m_propGridLines	 = new Prop( this );
	m_entityList.push_back( m_propRotatingCube );
	m_entityList.push_back( m_propBlinkingCube );
	m_entityList.push_back( m_propSphere );
	m_entityList.push_back( m_propGridLines );

	//----------------------------------------------------------------------------------------------------------------------
	// Camera
	UpdateCameras();

	//----------------------------------------------------------------------------------------------------------------------
	// Set Cube position
	m_propRotatingCube->m_position = Vec3(  2.0f,  2.0f, 0.0f );
	m_propBlinkingCube->m_position = Vec3( -2.0f, -2.0f, 0.0f );

	// Add Cubes to scene
	m_propRotatingCube->AddCubeToScene( 0.5f, 0.5f, 0.5f );
	m_propBlinkingCube->AddCubeToScene( 0.5f, 0.5f, 0.5f );
	m_propBlinkingCube->m_isBlinking = true;

	// Set Cube pitch and roll degrees
	m_propRotatingCube->m_angularVelocity.m_pitchDegrees = 30.0f;
	m_propRotatingCube->m_angularVelocity.m_rollDegrees  = 30.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Set Sphere position
	m_propSphere->m_position = Vec3( 10.0f, -5.0f, 1.0f );

	// Add Sphere to scene
	m_propSphere->AddSphereToScene( Vec3(0.0f, 0.0f, 0.0f), 1.0f, 32.0f, 16.0f, m_testTexture );

	// Rotate Sphere
	m_propSphere->m_angularVelocity.m_yawDegrees = 45.0f;		

	//----------------------------------------------------------------------------------------------------------------------
	// Set Grid Lines in scene
	m_propGridLines->AddGridLinesToScene();
	
	// Render DRS world Basis and text
	Render_DRS_WorldBasisText();
	Render_DRS_WorldBasis();
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Update()
{
	float deltaSeconds = m_clock.GetDeltaSeconds();

	if (m_AttractModeIsOn)
	{
		// Take input from attractModeInput()
		AttractModeInput();
		return;
	}

	Update_DRS_Input();
	UpdatePauseQuitAndSlowMo();
	UpdateReturnToAttractMode();
	UpdateEntities( deltaSeconds );
	UpdateCameras();
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Clear screen
	g_theRenderer->ClearScreen( Rgba8::LIGHTBLACK );

	// Draw attract mode
	if (m_AttractModeIsOn)
	{
		// Clear AttractMode screen
		g_theRenderer->ClearScreen( Rgba8::DARK_RED );
		// uses attractModeCam and draws relevant stuff
		RenderAttractMode();
		return;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Begin WorldCamera
	g_theRenderer->BeginCamera(m_player->m_worldCamera);

	DebugRenderWorld( m_player->m_worldCamera );

	// Render Entities
	RenderEntities();

	//----------------------------------------------------------------------------------------------------------------------
	// #ToDo delete
	// Test debugRenderSystem Billboard Text
//if ( g_theInput->IsKeyDown( '4' ) )
//{
//	float duration = 10.0f;
//	std::string text = "Camera Orientation: ";
//	float textHeight = 2.0f;
//	Vec3 alignment = Vec3( 0.0, 0.0f, 0.0f );
//	Vec3 origin = Vec3( 0.0, 0.0f, 0.0f );
//	//		DebugAddWorldBillboardText( text, origin, textHeight, alignment, duration, Rgba8::WHITE, Rgba8::RED, DebugRenderMode::USE_DEPTH );

	// End m_worldCam
	g_theRenderer->EndCamera(m_player->m_worldCamera);

	// RenderUI()
	RenderUI();
}
 
//----------------------------------------------------------------------------------------------------------------------
void Game::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if (g_theInput->WasKeyJustPressed('P') || g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::BUTTON_START))
	{
		SoundID testSound = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
		UNUSED( testSound );
//		g_theAudio->StartSound(testSound);			// Comment out this line of code to remove pause sound playing

		m_clock.TogglePause();
	}

	// Slow-Mo functionality
	m_isSlowMo = g_theInput->IsKeyDown('T');
	if (m_isSlowMo)
	{
		m_clock.SetTimeScale( 0.1f );
	}
	if ( g_theInput->WasKeyJustReleased( 'T' ) )
	{
		m_clock.SetTimeScale( 1.0f );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateReturnToAttractMode()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) && m_AttractModeIsOn == false)
	{
		m_AttractModeIsOn = true;
	}
}
  
//----------------------------------------------------------------------------------------------------------------------
void Game::AttractModeInput()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->HandleQuitRequested();
	}
	
	XboxController const& controller = g_theInput->GetController(0);
	
	if ( g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N') || controller.WasButtonJustPressed(BUTTON_START) || controller.WasButtonJustPressed(BUTTON_A))
	{
		m_AttractModeIsOn = false;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateEntities(float deltaSeconds)
{	
	//----------------------------------------------------------------------------------------------------------------------
	// Call Update() for each entity inside entityList
	for ( int i = 0; i < m_entityList.size(); i++ )
	{
		m_entityList[i]->Update( deltaSeconds );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderEntities() const
{
	for ( int j = 0; j < m_entityList.size(); j++)
	{
		m_entityList[j]->Render();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );

	// Pass m_screenCamera for DebugRenderScreen to use
	DebugRenderScreen( m_screenCamera );

	Render_DRS_UI_Text();

	// End UI Camera
	g_theRenderer->EndCamera( m_screenCamera );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Update_DRS_Input()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Test debugRenderSystem variables
	Vec3 center		= Vec3( 0.0f, 0.0f, 0.0f );
	float radius	= 1.0f;
	float duration	= 10.0f;

	// Test debugRenderSystem Sphere
	if ( g_theInput->IsKeyDown('1') )
	{
		duration			= 5.0f;
		Mat44 matrix		= m_player->GetModelMatrix( m_player->m_position, m_player->m_orientationDegrees );
		Vec3 iBasis			= matrix.GetIBasis3D();
		Vec3 stepsForward	= (iBasis * 2.0f);
		Vec3 posInFront		= m_player->m_position + stepsForward;
		DebugAddWorldWireSphere( posInFront, radius, duration, Rgba8::GREEN, Rgba8::RED, DebugRenderMode::USE_DEPTH );
	}

	// Test debugRenderSystem Line 
	if ( g_theInput->IsKeyDown( '2' ) )
	{
		duration			= 10.0f;
		Mat44 matrix		= m_player->GetModelMatrix( m_player->m_position, m_player->m_orientationDegrees );
		Vec3 iBasis			= matrix.GetIBasis3D();
		Vec3 stepsForward	= (iBasis * 20.0f);
		Vec3 stepsBehind	= (iBasis * 0.2f);
		Vec3 end			= m_player->m_position + stepsForward;
		Vec3 posBehind		= m_player->m_position - stepsBehind;
		float lineRadius	= 0.1f;
		DebugAddWorldLine( posBehind, end, lineRadius, duration, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::X_RAY );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Test debugRenderSystem Arrow IJK WorldBasis
	if ( g_theInput->IsKeyDown( '3' ) )
	{
		duration			= 20.0f;
		Mat44 matrix		= m_player->GetModelMatrix( m_player->m_position, m_player->m_orientationDegrees );
		Vec3 iBasis			= matrix.GetIBasis3D();
		Vec3 jBasis			= matrix.GetJBasis3D();
		Vec3 kBasis			= matrix.GetKBasis3D();
		Vec3 stepsForward	= (iBasis * 1.5f);
		Vec3 stepsLeft		= (jBasis * 1.5f);
		Vec3 stepsSkyward	= (kBasis * 1.5f);
		Vec3 iEnd			= m_player->m_position + stepsForward;
		Vec3 jEnd			= m_player->m_position + stepsLeft;
		Vec3 kEnd			= m_player->m_position + stepsSkyward;
		radius				= 0.2f;

		// X-axis Red
		DebugAddWorldArrow( m_player->m_position, iEnd, radius, duration, Rgba8::RED, Rgba8::RED );
		// Y-axis Green
		DebugAddWorldArrow( m_player->m_position, jEnd, radius, duration, Rgba8::GREEN, Rgba8::GREEN );
		// Z-axis Blue																					  
		DebugAddWorldArrow( m_player->m_position, kEnd, radius, duration, Rgba8::BLUE, Rgba8::BLUE );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Test debugRenderSystem Billboard Text
	if ( g_theInput->IsKeyDown( '4' ) )
	{
		duration			= 10.0f;
		std::string text	= "Camera Orientation: ";
		float textHeight	= 20.0f;
		Vec2 alignment		= Vec2( 0.5, 0.5f );
//		Vec3 origin			= Vec3( 0.0, 0.0f, 0.0f );
		Vec3 origin			= m_player->m_position;
//		FULL_CAMERA_FACING
		DebugAddWorldBillboardText( text, origin, textHeight, alignment, duration, Rgba8::WHITE, Rgba8::RED, DebugRenderMode::USE_DEPTH );

		//----------------------------------------------------------------------------------------------------------------------
		// Test code
//		std::vector<Vertex_PCU> verts;
//		BitmapFont* g_testFont = nullptr;
//		g_testFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Images/SquirrelFixedFont" );
//		g_testFont->AddVertsForText3D( verts, Vec2(0.0f, 0.0f), 1.0f, text );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Test debugRenderSystem transformable cylinder
	if ( g_theInput->IsKeyDown( '5' ) )
	{
		duration			= 10.0f;
		Mat44 matrix		= m_player->GetModelMatrix( m_player->m_position, m_player->m_orientationDegrees );
		Vec3 iBasis			= matrix.GetIBasis3D();
		Vec3 stepsForward	= (iBasis * 2.0f);
		Vec3 top			= m_player->m_position + stepsForward;

		DebugAddWorldWireCylinder( m_player->m_position + Vec3( 0.0f, 0.0f, 0.0f), m_player->m_position + Vec3( 0.0f, 0.0f, 1.0f ), radius, duration, Rgba8::WHITE, Rgba8::RED, DebugRenderMode::USE_DEPTH );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Test debugRenderSystem World Point
	if ( g_theInput->IsKeyDown( '6' ) )
	{
		duration = 60.0f;
		DebugAddWorldPoint( m_player->m_position, radius, duration, Rgba8::BROWN, Rgba8::BROWN, DebugRenderMode::USE_DEPTH );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Test debugRenderSystem UI Text
	if ( g_theInput->IsKeyDown( '7' ) )
	{
		duration			= 5.0f;
		std::string text	= "Camera Orientation: ";
		Vec2 position		= Vec2 ( SCREEN_SIZE_X * 0.1f, SCREEN_SIZE_Y * 0.9f );
		float size			= 2.0f;
		Vec2 alignment		= Vec2( 0.0, 0.0f );
		DebugAddScreenText( text, position, size, alignment, duration );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Render_DRS_WorldBasisText()
{
	// DebugRenderSystem world Text	// World basis text
	float duration				= 10.0f;
	std::string xForwardText	= "X - Forward";
	std::string yLeftText		= "Y - Left";
	std::string zUpText			= "Z - Up";

	float textHeight			= 0.5f;
	Vec2 alignment				= Vec2( 0.0, 0.0f );

	Mat44 xTransform;
	Mat44 yTransform;
	Mat44 zTransform;
	xTransform.AppendZRotation( -90.0f );
	yTransform.AppendZRotation( 180.0f );
	zTransform.AppendYRotation( 180.0f );
	zTransform.AppendXRotation( -90.0f );

	xTransform.SetTranslation3D( Vec3( 0.2f,  0.0f, 0.2f ) );
	yTransform.SetTranslation3D( Vec3( 0.0f,  4.2f, 0.2f ) );
	zTransform.SetTranslation3D( Vec3( 0.0f, -0.8f, 0.2f ) );

	DebugAddWorldText( xForwardText, xTransform, textHeight, alignment, duration,   Rgba8::RED,   Rgba8::RED, DebugRenderMode::ALWAYS );		// X - Forward
	DebugAddWorldText(	  yLeftText, yTransform, textHeight, alignment, duration, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::ALWAYS );		// Y - Left
	DebugAddWorldText(		zUpText, zTransform, textHeight, alignment, duration,  Rgba8::BLUE,  Rgba8::BLUE, DebugRenderMode::ALWAYS );		// Z - Up
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Render_DRS_WorldBasis()
{
	// DebugRenderSystem World Basis
	float radius	= 0.1f;
	float duration = -1.0f;
	DebugAddWorldArrow( Vec3::ZERO, Vec3( 1.0f, 0.0f, 0.0f ), radius, duration,   Rgba8::RED, Rgba8::RED   );		// iBasisArrow
	DebugAddWorldArrow( Vec3::ZERO, Vec3( 0.0f, 1.0f, 0.0f ), radius, duration, Rgba8::GREEN, Rgba8::GREEN );		// jBasisArrow
	DebugAddWorldArrow( Vec3::ZERO, Vec3( 0.0f, 0.0f, 1.0f ), radius, duration,  Rgba8::BLUE, Rgba8::BLUE  );		// kBasisArrow
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Setting perspective view
	m_player->m_worldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 100.0f );
	m_player->m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f ), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) );

//	m_worldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 100.0f );
//	m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f), Vec3( -1.0f, 0.0f, 0.0f), Vec3( 0.0f, 1.0f, 0.0f) );

	//----------------------------------------------------------------------------------------------------------------------
	// Setting ortho views
//	  m_worldCamera.SetOrthoView( Vec2(-1.0f, -1.0f), Vec2(1.0f, 1.0f) );
	m_attractCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2(1600.0f, 800.0f) );
	m_screenCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y) );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const
{
	// Draw everything in screen space
	g_theRenderer->BeginCamera(m_attractCamera);

	g_theRenderer->SetModelConstants();

//	//----------------------------------------------------------------------------------------------------------------------
//	// Drawing triangle in attractMode
//	Vertex_PCU tempAttractModeVerts[NUM_SINGLE_TRI_VERTS];
//	for (int i = 0; i < NUM_SINGLE_TRI_VERTS; i++)
//	{
//		tempAttractModeVerts[i] = m_localVerts[i];
//	}
//
//	TransformVertexArrayXY3D(NUM_SINGLE_TRI_VERTS, tempAttractModeVerts, 80.0f, 0.0f, Vec2(ATTRACT_MODE_CENTER_X, ATTRACT_MODE_CENTER_Y));
//	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
//	g_theRenderer->BindTexture( nullptr );
//	g_theRenderer->SetModelConstants();
//	g_theRenderer->DrawVertexArray(NUM_SINGLE_TRI_VERTS, tempAttractModeVerts);

	//----------------------------------------------------------------------------------------------------------------------
	// Drawing "First Triangle"
	// #Question // How are we setting the worldPos of the triangle?
	Vertex_PCU vertices[] =
	{
		Vertex_PCU( Vec3( 1200.0f, 200.0f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
		Vertex_PCU( Vec3(  800.0f, 600.0f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
		Vertex_PCU( Vec3(  400.0f, 200.0f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
	};

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 3, vertices );

	// End attractMode Camera
	g_theRenderer->EndCamera(m_attractCamera);
} 

//----------------------------------------------------------------------------------------------------------------------
void Game::InitializeAttractModeVerts()
{
	m_localVerts[0].m_position = Vec3( 0.0f,  2.0f, 0.0f);		// Triangle A, position of vert A
	m_localVerts[1].m_position = Vec3( 4.0f, -3.0f, 0.0f);		// Triangle A, position of vert B
	m_localVerts[2].m_position = Vec3(-4.0f, -3.0f, 0.0f);		// Triangle A, position of vert C

	m_localVerts[0].m_color = Rgba8::WHITE;						// Triangle A, color of vert A
	m_localVerts[1].m_color = Rgba8::WHITE;						// Triangle A, color of vert B
	m_localVerts[2].m_color = Rgba8::WHITE;						// Triangle A, color of vert C
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Render_DRS_UI_Text() const
{
	// Initialize and set UI variables
	float cellHeight			= 2.0f;
	float duration				= 0.0f;
	Vec2 playerPos				= Vec2( 0.0f, 100.0f );
	Vec2 playerAlignment		= Vec2( 0.0f, 1.0f );
	Vec2 timePosition			= Vec2( SCREEN_SIZE_X, ( SCREEN_SIZE_Y ) );
	Vec2 timeAlignment			= Vec2( 1.0f, 1.0f );
	float fps					= 1.0f / m_clock.GetDeltaSeconds();
	float scale					= m_clock.GetTimeScale();
	std::string playerPosText	= Stringf( "Player position: %f, %f, %f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z );
	std::string timeText		= Stringf( "Time: %f. FPS: %f, Scale %f.", m_clock.GetTotalSeconds(), fps, scale );

	// Render DRS UI text
	DebugAddScreenText( playerPosText, playerPos, cellHeight, playerAlignment, duration );
	DebugAddScreenText( timeText, timePosition, cellHeight, timeAlignment, duration );
}
