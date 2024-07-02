#include "Game/App.hpp"
#include "Game/Game.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Networking/NetSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------
InputSystem*			g_theInput			= nullptr;
Window*					g_theWindow			= nullptr;
Renderer*				g_theRenderer		= nullptr; //Created and owned by the App
RandomNumberGenerator*	g_theRNG			= nullptr;
AudioSystem*			g_theAudio			= nullptr;
App*					g_theApp			= nullptr; //Created and owned by Main_Windows.cpp
// NetSystem*				g_theNetSystem		= nullptr; 

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
	// Load Game Config
	LoadGameConfig( "Data/GameConfig.xml" );

	// Creating EventSystem
	g_theEventSystem = new EventSystem();

	// Create engine subsystems and game
	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem( inputSystemConfig );

	// Creating Window
	WindowConfig windowConfig;
//	windowConfig.m_windowTitle	= "ModelViewer_Client";
	windowConfig.m_windowTitle	= std::string( "modelViewer " ) + std::string( m_commandLineArg );
	windowConfig.m_clientAspect = 2.0f;
	windowConfig.m_inputSystem	= g_theInput;
	windowConfig.m_isFullScreen	= false;
	g_theWindow					= new Window( windowConfig );

	// Creating Renderer
	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer			= new Renderer( rendererConfig );

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

	// Subscribe and Fire Command_LoadGameConfig event
	g_theEventSystem->SubscribeToEvent(    "LoadGameConfig", Command_LoadGameConfig		);	
	g_theEventSystem->SubscribeToEvent( "quit", App::Quit );
	g_theEventSystem->SubscribeToEvent(  "debugrenderclear", Command_DebugRenderClear   );
	g_theEventSystem->SubscribeToEvent( "debugrendertoggle", Command_DebugRenderToggle  );	
	LoadAdditionalGameConfig( m_commandLineArg );

	// Creating NetSystem
	NetSystemConfig netSystemConfig;
	netSystemConfig.m_modeString		= g_gameConfigBlackboard.GetValue( "netMode", "None" );
	netSystemConfig.m_hostAddressString = g_gameConfigBlackboard.GetValue( "netHostAddress", "127.0.0.1" );
	netSystemConfig.m_recvBufferSize	= g_gameConfigBlackboard.GetValue( "netRecvBufferSize", -1 );
	netSystemConfig.m_sendBufferSize	= g_gameConfigBlackboard.GetValue( "netSendBufferSize", -1 );
	g_theNetSystem						= new NetSystem( netSystemConfig );

	// Start up engine subsystems and game
	g_theEventSystem->Startup();
	 g_theDevConsole->Startup();
  	      g_theInput->StartUp();
	     g_theWindow->Startup();
	   g_theRenderer->Startup();
	      g_theAudio->Startup();
	  g_theNetSystem->Startup();

	m_theGame = new Game();
	m_theGame->StartUp();


	//----------------------------------------------------------------------------------------------------------------------
	// Debug keys for Protogame3D
	g_theDevConsole->AddLine( Rgba8::GREEN, "Keys"																	);
	g_theDevConsole->AddLine( Rgba8::GREEN, "Mouse                   - Aim"											);
	g_theDevConsole->AddLine( Rgba8::GREEN, "WASD                    - Move Forward"								);
	g_theDevConsole->AddLine( Rgba8::GREEN, "E / Z                   - Elevate"										);	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Q / C                   - De-Elevate"									);	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Shift                   - Move Faster"									);	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Control                 - Move Slower"									);	
	g_theDevConsole->AddLine( Rgba8::GREEN, "P                       - Pause Game"									);	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Escape                  - Exit Game"									);	
	g_theDevConsole->AddLine( Rgba8::GREEN, "Space                   - Start Game"									);	
	g_theDevConsole->AddLine( Rgba8::GREEN, "LoadModel File=<path>   - Use file specified in<path>to load model"	);
	g_theDevConsole->AddLine( Rgba8::GREEN, "L                       - Load Model"									);

//	std::string testString = "\"Something 1\" x y z";
//	Strings  newString = SplitStringWithQuotes( testString, ' ' );

	//----------------------------------------------------------------------------------------------------------------------
	// Test netSystem
//	std::string testString = "ABCDE";
//	g_theNetSystem->m_sendQueue.push_back( testString );
}

//----------------------------------------------------------------------------------------------------------------------
void App::Shutdown()
{
	m_theGame->Shutdown();
	
	delete m_theGame;
	m_theGame = nullptr;
	
	  g_theRenderer->Shutdown();
	     g_theAudio->Shutdown();
	    g_theWindow->Shutdown();
	     g_theInput->ShutDown();
	g_theDevConsole->Shutdown();
	DebugRenderSystemShutdown();
	 g_theNetSystem->Shutdown();

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
void App::LoadGameConfig( std::string filepath )
{
	XmlDocument document;
	XmlResult result = document.LoadFile( filepath.c_str() );
	GUARANTEE_OR_DIE( result == tinyxml2::XML_SUCCESS, Stringf( "Failed to open required game config file %s", filepath.c_str() ) );

	XmlElement* rootElement = document.RootElement();
	GUARANTEE_OR_DIE( rootElement, "is invalid" );

	g_gameConfigBlackboard.PopulateFromXmlElementAttributes( *rootElement );

//	std::string modelToLoad = g_gameConfigBlackboard.GetValue( "defaultMap", std::string( "Invalid map name") );
//	m_theGame-> g_theModel->ParseXmlData( "Data/Models/" + modelToLoad );
}

//----------------------------------------------------------------------------------------------------------------------
bool App::Command_LoadGameConfig( EventArgs& args )
{
	std::string filePath = args.GetValue( "File", "Invalid File");
	if ( filePath == "Invalid File" )
	{
		return false;
	}

	XmlDocument document;
	XmlResult result		= document.LoadFile( filePath.c_str() );
	GUARANTEE_OR_DIE( result == tinyxml2::XML_SUCCESS, Stringf( "Failed to open required game config file %s", filePath.c_str() ) );

	XmlElement* rootElement = document.RootElement();
	GUARANTEE_OR_DIE( rootElement, "is invalid" );

	g_gameConfigBlackboard.PopulateFromXmlElementAttributes( *rootElement );
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
void App::LoadAdditionalGameConfig( std::string const& commandLineArg )
{
	if ( !commandLineArg.empty() )
	{
		g_theDevConsole->Execute( commandLineArg );
	}
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
	 g_theNetSystem->BeginFrame();

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
	 g_theNetSystem->EndFrame();
	
	DebugRenderEndFrame();
}
