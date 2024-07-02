#include "Game/TileDefinition.hpp"
#include "Game/Bullet.hpp"
#include "Game/Scorpio.hpp"
#include "Game/Aries.hpp"
#include "Game/Leo.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Map.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/Clock.hpp"
 
//----------------------------------------------------------------------------------------------------------------------
Game::Game()
{
	m_attractModeTestTexture	= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	m_attractModeTestSprite		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" );
	m_attractScreenTexture		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/AttractScreen.png" );
	m_victoryScreenTexture		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/VictoryScreen.jpg" );
	m_youDiedScreenTexture		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/YouDiedScreen.png" );
	m_enemyAriesTexture			= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/EnemyAries.png" );

	// test code 
	m_animTexture				= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TestSpriteSheet_8x2.png" );
}

//----------------------------------------------------------------------------------------------------------------------
Game::~Game()
{
}
 
//----------------------------------------------------------------------------------------------------------------------
void Game::Startup()
{
	std::string TestString("ABC,DEF,GHI");
	Strings choppedStrings = SplitStringOnDelimiter( TestString, ',' );

	InitializePauseScreenVerts();
	InitializeTileDefs();

	// #ToDo: remove worldSize constants( each map has different sizes)
	m_currentMap = new Map( this, IntVec2( 16, 25 ) );

	m_playerTank = dynamic_cast<PlayerTank*>( m_currentMap->SpawnNewEntityOfType( ENTITY_TYPE_GOOD_PLAYER, m_shipSpawnPosition, 0.0f ) );
	m_currentMap->SpawnNewEntityOfType( ENTITY_TYPE_EVIL_SCORPIO, Vec2( 3.5f, 6.5f ) , 0.0f );
	m_currentMap->SpawnNewEntityOfType( ENTITY_TYPE_EVIL_ARIES, Vec2( 6.5f, 5.5f ) , 0.0f );
	m_currentMap->SpawnNewEntityOfType( ENTITY_TYPE_EVIL_LEO, Vec2( 6.5f, 3.5f ) , 0.0f );

//	m_currentMap = m_maps[0];

	attractMusic	= g_theAudio->CreateOrGetSound( "Data/Audio/AttractMusic.mp3"	);
	gameplayMusic	= g_theAudio->CreateOrGetSound( "Data/Audio/GameplayMusic.mp3"	);
	pauseSound		= g_theAudio->CreateOrGetSound( "Data/Audio/Pause.mp3"			);

	SpriteSheet spriteSheet( *m_attractModeTestSprite, IntVec2( 8, 8 ) );

	SpriteAnimDefinition sprite( spriteSheet, 8, 10, 20.0f, SpriteAnimPlaybackType::LOOP );
	sprite.GetSpriteDefAtTime( 0.99f );

	m_screenCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( SCREEN_SIZE_X, SCREEN_SIZE_Y ) );
	m_attractCamera.SetOrthoView(Vec2( 0.0f, 0.0f ), Vec2( ATTRACT_MODE_SIZE_X, ATTRACT_MODE_SIZE_Y ));
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	if ( g_theAudio != nullptr )
	{
		g_theAudio->StopSound(m_gameMusic);
	}

	delete m_currentMap;
	m_currentMap = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	UpdateAttractMode();
	UpdatePauseQuitAndSlowMo( deltaSeconds );
	UpdateReturnToAttractMode();
	if ( !m_AttractModeIsOn )
	{
		UpdateCameras();
		m_currentMap->Update(deltaSeconds);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Clear screen to BLUE
	g_theRenderer->ClearScreen( Rgba8( 0, 0, 150 ));

	// Draw attract mode
	if (m_AttractModeIsOn)
	{
		// uses attractModeCam and draws relevant stuff
		RenderAttractMode();
		return;
	}
	else if (m_AttractModeIsOn == false && m_isPaused == true)
	{
		g_theRenderer->BeginCamera(m_screenCamera);
		g_theRenderer->EndCamera(m_screenCamera);
	}

	// Draw everything in world space
	g_theRenderer->BeginCamera(m_worldCamera);
	// Since attractModeisOn = false, RenderEntities()
	m_currentMap->Render();
	// DebugDrawFont();	// #ToDo remove this, for testing purposes only
	// End m_worldCam
	g_theRenderer->EndCamera(m_worldCamera);

	// Call UI_Camera and draw UI, etc
	g_theRenderer->BeginCamera(m_screenCamera);
	RenderPauseScreen();
	// End UI_Camera 
	g_theRenderer->EndCamera(m_screenCamera);
}
 
//----------------------------------------------------------------------------------------------------------------------
void Game::UpdatePauseQuitAndSlowMo(float& deltaSeconds)
{	 
	if (g_theInput->WasKeyJustPressed('P') || g_theInput->GetController(0).WasButtonJustPressed( XboxButtonID::BUTTON_START ) )
	{
		m_pauseSound = g_theAudio->StartSound(pauseSound);
		// m_isPaused = !m_isPaused;
		Clock::GetSystemClock().TogglePause();
		
		if ( Clock::GetSystemClock().IsPaused() )
		{
			deltaSeconds = 0.0f;
			g_theAudio->SetSoundPlaybackSpeed(m_gameMusic, 0.0f);
		}
		else if ( !Clock::GetSystemClock().IsPaused() )
		{
			g_theAudio->SetSoundPlaybackSpeed(m_gameMusic, 1.0f);
		}
	}

	if ( Clock::GetSystemClock().IsPaused() )
	{
		deltaSeconds = 0.0f;
	}

	// SlowMo functionality
	else if ( g_theInput->IsKeyDown('T') && m_AttractModeIsOn == false )
	{
		deltaSeconds *= 0.5f;
		
		m_isSlowMo = true;
		g_theAudio->SetSoundPlaybackSpeed( m_gameMusic, 0.5 );
	}
	else if ( g_theInput->WasKeyJustReleased('T') && m_AttractModeIsOn == false )
	{
		m_isPaused = false;
		g_theAudio->SetSoundPlaybackSpeed( m_gameMusic, 1.0f );
	}

	// FastMo functionality
	else if ( g_theInput->IsKeyDown('Y') && m_AttractModeIsOn == false )
	{
		deltaSeconds *= 4.0f;

		m_isFastMo = true;
		g_theAudio->SetSoundPlaybackSpeed(m_gameMusic, 4.0f );
	} 
	else if ( g_theInput->WasKeyJustReleased('Y') && m_AttractModeIsOn == false )
	{
		m_isFastMo = false;
		g_theAudio->SetSoundPlaybackSpeed(m_gameMusic, 1.0f );
	}

	// Single step
	if ( g_theInput->IsKeyDown( 'O' ) && m_AttractModeIsOn == false )
	{
		Clock::GetSystemClock().StepSingleFrame();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateReturnToAttractMode()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_ESC ) && m_AttractModeIsOn == false && !m_isPaused )
	{
		m_isPaused = true;
		g_theAudio->SetSoundPlaybackSpeed( m_gameMusic, 0.0f );
		m_pauseSound = g_theAudio->StartSound( pauseSound );
	}
	else if ( g_theInput->WasKeyJustPressed( KEYCODE_ESC ) && m_AttractModeIsOn == false )
	{
		m_AttractModeIsOn = true;
		m_shouldAttractMusicRepeat = true;
		m_shouldGameMusicRepeat = true;
		g_theAudio->StopSound( m_gameMusic );

		m_isPaused = false;

		delete m_currentMap;
//		delete m_playerTank;
		m_currentMap = nullptr;
		m_playerTank = nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateAttractModeInput()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_ESC ) )
	{
		g_theApp->HandleQuitRequested();
	}
	
	XboxController const& controllerID = g_theInput->GetController(0);
	if ( g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N') || controllerID.WasButtonJustPressed(BUTTON_START) || controllerID.WasButtonJustPressed(BUTTON_A))
	{
		m_AttractModeIsOn = false;
		if(m_currentMap == nullptr )
		{
			m_currentMap = new Map( this, IntVec2( 20, 30 ) );
			m_playerTank = dynamic_cast<PlayerTank*>( m_currentMap->SpawnNewEntityOfType( ENTITY_TYPE_GOOD_PLAYER, m_shipSpawnPosition, 0.0f ) );
			m_currentMap->SpawnNewEntityOfType( ENTITY_TYPE_EVIL_SCORPIO, Vec2( 3.5f, 6.5f ), 0.0f );
			m_currentMap->SpawnNewEntityOfType( ENTITY_TYPE_EVIL_ARIES, Vec2( 6.5f, 5.5f ), 0.0f );
			m_currentMap->SpawnNewEntityOfType( ENTITY_TYPE_EVIL_LEO, Vec2( 6.5f, 3.5f ), 0.0f );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_isDebugCamOn = !m_isDebugCamOn;
	}
	
	// Got mapBounds and worldCamBounds set to 2:1 aspect
	AABB2 mapBounds = AABB2( 0.0f, 0.0f, float( m_currentMap->m_dimensions.x ), float( m_currentMap->m_dimensions.y ) );
	float tilesVisibleOnScreenHorizontally = m_tilesVisibleOnScreenVertically * CLIENT_ASPECT;
	AABB2 worldCamBounds = AABB2( 0.0f, 0.0f, tilesVisibleOnScreenHorizontally, m_tilesVisibleOnScreenVertically );
	Vec2 playerPosition = m_currentMap->GetPlayerPosition();
	worldCamBounds.SetCenter( playerPosition );

	// clamping worldCamBounds mins and maxs
	if ( worldCamBounds.m_mins.x < mapBounds.m_mins.x )
	{
		worldCamBounds.Translate( Vec2( mapBounds.m_mins.x - worldCamBounds.m_mins.x, 0.0f ) );
	}
	if ( worldCamBounds.m_maxs.x > mapBounds.m_maxs.x ) 
	{
		worldCamBounds.Translate( Vec2( mapBounds.m_maxs.x - worldCamBounds.m_maxs.x, 0.0f ) );
	}
	if ( worldCamBounds.m_mins.y < mapBounds.m_mins.y )
	{
		worldCamBounds.Translate( Vec2( 0.0f, mapBounds.m_mins.y - worldCamBounds.m_mins.y) );
	}
	if ( worldCamBounds.m_maxs.y > mapBounds.m_maxs.y )
	{
		worldCamBounds.Translate( Vec2( 0.0f, mapBounds.m_maxs.y - worldCamBounds.m_maxs.y ) );
	}

	m_worldCamera.SetOrthoView( worldCamBounds.m_mins, worldCamBounds.m_maxs );

	if ( m_isDebugCamOn )
	{
		m_worldCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( (float)m_currentMap->m_dimensions.x * 3, (float)m_currentMap->m_dimensions.y) );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const
{
	g_theRenderer->BeginCamera(m_attractCamera);

	// Testing render 2 AABB2s
	SpriteSheet spriteSheet( *m_attractScreenTexture, IntVec2( 8, 8 ) );
	Rgba8 whiteColor = Rgba8( 255, 255, 255, 255 );

	AABB2 uvsFromSpriteSheet	= spriteSheet.GetSpriteUVs( 3 );
	AABB2 textureBounds3		= AABB2( Vec2(  20.0f, 20.0f ), Vec2( 80.0f, 80.0f ) );
	AABB2 textureBounds4		= AABB2( Vec2( 120.0f, 20.0f ), Vec2( 180.0f, 80.0f ) );

	constexpr float Q = 1.0f / 8.0f;
	constexpr float S = Q * 2.0f;
	constexpr float R = Q * 3.0f;

	std::vector<Vertex_PCU> testVerts;
	AddVertsForAABB2D( testVerts, textureBounds3, whiteColor, uvsFromSpriteSheet );
	AddVertsForAABB2D( testVerts, textureBounds4, whiteColor, AABB2( S, S, R, R ) );

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( m_attractModeTestSprite );
	g_theRenderer->DrawVertexArray( (int)testVerts.size(), testVerts.data() );

	// Testing render line, disc, and ring
	std::vector<Vertex_PCU> testVerts1;
	AddVertsForLineSegment2D( testVerts1, Vec2( 100.0f, 50.0f ), Vec2( 180.0f, 20.0f ), 1.25f, Rgba8( 255, 128, 32, 255 ) );
	AddVertsForDisc2D( testVerts1, Vec2( 50.0f, 60.0f ), 10.0f, Rgba8( 255, 128, 32, 255 ) );
	AddVertsForRing2D( testVerts1, Vec2( 50.0f, 60.0f ), 15.0f, 5.0f, Rgba8( 155, 128, 32, 255 ) );
	
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( (int)testVerts1.size(), testVerts1.data() );

//	DebugDrawFont(Vec2( 100.0f, 50.0f ));
	DebugDrawFont();
	DebugDrawAnimations();

	g_theRenderer->EndCamera(m_attractCamera);
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderPauseScreen() const
{
	if (!m_isPaused) return;

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray(NUM_PAUSE_SCREEN_TRI_VERTS, m_pauseScreenLocalVerts);
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateAttractMode()
{
	if (m_AttractModeIsOn)
	{
		if (m_shouldAttractMusicRepeat == true)
		{
			m_attractMusic = g_theAudio->StartSound( attractMusic, true );
			m_shouldAttractMusicRepeat = false;
		}
		
		g_theAudio->SetSoundPlaybackSpeed(m_attractMusic, 1.0f);

		// Updates world and screen camera
		//UpdateCameras();
		// Take input from attractModeInput()
		UpdateAttractModeInput();
		return;
	}

	if (!m_AttractModeIsOn && m_shouldGameMusicRepeat == true)
	{
		g_theAudio->SetSoundPlaybackSpeed(m_attractMusic, 0.0f);
		m_gameMusic = g_theAudio->StartSound( gameplayMusic, true );
		m_shouldGameMusicRepeat = false; 
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::InitializePauseScreenVerts()
{
	m_pauseScreenLocalVerts[0].m_position = Vec3(  0.0f,  0.0f, 0.0f );							// Triangle A, position of vert A
	m_pauseScreenLocalVerts[1].m_position = Vec3( SCREEN_SIZE_X,  0.0f, 0.0f );					// Triangle A, position of vert B
	m_pauseScreenLocalVerts[2].m_position = Vec3( SCREEN_SIZE_X, SCREEN_SIZE_Y, 0.0f );			// Triangle A, position of vert C
	
	m_pauseScreenLocalVerts[3].m_position = Vec3(  0.0f, 0.0f, 0.0f);							// Triangle A, position of vert A
	m_pauseScreenLocalVerts[4].m_position = Vec3( SCREEN_SIZE_X, SCREEN_SIZE_Y, 0.0f );			// Triangle A, position of vert B
	m_pauseScreenLocalVerts[5].m_position = Vec3( 0.0f,  SCREEN_SIZE_Y, 0.0f);					// Triangle A, position of vert C
	
	m_pauseScreenLocalVerts[0].m_color = Rgba8( 0, 0, 0, 155 );									// Triangle A, color of vert A
	m_pauseScreenLocalVerts[1].m_color = Rgba8( 0, 0, 0, 155 );									// Triangle A, color of vert B
	m_pauseScreenLocalVerts[2].m_color = Rgba8( 0, 0, 0, 155 );									// Triangle A, color of vert C
		    			   
	m_pauseScreenLocalVerts[3].m_color = Rgba8( 0, 0, 0, 155 );									// Triangle A, color of vert A
	m_pauseScreenLocalVerts[4].m_color = Rgba8( 0, 0, 0, 155 );									// Triangle A, color of vert B
	m_pauseScreenLocalVerts[5].m_color = Rgba8( 0, 0, 0, 155 );									// Triangle A, color of vert C
}

//----------------------------------------------------------------------------------------------------------------------
void Game::InitializeTileDefs()
{
	TileDefinition::s_definitions.resize( NUM_TILE_TYPES );

	TileDefinition::s_definitions[ TILE_TYPE_GRASS ].m_isSolid = false;
	TileDefinition::s_definitions[ TILE_TYPE_GRASS ].m_spriteCoords = IntVec2( 4, 0 );

	TileDefinition::s_definitions[ TILE_TYPE_STONE ].m_isSolid = true;
	TileDefinition::s_definitions[ TILE_TYPE_STONE ].m_spriteCoords = IntVec2( 2, 0 );

	TileDefinition::s_definitions[TILE_TYPE_TEST_RED].m_isSolid = true;
	TileDefinition::s_definitions[TILE_TYPE_TEST_RED].m_spriteCoords = IntVec2( 8, 8 );

	TileDefinition::s_definitions[TILE_TYPE_TEST_BLUE].m_isSolid = true;
	TileDefinition::s_definitions[TILE_TYPE_TEST_BLUE].m_spriteCoords = IntVec2( 5, 8 );
}
 
//----------------------------------------------------------------------------------------------------------------------
void Game::DebugDrawFont() const
{
	BitmapFont* g_testFont = nullptr;
	g_testFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
	
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> boxVerts;
//	g_testFont->AddVertsForText2D( textVerts, Vec2( 40.0f, 50.0f ), 10.f, "Hello, world" );
//	g_testFont->AddVertsForText2D( textVerts, Vec2( 20.0f, 20.0f ), 10.0f, "It's nice to have options!", Rgba8::RED, 0.5f );
//	g_theRenderer->BindTexture( &g_testFont->GetTexture() );
//	g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() )

//----------------------------------------------------------------------------------------------------------------------
// Text-in-box test code
	AABB2 box = AABB2( Vec2( 0.0f, 0.0f ), Vec2( 80.f, 50.0f ) );
	box.SetCenter( Vec2( ATTRACT_MODE_SIZE_X * 0.5f, ATTRACT_MODE_SIZE_Y * 0.5f ) );
	AddVertsForAABB2D( boxVerts, box, Rgba8::RED );
	g_testFont->AddVertsForTextInBox2D( textVerts, box, 4.0f, "Hello Prof Squirrel\n Press Spacebar to \n start the game", Rgba8::GREEN, 1.0f, Vec2( 0.5f, 1.0f ), TextDrawMode::OVERRUN, 999999);

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( (int)boxVerts.size(), boxVerts.data() );

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_testFont->GetTexture() );
	g_theRenderer->DrawVertexArray( (int)textVerts.size(), textVerts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::DebugDrawAnimations() const
{
	static float currentTime = -5.0f;
	currentTime += ( 1.0f / 60.0f );

	AABB2 testBounds = AABB2( 10.0f, 10.0f, 50.0f, 80.0f );
	SpriteSheet testSheet( *m_animTexture, IntVec2( 8, 2 ) );
	SpriteAnimDefinition testAnimDef( testSheet, 8, 15, 10.0f, SpriteAnimPlaybackType::ONCE );
//	SpriteDefinition const& testDefinition = testSheet.GetSpriteDef( 3 );
	SpriteDefinition const& testDefinition = testAnimDef.GetSpriteDefAtTime( currentTime );

	std::vector<Vertex_PCU> testVerts; 
	AddVertsForAABB2D( testVerts, testBounds, Rgba8::WHITE, testDefinition.GetUVs() );
	
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( m_animTexture );
	g_theRenderer->DrawVertexArray( (int)testVerts.size(), testVerts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
//void DrawTestMouseCursor( Camera const& camera )
//{
//	Vec2 normalizedMousePos = g_theWindow->GetCursorNormalizedPos);
//
//}