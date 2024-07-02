#include "GameModeNearestPoint2D.hpp"
#include "Game/App.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EngineCommon.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeNearestPoint2D::GameModeNearestPoint2D()
{
	Vec2 iBasis = Vec2::MakeFromPolarDegrees( 25.0f );
	m_obb2D	= new OBB2D( Vec2( 160.0f, 70.0f ), iBasis, Vec2( 3.0f, 7.0f ) );
}

//----------------------------------------------------------------------------------------------------------------------
GameModeNearestPoint2D::~GameModeNearestPoint2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeNearestPoint2D::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeNearestPoint2D::Update(float deltaSeconds)
{
	deltaSeconds = m_clock.GetDeltaSeconds();

	UpdateGameCamNearestPoint2D();
	UpdatePauseQuitAndSlowMo();
	UpdatePlayerDot( deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeNearestPoint2D::Render() const
{
	// Draw everything in world space
	g_theRenderer->BeginCamera(m_worldCamera);
	RenderEntities();
	g_theRenderer->EndCamera(m_worldCamera);

	// Call m_screenCam and draw UI, etc
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->EndCamera(m_screenCamera);
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeNearestPoint2D::Reshuffle()
{
	m_discCenterPos		= Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.x - 20.0f ) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
	m_lineSegmentStart	= Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.x - 20.0f ) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
	m_lineSegmentEnd	= Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.x - 20.0f ) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
	m_infiniteLineStart = Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.x - 20.0f ) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
	m_infiniteLineEnd	= Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.x - 20.0f ) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
	Vec2 mins			= Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.x - 20.0f ) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
	Vec2 dimensions		= Vec2( g_theRNG->RollRandomFloatInRange( 10.0f, ( m_worldSize.x - 140.0f ) ), g_theRNG->RollRandomFloatInRange( 10.0f, ( m_worldSize.y - 80.0f ) ) );
	m_box				= AABB2( mins, mins + dimensions );
	m_obb2D->m_center	= Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, (m_worldSize.x - 20.0f) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
	m_capsuleBoneStart  = Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, (m_worldSize.x - 20.0f) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
	m_capsuleBoneEnd	= Vec2( g_theRNG->RollRandomFloatInRange( 20.0f, (m_worldSize.x - 20.0f) ), g_theRNG->RollRandomFloatInRange( 20.0f, ( m_worldSize.y - 20.0f ) ) );
	m_capsuleRadius		= g_theRNG->RollRandomFloatInRange( 1.0f, 6.0f );
}
 
//----------------------------------------------------------------------------------------------------------------------
void GameModeNearestPoint2D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeNearestPoint2D::UpdatePauseQuitAndSlowMo()
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
void GameModeNearestPoint2D::RenderEntities() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render all shapes dark blue
	
		std::vector<Vertex_PCU> verts;
		verts.reserve(300);

		// Render Disc Blue
		AddVertsForDisc2D( verts, m_discCenterPos, m_discRadius, Rgba8::BLUE );
		// Render AABB2D Blue
		AddVertsForAABB2D( verts, m_box, Rgba8::BLUE );
		// Render LineSegment Blue
		AddVertsForLineSegment2D( verts, m_lineSegmentStart, m_lineSegmentEnd, m_lineRadius, Rgba8::BLUE );
		// Render InfiniteLine Blue
		AddVertsForInfiniteLine( verts, m_infiniteLineStart, m_infiniteLineEnd, m_lineRadius, Rgba8::BLUE );
		// Render OBB2D Blue
		AddVertsForOBB2D( verts, *m_obb2D, Rgba8::BLUE );
		// Render Capsule2D
		AddVertsForCapsule2D( verts, m_capsuleBoneStart, m_capsuleBoneEnd, m_capsuleRadius, Rgba8::BLUE );
				
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );

	//----------------------------------------------------------------------------------------------------------------------
	// Render relevant shape lightBlue if playerPos is Inside
		
		std::vector<Vertex_PCU> tempVerts;
	
		// Disc2D
		if ( IsPointInsideDisc2D( m_playerDotPos, m_discCenterPos, m_discRadius ) )
		{
			// Render Disc lightBlue
			AddVertsForDisc2D( tempVerts, m_discCenterPos, m_discRadius, Rgba8::LIGHTBLUE );
			
			g_theRenderer->SetBlendMode( BlendMode::ALPHA );
			g_theRenderer->SetModelConstants();
			g_theRenderer->BindTexture( nullptr );
			g_theRenderer->DrawVertexArray( static_cast<int>( tempVerts.size() ), tempVerts.data() );
		}
		// AABB2D
		if ( IsPointInsideAABB2D( m_playerDotPos, m_box ) )
		{
			// Render AABB2D
			AddVertsForAABB2D( tempVerts, m_box, Rgba8::LIGHTBLUE );
			
			g_theRenderer->SetBlendMode( BlendMode::ALPHA );
			g_theRenderer->SetModelConstants();
			g_theRenderer->BindTexture( nullptr ); 
			g_theRenderer->DrawVertexArray( static_cast<int>( tempVerts.size() ), tempVerts.data() );
		}
		// OBB2D
		if ( IsPointInsideOBB2D( m_playerDotPos, *m_obb2D ) )
		{
			AddVertsForOBB2D( tempVerts, *m_obb2D, Rgba8::LIGHTBLUE );
			
			g_theRenderer->SetBlendMode( BlendMode::ALPHA );
			g_theRenderer->SetModelConstants();
			g_theRenderer->BindTexture( nullptr );
			g_theRenderer->DrawVertexArray( static_cast<int>( tempVerts.size() ), tempVerts.data() );
		}
		// Capsule2D
		if ( IsPointInsideCapsule2D( m_playerDotPos, m_capsuleBoneStart, m_capsuleBoneEnd, m_capsuleRadius ) )
		{
			AddVertsForCapsule2D( tempVerts, m_capsuleBoneStart, m_capsuleBoneEnd, m_capsuleRadius, Rgba8::LIGHTBLUE );
			
			g_theRenderer->SetBlendMode( BlendMode::ALPHA );
			g_theRenderer->SetModelConstants();
			g_theRenderer->BindTexture( nullptr );
			g_theRenderer->DrawVertexArray( static_cast<int>( tempVerts.size() ), tempVerts.data() );
		}

	//----------------------------------------------------------------------------------------------------------------------
	// GetNearestPointOn Shapes then Render dot on Disc's nearestPointToPlayer
	
		std::vector<Vertex_PCU> nearestPointVerts;

		// GetNearestPointOnDisc 
		Vec2 nearestPointOnDisc = GetNearestPointOnDisc2D( m_playerDotPos, m_discCenterPos, m_discRadius );
		AddVertsForDisc2D( nearestPointVerts, nearestPointOnDisc, 1.0f, Rgba8::ORANGE );
		
		// GetNearestPointOnAABB2D 
		Vec2 const& nearestPointOnBox = m_box.GetNearestPoint( m_playerDotPos );
		AddVertsForDisc2D( nearestPointVerts, nearestPointOnBox, 1.0f, Rgba8::ORANGE );
		
		// GetNearestPointOnLineSegment 
		Vec2 nearestPointOnLineSegment = GetNearestPointOnLineSegment2D( m_playerDotPos,m_lineSegmentStart, m_lineSegmentEnd );
		AddVertsForDisc2D( nearestPointVerts, nearestPointOnLineSegment, 1.0f, Rgba8::ORANGE );
		
		// GetNearestPointOnInfiniteLine
		Vec2 nearestPointOnInfiniteLine = GetNearestPointOnInfiniteLine2D( m_playerDotPos, m_infiniteLineStart, m_infiniteLineEnd );
		AddVertsForDisc2D( nearestPointVerts, nearestPointOnInfiniteLine, 1.0f, Rgba8::ORANGE );

		// GetNearestPointOnOBB2D 
		Vec2 nearestPointOnOBB2D = GetNearestPointOnOBB2D( m_playerDotPos, *m_obb2D );
		AddVertsForDisc2D( nearestPointVerts, nearestPointOnOBB2D, 1.0f, Rgba8::ORANGE );

		// GetNearestPointOnCapsule2D 
		Vec2 nearestPointOnCapsule2D = GetNearestPointOnCapsule2D( m_playerDotPos, m_capsuleBoneStart, m_capsuleBoneEnd, m_capsuleRadius );
		AddVertsForDisc2D( nearestPointVerts, nearestPointOnCapsule2D, m_lineRadius, Rgba8::ORANGE );

		// Draw line from nearestPoint to PlayerDot
		AddVertsForLineSegment2D( nearestPointVerts,   		 nearestPointOnDisc, m_playerDotPos, 0.3f, Rgba8::TRANSLUCENT_WHITE );
		AddVertsForLineSegment2D( nearestPointVerts,   		  nearestPointOnBox, m_playerDotPos, 0.3f, Rgba8::TRANSLUCENT_WHITE );
		AddVertsForLineSegment2D( nearestPointVerts,  nearestPointOnLineSegment, m_playerDotPos, 0.3f, Rgba8::TRANSLUCENT_WHITE );
		AddVertsForLineSegment2D( nearestPointVerts, nearestPointOnInfiniteLine, m_playerDotPos, 0.3f, Rgba8::TRANSLUCENT_WHITE );
		AddVertsForLineSegment2D( nearestPointVerts,		nearestPointOnOBB2D, m_playerDotPos, 0.3f, Rgba8::TRANSLUCENT_WHITE );
		AddVertsForLineSegment2D( nearestPointVerts,	nearestPointOnCapsule2D, m_playerDotPos, 0.3f, Rgba8::TRANSLUCENT_WHITE );
		AddVertsForDisc2D( nearestPointVerts, m_playerDotPos, 0.5f, Rgba8::WHITE );
		
		// DrawCall
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->DrawVertexArray( static_cast<int>( nearestPointVerts.size() ), nearestPointVerts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeNearestPoint2D::UpdateGameCamNearestPoint2D()
{
	m_worldCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( m_worldSize.x, m_worldSize.y ) );
	m_screenCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( m_worldSize.x, m_worldSize.y ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeNearestPoint2D::UpdatePlayerDot( float deltaSeconds )
{
	float dotSpeed = 40.0f;

	// Move North
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
		m_playerDotPos += Vec2( 0.0f, dotSpeed ) * deltaSeconds;
	}
	// Move South
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_playerDotPos += Vec2( 0.0f, -dotSpeed ) * deltaSeconds;
	}
	// Move East
	if ( g_theInput->IsKeyDown( 'F' ) )
	{
		m_playerDotPos += Vec2( dotSpeed , 0.0f ) * deltaSeconds;
	}
	// Move West
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_playerDotPos += Vec2( -dotSpeed, 0.0f ) * deltaSeconds;
	}
}
