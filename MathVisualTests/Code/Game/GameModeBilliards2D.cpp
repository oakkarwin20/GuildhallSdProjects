#include "Game/GameModeBilliards2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeBilliards2D::GameModeBilliards2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameModeBilliards2D::~GameModeBilliards2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBilliards2D::Startup()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Create Bumpers
	for ( int i = 0; i < m_numBumpers; i++ )
	{
		Bumper* bumper				= new Bumper();
		bumper->m_bumperPos			= m_bumperPosArray[i];
		bumper->m_bumperRadius		= m_bumperRadiusArray[i];
		bumper->m_bumperElasticity	= m_bumperElasticityArray[i];
		
		Rgba8 lerpedBumperColor		= Interpolate( Rgba8::RED, Rgba8::GREEN, bumper->m_bumperElasticity );
		bumper->m_bumperColor		= lerpedBumperColor;
		m_bumperList.push_back( bumper );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBilliards2D::Update( float deltaSeconds )
{
	deltaSeconds = m_gameClock.GetDeltaSeconds();

	UpdateInput( deltaSeconds );
	UpdateGameCamBilliards2D();
	UpdateRaycastResult2D();
	UpdatePauseQuitAndSlowMo();
	UpdateSpawnBilliards();
	UpdateBilliardCollisionAndClampToWorldBounds( deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBilliards2D::Render() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	g_theRenderer->BeginCamera( m_billiardsWorldCamera );
	
		//----------------------------------------------------------------------------------------------------------------------
		std::vector<Vertex_PCU> verts;
		verts.reserve( 300 );

		//----------------------------------------------------------------------------------------------------------------------
		// Render ring at Raycast arrow startPos
		AddVertsForRing2D( verts, m_rayStartPos, m_billiardRadius, m_arrowThickness, Rgba8::LIGHTBLUE );

		//----------------------------------------------------------------------------------------------------------------------
		// Render normal Raycast arrow if !impact
		AddVertsForArrow2D( verts, m_rayStartPos, m_rayEndPos, m_arrowSize, m_arrowThickness, Rgba8::GREEN );

		//----------------------------------------------------------------------------------------------------------------------
		// Render Bumpers
		for ( int j = 0; j < m_bumperList.size(); j++ )
		{
			AddVertsForDisc2D( verts, m_bumperList[j]->m_bumperPos, m_bumperList[j]->m_bumperRadius, m_bumperList[j]->m_bumperColor );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Render Billiards when Spawned
		for ( int i = 0; i < m_billiardsList.size(); i++ )
		{
			AddVertsForDisc2D( verts, m_billiardsList[i]->m_billiardPos, m_billiardsList[i]->m_billiardRadius, Rgba8::CYAN );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// rendering when raycast impacts disc
		if ( m_didDiscImpact )
		{
			// Tint disc that will be impacted
			if ( m_isOverlappingBumper )
			{
				AddVertsForDisc2D( verts, m_bumperList[m_closestDisc]->m_bumperPos, m_bumperList[m_closestDisc]->m_bumperRadius, Rgba8::LIGHTBLUE );
			}
			if ( m_isOverlappingBilliard && m_billiardsList.size() > 0 ) 
			{
				AddVertsForDisc2D( verts, m_billiardsList[m_closestDisc]->m_billiardPos, m_billiardsList[m_closestDisc]->m_billiardRadius, Rgba8::MAGENTA );
			}

			//----------------------------------------------------------------------------------------------------------------------
			// Render Raycast arrow
			// ray start to end 
			AddVertsForArrow2D( verts, m_rayStartPos, m_rayEndPos, m_arrowSize, m_arrowThickness, Rgba8::GRAY );
			// ray after impact // impactDist to rayEndPoint
			AddVertsForArrow2D( verts, m_updatedImpactPos, m_rayEndPos, m_arrowSize, m_arrowThickness, Rgba8::RED);
			// Disc between raycasts
			AddVertsForDisc2D( verts, m_updatedImpactPos, m_arrowThickness, Rgba8::WHITE );
			// impact normal // reflection 
			AddVertsForArrow2D( verts, m_updatedImpactPos, m_updatedImpactPos + ( m_updatedImpactNormal * 8.0f ), m_arrowSize, m_arrowThickness, Rgba8::YELLOW );
		}
	
		//----------------------------------------------------------------------------------------------------------------------
		// Draw call for World camera
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );

	//----------------------------------------------------------------------------------------------------------------------
	// End World Camera
	g_theRenderer->EndCamera( m_billiardsWorldCamera );

	RenderUIStuff();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBilliards2D::Reshuffle()
{	
	for ( int i = 0; i < m_bumperList.size(); i++ )
	{
		m_bumperList[i]->m_bumperPos	= Vec2( g_theRNG->RollRandomFloatInRange( 15.0f, ( WORLD_SIZE_X - 15.0f ) ), g_theRNG->RollRandomFloatInRange( 10.0f, ( WORLD_SIZE_Y - 10.0f ) ) );
		m_bumperList[i]->m_bumperRadius = g_theRNG->RollRandomFloatInRange( 2.0f, 10.0f );
	}

	// Remove billiards
	m_billiardsList.clear();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBilliards2D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBilliards2D::UpdatePauseQuitAndSlowMo()
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
void GameModeBilliards2D::UpdateInput( float deltaSeconds )
{
	// Mouse input Raycast startPos
	if ( g_theInput->IsKeyDown( KEYCODE_LEFT_MOUSE ) )
	{
		Vec2 cursorPos	= g_theWindow->GetNormalizedCursorPos();
		float lerpedX	= Interpolate( 0.0f, WORLD_SIZE_X, cursorPos.x );
		float lerpedY	= Interpolate( 0.0f, WORLD_SIZE_Y, cursorPos.y );
		m_rayStartPos	= Vec2( lerpedX, lerpedY );
	}

	// Mouse input Raycast end
	else if ( g_theInput->IsKeyDown( KEYCODE_RIGHT_MOUSE ) )
	{
		Vec2 cursorPos	= g_theWindow->GetNormalizedCursorPos();
		float lerpedX	= Interpolate( 0.0f, WORLD_SIZE_X, cursorPos.x );
		float lerpedY	= Interpolate( 0.0f, WORLD_SIZE_Y, cursorPos.y );
		m_rayEndPos		= Vec2( lerpedX, lerpedY );
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
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBilliards2D::UpdateGameCamBilliards2D()
{
	m_billiardsWorldCamera.SetOrthoView( Vec2(0.0f, 0.0f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y) );
	   m_billiardsUICamera.SetOrthoView( Vec2(0.0f, 0.0f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBilliards2D::UpdateRaycastResult2D()
{
	m_didDiscImpact = false;

	Vec2 startToEnd			= m_rayEndPos - m_rayStartPos;
	float maxDist			= startToEnd.GetLength();
	Vec2 raycastFwdNormal	= m_rayEndPos - m_rayStartPos;
	raycastFwdNormal.Normalize();

	float superLength = 500.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Loop through bumpers
	for ( int i = 0; i < m_numBumpers; i++ )
	{
		m_bumperRaycastResult2D	  = RaycastVsDisc2D( m_rayStartPos, raycastFwdNormal, maxDist, m_bumperList[i]->m_bumperPos, m_bumperList[i]->m_bumperRadius );

		// Raycast for billiards if there are any
		// loop through 10 discs
		// each time, find the closest disc by subtracting dist from each discCenter to rayStartPos
		// if dist[i] < superLength, superLength = dist[i]
		// once the closest disc is calculated, do a raycast on that disc

		if ( m_bumperRaycastResult2D.m_didImpact )
		{
			Vec2 distDiscImpactPointToRayStart = m_bumperRaycastResult2D.m_impactPos - m_rayStartPos;

			if ( distDiscImpactPointToRayStart.GetLength() < superLength )
			{
				superLength				= distDiscImpactPointToRayStart.GetLength();
				m_closestDisc			= i;
				m_didDiscImpact			= true;
				m_updatedImpactPos		= m_bumperRaycastResult2D.m_impactPos;
				m_updatedImpactNormal	= m_bumperRaycastResult2D.m_impactNormal;
				m_isOverlappingBumper	= true;
				m_isOverlappingBilliard	= false;
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Loop through billiards
	for ( int j = 0; j < m_billiardsList.size(); j++ )
	{
		m_billiardRaycastResult2D = RaycastVsDisc2D( m_rayStartPos, raycastFwdNormal, maxDist, m_billiardsList[j]->m_billiardPos, m_billiardsList[j]->m_billiardRadius );
		
		if ( m_billiardRaycastResult2D.m_didImpact )
		{
			Vec2 distDiscImpactPointToRayStart = m_billiardRaycastResult2D.m_impactPos - m_rayStartPos;

			if ( distDiscImpactPointToRayStart.GetLength() < superLength )
			{
				superLength				= distDiscImpactPointToRayStart.GetLength();
				m_closestDisc			= j;
				m_didDiscImpact			= true;
				m_updatedImpactPos		= m_billiardRaycastResult2D.m_impactPos;
				m_updatedImpactNormal	= m_billiardRaycastResult2D.m_impactNormal;
				m_isOverlappingBilliard = true;
				m_isOverlappingBumper	= false;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBilliards2D::RenderUIStuff() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_billiardsUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	float cellHeight	= 2.0f;
	AABB2 textbox1		= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	AABB2 textbox2		= AABB2( textbox1.m_mins, Vec2( textbox1.m_maxs.x, textbox1.m_maxs.y - cellHeight ) );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, "Mode (F6/F7 for prev/next): Billiards (2D)", Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, "F8 to reset & randomize; LMB/RMB set ray start/end; hold T slow, space = launch billiard, [P]ause, [O]ne step only", Rgba8::GREEN, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_billiardsUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw for UI camera
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBilliards2D::UpdateSpawnBilliards()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Shoot Billiards if 'SPACEBAR' is pressed
	if ( g_theInput->WasKeyJustPressed( ' ' ) )
	{
		// Create Billiard with velocity of raycast's magnitude
		Billiards* billiard = new Billiards();
		billiard->m_billiardVelocity = m_rayEndPos - m_rayStartPos;

		// Updating m_billiardPos to m_rayStartPos each frame to pass correct info when Rendering Billiard
		billiard->m_billiardPos = m_rayStartPos;

		m_billiardsList.push_back( billiard );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBilliards2D::UpdateBilliardCollisionAndClampToWorldBounds( float deltaSeconds )
{
	// loop through all billiards and update their velocities
	for ( int i = 0; i < m_billiardsList.size(); i++ )
	{
		// Update each billiard position by adding velocity
		m_billiardsList[i]->m_billiardPos += ( m_billiardsList[i]->m_billiardVelocity * deltaSeconds );

		//----------------------------------------------------------------------------------------------------------------------
		// Clamping BilliardsPos to world bounds // Bounce Billiard by flipping x or y axis if pos > world bounds
		if ( m_billiardsList[i]->m_billiardPos.x >= ( WORLD_SIZE_X - m_billiardsList[i]->m_billiardRadius ) )
		{
			m_billiardsList[i]->m_billiardPos.x		  = ( WORLD_SIZE_X - m_billiardsList[i]->m_billiardRadius );		// Clamp Billiard position to World.maxX
			m_billiardsList[i]->m_billiardVelocity.x *= -0.9f;													// Reverse Billiard.x velocity to bounce off wall
		}
		if ( m_billiardsList[i]->m_billiardPos.x <= 0.0f + m_billiardsList[i]->m_billiardRadius )
		{
			m_billiardsList[i]->m_billiardPos.x		  = ( 0.0f + m_billiardsList[i]->m_billiardRadius );				// Clamp Billiard position to World.minX
			m_billiardsList[i]->m_billiardVelocity.x *= -0.9f;													// Reverse Billiard.x velocity to bounce off wall
		}
		if ( m_billiardsList[i]->m_billiardPos.y >= ( WORLD_SIZE_Y - m_billiardsList[i]->m_billiardRadius ) )
		{
			m_billiardsList[i]->m_billiardPos.y		  = ( WORLD_SIZE_Y - m_billiardsList[i]->m_billiardRadius );		// Clamp Billiard position to World.maxY
			m_billiardsList[i]->m_billiardVelocity.y *= -0.9f;													// Reverse Billiard.y velocity to bounce off wall
		}
		if ( m_billiardsList[i]->m_billiardPos.y <= 0.0f + m_billiardsList[i]->m_billiardRadius )
		{
			m_billiardsList[i]->m_billiardPos.y		  = ( 0.0f + m_billiardsList[i]->m_billiardRadius );				// Clamp Billiard position to World.minY
			m_billiardsList[i]->m_billiardVelocity.y *= -0.9f;													// Reverse Billiard.y velocity to bounce off wall
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Bounce this disc off every other disc // (current 'i' in the for-loop) 
		for ( int j = i + 1; j < m_billiardsList.size(); j++ )
		{
			BounceDiscOffEachOther2D( m_billiardsList[i]->m_billiardPos,		m_billiardsList[i]->m_billiardRadius,		m_billiardsList[i]->m_billiardVelocity,
									  m_billiardsList[j]->m_billiardPos,		m_billiardsList[j]->m_billiardRadius,		m_billiardsList[j]->m_billiardVelocity,
									  m_billiardsList[i]->m_billiardElasticity, m_billiardsList[j]->m_billiardElasticity );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Check billiards collision against bumpers
		for ( int k = 0; k < m_bumperList.size(); k++ )
		{
			// Reflect Billiard velocity off this Bumper 
			BounceDiscOffFixedDiscBumper2D(	m_billiardsList[i]->m_billiardPos,			m_billiardsList[i]->m_billiardRadius,	m_billiardsList[i]->m_billiardVelocity,
											   m_bumperList[k]->m_bumperPos,			   m_bumperList[k]->m_bumperRadius, 
											m_billiardsList[i]->m_billiardElasticity,	   m_bumperList[k]->m_bumperElasticity );
		}
	}
}