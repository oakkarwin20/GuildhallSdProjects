#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/Chunk.hpp"
#include "Game/Prop.hpp"
#include "Game/World.hpp"
#include "Game/BlockDef.hpp"

#include "Engine/ThirdParty/Squirrel/Noise/SmoothNoise.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
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
	m_testTexture  = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TestUV.png" );
	m_blockTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/BasicSprites_64x64.png" );

	// Initialize and Create spriteSheet
	IntVec2	simpleGridLayout = IntVec2( SPRITESHEET_GRID_LAYOUT_X, SPRITESHEET_GRID_LAYOUT_Y );			// Size of current spriteSheet being used 
	m_blockSpriteSheet		 = new SpriteSheet( *m_blockTexture, simpleGridLayout );					// Create spriteSheet

	// Create blocksDef
	BlockDef::InitializeBlockDefs();
}

//----------------------------------------------------------------------------------------------------------------------
Game::~Game()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Game::StartUp()
{
//	// Test Code
//	std::vector<unsigned char> bytes;
//	bool wasSuccessful = FileReadToBuffer(bytes, "Data/Images/BasicSprites_64x64.png");

	g_simpleMinerCBO = g_theRenderer->CreateConstantBuffer( sizeof( SimpleMinerGPUData ) );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	delete m_currentWorld;
	m_currentWorld = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Update()
{
	float deltaSeconds = m_clock.GetDeltaSeconds();

	//----------------------------------------------------------------------------------------------------------------------
	// Game States
	//----------------------------------------------------------------------------------------------------------------------

	// Set currentState correctly if requestedState has changed
	if ( m_requestedState != m_currentState )
	{
		// exit current state
		ExitState( m_currentState );
		// set current state
		m_currentState = m_requestedState;
		// enter current state
		EnterState( m_currentState );
	}

	// Update Based on Game States
	if ( m_currentState == GameState::ATTRACT )
	{
		// Update Input
		UpdateAttractModeInput();
	}
	else if ( m_currentState == GameState::PLAYING )
	{
		// Core Input
		UpdatePauseQuitAndSlowMo();
		
		// Attract Mode
		UpdateReturnToAttractMode();
		
		// Entities
		UpdateEntities( deltaSeconds );

		// World
		m_currentWorld->Update();

		CheckDebugCheats();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Cameras
	//----------------------------------------------------------------------------------------------------------------------
	UpdateCameras();
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
		//----------------------------------------------------------------------------------------------------------------------
		// Begin WorldCamera
		g_theRenderer->BeginCamera( m_player->m_worldCamera );

		// Clear screen
		Rgba8 skyColor			= Rgba8::SUNSET_ORANGE;
		double timeNow			= GetCurrentTimeSeconds();
		float sine				= SinDegrees( float(timeNow * 12.0f) );
		sine					= RangeMapClamped( sine, -1.0f, 1.0f, 0.0f, 1.0f );
//		float redScalar			= RangeMapClamped( sine, -0.2f, 0.2f, 0.0f, 255.0f );
//		skyColor.r				= redScalar;
//
//		float sine2				= SinDegrees( float(timeNow * 80.0f) );
//		float greenScalar		= RangeMapClamped( sine2, -0.2f, 0.2f, 0.0f, 255.0f );
//		skyColor.g				= greenScalar;

//		float brightnessScalar	= RangeMapClamped( sine, -1.0f, 1.0f, 0.0f, 255.0f );
//		skyColor.r				= brightnessScalar;

		Rgba8 lerpedColor = Interpolate( Rgba8::LIGHTBLUE, Rgba8::SUNSET_ORANGE, sine );
		skyColor		  = lerpedColor;
		g_theRenderer->ClearScreen( skyColor );

		// Render Entities
		RenderEntities();

		if ( m_currentWorld )
		{
			m_currentWorld->Render();
		}

		// Render DRS world Basis and text
		Render_DRS_WorldBasisText();
		Render_DRS_WorldBasis();
		// DRS
		DebugRenderWorld( m_player->m_worldCamera );

		// End m_worldCam
		g_theRenderer->EndCamera( m_player->m_worldCamera );

		// RenderUI()
		RenderUI();
	}
}
 
//----------------------------------------------------------------------------------------------------------------------
void Game::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if (g_theInput->WasKeyJustPressed('P') || g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::BUTTON_START))
	{
		SoundID testSound = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
		g_theAudio->StartSound(testSound);			// Comment out this line of code to remove pause sound playing

		m_clock.TogglePause();
	}

	// Slow-Mo functionality
	if ( g_theInput->IsKeyDown( 'T' ) )
	{
		m_clock.SetTimeScale( 50.0f );
	}
	else
	{
		m_clock.SetTimeScale( 1.0f );
	}
	// Fast-Mo functionality
	if ( g_theInput->IsKeyDown( 'Y' ) )
	{
		m_clock.SetTimeScale( 50.0f );
	}
	else
	{
		m_clock.SetTimeScale( 1.0f );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateReturnToAttractMode()
{
	XboxController const& controller = g_theInput->GetController( 0 );

	if ( ( g_theInput->WasKeyJustPressed( KEYCODE_ESC ) || controller.IsButtonDown( BUTTON_SELECT ) ) &&
		m_currentState == GameState::PLAYING )
	{
		// Set game state back to attractMode
		m_requestedState = GameState::ATTRACT;
	}
}
  
//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateAttractModeInput()
{
	XboxController const& controller = g_theInput->GetController( 0 );

	// Quit Game 
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->HandleQuitRequested();
	}

	// Start game if 'SPACEBAR' was pressed
	if ( g_theInput->WasKeyJustPressed( ' ' )			 || 
		 g_theInput->WasKeyJustPressed( 'N' )			 || 
		 controller.WasButtonJustPressed( BUTTON_START ) || 
		 controller.WasButtonJustPressed( BUTTON_A )	 )
	{
		m_requestedState = GameState::PLAYING;		// Set game state back to PLAYING

		SoundID clickSound = g_theAudio->CreateOrGetSound( "Data/Audio/Click.mp3" );
		g_theAudio->StartSound( clickSound );			// Comment out this line of code to remove pause sound playing
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateEntities(float deltaSeconds)
{	
	//----------------------------------------------------------------------------------------------------------------------
	// Call Update() for each entity inside entityList
	for ( int i = 0; i < m_entityList.size(); i++ )
	{
		if ( m_entityList[i] != nullptr )
		{
			m_entityList[i]->Update( deltaSeconds );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderEntities() const
{
	for ( int j = 0; j < m_entityList.size(); j++)
	{
		if ( m_entityList[j] != nullptr )
		{
			m_entityList[j]->Render();
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::DeleteEntities()
{
	for ( int j = 0; j < m_entityList.size(); j++ )
	{
		if ( m_entityList[j] != nullptr )
		{
			delete m_entityList[j];
			m_entityList[j] = nullptr;
		}
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
void Game::Render_DRS_WorldBasisText() const
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
void Game::Render_DRS_WorldBasis() const
{
	// DebugRenderSystem World Basis
	float radius	= 0.1f;
	float duration = -1.0f;
	DebugAddWorldArrow( Vec3::ZERO, Vec3( 10.0f,  0.0f,  0.0f ), radius, duration,   Rgba8::RED, Rgba8::RED   );		// iBasisArrow
	DebugAddWorldArrow( Vec3::ZERO, Vec3(  0.0f, 10.0f,  0.0f ), radius, duration, Rgba8::GREEN, Rgba8::GREEN );		// jBasisArrow
	DebugAddWorldArrow( Vec3::ZERO, Vec3(  0.0f,  0.0f, 10.0f ), radius, duration,  Rgba8::BLUE, Rgba8::BLUE  );		// kBasisArrow
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	if ( m_currentState == GameState::ATTRACT )
	{
		UpdateAttractCamera();
	}
	else if ( m_currentState == GameState::PLAYING )
	{
		UpdateWorldCamera();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateWorldCamera()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Setting perspective view
	m_player->m_worldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 1000.0f );
	m_player->m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f ), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateAttractCamera()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Setting ortho views
	m_attractCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( ATTRACT_MODE_SIZE_X, ATTRACT_MODE_SIZE_Y ) );
	m_screenCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( SCREEN_SIZE_X, SCREEN_SIZE_Y ) );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const
{
	// Draw everything in screen space
	g_theRenderer->BeginCamera(m_attractCamera);

	//----------------------------------------------------------------------------------------------------------------------
	// Drawing "First Triangle"
	Vertex_PCU vertices[] =
	{
		Vertex_PCU( Vec3( 1200.0f, 200.0f, 0.0f ), Rgba8::WHITE, Vec2( 0.0f, 0.0f ) ),
		Vertex_PCU( Vec3(  800.0f, 600.0f, 0.0f ), Rgba8::WHITE, Vec2( 0.0f, 0.0f ) ),
		Vertex_PCU( Vec3(  400.0f, 200.0f, 0.0f ), Rgba8::WHITE, Vec2( 0.0f, 0.0f ) ),
	};

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 3, vertices );

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
		//----------------------------------------------------------------------------------------------------------------------
		// Adding player to scene
		m_player										= new Player( this );
		m_player->m_position							= Vec3( 1.0f, 0.0f, 80.0f );
//		m_player->m_angularVelocity.m_yawDegrees		= 90.0f; 
//		m_player->m_angularVelocity.m_pitchDegrees		= 15.0f;
		m_entityList.push_back( m_player );

		//----------------------------------------------------------------------------------------------------------------------
		// Create world 
		if ( m_currentWorld == nullptr )
		{
			m_currentWorld = new World();
		}
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
		// Remove player from game
//		delete m_player;
//		m_player = nullptr;
		
		DeleteEntities();
	
		// Delete and NULL world pointer		
//		delete m_currentWorld;
//		m_currentWorld = nullptr;
	}
	else if ( state == GameState::LOBBY )
	{
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Render_DRS_UI_Text() const
{
	// Initialize and set common UI variables
	float cellHeight			= 2.0f;
	float duration				= 0.0f;
	Rgba8 color					= Rgba8::WHITE;

	// Time and FPS
	Vec2 timePosition			= Vec2( SCREEN_SIZE_X, ( SCREEN_SIZE_Y ) );
	Vec2 timeAlignment			= Vec2( 1.0f, 1.0f );
	float fps					= 1.0f / m_clock.GetDeltaSeconds();
	float scale					= m_clock.GetTimeScale();
	std::string timeText		= Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.", m_clock.GetTotalSeconds(), fps, scale );
	DebugAddScreenText( timeText, timePosition, cellHeight, timeAlignment, duration, color );

	// Player position in worldBlockCoords
	Vec2 playerTextPos			= Vec2( 0.0f, 100.0f );
	Vec2 playerTextAlignment	= Vec2( 0.0f, 1.0f );
	int worldCoordsX			= RoundDownToInt( m_player->m_position.x );
	int worldCoordsY			= RoundDownToInt( m_player->m_position.y );
	int worldCoordsZ			= RoundDownToInt( m_player->m_position.z );
//	std::string playerPosText	= Stringf( "Player position: %0.2f, %0.2f, %0.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z );
	std::string playerPosText	= Stringf( "WorldBlockCoords: %d, %d, %d", worldCoordsX, worldCoordsY, worldCoordsZ );
	DebugAddScreenText( playerPosText, playerTextPos, cellHeight, playerTextAlignment, duration, color );

	// Player position in localBlockCoords
	Vec2 playerLBCTextPos				= Vec2( playerTextPos.x, playerTextPos.y - cellHeight );						// LBC = localBlockCoords
	IntVec3 localBlockCoords			= m_currentWorld->GetLocalBlockCoordsFromWorldPos( m_player->m_position );
	std::string localBlockCoordsText	= Stringf( "LocalBlockCoords: %d, %d, %d", localBlockCoords.x, localBlockCoords.y, localBlockCoords.z );
	DebugAddScreenText( localBlockCoordsText, playerLBCTextPos, cellHeight, playerTextAlignment, duration, color );

	// Player position in ChunkCoords
	Vec2 playerCCTextPos				= Vec2( playerLBCTextPos.x, playerLBCTextPos.y - cellHeight );						// CC = ChunkCoords
	IntVec2 chunkCoords					= m_currentWorld->GetChunkCoordsFromWorldPos( Vec2(m_player->m_position.x, m_player->m_position.y) );
	std::string chunkCoordsText			= Stringf( "ChunkCoords:      %d, %d", chunkCoords.x, chunkCoords.y );
	DebugAddScreenText( chunkCoordsText, playerCCTextPos, cellHeight, playerTextAlignment, duration, color );

	// Num Chunks activated
	int numChunks				= m_currentWorld->GetNumActiveChunks();
	Vec2 chunkTextPos			= Vec2( playerCCTextPos.x, (playerCCTextPos.y - (cellHeight * 2.0f)) );
	std::string chunkText		= Stringf( "Chunks: %0.2d/%0.2d", numChunks, m_currentWorld->m_maxNumChunks );
	DebugAddScreenText( chunkText, chunkTextPos, cellHeight, playerTextAlignment, duration, color );

	// Ocean noise factors at currentBlockColumn in currentChunkCoords 
	Vec2 biomeNoiseTextPos			= Vec2( chunkTextPos.x, chunkTextPos.y - cellHeight );						// CC = ChunkCoords	
	int columnIndex					= localBlockCoords.x + ( CHUNK_SIZE_X * localBlockCoords.y );
	Chunk* currentChunk				= m_currentWorld->GetChunkAtCoords( chunkCoords );
	float oceanNoiseAtThisColumn	= -99;
	int groundHeightAfterOceanInfluenceThisColumn = -99;
	if ( currentChunk != nullptr )
	{
		oceanNoiseAtThisColumn = currentChunk->m_oceanNoiseForEachColumn[columnIndex];
		groundHeightAfterOceanInfluenceThisColumn = currentChunk->m_groundPulledDownByOceanHeightForEachColumn[columnIndex];
	}
	std::string biomeNoiseText = Stringf( "Ocean height this column: %d, OceanNoise: %0.2f", groundHeightAfterOceanInfluenceThisColumn, oceanNoiseAtThisColumn );
	DebugAddScreenText( biomeNoiseText, biomeNoiseTextPos, cellHeight, playerTextAlignment, duration, color );

	// leftEndEffector position in worldBlockCoords 
	Vec2 playerCCTextPos				= Vec2( playerLBCTextPos.x, playerLBCTextPos.y - cellHeight );						// CC = ChunkCoords
	IntVec2 chunkCoords					= m_currentWorld->GetChunkCoordsFromWorldPos( Vec2(m_player->m_position.x, m_player->m_position.y) );
	std::string chunkCoordsText			= Stringf( "ChunkCoords:      %d, %d", chunkCoords.x, chunkCoords.y );
	DebugAddScreenText( chunkCoordsText, playerCCTextPos, cellHeight, playerTextAlignment, duration, color );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::CheckDebugCheats()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		g_debugDrawChunkBoundaries = !g_debugDrawChunkBoundaries;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F2 ) )
	{
		g_debugDrawLightValues = !g_debugDrawLightValues;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F3 ) )
	{
		g_debugDrawCurrentBlockIter = !g_debugDrawCurrentBlockIter;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F5 ) )
	{
		g_debugDrawCaves = !g_debugDrawCaves;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F6 ) )
	{
		g_debugUseWhiteTexture = !g_debugUseWhiteTexture;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F7 ) )
	{
		g_debugUseWorldShader = !g_debugUseWorldShader;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F9 ) )
	{
		g_debugDrawRaycast = !g_debugDrawRaycast;
	}
}
