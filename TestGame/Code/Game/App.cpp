#include "Game/App.hpp"
#include "Game/Game.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Window/Window.hpp"

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
	// Creating EventSystem
	g_theEventSystem = new EventSystem();

	// Create engine subsystems and game
	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem( inputSystemConfig );

	// Creating Window
	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "Protogame3D";
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

	JobSystemConfig jobSystemConfig;
	jobSystemConfig.m_preferredNumberOfWorkers = -1;
	g_theJobSystem = new JobSystem( jobSystemConfig );

	// Start up engine subsystems and game
	g_theEventSystem->Startup();
	 g_theDevConsole->Startup();
  	      g_theInput->StartUp();
	     g_theWindow->Startup();
	   g_theRenderer->Startup();
	      g_theAudio->Startup();
	  g_theJobSystem->Startup();


	m_theGame = new Game();
	m_theGame->StartUp();

	g_theEventSystem->SubscribeToEvent( "quit", App::Quit );
	g_theEventSystem->SubscribeToEvent( "debugrenderclear", Command_DebugRenderClear );
	g_theEventSystem->SubscribeToEvent( "debugrendertoggle", Command_DebugRenderToggle );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug keys for Protogame3D
	g_theDevConsole->AddLine( Rgba8::GREEN, "Keys" );
	g_theDevConsole->AddLine( Rgba8::GREEN, "Mouse    - Aim" );
	g_theDevConsole->AddLine( Rgba8::GREEN, "W        - Move Forward" );
	g_theDevConsole->AddLine( Rgba8::GREEN, "A        - Strafe Left"  );
	g_theDevConsole->AddLine( Rgba8::GREEN, "S        - Move Back"	  );
	g_theDevConsole->AddLine( Rgba8::GREEN, "D        - Strafe Right" );
	g_theDevConsole->AddLine( Rgba8::GREEN, "Q        - Roll Left" );
	g_theDevConsole->AddLine( Rgba8::GREEN, "E        - Roll Right" );
	g_theDevConsole->AddLine( Rgba8::GREEN, "Left     - Turn Left"	  );
	g_theDevConsole->AddLine( Rgba8::GREEN, "Right    - Turn Right"	  );
	g_theDevConsole->AddLine( Rgba8::GREEN, "Up       - Pitch Up"	  );
	g_theDevConsole->AddLine( Rgba8::GREEN, "Down     - Pitch Down"	  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Z        - Elevate"	  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "C        - De-Elevate"	  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Shift    - Move Faster"  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Control  - Move Slower"  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "P        - Pause Game"	  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Escape   - Exit Game"	  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Space    - Start Game"	  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "H        - Reset Camera to Origin"  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "1        - Spawn Wireframe Sphere"	  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "2        - Spawn Line"	  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "3        - Spawn Basis"	  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "4        - Spawn Billboarded Text"	  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "5        - Spawn Wireframe Cylinder"	  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "6        - Spawn Point"	  );	
	g_theDevConsole->AddLine( Rgba8::GREEN, "7        - Add Message"	  );	

}

//----------------------------------------------------------------------------------------------------------------------
void App::Shutdown()
{
	     g_theAudio->Shutdown();
	  g_theRenderer->Shutdown();
	    g_theWindow->Shutdown();
	     g_theInput->ShutDown();
	g_theDevConsole->Shutdown();
	g_theJobSystem->Shutdown();
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
	 g_theJobSystem->BeginFrame();

	DebugRenderBeginFrame();
}	 
 
//----------------------------------------------------------------------------------------------------------------------
void App::Update() 
{ 
	m_theGame->Update();

	//----------------------------------------------------------------------------------------------------------------------
	// Mouse is visible if ( window does NOT have focus || devConsole is open || m_attractModeIsOn
	if ( g_theWindow->HasFocus() == false || g_theDevConsole->m_isOpen == true || m_theGame->m_AttractModeIsOn == true )
	{
		g_theInput->SetCursorMode( false, false );
	}
	// Else, Mouse is hidden
	else
	{
		g_theInput->SetCursorMode( true, true );
	}
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
	 g_theJobSystem->EndFrame();

	DebugRenderEndFrame();
}