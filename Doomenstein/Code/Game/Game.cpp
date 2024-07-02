#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Map.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/Actor.hpp"

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
	// Set Audio Num Listeners
	g_theAudio->SetNumListeners( g_theApp->m_numPlayers );
}

//----------------------------------------------------------------------------------------------------------------------
Game::~Game()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Game::StartUp()
{
	// Initialize textures //
	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TestUV.png" );

	// Initialize definitions //
	 ActorDefinition::InitializeActorDef (	"Data/Definitions/ProjectileActorDefinitions.xml"	);				
	WeaponDefinition::InitializeWeaponDef(	"Data/Definitions/WeaponDefinitions.xml"			);				
	 ActorDefinition::InitializeActorDef (	"Data/Definitions/ActorDefinitions.xml"				);				
	   MapDefinition::InitializeMapDef	 (	"Data/Definitions/MapDefinitions.xml"				);				
	  TileDefinition::InitializeTileDef	 (	"Data/Definitions/TileDefinitions.xml"				);			

	// Camera //
	UpdateCameras();

	// Initialize Audio //
	// Music
	m_attractMusic	= g_theAudio->CreateOrGetSound( "Data/Audio/Music/MainMenu_InTheDark.mp2"	);
	m_gameMusic		= g_theAudio->CreateOrGetSound( "Data/Audio/Music/E1M1_AtDoomsGate.mp2"		);
	// SFX
	bool is3D = true;
	g_theAudio->CreateOrGetSound( "Data/Audio/PistolFire.wav"	, is3D );
	g_theAudio->CreateOrGetSound( "Data/Audio/PlasmaFire.wav"	, is3D );
	g_theAudio->CreateOrGetSound( "Data/Audio/PlasmaHit1.wav"	, is3D );
	g_theAudio->CreateOrGetSound( "Data/Audio/DemonAttack.wav"	, is3D );
	g_theAudio->CreateOrGetSound( "Data/Audio/PlayerDeath.wav"	, is3D );
	g_theAudio->CreateOrGetSound( "Data/Audio/DemonHurt.wav"	, is3D );
	g_theAudio->CreateOrGetSound( "Data/Audio/EnemyShoot.wav"	, is3D );
	// Start Attract Music
	m_attractPlaybackID = g_theAudio->StartSound(m_attractMusic, true );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Update()
{
	float deltaSeconds = m_clock.GetDeltaSeconds();

	if ( m_currentState == GameState::ATTRACT )
	{
//		if ( m_startAttractMusic )
//		{		
//			// Reset bool to avoid music restarting every frame
//			m_startAttractMusic = false;
//		}

		// Update Input
		UpdateAttractModeInput();								// Update attractModeInput()
	}
	else if ( m_currentState == GameState::PLAYING )
	{
		// Player Controls //
		UpdatePauseQuitAndSlowMo();								// UpdateGameInput
		UpdateToggleControlsToActor( deltaSeconds );
		m_player->Update( deltaSeconds );
		
		// Audio //
//		g_theAudio->SetSoundPosition( m_soundPlaybackID, m_player->m_currentActor->m_position );
		g_theAudio->SetSoundPosition( m_soundPlaybackID, m_soundPosition );
		
		// Map //
		m_currentMap->Update( deltaSeconds );

		// Attract Mode //
		UpdateReturnToAttractMode();

		// Lighting //
		UpdateLightingDebugInput();

		// Cameras //
		UpdateCameras();
	}
	else if ( m_currentState == GameState::LOBBY )
	{
		UpdateLobbyInput();
	}

	if ( m_requestedState != m_currentState )
	{
		// exit current state
		ExitState( m_currentState );
		// set current state
		m_currentState = m_requestedState;
		// enter current state
		EnterState( m_currentState );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	if ( m_currentState == GameState::ATTRACT )
	{
		g_theRenderer->ClearScreen( Rgba8::DARK_RED );			// Clear AttractMode screen
		RenderAttractMode();									// Render AttractMode 
	}
	else if ( m_currentState == GameState::PLAYING )
	{
		g_theRenderer->ClearScreen( Rgba8::LIGHTBLACK );		// Clear screen
		g_theRenderer->BeginCamera(m_player->m_worldCamera);	// Begin WorldCamera
		m_player->Render();										// Call Player::Render in world space
		m_currentMap->Render();									// Call Map::Render in world space
		DebugRenderWorld( m_player->m_worldCamera );			// Call DRS::Render for objects in world space
		g_theRenderer->EndCamera(m_player->m_worldCamera);		// End m_worldCam

		//----------------------------------------------------------------------------------------------------------------------
		RenderUI();			// Render in UI space aka screen space
	}
	else if ( m_currentState == GameState::LOBBY )
	{
		g_theRenderer->ClearScreen( Rgba8::DARK_RED );			// Clear AttractMode screen
		RenderLobby();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if (g_theInput->WasKeyJustPressed('P') || g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::BUTTON_START))
	{
		SoundID testSound = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
		UNUSED( testSound );
		g_theAudio->StartSound(testSound);			// Comment out this line of code to remove pause sound playing

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
	XboxController const& controller = g_theInput->GetController( 0 );

	if (  ( g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.IsButtonDown( BUTTON_SELECT ) ) && 
		m_currentState == GameState::PLAYING  )
	{
		// Set game state back to attractMode
		m_requestedState = GameState::ATTRACT;
	}
}
  
//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateAttractModeInput()
{
	XboxController const& controller = g_theInput->GetController(0);

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(BUTTON_SELECT) )
	{
		g_theApp->HandleQuitRequested();
	}
	
	// Start game if 'SPACEBAR' was pressed
	if ( g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N') || controller.WasButtonJustPressed(BUTTON_START) || controller.WasButtonJustPressed(BUTTON_A))
	{
//		m_requestedState = GameState::PLAYING;		// Set game state back to PLAYING
		m_requestedState = GameState::LOBBY;		// Set game state back to PLAYING

		SoundID clickSound = g_theAudio->CreateOrGetSound( "Data/Audio/Click.mp3" );
		g_theAudio->StartSound( clickSound );			// Comment out this line of code to remove pause sound playing
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateLobbyInput()
{
	XboxController const& controller = g_theInput->GetController(0);

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(BUTTON_SELECT) )
	{
		m_requestedState = GameState::ATTRACT;		// Set game state back to PLAYING
	}

	// Start game if 'SPACEBAR' was pressed
	if ( g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N') || controller.WasButtonJustPressed(BUTTON_START) || controller.WasButtonJustPressed(BUTTON_A))
	{
		m_requestedState = GameState::PLAYING;		// Set game state back to PLAYING
	}
} 

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderLobby() const
{
	// Draw everything in screen space
	g_theRenderer->BeginCamera(m_attractCamera);

	g_theRenderer->SetModelConstants();

	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	// Drawing "First Triangle"
//	Vertex_PCU vertices[] =
//	{
//		Vertex_PCU( Vec3( 1200.0f, 200.0f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
//		Vertex_PCU( Vec3(  800.0f, 600.0f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
//		Vertex_PCU( Vec3(  400.0f, 200.0f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
//	};
//
//	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
//	g_theRenderer->BindTexture( nullptr );
//	g_theRenderer->SetModelConstants();
//	g_theRenderer->DrawVertexArray( 3, vertices );
	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	BitmapFont* bitmapFont = nullptr;
	bitmapFont = g_theRenderer->CreateOrGetBitmapFontFromFile( std::string( "Data/Fonts/SquirrelFixedFont" ).c_str() );

	std::vector<Vertex_PCU> textVerts;
	Vec2 textMins			= Vec2::ZERO;
	Vec2 textMaxs			= Vec2( ATTRACT_MODE_CENTER_X, ATTRACT_MODE_CENTER_Y );
	AABB2 textBounds		= AABB2( textMins, textMaxs );
	textBounds.SetCenter( Vec2(ATTRACT_MODE_CENTER_X, ATTRACT_MODE_CENTER_Y * 1.5) );
	float cellHeight		= 50.0f;
	std::string textString = std::string( "LOBBY" );
	bitmapFont->AddVertsForTextInBox2D( textVerts, textBounds, cellHeight, textString, Rgba8::WHITE );

	textBounds.SetCenter( Vec2(ATTRACT_MODE_CENTER_X, ATTRACT_MODE_SIZE_Y * 0.3) );
	cellHeight						= 15.0f;
	std::string instructionString	= std::string( "Press SPACE to start \nPress ESCAPE to leave " );
	bitmapFont->AddVertsForTextInBox2D( textVerts, textBounds, cellHeight, instructionString, Rgba8::WHITE );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw call for text
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( &bitmapFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() );

	// Unbind texture and shader
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------

	// End attractMode Camera
	g_theRenderer->EndCamera(m_attractCamera);
}

//----------------------------------------------------------------------------------------------------------------------
void Game::EnterState( GameState state )
{
	if ( state == GameState::ATTRACT )
	{
	}
	else if ( state == GameState::PLAYING )
	{
		// Stop attract music 
		g_theAudio->StopSound( m_attractPlaybackID );

		// Start game music sound
		m_gamePlaybackID = g_theAudio->StartSound( m_gameMusic, true );

		m_player = new PlayerController();																	// Adding player to scene
//		m_player->m_position				= Vec3( 2.5f, 8.5f, 0.5f );																	// Set playerPos inside Map		
		std::string mapName					= g_gameConfigBlackboard.GetValue( "defaultMap", std::string( "Invalid map name") );		// Get mapDef from gameConfig XML data
		MapDefinition const* m_mapDefByName = MapDefinition::GetMapDefByName( mapName );							
		m_currentMap						= new Map( m_mapDefByName, m_player );														// Creating map with mapDef from XML data
	}
	else if ( state == GameState::LOBBY )
	{
	}
}
 
//----------------------------------------------------------------------------------------------------------------------
void Game::ExitState( GameState state )
{
	if ( state == GameState::ATTRACT )
	{
	}
	else if ( state == GameState::PLAYING )
	{
		// Remove player and map from game
		delete m_player;
		m_player = nullptr;
		delete m_currentMap;
		m_currentMap = nullptr;

		// Stop game music sound
		g_theAudio->StopSound( m_gamePlaybackID );
		m_restartAttractMusic = true;

		if ( !g_theAudio->IsPlaying( m_gamePlaybackID ) )
		{
			// Start attractMusic
			m_attractPlaybackID = g_theAudio->StartSound( m_attractMusic, true );
		}
	}
	else if ( state == GameState::LOBBY )
	{
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );

	DebugRenderScreen( m_screenCamera );				// Pass m_screenCamera for DebugRenderScreen to use
	m_currentMap->RenderUI();
	Render_DRS_UI_Text();								// Render UI text

	// End UI Camera
	g_theRenderer->EndCamera( m_screenCamera );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateToggleControlsToActor( float deltaSeconds )
{
	//// Press F1 to switch controls from playerCamera to projectile actor
	//if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	//{
	//	// Set bool for disabling playerCam WASD controls
	//	m_player->m_playerIsControllable = !m_player->m_playerIsControllable;
	//}

	if ( !m_player->m_playerIsControllable )
	{
		Mat44 playerMatrix	= m_player->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
		Vec3 forward		= playerMatrix.GetIBasis3D();
		Vec3 left			= playerMatrix.GetJBasis3D();
		Vec3 up				= playerMatrix.GetKBasis3D();

		for ( int i = 0; i < m_currentMap->m_actorList.size(); i++ )
		{
			// If actor[i] is movable, update keyboard input to move projectile actor 
			if ( m_currentMap->m_actorList[i]->m_isMovable == true )
			{	
				// Forward
				if ( g_theInput->IsKeyDown( 'W' ) )
				{
					m_currentMap->m_actorList[i]->m_position += ( m_player->m_currentSpeed * forward * deltaSeconds );
				}
				
				// Backward
				if ( g_theInput->IsKeyDown( 'S' ) )
				{
					m_currentMap->m_actorList[i]->m_position -= ( m_player->m_currentSpeed * forward * deltaSeconds );
				}

				// Left
				if ( g_theInput->IsKeyDown( 'A' ) )
				{
					m_currentMap->m_actorList[i]->m_position += ( m_player->m_currentSpeed * left * deltaSeconds );
				}

				// Right
				if ( g_theInput->IsKeyDown( 'D' ) )
				{
					m_currentMap->m_actorList[i]->m_position -= ( m_player->m_currentSpeed * left * deltaSeconds );
				}

				// Elevate
				if ( g_theInput->IsKeyDown( 'Z' ) )
				{
					m_currentMap->m_actorList[i]->m_position.z += ( m_player->m_currentSpeed * deltaSeconds );
				}

				// De-Elevate
				if ( g_theInput->IsKeyDown( 'C' ) )
				{
					m_currentMap->m_actorList[i]->m_position.z -= ( m_player->m_currentSpeed * deltaSeconds );
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateLightingDebugInput()
{
	// Debug controls
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		// Show debug message
		m_showLightingDebug = !m_showLightingDebug;
		// Sun dir, sun intensity, ambient intensity, and hotkeys for changin these values (F2, F2)
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F2 ) )
	{
		m_currentMap->m_sunDirection.x -= 1.0f;
		// Add debug message 
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F3 ) )
	{
		m_currentMap->m_sunDirection.x += 1.0f;
		// Add debug message 
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F4 ) )
	{
		m_currentMap->m_sunDirection.y -= 1.0f;
		// Add debug message 
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F5 ) )
	{
		m_currentMap->m_sunDirection.y += 1.0f;
		// Add debug message 
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F6 ) )
	{
		m_currentMap->m_sunIntensity -= 0.05f;
		// Add debug message 
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F7 ) )
	{
		m_currentMap->m_sunIntensity += 0.05f;
		// Add debug message 
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F8 ) )
	{
		m_currentMap->m_ambientIntensity -= 0.05f;
		// Add debug message 
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F9 ) )
	{
		m_currentMap->m_ambientIntensity += 0.05f;
		// Add debug message 
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Setting perspective view
	if ( m_player != nullptr )
	{
		m_player->UpdateCamera();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Setting ortho views
	m_attractCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( 1600.0f, 800.0f ) );
	m_screenCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( SCREEN_SIZE_X, SCREEN_SIZE_Y ) );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const
{
	// Draw everything in screen space
	g_theRenderer->BeginCamera(m_attractCamera);

	g_theRenderer->SetModelConstants();

	//----------------------------------------------------------------------------------------------------------------------
	// Drawing "First Triangle"
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
void Game::Render_DRS_UI_Text() const
{
	// Initialize and set UI variables
	float cellHeight			  = 2.0f;
	float duration				  = 0.0f;
	Vec2 timePosition			  = Vec2( SCREEN_SIZE_X, ( SCREEN_SIZE_Y ) );
	Vec2 timeAlignment			  = Vec2( 1.0f, 1.0f );
	Vec2 controlModePosition	  = Vec2( 0.0f, ( SCREEN_SIZE_Y ) );
	Vec2 controlModeAlignment	  = Vec2( 0.0f, 1.0f );
	float fps					  = 1.0f / m_clock.GetDeltaSeconds();
	float scale					  = m_clock.GetTimeScale();
	std::string timeText		  = Stringf( "[GAME CLOCK] Time: %.2f. FPS: %.2f, Scale %.2f. ", m_clock.GetTotalSeconds(), fps, scale );
//	std::string controlModeText	  = Stringf( "[F1] Control Mode %s ", std::string("Player").c_str() );
	
	if ( m_player->m_playerIsControllable )
	{
//		std::string controlModeString = "Camera";
//		std::string controlModeText	  = Stringf( "[F1] Control Mode: %s", controlModeString.c_str() );
//		DebugAddScreenText( controlModeText, controlModePosition, cellHeight, controlModeAlignment, duration );
	}
	else if ( !m_player->m_playerIsControllable )
	{
		std::string controlModeString = "Actor";
		std::string controlModeText = Stringf( "[F1] Control Mode: %s", controlModeString.c_str() );
		DebugAddScreenText( controlModeText, controlModePosition, cellHeight, controlModeAlignment, duration, Rgba8::SKYBLUE );
	}

	// Render DRS UI text
	DebugAddScreenText( timeText, timePosition, cellHeight, timeAlignment, duration );

	//----------------------------------------------------------------------------------------------------------------------
	// Lighting debug messages
	if ( m_showLightingDebug )
	{
		// Sun Direction X
		std::string sunDirXText	= Stringf( "Sun direction x: %.2f   ( F2 / F3 to change )", m_currentMap->m_sunDirection.x );
		Vec2 sunDirPosX			= Vec2( 0.0f, SCREEN_SIZE_Y );
		cellHeight				= 2.0f;
		Vec2 sunDirAlignment	= Vec2( 0.0f, 1.0f );
		DebugAddScreenText( sunDirXText, sunDirPosX, cellHeight, sunDirAlignment, duration );

		// Sun Direction Y
		std::string sunDirYText	= Stringf( "Sun direction y: %.2f   ( F4 / F5 to change )", m_currentMap->m_sunDirection.y );
		Vec2 sunDirPosY			= Vec2( 0.0f, SCREEN_SIZE_Y * 0.98f );
		DebugAddScreenText( sunDirYText, sunDirPosY, cellHeight, sunDirAlignment, duration );

		// Sun Intensity
		std::string sunIntensityText	= Stringf( "Sun intensity: %.2f     ( F6 / F7 to change )", m_currentMap->m_sunIntensity );
		Vec2		sunIntensityPos		= Vec2( 0.0f, SCREEN_SIZE_Y * 0.96f );
		DebugAddScreenText( sunIntensityText, sunIntensityPos, cellHeight, sunDirAlignment, duration );

		// Ambient Intensity
		std::string ambientIntensityText	= Stringf( "Ambient Intensity: %.2f ( F8 / F9 to change )", m_currentMap->m_ambientIntensity );
		Vec2		ambientIntensityPos		= Vec2( 0.0f, SCREEN_SIZE_Y * 0.94f );
		DebugAddScreenText( ambientIntensityText, ambientIntensityPos, cellHeight, sunDirAlignment, duration );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render player currentHealth if actor is alive
//	Actor* playerActor = m_player->GetActor();
//	if ( playerActor != nullptr )
//	{
//		std::string playerHealthText	= std::to_string( playerActor->m_currentHealth );
//		Vec2 playerHealthPos			= Vec2( SCREEN_CENTER_X, (SCREEN_SIZE_Y * 0.1f) );
//		cellHeight						= 9.0f;
//		Vec2 playerAlignment			= Vec2( 0.5f, 0.5f );	
//		playerHealthText				= Stringf( "%.f", playerActor->m_currentHealth  );
//		DebugAddScreenText( playerHealthText, playerHealthPos, cellHeight, playerAlignment, duration );
//		return;
//	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render player currentHealth if actor is dead
//	std::string playerHealthText	= "0.0f";
//	Vec2 playerHealthPos			= Vec2( SCREEN_CENTER_X, ( SCREEN_SIZE_Y * 0.1f ) );
//	cellHeight						= 9.0f;
//	Vec2 playerAlignment			= Vec2( 0.5f, 0.5f );
//	DebugAddScreenText( playerHealthText, playerHealthPos, cellHeight, playerAlignment, duration );

//	//----------------------------------------------------------------------------------------------------------------------
//	// Get or Create font
//	BitmapFont* bitmapFont	= nullptr;
//	bitmapFont				= g_theRenderer->CreateOrGetBitmapFontFromFile( std::string( "Data/Fonts/SquirrelFixedFont" ).c_str() );
//
//	std::vector<Vertex_PCU> verts;
//	Vec2 healthMins				= Vec2::ZERO; 
//	Vec2 healthMaxs				= Vec2( SCREEN_CENTER_X, SCREEN_CENTER_Y );
//	AABB2 healthBounds			= AABB2( healthMins, healthMaxs );
//	cellHeight					= 10.0f;
//	std::string healthString	= std::to_string( playerActor->m_currentHealth );
//	bitmapFont->AddVertsForTextInBox2D( verts, healthBounds, cellHeight, healthString );
}