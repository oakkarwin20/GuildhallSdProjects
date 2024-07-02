#include "Game/GameModeRaycastVsOBB2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeRaycastVsOBB2D::GameModeRaycastVsOBB2D()
{
	float angleX		=  0.6f;
	float angleY		=  0.8f;
	float angleOffset	= 15.0f;
	for ( int i = 0; i < m_numOBB2s; i++ )
	{
//		Vec2 center				= Vec2( g_theRNG->RollRandomFloatInRange( m_randMinXInclusive, m_randMaxXInclusive ),  g_theRNG->RollRandomFloatInRange( m_randMinYInclusive, m_randMaxYInclusive ) );	
		Vec2 center				= Vec2( 10.0f, 10.f );	
		float width				= g_theRNG->RollRandomFloatInRange( 5.0f, 10.0f );
		float height 			= g_theRNG->RollRandomFloatInRange( 5.0f, 15.0f );
		Vec2 iBasis				= Vec2( angleX, angleY );
		iBasis.Normalize();
//		OBB2D* currentOBB2		= new OBB2D( center, iBasis, Vec2( width, height ) );
		OBB2D* currentOBB2		= new OBB2D( center, iBasis, Vec2( 10.0f, 5.0f ) );
		m_obb2List.push_back( currentOBB2 );

		float yOffset = g_theRNG->RollRandomFloatInRange( -60.0f, 60.0f );
		angleX += angleOffset;
		angleY += yOffset;
	}

}

//----------------------------------------------------------------------------------------------------------------------
GameModeRaycastVsOBB2D::~GameModeRaycastVsOBB2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB2D::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB2D::Update( float deltaSeconds )
{
	deltaSeconds = m_gameClock.GetDeltaSeconds();

	UpdateGameCameraRaycastVsAABB2();
	UpdateInput( deltaSeconds );
	UpdatePauseQuitAndSlowMo();
	UpdateRaycastResult2D();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB2D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB2D::Reshuffle()
{
/*
	for ( int i = 0; i < m_obb2List.size(); i++ )
	{
		Vec2 center  = Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, 180.0f ), g_theRNG->RollRandomFloatInRange( 20.0f, 80.0f ) );
		float width  = g_theRNG->RollRandomFloatInRange( 5.0f, 25.0f );
		float height = g_theRNG->RollRandomFloatInRange( 5.0f, 40.0f );
		AABB2 randAABB2 = AABB2( center, width, height );
		m_obb2List[i]->m_mins = randAABB2.m_mins;
		m_obb2List[i]->m_maxs = randAABB2.m_maxs;
	}
*/
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB2D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB2D::UpdateGameCameraRaycastVsAABB2()
{
	m_raycastVsOBB2DWorldCamera.SetOrthoView( Vec2(0.0f, 0.0f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y) );
	   m_raycastVsOBB2DUICamera.SetOrthoView( Vec2(0.0f, 0.0f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB2D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	g_theRenderer->BeginCamera( m_raycastVsOBB2DWorldCamera );
	
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;
	verts.reserve( 300 );

	g_theApp->m_textFont->AddVertsForText2D( textVerts, Vec2( 60.0f, 80.0f ), 5.0f, "Raycast Vs Obb2D", Rgba8::GREEN );

	//----------------------------------------------------------------------------------------------------------------------
	// Render AABB2
//	for ( int i = 0; i < m_obb2List.size(); i++ )
	for ( int i = 0; i < 1; i++ )
	{
		AddVertsForOBB2D( verts, *m_obb2List[i], m_obb2Color );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render Raycast arrow
	// Ray start to end with no impact
	AddVertsForArrow2D( verts, m_rayStartPos, m_rayEndPos, m_arrowSize, m_arrowThickness, m_rayDefaultColor );

	// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
	if ( m_didOBB2Impact )
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
	
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	// End World Camera
	g_theRenderer->EndCamera( m_raycastVsOBB2DWorldCamera );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB2D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_raycastVsOBB2DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	float cellHeight = 2.0f;
	AABB2 textbox1 = AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	AABB2 textbox2 = AABB2( textbox1.m_mins, Vec2( textbox1.m_maxs.x, textbox1.m_maxs.y - cellHeight ) );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, "Mode (F6/F7 for prev/next): Raycast Vs OBB (2D)", Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, "F8 to reset & randomize; LMB/RMB set ray start/end; ESDF move start, IJKL move end, arrows move ray, hold T slow ", Rgba8::GREEN, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_raycastVsOBB2DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw for UI camera
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB2D::UpdateRaycastResult2D()
{
	m_didOBB2Impact		= false;
	Vec2 startEndDisp	= m_rayEndPos - m_rayStartPos;
//	float rayMaxLength	= startEndDisp.GetLength();
	float rayMaxLength	= 500.0f;
	Vec2 rayfwdNormal	= startEndDisp.GetNormalized();

	float superDist = 500.0f;
	for ( int i = 0; i < m_obb2List.size(); i++ )
	{
		// Check if raycast impacted a lineSegment
		m_raycastVsOBB2DResult = RaycastVsOBB2D( m_rayStartPos, rayfwdNormal, rayMaxLength, *m_obb2List[i] );
		if ( m_raycastVsOBB2DResult.m_didImpact )														 
		{
			Vec2 distFromCurrentLineToRay = m_raycastVsOBB2DResult.m_impactPos - m_rayStartPos;
			if ( distFromCurrentLineToRay.GetLength() < superDist )
			{
				// Check for closest line segment to raycast 
				superDist				= distFromCurrentLineToRay.GetLength();
				m_currentLine			= i;
				m_didOBB2Impact			= true;
				// Use the updated values below ( impactPos and impactNormal ) for rendering raycast 
				m_updatedImpactPos		= m_raycastVsOBB2DResult.m_impactPos;
				m_updatedImpactNormal	= m_raycastVsOBB2DResult.m_impactNormal;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB2D::UpdateInput( float deltaSeconds )
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
void GameModeRaycastVsOBB2D::UpdatePauseQuitAndSlowMo()
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

