#include "Game/App.hpp"
#include "Game/Game.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"

//----------------------------------------------------------------------------------------------------------------------
InputSystem*			g_theInput = nullptr;
Window*					g_theWindow = nullptr;
Renderer*				g_theRenderer = nullptr; //Created and owned by the App
RandomNumberGenerator*	g_theRNG = nullptr;
AudioSystem*			g_theAudio = nullptr;
App*					g_theApp = nullptr; //Created and owned by Main_Windows.cpp

//----------------------------------------------------------------------------------------------------------------------
App::App()
{	
	//----------------------------------------------------------------------------------------------------------------------
	// hack temp test code for EventSystem()
	DebuggerPrintf( "Hello, world\n" );
	
	g_theEventSystem = new EventSystem();

//	EventArgs sampleArgs;
//	sampleArgs.SetValue( "name", "Oak" );
//
//	FireEvent( "introduction", sampleArgs );
//	g_theEventSystem->SubscribeToEvent( "introduction", TestFuncForEventSystem );
//	FireEvent( "introduction", sampleArgs );
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// hack temp test code for PopulateFromXmlElementAttributes()
	XmlDocument gameConfigDoc;
	gameConfigDoc.LoadFile( "Data/GameConfig.xml");
	XmlElement* rootElement = gameConfigDoc.RootElement();

	g_gameConfigBlackboard.PopulateFromXmlElementAttributes( *rootElement );
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// hack temp test code for NamedStrings
//	NamedStrings relatives;
//	relatives.SetValue( "mom", "Linda" );
////	relatives.SetValue( "dad", "Barney" );
//	relatives.SetValue( "sister", "Beth" );
//	relatives.SetValue( "position", "3.5, 2.5" );
//	relatives.SetValue( "IsDaytime", "true" );
//	relatives.SetValue( "Color", "255, 255, 255, 255" );
//
//	std::string dadName		= relatives.GetValue( "dad", "Unknown" );
//	std::string uncleName	= relatives.GetValue( "uncle", "Unknown" );
//	bool		isThisTrue	= relatives.GetValue( "IsDaytime", false );
//	int			intNum		= relatives.GetValue( "position", 100 );
//	float		floatNum	= relatives.GetValue( "position", 3070.0f );
//	Rgba8		color		= relatives.GetValue( "Color", Rgba8( 255, 0, 0, 255 ) );
//	Vec2		position	= relatives.GetValue( "position", Vec2( -1.0f, -1.0f ) );
//	IntVec2		intVec2Num	= relatives.GetValue( "position", IntVec2( 1500, 250) );
	//----------------------------------------------------------------------------------------------------------------------

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
	windowConfig.m_windowTitle = "Libra";
	windowConfig.m_clientAspect = 2.0f;
	windowConfig.m_inputSystem = g_theInput;
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
	
	// Start up engine subsystems and game
	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->StartUp();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theAudio->Startup();

	m_theGame = new Game();
	m_theGame->Startup();

	g_theEventSystem->SubscribeToEvent( "quit", App::Quit );
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
	
	m_theGame->Shutdown();

	delete m_theGame;
	m_theGame = nullptr;

}
 
//-----------------------------------------------------------------------------------------------
// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
void App::RunFrame()
{
	float deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();

	BeginFrame();	
	Update(deltaSeconds);
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
void App::HardRestartDevCheat()
{
	// if F8 is pressed, hard restart the game
	// #ToDo change input key to "ESC" instead of "O"
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8 ))
	{
		m_theGame->Shutdown();
		delete m_theGame;
		
		m_theGame = new Game();
		m_theGame->Startup();
	}
}

//----------------------------------------------------------------------------------------------------------------------
// #ToDo delete this function when done testing
// Test code for event system
bool App::TestFuncForEventSystem( EventArgs& eventArgs )
{
	eventArgs.SetValue( "name", "pinnarKyaw" );
	DebuggerPrintf( "EventSystem TestFunc was called\n" );
	return true;
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

	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theAudio->BeginFrame();
}
 
//----------------------------------------------------------------------------------------------------------------------
void App::Update(float deltaSeconds) 
{ 
//	DebuggerPrintf( "%f\n", Clock::GetSystemClock().GetDeltaSeconds());

//	g_theGame->Update( deltaSeconds );
	m_theGame->Update( deltaSeconds );
	HardRestartDevCheat();
}

//----------------------------------------------------------------------------------------------------------------------
void App::Render() const
{
//	g_theGame->Render();
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
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theAudio->EndFrame();
	g_theRenderer->EndFrame();
}