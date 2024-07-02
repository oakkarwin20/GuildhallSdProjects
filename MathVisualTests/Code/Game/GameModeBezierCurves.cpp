#include "Game/GameModeBezierCurves.hpp"
#include "Game/App.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeBezierCurves::GameModeBezierCurves()
	: m_cubicBezier( Vec2::ZERO, Vec2::ZERO, Vec2::ZERO, Vec2::ZERO )
{
	m_debugBackground = true;
	m_easingModeIndex   = 1;
}

//----------------------------------------------------------------------------------------------------------------------
GameModeBezierCurves::~GameModeBezierCurves()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBezierCurves::Startup()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Initialize m_cubicBezier positions
	m_cubicBezier.m_startPos	= Vec2( g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.x, m_bezierPanelBounds.m_maxs.x ), 
										g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.y, m_bezierPanelBounds.m_maxs.y ) );

	m_cubicBezier.m_guidePos1	= Vec2( g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.x + m_beizerOffset, m_bezierPanelBounds.m_maxs.x ), 
										g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.y, m_bezierPanelBounds.m_maxs.y ) );
	
	m_cubicBezier.m_guidePos2	= Vec2( g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.x + (m_beizerOffset * 1.0f), m_bezierPanelBounds.m_maxs.x ), 
										g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.y, m_bezierPanelBounds.m_maxs.y ) );
	
	m_cubicBezier.m_endPos		= Vec2( g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.x + (m_beizerOffset * 2.0f), m_bezierPanelBounds.m_maxs.x ), 
										g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.y, m_bezierPanelBounds.m_maxs.y ) );

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Spline
	m_splinePoint1  = Vec2( g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.x, m_splinePanelBounds.m_maxs.x ),
				 			g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.y, m_splinePanelBounds.m_maxs.y ) );

	m_splinePoint2  = Vec2( g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.x, m_splinePanelBounds.m_maxs.x ),
				 			g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.y, m_splinePanelBounds.m_maxs.y + m_offsetY ) );

	m_splinePoint3  = Vec2( g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.x, m_splinePanelBounds.m_maxs.x ),
				 			g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.y, m_splinePanelBounds.m_maxs.y ) );

	m_splinePoint4  = Vec2( g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.x, m_splinePanelBounds.m_maxs.x ),
				 			g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.y, m_splinePanelBounds.m_maxs.y + m_offsetY ) );

	std::vector<Vec2> positions;
	positions.push_back(m_splinePoint1);
	positions.push_back(m_splinePoint2);
	positions.push_back(m_splinePoint3);
	positions.push_back(m_splinePoint4);
	m_spline = Spline( positions );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBezierCurves::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	DetermineEasingResult();
	UpdatePauseQuitAndSlowMo();
	UpdateGameCameraBezier();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBezierCurves::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBezierCurves::Reshuffle()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Reshuffle Bezier Curve positions
	m_easingModeIndex = g_theRNG->RollRandomIntInRange( 0, m_maxGameModeIndex ); 

	//----------------------------------------------------------------------------------------------------------------------
	// Reshuffle Bezier Curve positions
	m_cubicBezier.m_startPos	= Vec2( g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.x, m_bezierPanelBounds.m_maxs.x ), 
										g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.y, m_bezierPanelBounds.m_maxs.y ) );

	m_cubicBezier.m_guidePos1	= Vec2( g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.x + m_beizerOffset, m_bezierPanelBounds.m_maxs.x ), 
										g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.y, m_bezierPanelBounds.m_maxs.y ) );
	
	m_cubicBezier.m_guidePos2	= Vec2( g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.x + (m_beizerOffset * 1.0f), m_bezierPanelBounds.m_maxs.x ), 
										g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.y, m_bezierPanelBounds.m_maxs.y ) );
	
	m_cubicBezier.m_endPos		= Vec2( g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.x + (m_beizerOffset * 2.0f), m_bezierPanelBounds.m_maxs.x ), 
										g_theRNG->RollRandomFloatInRange( m_bezierPanelBounds.m_mins.y, m_bezierPanelBounds.m_maxs.y ) );			

	//----------------------------------------------------------------------------------------------------------------------
	// Reshuffle Spline positions
	m_splinePoint1  = Vec2( g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.x, m_splinePanelBounds.m_maxs.x ),
				 			g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.y, m_splinePanelBounds.m_maxs.y ) );

	m_splinePoint2  = Vec2( g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.x, m_splinePanelBounds.m_maxs.x ),
				 			g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.y, m_splinePanelBounds.m_maxs.y + m_offsetY ) );

	m_splinePoint3  = Vec2( g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.x, m_splinePanelBounds.m_maxs.x ),
				 			g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.y, m_splinePanelBounds.m_maxs.y ) );

	m_splinePoint4  = Vec2( g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.x, m_splinePanelBounds.m_maxs.x ),
				 			g_theRNG->RollRandomFloatInRange( m_splinePanelBounds.m_mins.y, m_splinePanelBounds.m_maxs.y + m_offsetY ) );

	std::vector<Vec2> positions;
	positions.push_back(m_splinePoint1);
	positions.push_back(m_splinePoint2);
	positions.push_back(m_splinePoint3);
	positions.push_back(m_splinePoint4);
	m_spline = Spline( positions );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBezierCurves::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBezierCurves::UpdateGameCameraBezier()
{
	m_bezierWorldCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
	   m_bezierUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBezierCurves::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_bezierWorldCamera );

	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> textVerts;

	//----------------------------------------------------------------------------------------------------------------------
	if ( m_debugBackground )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Render topLeft box (Easing)
		AddVertsForAABB2D( verts, m_easingPanelBounds, Rgba8::RED );	
		// Render topRight box (Bezier Curve)
		AddVertsForAABB2D( verts, m_bezierPanelBounds, Rgba8::DARK_GREEN );
		// Render bottom box (Spline)
		AddVertsForAABB2D( verts, m_splinePanelBounds, Rgba8::DARK_BLUE );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Easing functions
	// Rendering Easing bounds
	AddVertsForAABB2D( verts, m_easingGraphBounds, Rgba8::DARK_BLUE );
	
	// Calculate and render curved line graph
	for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
	{
		// Calculate subdivisions
		float tPerSubdivision		= 1.0f / static_cast<float>(m_numSubdivisions);
		float tAtStartOfLineSegment	= tPerSubdivision * static_cast<float>(subdivisionIndex);
		float tAtEndOfLineSegment	= tAtStartOfLineSegment + tPerSubdivision;

		// Calculate U(s)
		float startU = tAtStartOfLineSegment;
		float startV = DetermineEasingFunction( startU );

		// Calculate V(s)
		float endU   = tAtEndOfLineSegment;
		float endV	 = DetermineEasingFunction( endU );

		// Get start and end points at UV
		Vec2 startXY = m_easingGraphBounds.GetPointAtUV( Vec2(startU, startV) );
		Vec2 endXY	 = m_easingGraphBounds.GetPointAtUV( Vec2(  endU,   endV) );

		// Render curved line graph
		AddVertsForLineSegment2D( verts, startXY, endXY, m_thickness, Rgba8::CYAN );
	}

	// Render lines at UV points
	float easingPointU = m_parametricT;
	float easingPointV = m_easingResult; 
	Vec2 pointOnCurve  = m_easingGraphBounds.GetPointAtUV( Vec2(easingPointU, easingPointV) );
	AddVertsForLineSegment2D( verts, pointOnCurve, Vec2(m_easingGraphBounds.m_mins.x,				pointOnCurve.y), m_thickness, Rgba8::TRANSLUCENT_WHITE );
	AddVertsForLineSegment2D( verts, pointOnCurve, Vec2(			  pointOnCurve.x, m_easingGraphBounds.m_mins.y), m_thickness, Rgba8::TRANSLUCENT_WHITE );

	// Render point on curve
	AddVertsForDisc2D( verts, pointOnCurve, m_thickness, Rgba8::WHITE );

	// Render easing function name text
	float cellHeight		= 2.5f;
	Vec2  textMins			= Vec2( m_easingGraphBounds.m_mins.x + 2.0f, m_easingGraphBounds.m_mins.y - (cellHeight * 1.5f) );
	g_theApp->m_textFont->AddVertsForText2D( textVerts, textMins, cellHeight, m_easingName );

	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	// Bezier Curves
	// Rendering Easing bounds
	Vec2 bezierDotPos = m_cubicBezier.GetPointAtTime( m_parametricT );
	// Calculate and render curved line graph
	Vec2 previousBezierDotPos = m_cubicBezier.m_startPos;
	for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
	{
		// Calculate subdivisions
		float t						= (1.0f / static_cast<float>(m_numSubdivisions)) * (subdivisionIndex + 1);
		Vec2 currentBezierDotPos	= m_cubicBezier.GetPointAtTime( t );
		
		// Render curved line graph
		AddVertsForLineSegment2D( verts, previousBezierDotPos, currentBezierDotPos, m_thickness, Rgba8::GREEN );
		previousBezierDotPos = currentBezierDotPos;
	}
	
	AddVertsForLineSegment2D( verts,  m_cubicBezier.m_startPos, m_cubicBezier.m_guidePos1, m_thickness, Rgba8::CYAN );
	AddVertsForLineSegment2D( verts, m_cubicBezier.m_guidePos1, m_cubicBezier.m_guidePos2, m_thickness, Rgba8::CYAN );
	AddVertsForLineSegment2D( verts, m_cubicBezier.m_guidePos2,	   m_cubicBezier.m_endPos, m_thickness, Rgba8::CYAN );

	AddVertsForDisc2D( verts,  m_cubicBezier.m_startPos, m_thickness, Rgba8::BLUE );
	AddVertsForDisc2D( verts, m_cubicBezier.m_guidePos1, m_thickness, Rgba8::BLUE );
	AddVertsForDisc2D( verts, m_cubicBezier.m_guidePos2, m_thickness, Rgba8::BLUE );
	AddVertsForDisc2D( verts,    m_cubicBezier.m_endPos, m_thickness, Rgba8::BLUE );
	AddVertsForDisc2D( verts,			   bezierDotPos, m_thickness, Rgba8::WHITE );

	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	// Splines
	// Rendering Easing bounds
	for ( int i = 0; i < m_spline.m_bezierCurvesList.size(); i++ )
	{
		CubicBezierCurve2D bezierCurve2D = m_spline.m_bezierCurvesList[i];

		Vec2 splineBezierDotPos = bezierCurve2D.GetPointAtTime( m_parametricT );

		// Calculate and render curved line graph
		Vec2 previousSplineBezierDotPos = bezierCurve2D.m_startPos;
		for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
		{
			// Calculate subdivisions
			float t							= (1.0f / static_cast<float>(m_numSubdivisions)) * (subdivisionIndex + 1);
			Vec2 currentSplineBezierDotPos	= bezierCurve2D.GetPointAtTime( t );

			// Render curved line graph
			AddVertsForLineSegment2D( verts, previousSplineBezierDotPos, currentSplineBezierDotPos, m_thickness, Rgba8::GREEN );
			previousSplineBezierDotPos = currentSplineBezierDotPos;
		}

		AddVertsForLineSegment2D( verts,  bezierCurve2D.m_startPos, bezierCurve2D.m_guidePos1, m_thickness, Rgba8::CYAN );
		AddVertsForLineSegment2D( verts, bezierCurve2D.m_guidePos1, bezierCurve2D.m_guidePos2, m_thickness, Rgba8::CYAN );
		AddVertsForLineSegment2D( verts, bezierCurve2D.m_guidePos2,	   bezierCurve2D.m_endPos, m_thickness, Rgba8::CYAN );

		AddVertsForDisc2D( verts,  bezierCurve2D.m_startPos, m_thickness, Rgba8::BLUE );
		AddVertsForDisc2D( verts, bezierCurve2D.m_guidePos1, m_thickness, Rgba8::BLUE );
		AddVertsForDisc2D( verts, bezierCurve2D.m_guidePos2, m_thickness, Rgba8::BLUE );
		AddVertsForDisc2D( verts,    bezierCurve2D.m_endPos, m_thickness, Rgba8::BLUE );
		AddVertsForDisc2D( verts,		 splineBezierDotPos, m_thickness, Rgba8::WHITE );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// End world camera
	g_theRenderer->EndCamera( m_bezierWorldCamera );

	// Draw for world objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );

	// Draw for world text
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBezierCurves::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_bezierUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	float cellHeight	= 2.0f;
	AABB2 textbox1		= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, "Mode (F6/F7 for prev/next): Easing, Curves, Splines (2D)", Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );
	
	AABB2 textbox2		= AABB2( textbox1.m_mins, Vec2( textbox1.m_maxs.x, textbox1.m_maxs.y - cellHeight ) );
	std::string UItext  = Stringf( "F8 to randomize; W/E = prev/next Easing function; N/M = curve subdivisions (%d), hold T = slow, 1 to toggle background panels", m_numSubdivisions );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, UItext, Rgba8::CYAN, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_bezierUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw for UI camera
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBezierCurves::UpdatePauseQuitAndSlowMo()
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

	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		m_debugBackground = !m_debugBackground;
	}
	
	if ( g_theInput->WasKeyJustPressed( 'W' ) )
	{
		m_easingModeIndex--;
		if ( m_easingModeIndex <= 0 )
		{
			m_easingModeIndex = m_maxGameModeIndex;
		}
	}

	if ( g_theInput->WasKeyJustPressed( 'E' ) )
	{
		m_easingModeIndex++;
		if ( m_easingModeIndex >= m_maxGameModeIndex )
		{
			m_easingModeIndex = 0;
		}
	}
	
	if ( g_theInput->WasKeyJustPressed( 'N' ) )
	{
		if ( m_numSubdivisions <= 1 )
		{
			return;
		}
		else
		{
			m_numSubdivisions /= 2;
		}
	}

	if ( g_theInput->WasKeyJustPressed( 'M' ) )
	{
		m_numSubdivisions *= 2;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBezierCurves::DetermineEasingResult()
{
	float totalSeconds  = m_gameClock.GetTotalSeconds();
	m_parametricT		= fmodf( totalSeconds * 0.5f, 1.0f );
//	m_parametricT		= fmodf( totalSeconds, 1.0f );

	switch ( m_easingModeIndex )
	{
	default:
		m_easingResult = m_parametricT;					// Identity
		m_easingName   = std::string( "Identity" );
		break;
	case 1:
		m_easingResult = SmoothStart2( m_parametricT );
		m_easingName   = std::string( "SmoothStart 2" );
		break;
	case 2:
		m_easingResult = SmoothStart3( m_parametricT );
		m_easingName = std::string( "SmoothStart 3" );
		break;
	case 3:
		m_easingResult = SmoothStart4( m_parametricT );
		m_easingName = std::string( "SmoothStart 4" );
		break;
	case 4:
		m_easingResult = SmoothStart5( m_parametricT );
		m_easingName = std::string( "SmoothStart 5" );
		break;
	case 5:
		m_easingResult = SmoothStart6( m_parametricT );
		m_easingName = std::string( "SmoothStart 6" );
		break;
	case 6:
		m_easingResult = SmoothStop2( m_parametricT );
		m_easingName = std::string( "SmoothStop 2" );
		break;
	case 7:
		m_easingResult = SmoothStop3( m_parametricT );
		m_easingName = std::string( "SmoothStop 3" );
		break;
	case 8:
		m_easingResult = SmoothStop4( m_parametricT );
		m_easingName = std::string( "SmoothStop 4" );
		break;
	case 9:
		m_easingResult = SmoothStop5( m_parametricT );
		m_easingName = std::string( "SmoothStop 5" );
		break;
	case 10:
		m_easingResult = SmoothStop6( m_parametricT );
		m_easingName = std::string( "SmoothStop 6" );
		break;
	case 11:
		m_easingResult = SmoothStep3( m_parametricT );
		m_easingName = std::string( "SmoothStep 3" );
		break;
	case 12:
		m_easingResult = SmoothStep5( m_parametricT );
		m_easingName = std::string( "SmoothStep 5" );
		break;
	case 13:
		m_easingResult = Hesitate3( m_parametricT );
		m_easingName = std::string( "Hesitate 3" );
		break;
	case 14:
		m_easingResult = Hesitate5( m_parametricT );
		m_easingName = std::string( "Hesitate 5" );
		break;
	case 15:
		m_easingResult = CustomFunkyEasingFunction( m_parametricT );
		m_easingName = std::string( "CustomFunky" );
		break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
float GameModeBezierCurves::DetermineEasingFunction( float t ) const
{
	float result = -1.0f;
	switch ( m_easingModeIndex )
	{
	default:
		result = t;						// Identity
		break;
	case 1:
		result = SmoothStart2( t );
		break;
	case 2:
		result = SmoothStart3( t );
		break;
	case 3:
		result = SmoothStart4( t );
		break;
	case 4:
		result = SmoothStart5( t );
		break;
	case 5:
		result = SmoothStart6( t );
		break;
	case 6:
		result = SmoothStop2( t );
		break;
	case 7:
		result = SmoothStop3( t );
		break;
	case 8:
		result = SmoothStop4( t );
		break;
	case 9:
		result = SmoothStop5( t );
		break;
	case 10:
		result = SmoothStop6( t );
		break;
	case 11:
		result = SmoothStep3( t );
		break;
	case 12:
		result = SmoothStep5( t );
		break;
	case 13:
		result = Hesitate3( t );
		break;
	case 14:
		result = Hesitate5( t );
		break;
	case 15:
		result = CustomFunkyEasingFunction( t );
		break;
	};

	return result;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 GameModeBezierCurves::GetSplinePointAtTime( float t ) const
{
	int totalSegments			= int(m_spline.m_bezierCurvesList.size());
	float splineParametric		= totalSegments * t;
	int curveIndex				= int(splineParametric);
	float curveLocalParametric	= splineParametric - curveIndex;

	CubicBezierCurve2D curve = m_spline.m_bezierCurvesList[curveIndex];
	Vec2 pointOnSpline		 = curve.GetPointAtTime( curveLocalParametric );
	return pointOnSpline;
}
