#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EngineCommon.hpp"

//----------------------------------------------------------------------------------------------------------------------
Game::Game()
{
}

//----------------------------------------------------------------------------------------------------------------------
Game::~Game()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Game::StartUp()
{
	if (m_AttractModeIsOn)
	{
		// Get main menu visuals verts
		InitializeAttractModeVerts();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	if (m_AttractModeIsOn)
	{
		// updates world and screen camera
		UpdateCameras();

		// Take input from attractModeInput()
		AttractModeInput();
		return;
	}

	UpdatePauseQuitAndSlowMo( deltaSeconds );
	UpdateReturnToAttractMode();
	UpdateEntities( deltaSeconds );

	UpdateCameras();
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	// clear screen
	Rgba8 magentaColor( 255, 0, 0, 255 );
	g_theRenderer->ClearScreen( magentaColor );

	// Draw attract mode
	if (m_AttractModeIsOn)
	{
		// uses attractModeCam and draws relevant stuff
		RenderAttractMode();
		return;
	}

	// Draw everything in world space
	g_theRenderer->BeginCamera(m_worldCamera);
	// Since attractModeisOn = false, RenderEntities()
	RenderEntities();
	// End m_worldCam
	g_theRenderer->EndCamera(m_worldCamera);

	// Call m_screenCam and draw UI, etc
	g_theRenderer->BeginCamera(m_screenCamera);
	// End attractModeCam
	g_theRenderer->EndCamera(m_screenCamera);

	// Call attractModeCam, draw everything in attractMode space
	g_theRenderer->BeginCamera(m_attractCamera);
	// End attractModeCam
	g_theRenderer->EndCamera(m_attractCamera);
}
 
//----------------------------------------------------------------------------------------------------------------------
void Game::UpdatePauseQuitAndSlowMo(float& deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed('P') || g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::BUTTON_START))
	{
		SoundID testSound = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
		g_theAudio->StartSound(testSound);

		m_isPaused = !m_isPaused;
	}

	m_isSlowMo = g_theInput->IsKeyDown('T');

	if (m_isPaused)
	{
		deltaSeconds = 0.0f;
	}

	if (m_isSlowMo)
	{
		deltaSeconds *= 0.1f;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateReturnToAttractMode()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) && m_AttractModeIsOn == false)
	{
		m_AttractModeIsOn = true;
	}
}
  
//----------------------------------------------------------------------------------------------------------------------
void Game::AttractModeInput()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->HandleQuitRequested();
	}
	
	XboxController const& controller = g_theInput->GetController(0);
	
	if ( g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N') || controller.WasButtonJustPressed(BUTTON_START) || controller.WasButtonJustPressed(BUTTON_A))
	{
		m_AttractModeIsOn = false;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateEntities(float deltaSeconds)
{
	UNUSED( deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderEntities() const
{
	Vertex_PCU tempWorldVerts[NUM_SINGLE_TRI_VERTS];
	for (int i = 0; i < NUM_SINGLE_TRI_VERTS; i++)
	{
		tempWorldVerts[i] = m_localVerts[i];
	}

	TransformVertexArryXY3D( NUM_SINGLE_TRI_VERTS, tempWorldVerts, 10.0f, 90.0f, Vec2( WORLD_CENTER_X, WORLD_CENTER_Y ));
	g_theRenderer->DrawVertexArray( NUM_SINGLE_TRI_VERTS, tempWorldVerts );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	  m_worldCamera.SetOrthoView(Vec2( 0.0f, 0.0f ), Vec2(	200.0f, 100.0f ) );
	 m_screenCamera.SetOrthoView(Vec2( 0.0f, 0.0f ), Vec2(	200.0f, 100.0f ) );
	m_attractCamera.SetOrthoView(Vec2( 0.0f, 0.0f ), Vec2( 1600.0f, 800.0f ) );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const
{
	// Draw everything in screen space
	g_theRenderer->BeginCamera(m_attractCamera);

	//----------------------------------------------------------------------------------------------------------------------
	// Drawing triangle in attractMode
	Vertex_PCU tempAttractModeVerts[NUM_SINGLE_TRI_VERTS];
	for (int i = 0; i < NUM_SINGLE_TRI_VERTS; i++)
	{
		tempAttractModeVerts[i] = m_localVerts[i];
	}

	TransformVertexArryXY3D(NUM_SINGLE_TRI_VERTS, tempAttractModeVerts, 80.0f, 0.0f, Vec2(ATTRACT_MODE_CENTER_X, ATTRACT_MODE_CENTER_Y));
	g_theRenderer->DrawVertexArray(NUM_SINGLE_TRI_VERTS, tempAttractModeVerts);

	//----------------------------------------------------------------------------------------------------------------------
	// Drawing "first triangle"
	// #Question // How are we setting the worldPos of the triangle?
	Vertex_PCU vertices[] =
	{
		Vertex_PCU( Vec3(  400.0f, 200.0f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
		Vertex_PCU( Vec3(  800.0f, 600.0f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
		Vertex_PCU( Vec3( 1200.0f, 200.0f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
	};

	g_theRenderer->DrawVertexArray( 3, vertices );

	// ... draw UI, etc
	g_theRenderer->EndCamera(m_attractCamera);
}

//----------------------------------------------------------------------------------------------------------------------
void Game::InitializeAttractModeVerts()
{
	m_localVerts[0].m_position = Vec3(1.0f, 0.0f, 0.0f);		// Triangle A, position of vert A
	m_localVerts[1].m_position = Vec3(-1.0f, 1.0f, 0.0f);		// Triangle A, position of vert B
	m_localVerts[2].m_position = Vec3(-1.0f, -1.0f, 0.0f);		// Triangle A, position of vert C

	m_localVerts[0].m_color = Rgba8(255, 0, 0, 155);			// Triangle A, color of vert A
	m_localVerts[1].m_color = Rgba8(255, 0, 0, 155);			// Triangle A, color of vert B
	m_localVerts[2].m_color = Rgba8(255, 0, 0, 155);			// Triangle A, color of vert C
}