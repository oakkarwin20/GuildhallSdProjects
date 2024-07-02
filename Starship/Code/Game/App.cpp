#include "Game/App.hpp"
#include "Game/Game.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"

//----------------------------------------------------------------------------------------------------------------------
InputSystem*			g_theInput			= nullptr;
Window*					g_theWindow			= nullptr;
Renderer*				g_theRenderer		= nullptr; //Created and owned by the App
RandomNumberGenerator*	g_theRNG			= nullptr;
AudioSystem*			g_theAudio			= nullptr;
App*					g_theApp			= nullptr; //Created and owned by Main_Windows.cpp

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
	// Create the EventSystem
	g_theEventSystem = new EventSystem();

	// Create engine subsystems and game
	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem( inputSystemConfig );

	// Creating Window
	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "C32 starship Gold (re factored)";
	windowConfig.m_clientAspect = 2.0f;
	windowConfig.m_inputSystem = g_theInput;
	g_theWindow = new Window ( windowConfig );

	// Creating Renderer
	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer( rendererConfig );

	// Creating DevConsole
	DevConsoleConfig devConsoleConfig;
	m_devConsoleCamera.SetOrthoView( Vec2( 0.0f, 0.0f), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
	devConsoleConfig.m_camera	= &m_devConsoleCamera; 
	devConsoleConfig.m_renderer	= g_theRenderer;
	g_theDevConsole				= new DevConsole( devConsoleConfig );

	// Creating AudioSystem
	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );
	
	// Start up engine subsystems and game
	g_theEventSystem->Startup();
	 g_theDevConsole->Startup();
  	      g_theInput->StartUp();
	     g_theWindow->Startup();
	   g_theRenderer->Startup();
	      g_theAudio->Startup();

	m_theGame = new Game();
	m_theGame->StartUp();

	//----------------------------------------------------------------------------------------------------------------------
	// Subscribe to events
	g_theEventSystem->SubscribeToEvent( "quit", App::Quit );
	g_theEventSystem->SubscribeToEvent( "setgametimescale", App::SetGameTimeScale );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug keys for starship
	g_theDevConsole->AddLine( Rgba8::LIGHTBLUE, "Keys" );
	g_theDevConsole->AddLine( Rgba8::LIGHTBLUE, "P   - Pause"	   );
	g_theDevConsole->AddLine( Rgba8::LIGHTBLUE, "O   - Single Step" );
	g_theDevConsole->AddLine( Rgba8::LIGHTBLUE, "T   - Slow-Mo"	   );
	g_theDevConsole->AddLine( Rgba8::LIGHTBLUE, "N   - Respawn"	   );
	g_theDevConsole->AddLine( Rgba8::LIGHTBLUE, "F1  - DebugDraw"   );
	g_theDevConsole->AddLine( Rgba8::LIGHTBLUE, "F8  - New Game"	   );
	g_theDevConsole->AddLine( Rgba8::LIGHTBLUE, "E   - Thrust"	   );	
	g_theDevConsole->AddLine( Rgba8::LIGHTBLUE, "S   - Turn Left"   );	
	g_theDevConsole->AddLine( Rgba8::LIGHTBLUE, "F   - Turn Right"  );	
//	g_theDevConsole->AddLine( Rgba8::BLUE, "' ' - Start Game" );
}

//----------------------------------------------------------------------------------------------------------------------
void App::Shutdown()
{
		 g_theAudio->Shutdown();
	  g_theRenderer->Shutdown();
		g_theWindow->Shutdown();
		 g_theInput->ShutDown();
	g_theDevConsole->Shutdown();

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

	delete m_theGame;
	m_theGame = nullptr;

	m_theGame->Shutdown();
}
 
//-----------------------------------------------------------------------------------------------
// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
void App::RunFrame()
{
	BeginFrame();	
	Update();
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
bool App::SetGameTimeScale( EventArgs& args )
{
	float keyCode = args.GetValue( "scale", -1.0f );
	if ( keyCode <= -1 )
	{
		g_theDevConsole->AddLine( Rgba8::RED, ( "Invalid format" ) );
		g_theDevConsole->AddLine( Rgba8::YELLOW, ( "Example:setgametimescale scale=1.0" ) );
		return false;
	}

	g_theApp->m_theGame->m_gameClock.SetTimeScale( keyCode );
	return true;
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
}
 
//----------------------------------------------------------------------------------------------------------------------
void App::Update() 
{ 
	m_theGame->Update();
	hardRestartDevCheat();
}

//----------------------------------------------------------------------------------------------------------------------
void App::Render() const
{
	m_theGame->Render();

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
}

//----------------------------------------------------------------------------------------------------------------------
void App::hardRestartDevCheat()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F8 ) )
	{
		m_theGame->Shutdown();
		delete m_theGame;

		m_theGame = new Game();
		m_theGame->StartUp();
	}
}