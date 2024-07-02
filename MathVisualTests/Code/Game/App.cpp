#include "Game/App.hpp"
#include "Game/GameModeBase.hpp"

#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/DevConsole.hpp"

//----------------------------------------------------------------------------------------------------------------------
InputSystem*			g_theInput			= nullptr;
Window*					g_theWindow			= nullptr;
Renderer*				g_theRenderer		= nullptr;	// Created and owned by the App
RandomNumberGenerator*	g_theRNG			= nullptr;	   
AudioSystem*			g_theAudio			= nullptr;	   
App*					g_theApp			= nullptr;	// Created and owned by Main_Windows.cpp

//----------------------------------------------------------------------------------------------------------------------
GameMode				g_gameModeNum		= GAMEMODE_2D_NEAREST_POINT;

//----------------------------------------------------------------------------------------------------------------------
App::App()
{
}

//----------------------------------------------------------------------------------------------------------------------
App::~App()
{
}

//----------------------------------------------------------------------------------------------------------------------
void App::Startup()
{
	g_theEventSystem = new EventSystem();

	// Create engine subsystems and game
	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem( inputSystemConfig );

	// Creating Window
	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "Math Visual Test";
	windowConfig.m_clientAspect = 2.0f;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_isFullScreen = true;
	g_theWindow = new Window ( windowConfig );

	// Creating Renderer
	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer( rendererConfig );

	// Creating DevConsole
	DevConsoleConfig devConsoleConfig;
	m_devConsoleCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
	devConsoleConfig.m_camera = &m_devConsoleCamera;
	devConsoleConfig.m_renderer = g_theRenderer;
	g_theDevConsole = new DevConsole( devConsoleConfig );

	// Creating AudioSystem
	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );
	
	// Creating DebugRenderSystem
	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;
	DebugRenderSystemStartup( debugRenderConfig );

	g_theRNG = new RandomNumberGenerator();

	// Start up engine subsystems and game
	g_theEventSystem->Startup();
	 g_theDevConsole->Startup();
  	      g_theInput->StartUp();
	     g_theWindow->Startup();
	   g_theRenderer->Startup();
	      g_theAudio->Startup();

	if ( !m_attractModeIsOn )
	{
		m_theGameMode->Startup();
	}

	//----------------------------------------------------------------------------------------------------------------------
	g_theEventSystem->SubscribeToEvent( "quit", App::Quit );

	//----------------------------------------------------------------------------------------------------------------------
	// Initializing bitmap font text
	m_textFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
}

//----------------------------------------------------------------------------------------------------------------------
void App::Shutdown()
{
	     g_theAudio->Shutdown();
	  g_theRenderer->Shutdown();
	    g_theWindow->Shutdown();
	     g_theInput->ShutDown();
	g_theDevConsole->Shutdown();
	DebugRenderSystemShutdown();

	delete g_theAudio;
	g_theAudio = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_theWindow;
	g_theWindow = nullptr;

	delete g_theInput;
	g_theInput = nullptr;

	delete g_theDevConsole;
	g_theDevConsole = nullptr;

	delete m_theGameMode;
	m_theGameMode = nullptr;

//	m_theGameMode->Shutdown();
} 

//----------------------------------------------------------------------------------------------------------------------
void App::Run()
{
	// Program main loop; keep running frames until it's time to quit
	while ( !IsQuitting() )
	{
		g_theApp->RunFrame();
	}
}

//----------------------------------------------------------------------------------------------------------------------
// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
void App::RunFrame()
{
	float deltaSeconds = 1.0f/60.0f;

	BeginFrame();	
	Update( deltaSeconds );
	Render();	
	EndFrame();	
}
 
//----------------------------------------------------------------------------------------------------------------------
bool App::HandleKeyPressed(unsigned char keyCode)
{
	g_theInput->HandleKeyPressed( keyCode );
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool App::HandleKeyReleased(unsigned char keyCode)
{
	g_theInput->HandleKeyReleased( keyCode );
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
void App::UpdateReturnToAttractMode()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_ESC ) && m_attractModeIsOn == false )
	{
		m_attractModeIsOn = true;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void App::RenderAttractMode() const
{
	g_theRenderer->BeginCamera( m_attractCamera );

	std::vector<Vertex_PCU> attractModeVerts;
	attractModeVerts.reserve( 9 );

	AddVertsForAABB2D( attractModeVerts, AABB2( Vec2( 0.0f, 0.0f ), Vec2( ATTRACT_MODE_SIZE_X, ATTRACT_MODE_SIZE_Y ) ), Rgba8::BLACK );
	AddVertsForDisc2D( attractModeVerts, Vec2( ATTRACT_MODE_CENTER_X, ATTRACT_MODE_CENTER_Y ), 90, Rgba8::RED );
		
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( attractModeVerts.size() ), attractModeVerts.data() );
	
	RenderGameModeMenu();

	g_theRenderer->EndCamera( m_attractCamera );
}

//----------------------------------------------------------------------------------------------------------------------
void App::RenderGameModeMenu() const
{
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> boxVerts;

	//----------------------------------------------------------------------------------------------------------------------
	// Menu background filler box
	AABB2 backgroundBounds = AABB2( Vec2( 0.0f, 0.0f ), Vec2( 380.0f, 400.0f ) );
	backgroundBounds.SetCenter( Vec2( ATTRACT_MODE_SIZE_X * 0.78f , ATTRACT_MODE_SIZE_Y * 0.44f ) );
	AddVertsForAABB2D( textVerts, backgroundBounds, Rgba8::DARK_RED );

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	//----------------------------------------------------------------------------------------------------------------------
	// gameMode selection text
	AABB2 box = AABB2( Vec2( 0.0f, 0.0f ), Vec2( 400.0f, 250.0f ) );
	box.SetCenter( Vec2( ATTRACT_MODE_SIZE_X * 0.8f , ATTRACT_MODE_SIZE_Y * 0.5f ) );
	float cellHeight = 15.0f;
	m_textFont->AddVertsForTextInBox2D( textVerts, box, cellHeight, 
		"1: GetNearestPoint2D\n\n2: Raycast Vs Disc2D\n\n3: Billiards 2D\n\n4: TestShapes 3D\n\n5: Raycast Vs Line 2D\n\n6: Raycast Vs AABB 2D\n\n7: Pachinko Machine 2D\n\n8: Bezier Curves 2D\n\n9: Spin and Friction 2D\n\n0: Raycast Vs AABB 3D\n\nQ: Raycast Vs OBB 2D\n\nW: Raycast Vs OBB 3D\n\nE: Convex Scene 2D\n\n", 
		Rgba8::WHITE, 1.0f, Vec2( 0.0f, 0.9f ), TextDrawMode::SHRINK_TO_FIT, 999999 );
	
	//----------------------------------------------------------------------------------------------------------------------
	// "MATH VISUAL TEST" title
	AABB2 textBox = AABB2( Vec2( 0.0f, 0.0f ), Vec2( 800.0f, 300.0f ) );
	textBox.SetCenter( Vec2(ATTRACT_MODE_CENTER_X, ATTRACT_MODE_SIZE_Y * 0.7f) );
	cellHeight = 100.0f;
	m_textFont->AddVertsForTextInBox2D( textVerts, textBox, cellHeight, "MATH VISUAL TEST", Rgba8::RED, 1.0f, Vec2(0.0f, 0.9f), TextDrawMode::SHRINK_TO_FIT, 999999);

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
bool App::Quit( EventArgs& args )
{
	UNUSED( args );
	return g_theApp->HandleQuitRequested();
}

//----------------------------------------------------------------------------------------------------------------------
void App::AttractModeInput()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_ESC ) )
	{
		g_theApp->HandleQuitRequested();
	}

	XboxController const& controller = g_theInput->GetController( 0 );

	if ( g_theInput->WasKeyJustPressed( ' ' ) || g_theInput->WasKeyJustPressed( 'N' ) || g_theInput->WasKeyJustPressed( 'P' ) || 
			controller.WasButtonJustPressed( BUTTON_START ) || controller.WasButtonJustPressed( BUTTON_A ) )
	{
		g_gameModeNum = GAMEMODE_2D_NEAREST_POINT;
		m_theGameMode = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Setting up attractMode menu input
	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		g_gameModeNum	  = GAMEMODE_2D_NEAREST_POINT;
		m_theGameMode	  = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		g_gameModeNum	  = GAMEMODE_2D_RAYCAST_VS_DISC;
		m_theGameMode	  = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '3' ) )
	{
		g_gameModeNum	  = GAMEMODE_2D_BILLIARDS;
		m_theGameMode	  = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '4' ) )
	{
		g_gameModeNum	  = GAMEMODE_3D_TEST_SHAPES;
		m_theGameMode	  = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '5' ) )
	{
		g_gameModeNum		= GAMEMODE_2D_RAYCAST_VS_LINE;
		m_theGameMode		= GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn	= false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '6' ) )
	{
		g_gameModeNum		= GAMEMODE_2D_RAYCAST_VS_AABB2;
		m_theGameMode		= GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn	= false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '7' ) )
	{
		g_gameModeNum		= GAMEMODE_2D_PACHINKO_MACHINE;
		m_theGameMode		= GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn	= false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '8' ) )
	{
		g_gameModeNum		= GAMEMODE_2D_BEZIER_CURVES;
		m_theGameMode		= GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn	= false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '9' ) )
	{
		g_gameModeNum		= GAMEMODE_2D_SPIN_AND_FRICTION;
		m_theGameMode		= GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn	= false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '0' ) )
	{
		g_gameModeNum		= GAMEMODE_3D_RAYCAST_VS_AABB3D;
		m_theGameMode		= GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn	= false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( 'Q' ) )
	{
		g_gameModeNum		= GAMEMODE_2D_RAYCAST_VS_OBB2D;
		m_theGameMode		= GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn	= false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( 'W' ) )
	{ 
		g_gameModeNum		= GAMEMODE_3D_RAYCAST_VS_OBB3D;
		m_theGameMode		= GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn	= false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( 'E' ) )
	{ 
		g_gameModeNum		= GAMEMODE_2D_CONVEX_SCENE;
		m_theGameMode		= GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn	= false;
		m_theGameMode->Startup();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void App::BeginFrame()
{
	Clock::TickSystemClock();

	g_theDevConsole->BeginFrame();
	     g_theInput->BeginFrame();
	    g_theWindow->BeginFrame();
	  g_theRenderer->BeginFrame();
	     g_theAudio->BeginFrame();

		 DebugRenderBeginFrame();
}
 
//----------------------------------------------------------------------------------------------------------------------
void App::Update( float deltaSeconds ) 
{ 
//	float deltaSeconds = m_theGameMode->m_gameClock.GetDeltaSeconds();

	if ( m_attractModeIsOn )
	{
		// updates only attractMode cam
		UpdateAttractModeCam();
		AttractModeInput();
		return;
	}

 	m_theGameMode->Update( deltaSeconds );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Mouse is visible if ( window does NOT have focus || devConsole is open || m_attractModeIsOn
	if ( g_theWindow->HasFocus() == false || g_theDevConsole->m_isOpen == true || m_attractModeIsOn == true )
	{
		g_theInput->SetCursorMode( false, false );
	}
	else if ( g_gameModeNum == GAMEMODE_2D_RAYCAST_VS_DISC	 || 
			  g_gameModeNum == GAMEMODE_2D_BILLIARDS		 || 
			  g_gameModeNum == GAMEMODE_2D_RAYCAST_VS_LINE   || 
			  g_gameModeNum == GAMEMODE_2D_RAYCAST_VS_AABB2  || 
			  g_gameModeNum == GAMEMODE_2D_PACHINKO_MACHINE  ||
			  g_gameModeNum == GAMEMODE_2D_BEZIER_CURVES	 ||
			  g_gameModeNum == GAMEMODE_2D_SPIN_AND_FRICTION ||
			  g_gameModeNum == GAMEMODE_2D_RAYCAST_VS_OBB2D  ||
			  g_gameModeNum == GAMEMODE_2D_CONVEX_SCENE )
	{
		g_theInput->SetCursorMode( false, false );
	}

	// Else, Mouse is hidden
	else
	{
		g_theInput->SetCursorMode( true, true );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Reshuffle
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F8 ) )
	{
		m_theGameMode->Reshuffle();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// F7 for next gameMode
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F7 ) )
	{
		g_gameModeNum = static_cast<GameMode>( (g_gameModeNum +1) % NUM_GAMEMODES );
		if ( m_theGameMode )
		{
			m_theGameMode->Shutdown();
			delete m_theGameMode;
		}
		m_theGameMode = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_theGameMode->Startup();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// F6 for previous gameMode
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F6 ) )
	{
		g_gameModeNum = static_cast<GameMode>( (g_gameModeNum + NUM_GAMEMODES -1 ) % NUM_GAMEMODES );
		if ( m_theGameMode )
		{
			m_theGameMode->Shutdown();
			delete m_theGameMode;
		}
		m_theGameMode = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_theGameMode->Startup();
	}

	UpdateReturnToAttractMode();
}

//----------------------------------------------------------------------------------------------------------------------
void App::Render() const
{
	// Draw attract mode
	if ( m_attractModeIsOn )
	{
		// uses attractModeCam and draws relevant stuff
		RenderAttractMode();
	}

	if ( !m_attractModeIsOn )
	{
		g_theRenderer->ClearScreen( Rgba8::BLACK );
		m_theGameMode->Render();
	}

	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theDevConsole )
	{
		AABB2 bounds = AABB2( 0.0f, 0.0f, WORLD_SIZE_X, WORLD_SIZE_Y );
		g_theDevConsole->Render( bounds );
	}
} 

//----------------------------------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_theDevConsole->EndFrame();
	     g_theInput->EndFrame();
	    g_theWindow->EndFrame();
	  g_theRenderer->EndFrame();
	     g_theAudio->EndFrame();
		 DebugRenderEndFrame();
}

//----------------------------------------------------------------------------------------------------------------------
void App::UpdateAttractModeCam()
{
	// update AttractCam only if in AttractMode
	if ( m_attractModeIsOn )
	{
		float width = ATTRACT_MODE_SIZE_Y * g_theWindow->GetConfig().m_clientAspect;
		m_attractCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( width, ATTRACT_MODE_SIZE_Y ) );
		return;
	}
}