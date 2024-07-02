#include "Game/Game.hpp"
#include "Game/App.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Input/XboxController.hpp"

Game::Game()
{
}

Game::~Game()
{
}

void Game::StartUp()
{
	if (m_AttractModeIsOn)
	{
		InitializeAttractModeVerts();
	}

	//create new Playership
	m_playerShip = new PlayerShip(this, m_shipSpawnPosition);

	// Create new Asteroids
	SpawnAsteroids();

	//create new Bullets
	for (int i = 0; i < Max_Bullets; i++)
	{
		m_bullets[i] = new Bullet(this, m_bulletSpawnPosition);
	}
}

void Game::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Update()
{
	if (m_AttractModeIsOn)
	{
		// updates world and screen camera
		UpdateCameras();

		m_attractModeShipOrientation += 0.15f ;

		// Input from attractModeInput()
		AttractModeInput();
		return;
	}

	UpdatePauseAndSlowMo();
	UpdateEntities();

	UpdateCameras();
}

// Game Render Function
//---------------------------------------------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	Rgba8 clearColor( 0, 0, 0, 255 );
	g_theRenderer->ClearScreen( clearColor );

	// Draw attract mode
	if (m_AttractModeIsOn)
	{
		// uses screenCam and draws relevant stuff
		RenderAttractMode();
		return;
	}

	// Draw everything in world space
	g_theRenderer->BeginCamera(m_worldCamera);

	// if play button is pressed, call RenderEntities()
	RenderEntities();
	RenderDebug();

	g_theRenderer->EndCamera(m_worldCamera);

	// Draw everything in screen space
	g_theRenderer->BeginCamera(m_screenCamera);

	// ... draw UI, etc
	g_theRenderer->EndCamera(m_screenCamera);
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 Game::SpawnBullet(Vec2 tipOfShip, float orientationDegrees)
{
	//return Vec2 name ( Vec2( 0.0f, 0.0f ), 0.0f);
	return Vec2( ( m_playerShip->m_position.x, m_playerShip->m_position.y ), orientationDegrees );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::SpawnAsteroids()
{
	Vec2 randAstPositions[Num_Max_Asteriods] = {};

	for ( int randPos = 0; randPos < Num_Max_Asteriods; randPos++)
	{ 
		randAstPositions[randPos].x = g_theRNG->RollRandomFloatInRange( 10.f, 190.f );  // #ToDo hard values
		randAstPositions[randPos].y = g_theRNG->RollRandomFloatInRange( 10.f, 90.f );	// #ToDo remove hard values

		randNumX = randAstPositions[randPos].x;
		randNumY = randAstPositions[randPos].y;

		m_AstSpawnPosition = Vec2( randAstPositions[randPos].x , randAstPositions[randPos].y );
		m_asteroids[randPos] = new Asteroid( this, m_AstSpawnPosition );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateEntities()
{
	float deltaSeconds = m_gameClock.GetDeltaSeconds();

	m_playerShip->Update(deltaSeconds);

	// #ToDo, update asteroids
	for (int i = 0; i < Num_Max_Asteriods; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			m_asteroids[i]->Update(deltaSeconds);
		}
	}

	// #ToDo, update bullets
	for (int i = 0; i < Max_Bullets; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			m_bullets[i]->Update(deltaSeconds);
		}
	}
}
 
void Game::AttractModeInput()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->HandleQuitRequested();
	}
	
	XboxController const& controller = g_theInput->GetController(0);
	
	if ( g_theInput->WasKeyJustPressed( KEYCODE_SPACE_BAR ) || g_theInput->WasKeyJustPressed('N') || controller.WasButtonJustPressed(BUTTON_START) || controller.WasButtonJustPressed(BUTTON_A))
	{
		SoundID testSound = g_theAudio->CreateOrGetSound( "Data/Audio/Pause.mp3" );
		g_theAudio->StartSound( testSound );

		m_AttractModeIsOn = false;
	}
}

void Game::UpdatePauseAndSlowMo()
{
	if (g_theInput->WasKeyJustPressed('P') || g_theInput->GetController(0).WasButtonJustPressed( XboxButtonID::BUTTON_B))
	{

		SoundID testSound = g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
		g_theAudio->StartSound(testSound);

		m_gameClock.TogglePause();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Slow mo
	if ( g_theInput->WasKeyJustPressed( 'T' ) )
	{
		m_isSlowMo = !m_isSlowMo;
		if ( m_isSlowMo )
		{
			m_gameClock.SetTimeScale( 0.1f );
		}
		if ( !m_isSlowMo )
		{
			m_gameClock.SetTimeScale( 1.0f );
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Step one frame
	if ( g_theInput->WasKeyJustPressed( 'O' ) )
	{
		m_gameClock.StepSingleFrame();
	}

	//----------------------------------------------------------------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) && m_AttractModeIsOn == false )
	{
		SoundID testSound = g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
		g_theAudio->StartSound( testSound );

		m_AttractModeIsOn = true;
	}
}

void Game::RenderEntities() const
{
	// Render Playership
	m_playerShip->Render();

	// Render Asteroids
	for (int i = 0; i < Num_Starting_Asteriods; i++)
	{
		if (m_asteroids[i] != nullptr)
		{
			m_asteroids[i]->Render();
		}
	}

	// Render Bullets
	for (int i = 0; i < Max_Bullets; i++)
	{
		if (m_bullets[i] != nullptr)
		{
			m_bullets[i]->Render();
		}
	}
}

void Game::UpdateCameras()
{
	m_worldCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(200.0f, 100.0f));
	m_screenCamera.SetOrthoView(Vec2( 0.0f, 0.0f ), Vec2( 1600.0f, 800.0f ));
}


//----------------------------------------------------------------------------------------------------------------------
void Game::RenderDebug() const
{
	if ( isDebugDisplayOn )
	{
		Rgba8 cyan		= Rgba8( 0, 255, 255, 255 );
		Rgba8 magenta	= Rgba8( 255, 0, 255, 255 );
		Rgba8 blue		= Rgba8( 0, 0, 255, 255 );
		Rgba8 red		= Rgba8( 255, 0, 0, 255 );
		Rgba8 green		= Rgba8( 0, 255, 0, 255 );

		Vec2 playerPos				= m_playerShip->m_position;
		float& physicsRadiusLength	= m_playerShip->m_physicsRadius;
		float& cosmeticRadiusLength = m_playerShip->m_cosmeticRadius;

		Vec2 playerShipFowardOrientation	= m_playerShip->GetFowardNormal();
		Vec2 PlayerShipLeftOrientation		= playerShipFowardOrientation.GetRotated90Degrees();

		// tank forward normal
		Vec2 fowardNormal = playerPos + ( playerShipFowardOrientation * cosmeticRadiusLength );

		// tank left normal
		Vec2 leftNormal = playerPos + ( PlayerShipLeftOrientation * cosmeticRadiusLength );

		std::vector<Vertex_PCU> verts;
	
		// tank forward normal
		AddVertsForLineSegment2D( verts, playerPos, fowardNormal, 1.5f, red );
		// tank left normal
		AddVertsForLineSegment2D( verts, playerPos, leftNormal, 1.5f, green );
		// physics ring
		AddVertsForRing2D( verts, playerPos, physicsRadiusLength, 0.2f, cyan );
		// cosmetic ring
		AddVertsForRing2D( verts, playerPos, cosmeticRadiusLength, 0.2f, magenta );

		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->DrawVertexArray( (int)verts.size(), verts.data() );
	}
}

void Game::RenderAttractMode() const
{
	// Draw everything in screen space
	g_theRenderer->BeginCamera(m_screenCamera);

	Vertex_PCU tempWorldVerts[NUM_PLAYERSHIP_VERTS];
	for (int index = 0; index < NUM_PLAYERSHIP_VERTS; index++)
	{
		tempWorldVerts[index] = m_localVerts[index];
	}

//	float rotatingShipOrientation = g_theRNG->RollRandomFloatInRange( 1.0f, 20.0f );

	// Transform/Render attract mode stuff 
	// Size scaled by * 80. Ratio derrived = 10 * (screenCam size / worldCam size) = ( 1600/200 = 8, 800/100 = 8 ) 
	TransformVertexArrayXY3D( NUM_PLAYERSHIP_VERTS, tempWorldVerts, 80.0f, m_attractModeShipOrientation, Vec2( SCREEN_CENTER_X, SCREEN_CENTER_Y ));
	g_theRenderer->DrawVertexArray( 15, tempWorldVerts );

	// ... draw UI, etc
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::InitializeAttractModeVerts()
{
	// Positions
	m_localVerts[0].m_position = Vec3(-2, 1, 0);		// Triangle A, point A
	m_localVerts[1].m_position = Vec3(2, 1, 0);			// Triangle A, point B
	m_localVerts[2].m_position = Vec3(0, 2, 0);			// Triangle A, point C

	m_localVerts[3].m_position = Vec3(-2, 1, 0);		// Triangle B, point A
	m_localVerts[4].m_position = Vec3(-2, -1, 0);		// Triangle B, point B
	m_localVerts[5].m_position = Vec3(0, 1, 0);			// Triangle B, point C

	m_localVerts[6].m_position = Vec3(-2, -1, 0);		// Triangle C, point A
	m_localVerts[7].m_position = Vec3(0, -1, 0);		// Triangle C, point B
	m_localVerts[8].m_position = Vec3(0, 1, 0);			// Triangle C, point C

	m_localVerts[9].m_position = Vec3(0, 1, 0);			// Triangle D, point A
	m_localVerts[10].m_position = Vec3(0, -1, 0);		// Triangle D, point B
	m_localVerts[11].m_position = Vec3(1, 0, 0);		// Triangle D, point C

	m_localVerts[12].m_position = Vec3(-2, -1, 0);		// Triangle E, point A
	m_localVerts[13].m_position = Vec3(0, -2, 0);		// Triangle E, point B
	m_localVerts[14].m_position = Vec3(2, -1, 0);		// Triangle E, point C

	//Color
	m_localVerts[0].m_color = Rgba8(255, 0, 0, 255);	// Triangle A, point A
	m_localVerts[1].m_color = Rgba8(255, 0, 0, 255);	// Triangle A, point B
	m_localVerts[2].m_color = Rgba8(255, 0, 0, 255);	// Triangle A, point C

	m_localVerts[3].m_color = Rgba8(255, 0, 0, 255);	// Triangle B, point A
	m_localVerts[4].m_color = Rgba8(255, 0, 0, 255);	// Triangle B, point B
	m_localVerts[5].m_color = Rgba8(255, 0, 0, 255);	// Triangle B, point C

	m_localVerts[6].m_color = Rgba8(255, 0, 0, 255);	// Triangle C, point A
	m_localVerts[7].m_color = Rgba8(255, 0, 0, 255);	// Triangle C, point B
	m_localVerts[8].m_color = Rgba8(255, 0, 0, 255);	// Triangle C, point C

	m_localVerts[9].m_color = Rgba8(255, 0, 0, 255);	// Triangle D, point A
	m_localVerts[10].m_color = Rgba8(255, 0, 0, 255);	// Triangle D, point B
	m_localVerts[11].m_color = Rgba8(255, 0, 0, 255);	// Triangle D, point C

	m_localVerts[12].m_color = Rgba8(255, 0, 0, 255);	// Triangle E, point A
	m_localVerts[13].m_color = Rgba8(255, 0, 0, 255);	// Triangle E, point B
	m_localVerts[14].m_color = Rgba8(255, 0, 0, 255);	// Triangle E, point C
}