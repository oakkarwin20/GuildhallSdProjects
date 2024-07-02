#include "Game/GameModeRaycastVsAABB2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeRaycastVsAABB2D::GameModeRaycastVsAABB2D()
{
	for ( int i = 0; i < m_NUMLines; i++ )
	{
		Vec2 center				= Vec2( g_theRNG->RollRandomFloatInRange( m_randMinXInclusive, m_randMaxXInclusive ),  g_theRNG->RollRandomFloatInRange( m_randMinYInclusive, m_randMaxYInclusive ) );	
		float width				= g_theRNG->RollRandomFloatInRange( 5.0f, 10.0f );
		float height 			= g_theRNG->RollRandomFloatInRange( 5.0f, 15.0f );
		AABB2* currentAABB2		= new AABB2( center, width, height );
		m_aabb2List.push_back( currentAABB2 );
	}

}

//----------------------------------------------------------------------------------------------------------------------
GameModeRaycastVsAABB2D::~GameModeRaycastVsAABB2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsAABB2D::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsAABB2D::Update( float deltaSeconds )
{
	deltaSeconds = m_gameClock.GetDeltaSeconds();

	UpdateGameCameraRaycastVsAABB2();
	UpdateInput( deltaSeconds );
	UpdatePauseQuitAndSlowMo();
	UpdateRaycastResult2D();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsAABB2D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsAABB2D::Reshuffle()
{
	for ( int i = 0; i < m_aabb2List.size(); i++ )
	{
		Vec2 center  = Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, 180.0f ), g_theRNG->RollRandomFloatInRange( 20.0f, 80.0f ) );
		float width  = g_theRNG->RollRandomFloatInRange( 5.0f, 25.0f );
		float height = g_theRNG->RollRandomFloatInRange( 5.0f, 40.0f );
		AABB2 randAABB2 = AABB2( center, width, height );
		m_aabb2List[i]->m_mins = randAABB2.m_mins;
		m_aabb2List[i]->m_maxs = randAABB2.m_maxs;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsAABB2D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsAABB2D::UpdateGameCameraRaycastVsAABB2()
{
	m_raycastVsAABB2DWorldCamera.SetOrthoView( Vec2(0.0f, 0.0f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y) );
	   m_raycastVsAABB2DUICamera.SetOrthoView( Vec2(0.0f, 0.0f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsAABB2D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	g_theRenderer->BeginCamera( m_raycastVsAABB2DWorldCamera );
	
	std::vector<Vertex_PCU> verts;
	verts.reserve( 300 );

	//----------------------------------------------------------------------------------------------------------------------
	// Render AABB2
	for ( int i = 0; i < m_aabb2List.size(); i++ )
	{
		AddVertsForAABB2D( verts, *m_aabb2List[i], m_aabb2Color );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render Raycast arrow
	// Ray start to end with no impact
	AddVertsForArrow2D( verts, m_rayStartPos, m_rayEndPos, m_arrowSize, m_arrowThickness, m_rayDefaultColor );

	// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
	if ( m_didAABB2Impact )
	{
		// Ray until impact	// RayStart to impactPos
		AddVertsForArrow2D( verts, m_rayStartPos, m_updatedImpactPos, m_arrowSize, m_arrowThickness, m_rayImpactDistColor );
		// Ray after impact // ImpactDist to rayEndPoint
		AddVertsForArrow2D( verts, m_updatedImpactPos, m_rayEndPos, m_arrowSize, m_arrowThickness, m_rayAfterImpactColor );
		// Disc at raycasts impactPos 
		AddVertsForDisc2D( verts, m_updatedImpactPos, m_arrowThickness, m_rayImpactDiscColor );
		// Impact normal // raycast reflection off line 
		AddVertsForArrow2D( verts, m_updatedImpactPos, m_updatedImpactPos + ( m_updatedImpactNormal * 6.0f ), m_arrowSize, m_arrowThickness, m_rayImpactNormalColor );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Draw call for World camera
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );

	// End World Camera
	g_theRenderer->EndCamera( m_raycastVsAABB2DWorldCamera );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsAABB2D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_raycastVsAABB2DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	float cellHeight = 2.0f;
	AABB2 textbox1 = AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	AABB2 textbox2 = AABB2( textbox1.m_mins, Vec2( textbox1.m_maxs.x, textbox1.m_maxs.y - cellHeight ) );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, "Mode (F6/F7 for prev/next): Raycast Vs AABB (2D)", Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, "F8 to reset & randomize; LMB/RMB set ray start/end; ESDF move start, IJKL move end, arrows move ray, hold T slow ", Rgba8::GREEN, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_raycastVsAABB2DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw for UI camera
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsAABB2D::UpdateRaycastResult2D()
{
	m_didAABB2Impact = false;

	Vec2 startEndDisp	= m_rayEndPos - m_rayStartPos;
	float rayMaxLength	= startEndDisp.GetLength();
	Vec2 rayfwdNormal	= startEndDisp.GetNormalized();

	float superDist = 500.0f;
	for ( int i = 0; i < m_aabb2List.size(); i++ )
	{

		// Check if raycast impacted a lineSegment
		m_raycastVsAABB2Result2D = RaycastVsAABB2D( m_rayStartPos, rayfwdNormal, rayMaxLength, *m_aabb2List[i] );
		if ( m_raycastVsAABB2Result2D.m_didImpact )														 
		{
			Vec2 distFromCurrentLineToRay = m_raycastVsAABB2Result2D.m_impactPos - m_rayStartPos;
			if ( distFromCurrentLineToRay.GetLength() < superDist )
			{
				// Check for closest line segment to raycast 
				superDist			= distFromCurrentLineToRay.GetLength();
				m_currentLine		= i;
				m_didAABB2Impact	= true;
				// Use the updated values below ( impactPos and impactNormal ) for rendering raycast 
				m_updatedImpactPos		= m_raycastVsAABB2Result2D.m_impactPos;
				m_updatedImpactNormal	= m_raycastVsAABB2Result2D.m_impactNormal;
			}
		}
	}
	
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsAABB2D::UpdateInput( float deltaSeconds )
{
	// Mouse input Raycast startPos
	if ( g_theInput->IsKeyDown( KEYCODE_LEFT_MOUSE ) )
	{
		Vec2 cursorPos = g_theWindow->GetNormalizedCursorPos();
		float lerpedX = Interpolate( 0.0f, WORLD_SIZE_X, cursorPos.x );
		float lerpedY = Interpolate( 0.0f, WORLD_SIZE_Y, cursorPos.y );
		m_rayStartPos = Vec2( lerpedX, lerpedY );
	}

	// Mouse input Raycast end
	else if ( g_theInput->IsKeyDown( KEYCODE_RIGHT_MOUSE ) )
	{
		Vec2 cursorPos = g_theWindow->GetNormalizedCursorPos();
		float lerpedX = Interpolate( 0.0f, WORLD_SIZE_X, cursorPos.x );
		float lerpedY = Interpolate( 0.0f, WORLD_SIZE_Y, cursorPos.y );
		m_rayEndPos = Vec2( lerpedX, lerpedY );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// m_rayStartPos

	float moveSpeed = 40.0f;

	// Move North
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
		m_rayStartPos += Vec2( 0.0f, moveSpeed ) * deltaSeconds;
	}
	// Move South
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_rayStartPos += Vec2( 0.0f, -moveSpeed ) * deltaSeconds;
	}
	// Move East
	if ( g_theInput->IsKeyDown( 'F' ) )
	{
		m_rayStartPos += Vec2( moveSpeed, 0.0f ) * deltaSeconds;
	}
	// Move West
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_rayStartPos += Vec2( -moveSpeed, 0.0f ) * deltaSeconds;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// m_rayEndPos

	// Move North
	if ( g_theInput->IsKeyDown( 'I' ) )
	{
		m_rayEndPos += Vec2( 0.0f, moveSpeed ) * deltaSeconds;
	}
	// Move South
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		m_rayEndPos += Vec2( 0.0f, -moveSpeed ) * deltaSeconds;
	}
	// Move East
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		m_rayEndPos += Vec2( moveSpeed, 0.0f ) * deltaSeconds;
	}
	// Move West
	if ( g_theInput->IsKeyDown( 'J' ) )
	{
		m_rayEndPos += Vec2( -moveSpeed, 0.0f ) * deltaSeconds;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Move entire raycast
	// North
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) )
	{
		m_rayStartPos += Vec2( 0.0f, moveSpeed ) * deltaSeconds; 
		m_rayEndPos   += Vec2( 0.0f, moveSpeed ) * deltaSeconds;
	}
	// South
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) )
	{
		m_rayStartPos += Vec2( 0.0f, -moveSpeed ) * deltaSeconds;
		m_rayEndPos   += Vec2( 0.0f, -moveSpeed ) * deltaSeconds;
	}
	// West
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) )
	{
		m_rayStartPos += Vec2( -moveSpeed, 0.0f ) * deltaSeconds;
		m_rayEndPos   += Vec2( -moveSpeed, 0.0f ) * deltaSeconds;
	}
	// East
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
	{
		m_rayStartPos += Vec2( moveSpeed, 0.0f ) * deltaSeconds;
		m_rayEndPos   += Vec2( moveSpeed, 0.0f ) * deltaSeconds;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsAABB2D::UpdatePauseQuitAndSlowMo()
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

