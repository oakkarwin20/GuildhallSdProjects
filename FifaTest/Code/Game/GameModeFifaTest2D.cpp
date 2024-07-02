#include "Game/GameModeFifaTest2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Football2D.hpp"
#include "Game/Player2D.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeFifaTest2D::GameModeFifaTest2D()
{
	// Create Football and set position in game
	m_football						= new Football2D();
	m_football->m_footballPosition	= Vec2( WORLD_CENTER_X, WORLD_CENTER_Y );

	// Create Football and set position in game
	m_player						= new Player2D( m_football );
	m_player->m_playerPosition		= Vec2( WORLD_CENTER_X - 20.0f, WORLD_CENTER_Y );
	m_player->m_playerOrientation	= 0.0f;
}

//----------------------------------------------------------------------------------------------------------------------
GameModeFifaTest2D::~GameModeFifaTest2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest2D::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest2D::Update( float deltaSeconds )
{
	UpdatePauseQuitAndSlowMo();
	UpdateFifaTest2DCamera();
	UpdatePlayerPhysics( deltaSeconds );
	UpdateFootballPhysics( deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest2D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest2D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest2D::UpdateFifaTest2DCamera()
{
	m_fifaTestWorldCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
	   m_fifaTestUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest2D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	g_theRenderer->BeginCamera( m_fifaTestWorldCamera );

	// Initialize common variables
	std::vector<Vertex_PCU> verts;
	Rgba8 fieldBoundaryColor	= Rgba8::WHITE; 
	Rgba8 pitchColor			= Rgba8::DARK_GREEN; 
	float radius				= 10.0f; 
	float thickness				= 1.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Render Field quad
	AABB2 fieldBounds = AABB2( Vec2::ZERO, Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
	fieldBounds.AddPadding( WORLD_SIZE_X * 0.01f,WORLD_SIZE_Y * 0.05f );
	AddVertsForAABB2D( verts, fieldBounds, pitchColor );

	//----------------------------------------------------------------------------------------------------------------------
	// Render white field boundary lines
	float offsetX	= 5.0f;
	float offsetY	= 5.0f;
	Vec2 field_BL	= Vec2( fieldBounds.m_mins.x + offsetX, fieldBounds.m_mins.y + offsetY );		// Bottom Left
	Vec2 field_BR	= Vec2( fieldBounds.m_maxs.x - offsetX, fieldBounds.m_mins.y + offsetY );		// Bottom Right
	Vec2 field_TL	= Vec2( fieldBounds.m_mins.x + offsetX, fieldBounds.m_maxs.y - offsetY );		// Top Left
	Vec2 field_TR	= Vec2( fieldBounds.m_maxs.x - offsetX, fieldBounds.m_maxs.y - offsetY );		// Top Right
	Vec2 field_BM	= field_BL + ( (field_BR - field_BL) * 0.5f );									// Bottom Middle
	Vec2 field_TM	= field_TL + ( (field_TR - field_TL) * 0.5f );									// Top Middle

	AddVertsForLineSegment2D( verts, field_BL, field_BR, thickness, fieldBoundaryColor );		// BL to BR
	AddVertsForLineSegment2D( verts, field_TL, field_TR, thickness, fieldBoundaryColor );		// TL to TR
	AddVertsForLineSegment2D( verts, field_BL, field_TL, thickness, fieldBoundaryColor );		// BL to TL
	AddVertsForLineSegment2D( verts, field_BR, field_TR, thickness, fieldBoundaryColor );		// BR to TR
	AddVertsForLineSegment2D( verts, field_BR, field_TR, thickness, fieldBoundaryColor );		// BR to TR
	AddVertsForLineSegment2D( verts, field_BM, field_TM, thickness, fieldBoundaryColor );		// BM to TM

	//----------------------------------------------------------------------------------------------------------------------
	// Render Ring in the middle
	AddVertsForRing2D( verts, Vec2( WORLD_CENTER_X, WORLD_CENTER_Y ), radius, thickness, fieldBoundaryColor );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Penalty Box
//	AddVertsForLineSegment2D( verts, )

	//----------------------------------------------------------------------------------------------------------------------
	// Render Goals
	// Left Goal
	float goalBoxX				= 5.0f;
	float goalBoxY				= 20.0f;
	AABB2 goalBoxBounds			= AABB2( field_BL, Vec2( field_BL.x + goalBoxX, field_BL.y + goalBoxY) );
	Vec2  leftGoalCenterPoint	= field_BL + ( (field_TL - field_BL) * 0.5f );
	goalBoxBounds.SetCenter( Vec2( leftGoalCenterPoint.x - (goalBoxX * 0.5f), leftGoalCenterPoint.y ) );
	AddVertsForAABB2D( verts, goalBoxBounds, fieldBoundaryColor );

	// Right Goal
	Vec2 rightGoalCenterPoint	= field_BR + ( (field_TR - field_BR) * 0.5f );
	goalBoxBounds.SetCenter( Vec2( rightGoalCenterPoint.x + (goalBoxX * 0.5f), rightGoalCenterPoint.y ) );
	AddVertsForAABB2D( verts, goalBoxBounds, fieldBoundaryColor );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Football
	m_football->Render( verts );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Player
	m_player->Render( verts );

	//----------------------------------------------------------------------------------------------------------------------
	// End world camera
	g_theRenderer->EndCamera( m_fifaTestWorldCamera );

	// Draw for world objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest2D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_fifaTestUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	float cellHeight	= 2.0f;
	AABB2 textbox1		= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, "Mode (F6/F7 for prev/next): FIFA Test (2D)", Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_fifaTestUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw for UI camera
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest2D::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if ( g_theInput->WasKeyJustPressed( 'P' ) || g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::BUTTON_START ) )
	{
		SoundID testSound = g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
		g_theAudio->StartSound( testSound );			// Comment out this line of code to remove pause sound playing

		m_gameClock.TogglePause();
	}

	// Slow-Mo functionality
	m_isSlowMo = g_theInput->IsKeyDown( 'T' );
	if ( m_isSlowMo )
	{
		m_gameClock.SetTimeScale( 0.1f );
	}
	if ( g_theInput->WasKeyJustReleased( 'T' ) )
	{
		m_gameClock.SetTimeScale( 1.0f );
	}

	// Fast-Mo functionality
	m_isSlowMo = g_theInput->IsKeyDown( 'Y' );
	if ( m_isSlowMo )
	{
		m_gameClock.SetTimeScale( 2.0f );
	}
	if ( g_theInput->WasKeyJustReleased( 'Y' ) )
	{
		m_gameClock.SetTimeScale( 1.0f );
	}

	// Step one frame
	if ( g_theInput->WasKeyJustPressed( 'O' ) )
	{
		m_gameClock.StepSingleFrame();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest2D::UpdatePlayerPhysics( float deltaSeconds )
{
	m_player->Update( deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest2D::UpdateFootballPhysics( float deltaSeconds )
{
	// Push ball out of player if overlapping
	bool discsOverlap = DoDiscsOverlap( m_football->m_footballPosition, m_football->m_footballRadius, m_player->m_playerPosition, m_player->m_playerPhysicsRadius );
	if ( discsOverlap )
	{
		PushDiscOutOfFixedDisc2D( m_football->m_footballPosition, m_football->m_footballRadius, m_player->m_playerPosition, m_player->m_playerPhysicsRadius );
		m_player->m_ballIsPossessed = true;
	}
	else
	{
		m_player->m_ballIsPossessed = false;
	}

	bool ballIsInPossessionRange = DoDiscsOverlap( m_football->m_footballPosition, m_football->m_footballRadius, m_player->m_playerPosition, m_player->m_playerPossessionRadius );
	if ( ballIsInPossessionRange )
	{
		m_player->m_ballIsInPossessionRange = true;
	}
	else
	{
		m_player->m_ballIsInPossessionRange = false;
	}

	m_football->Update( deltaSeconds );
}
