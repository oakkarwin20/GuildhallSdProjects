#include "Game/GameMode2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameMode2D::GameMode2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameMode2D::~GameMode2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	UpdatePauseQuitAndSlowMo();
	UpdateGameMode2DCamera();
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::UpdateGameMode2DCamera()
{
	m_gameModeWorldCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
	   m_gameModeUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	g_theRenderer->BeginCamera( m_gameModeWorldCamera );

	// Initialize common variables
	std::vector<Vertex_PCU> verts;
	
	//----------------------------------------------------------------------------------------------------------------------
	// "GameMode2D" title
	AABB2 textBox = AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
	textBox.SetCenter( Vec2( WORLD_CENTER_X, WORLD_SIZE_Y * 0.75f ) );
	g_theApp->m_textFont->AddVertsForTextInBox2D( verts, textBox, 15.0f, "ProtoMode2D!", Rgba8::RED );

	//----------------------------------------------------------------------------------------------------------------------
	// End world camera
	g_theRenderer->EndCamera( m_gameModeWorldCamera );

	// Draw for world objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );
	g_theRenderer->BindTexture( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode2D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_gameModeUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	float cellHeight	= 2.0f;
	AABB2 textbox1		= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, "Mode (F6/F7 for prev/next): GameMode (2D)", Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	// Initialize and set UI variables
	float fps				= 1.0f / g_theApp->m_gameClock.GetDeltaSeconds();
	float scale				= g_theApp->m_gameClock.GetTimeScale();
	std::string timeText	= Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.", g_theApp->m_gameClock.GetTotalSeconds(), fps, scale );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, timeText, Rgba8::YELLOW, 0.75f, Vec2( 0.98f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_gameModeUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw for UI camera
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}
