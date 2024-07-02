#include "Game/App.hpp"
#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Window/Window.hpp"

//----------------------------------------------------------------------------------------------------------------------
InputSystem*			g_theInput			= nullptr;
Window*					g_theWindow			= nullptr;
Renderer*				g_theRenderer		= nullptr; //Created and owned by the App
RandomNumberGenerator*	g_theRNG			= nullptr;
AudioSystem*			g_theAudio			= nullptr;
App*					g_theApp			= nullptr; //Created and owned by Main_Windows.cpp

//----------------------------------------------------------------------------------------------------------------------
GameMode				g_gameModeNum = GAMEMODE_3D;

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
	// Creating EventSystem
	g_theEventSystem = new EventSystem();

	// Create engine subsystems and game
	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem( inputSystemConfig );

	// Creating Window
	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "ProtoMode";
	windowConfig.m_clientAspect = 2.0f;
	windowConfig.m_inputSystem = g_theInput;
	g_theWindow = new Window( windowConfig );

	// Creating Renderer
	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer( rendererConfig );

	// Creating DevConsole
	DevConsoleConfig devConsoleConfig;
	m_devConsoleCamera.SetOrthoView( Vec2( 0.0f, 0.0f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y) );
	devConsoleConfig.m_camera	= &m_devConsoleCamera; 
	devConsoleConfig.m_renderer	= g_theRenderer;
	g_theDevConsole				= new DevConsole( devConsoleConfig );

	// Creating AudioSystem
	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );
	
	// Creating DebugRenderSystem
	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;
	DebugRenderSystemStartup( debugRenderConfig );

	// Start up engine subsystems and game
	g_theEventSystem->Startup();
	 g_theDevConsole->Startup();
  	      g_theInput->StartUp();
	     g_theWindow->Startup();
	   g_theRenderer->Startup();
	      g_theAudio->Startup();

//	m_theGame = new GameModeProtogame3D();
//	m_theGame->StartUp();

	//----------------------------------------------------------------------------------------------------------------------
	// Initializing bitmap font text
	m_textFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	//----------------------------------------------------------------------------------------------------------------------
	// Initializing texture
//	m_attractModeTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/mainmenu.png" );

	//----------------------------------------------------------------------------------------------------------------------
	g_theEventSystem->SubscribeToEvent( "quit", App::Quit );
	g_theEventSystem->SubscribeToEvent( "debugrenderclear", Command_DebugRenderClear );
	g_theEventSystem->SubscribeToEvent( "debugrendertoggle", Command_DebugRenderToggle );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug keys for "FIFA_TEST_3D"
	g_theDevConsole->AddLine( Rgba8::GREEN, "Keys"														 );
	g_theDevConsole->AddLine( Rgba8::GREEN, "Mouse          - Aim (While camera controlled)"			 );
	g_theDevConsole->AddLine( Rgba8::GREEN, "WASD           - Move Actor"								 );
	g_theDevConsole->AddLine( Rgba8::GREEN, "IJKL           - Move point of contact on ball"			 );
	g_theDevConsole->AddLine( Rgba8::GREEN, "Space          - Ground Pass"								 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Q              - Elevate"									 );
	g_theDevConsole->AddLine( Rgba8::GREEN, "E              - Descend"									 );
	g_theDevConsole->AddLine( Rgba8::GREEN, "Shift          - Move Faster"								 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Ctrl           - Move Slower"								 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "P              - Pause Game"								 );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Escape         - Exit Game"								 );	
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
//	  m_theGameMode->Shutdown();

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

}
 
//-----------------------------------------------------------------------------------------------
// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
void App::RunFrame()
{
	 float deltaSeconds = m_gameClock.GetDeltaSeconds();

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
bool App::Quit( EventArgs& args )
{
	UNUSED( args ); 
	return g_theApp->HandleQuitRequested();
}

//----------------------------------------------------------------------------------------------------------------------
void App::Run()
{
	// Program main loop; keep running frames until it's time to quit
	while (!IsQuitting())
	{
		g_theApp->RunFrame();
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
	// Else, Mouse is hidden
	else
	{
		g_theInput->SetCursorMode( true, true );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// F7 for next gameMode
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F7 ) )
	{
		g_gameModeNum = static_cast<GameMode>( ( g_gameModeNum + 1 ) % NUM_GAMEMODES );
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
		g_gameModeNum = static_cast<GameMode>( ( g_gameModeNum + NUM_GAMEMODES - 1 ) % NUM_GAMEMODES );
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
		g_theRenderer->ClearScreen( Rgba8::BLACK );
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
		m_attractCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( ATTRACT_MODE_SIZE_X, ATTRACT_MODE_SIZE_Y ) );
		return;
	}
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
		g_gameModeNum = GAMEMODE_3D;
		m_theGameMode = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Setting up attractMode menu input
	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		g_gameModeNum	  = GAMEMODE_2D;
		m_theGameMode	  = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}
	if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		g_gameModeNum	  = GAMEMODE_3D;
		m_theGameMode	  = GameModeBase::CreateNewGameOfType( g_gameModeNum );
		m_attractModeIsOn = false;
		m_theGameMode->Startup();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void App::RenderAttractMode() const
{
	g_theRenderer->BeginCamera( m_attractCamera );

	std::vector<Vertex_PCU> attractModeVerts;
	attractModeVerts.reserve( 9 );

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( attractModeVerts.size() ), attractModeVerts.data() );
	g_theRenderer->BindTexture( nullptr );

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
//	AABB2 backgroundBounds = AABB2( Vec2( 0.0f, 0.0f ), Vec2( 380.0f, 300.0f ) );
//	backgroundBounds.SetCenter( Vec2( ATTRACT_MODE_SIZE_X * 0.78f , ATTRACT_MODE_SIZE_Y * 0.47f ) );
//	AddVertsForAABB2D( textVerts, backgroundBounds, Rgba8::DARK_RED );

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	//----------------------------------------------------------------------------------------------------------------------
	// gameMode selection text
	AABB2 box = AABB2( Vec2( 0.0f, 0.0f ), Vec2( ATTRACT_MODE_SIZE_X, ATTRACT_MODE_SIZE_Y ) );
	box.SetCenter( Vec2( ATTRACT_MODE_CENTER_X, ATTRACT_MODE_SIZE_Y * 0.5f ) );
	m_textFont->AddVertsForTextInBox2D( textVerts, box, 15.0f, 
		"1: Proto Mode 2D\n\n2: Proto Mode 3D", 
		Rgba8::WHITE );

	//----------------------------------------------------------------------------------------------------------------------
	// "ProtoMode" title
	AABB2 textBox = AABB2( Vec2( 0.0f, 0.0f ), Vec2( ATTRACT_MODE_SIZE_X, ATTRACT_MODE_SIZE_Y ) );
	textBox.SetCenter( Vec2(ATTRACT_MODE_CENTER_X, ATTRACT_MODE_SIZE_Y * 0.75f) );
	m_textFont->AddVertsForTextInBox2D( textVerts, textBox, 150.0f, "ProtoMode", Rgba8::CYAN );

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}