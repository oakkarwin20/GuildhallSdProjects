#include "Game/GameModeRaycastVsDisc2D.hpp"
#include "Game/App.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeRaycastVsDisc2D::GameModeRaycastVsDisc2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameModeRaycastVsDisc2D::~GameModeRaycastVsDisc2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsDisc2D::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsDisc2D::Update( float deltaSeconds )
{
	deltaSeconds = m_clock.GetDeltaSeconds();

	UpdateGameCamRaycastVsDisc2D();
	UpdatePauseQuitAndSlowMo();
	UpdateInput( deltaSeconds );
	UpdateRaycastResult2D();
	//----------------------------------------------------------------------------------------------------------------------
	// #need to figure out a way to randomize discs
//	m_discCenterPos = Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.x - 20.0f ) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsDisc2D::Render() const
{
	g_theRenderer->BeginCamera( m_worldCamera );
	RenderRandDiscs();
	g_theRenderer->EndCamera( m_worldCamera );

	// Call m_screenCam and draw UI, etc
	g_theRenderer->BeginCamera( m_screenCamera );
	g_theRenderer->EndCamera( m_screenCamera );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsDisc2D::Reshuffle()
{
	m_discCenterArray[0]		= Vec2( g_theRNG->RollRandomFloatInRange( 10.0f, ( m_worldSize.x - 10.0f ) ), g_theRNG->RollRandomFloatInRange(  5.0f, ( m_worldSize.y - 5.0f ) ) );
	m_discCenterArray[1]		= Vec2( g_theRNG->RollRandomFloatInRange( 10.0f, ( m_worldSize.x - 10.0f ) ), g_theRNG->RollRandomFloatInRange( 10.0f, ( m_worldSize.y - 10.0f ) ) );
	m_discCenterArray[2]		= Vec2( g_theRNG->RollRandomFloatInRange( 15.0f, ( m_worldSize.x - 15.0f ) ), g_theRNG->RollRandomFloatInRange( 10.0f, ( m_worldSize.y - 10.0f ) ) );
	m_discCenterArray[3]		= Vec2( g_theRNG->RollRandomFloatInRange( 15.0f, ( m_worldSize.x - 15.0f ) ), g_theRNG->RollRandomFloatInRange( 10.0f, ( m_worldSize.y - 10.0f ) ) );
	m_discCenterArray[4]		= Vec2( g_theRNG->RollRandomFloatInRange( 15.0f, ( m_worldSize.x - 15.0f ) ), g_theRNG->RollRandomFloatInRange( 10.0f, ( m_worldSize.y - 10.0f ) ) );
	m_discCenterArray[5]		= Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.x - 20.0f ) ), g_theRNG->RollRandomFloatInRange( 10.0f, ( m_worldSize.y - 10.0f ) ) );
	m_discCenterArray[6]		= Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.x - 20.0f ) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
	m_discCenterArray[7]		= Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.x - 20.0f ) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
	m_discCenterArray[8]		= Vec2( g_theRNG->RollRandomFloatInRange( 30.0f, ( m_worldSize.x - 30.0f ) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
	m_discCenterArray[9]		= Vec2( g_theRNG->RollRandomFloatInRange( 30.0f, ( m_worldSize.x - 30.0f ) ), g_theRNG->RollRandomFloatInRange( 30.0f, ( m_worldSize.y - 30.0f ) ) );
}						

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsDisc2D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsDisc2D::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if (g_theInput->WasKeyJustPressed('P') || g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::BUTTON_START))
	{
		SoundID testSound = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
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

	// Fast-Mo functionality
	m_isSlowMo = g_theInput->IsKeyDown( 'Y' );
	if ( m_isSlowMo )
	{
		m_clock.SetTimeScale( 2.0f );
	}
	if ( g_theInput->WasKeyJustReleased( 'Y' ) )
	{
		m_clock.SetTimeScale( 1.0f );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsDisc2D::UpdateInput( float deltaSeconds )
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
void GameModeRaycastVsDisc2D::UpdateGameCamRaycastVsDisc2D()
{
	m_worldCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( m_worldSize.x, m_worldSize.y ) );
	m_screenCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( m_worldSize.x, m_worldSize.y ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsDisc2D::RenderRandDiscs() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render discs blue
	std::vector<Vertex_PCU> verts;
	verts.reserve( 300 );

	// Disc 1
	AddVertsForDisc2D( verts, m_discCenterArray[0], m_discRadiusArray[0], Rgba8::BLUE );
	// Disc 2
	AddVertsForDisc2D( verts, m_discCenterArray[1], m_discRadiusArray[1], Rgba8::BLUE );
	// Disc 3
	AddVertsForDisc2D( verts, m_discCenterArray[2], m_discRadiusArray[2], Rgba8::BLUE );
	// Disc 4
	AddVertsForDisc2D( verts, m_discCenterArray[3], m_discRadiusArray[3], Rgba8::BLUE );
	// Disc 5
	AddVertsForDisc2D( verts, m_discCenterArray[4], m_discRadiusArray[4], Rgba8::BLUE );
	// Disc 6
	AddVertsForDisc2D( verts, m_discCenterArray[5], m_discRadiusArray[5], Rgba8::BLUE );
	// Disc 7
	AddVertsForDisc2D( verts, m_discCenterArray[6], m_discRadiusArray[6], Rgba8::BLUE );
	// Disc 8
	AddVertsForDisc2D( verts, m_discCenterArray[7], m_discRadiusArray[7], Rgba8::BLUE );
	// Disc 9
	AddVertsForDisc2D( verts, m_discCenterArray[8], m_discRadiusArray[8], Rgba8::BLUE );
	// Disc 10 
	AddVertsForDisc2D( verts, m_discCenterArray[9], m_discRadiusArray[9], Rgba8::BLUE );

	// normal raycast if !impact
	AddVertsForArrow2D( verts, m_rayStartPos, m_rayEndPos, m_arrowSize, m_arrowThickness, Rgba8::GREEN );

//	//----------------------------------------------------------------------------------------------------------------------
//	// Render relevant shape lightBlue if playerPos is Inside
//
//	std::vector<Vertex_PCU> verts;
//	verts.reserve( 300 );
//
//	// Disc 1
//	AddVertsForDisc2D( verts, m_disc1CenterPos + Vec2( 180.0f, 90.0f ), m_disc1Radius, Rgba8::BLUE );
//	// Disc 2
//	AddVertsForDisc2D( verts, m_disc2CenterPos + Vec2( 20.0f, 15.0f ), m_disc2Radius, Rgba8::BLUE );
//	// Disc 3
//	AddVertsForDisc2D( verts, m_disc3CenterPos + Vec2( 120.0f, 40.0f ), m_disc3Radius, Rgba8::BLUE );
//	// Disc 4
//	AddVertsForDisc2D( verts, m_disc4CenterPos + Vec2( 80.0f, 20.0f ), m_disc4Radius, Rgba8::BLUE );
//	// Disc 5
//	AddVertsForDisc2D( verts, m_disc5CenterPos + Vec2( 155.0f, 46.0f ), m_disc5Radius, Rgba8::BLUE );
//	// Disc 6
//	AddVertsForDisc2D( verts, m_disc6CenterPos + Vec2( 30.0f, 70.0f ), m_disc6Radius, Rgba8::BLUE );
//	// Disc 7
//	AddVertsForDisc2D( verts, m_disc7CenterPos + Vec2( 60.0f, 60.0f ), m_disc7Radius, Rgba8::BLUE );
//	// Disc 8
//	AddVertsForDisc2D( verts, m_disc8CenterPos + Vec2( 110.0f, 70.0f ), m_disc8Radius, Rgba8::BLUE );
//	// Disc 9
//	AddVertsForDisc2D( verts, m_disc9CenterPos + Vec2( 90.0f, 30.0f ), m_disc9Radius, Rgba8::BLUE );
//	// Disc 10 
//	AddVertsForDisc2D( verts, m_disc10CenterPos + Vec2( 50.0f, 40.0f ), m_disc10Radius, Rgba8::BLUE );

//	g_theRenderer->BindTexture( nullptr );
//	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );

	//----------------------------------------------------------------------------------------------------------------------
	// rendering when raycast impacts disc

		// disc 3
		if ( m_didDiscImpact )
		{
			AddVertsForDisc2D( verts, m_discCenterArray[m_closestDisc], m_discRadiusArray[m_closestDisc], Rgba8::LIGHTBLUE );
		
			// ray start to end 
			AddVertsForArrow2D( verts, m_rayStartPos, m_rayEndPos, m_arrowSize, m_arrowThickness, Rgba8::GRAY );
			// ray after impact // impactDist to rayEndPoint
			AddVertsForArrow2D( verts, m_updatedImpactPos, m_rayEndPos, m_arrowSize, m_arrowThickness, Rgba8::RED);
			// impact normal // reflection 
			AddVertsForArrow2D( verts, m_updatedImpactPos, m_updatedImpactPos + ( m_updatedImpactNormal * 8.0f ), m_arrowSize, m_arrowThickness, Rgba8::YELLOW );
		}

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsDisc2D::UpdateRaycastResult2D()
{
	m_didDiscImpact			= false;

	Vec2 startToEnd			= m_rayEndPos - m_rayStartPos;
	float maxDist			= startToEnd.GetLength();
	Vec2 rayCastFwdNormal	= m_rayEndPos - m_rayStartPos;
	rayCastFwdNormal.Normalize();

//	Vec2	closestDiscCenter;
//	float	closestDiscRadius;

	float superLength = 500.0f;
	for ( int i = 0; i < m_numDiscs; i++ )
	{
		m_raycastResult2D = RaycastVsDisc2D( m_rayStartPos, rayCastFwdNormal, maxDist, m_discCenterArray[i], m_discRadiusArray[i] );
		// loop through 10 discs
		// each time, find the closest disc by subtracting dist from each discCenter to rayStartPos
		// if dist[i] < superLength, superLength = dist[i]
		// once the closest disc is calculated, do a raycast on that disc

		if ( m_raycastResult2D.m_didImpact )
		{
			Vec2 distDiscImpactPointToRayStart = m_raycastResult2D.m_impactPos - m_rayStartPos;

			if ( distDiscImpactPointToRayStart.GetLength() < superLength )
			{
				superLength				= distDiscImpactPointToRayStart.GetLength();
				m_closestDisc			= i;
				m_didDiscImpact			= true;
				m_updatedImpactPos		= m_raycastResult2D.m_impactPos;
				m_updatedImpactNormal	= m_raycastResult2D.m_impactNormal;
			}
		}
	}
		
	/*Vec2 SC = m_disc3CenterPos - m_raycast2D.m_rayStartPos;
	float SCi	= GetProjectedLength2D( SC, m_raycast2D.m_rayForwardNormal );
	Vec2 j		= m_raycast2D.m_rayForwardNormal.GetRotated90Degrees();
	float SCj	= GetProjectedLength2D( SC, j );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Checking miss cases
	// disc is too far right or left
	if ( SCj > m_disc3Radius || SCj < m_disc3Radius )
	{
		m_raycast2D.m_didImpact = false;
		return;
	}
	// disc is before or after ray
	if ( SCj < -m_disc3Radius || SCj > m_raycast2D.m_rayMaxLenth + m_disc3Radius )
	{
		m_raycast2D.m_didImpact = false;
		return;
	}

	float a							= sqrtf( (SCj * SCj) - (m_disc3Radius * m_disc3Radius) );
	m_raycast2D.m_impactDist		= SCi - a;
	// raycast is too short or too long
	if ( m_raycast2D.m_impactDist < 0 || m_raycast2D.m_impactDist > m_raycast2D.m_rayMaxLenth )
	{
		m_raycast2D.m_didImpact = false;
		return;
	}
	else 
	{ 
		m_raycast2D.m_impactPos		= m_raycast2D.m_rayStartPos + ( m_raycast2D.m_rayForwardNormal * m_raycast2D.m_impactDist );
		m_raycast2D.m_impactNormal	= m_raycast2D.m_impactPos - m_disc3CenterPos.GetNormalized();
		m_raycast2D.m_didImpact		= true;
		return;
	}
	*/
}
