#include "Game/GameModeRaycastVsOBB3D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeRaycastVsOBB3D::GameModeRaycastVsOBB3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameModeRaycastVsOBB3D::~GameModeRaycastVsOBB3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB3D::Startup()
{
	// Initializing textures;
	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );

	for ( int i = 0; i < m_numBoxes; i++ )
	{
		float randX = g_theRNG->RollRandomFloatInRange( -50.0f, 50.0f );
		float randY = g_theRNG->RollRandomFloatInRange( -50.0f, 50.0f );
		float randZ = g_theRNG->RollRandomFloatInRange( -50.0f, 50.0f );
		OBB3D* box = new OBB3D( Vec3( randX, randY, randZ ), Vec3( 1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ), Vec3( 0.0f, 0.0f, 1.0f ), Vec3( 10.0f, 10.0f, 10.0f ) );
		m_obb3List.push_back( box );
	}

	m_rayStartPos	= Vec3::ZERO;
	m_rayEndPos		= Vec3( 20.0f, 0.0f, 0.0f );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB3D::Update( float deltaSeconds )
{
	deltaSeconds = m_clock.GetDeltaSeconds();

	// Implement camera disp to mousePos
	Vec2 cursorClientDelta	= g_theInput->GetCursorClientDelta();
	float mouseSpeed		= 0.05f;
	float yaw				= cursorClientDelta.x * mouseSpeed;
	float pitch				= cursorClientDelta.y * mouseSpeed;

	m_angularVelocity.m_yawDegrees	 -= yaw;
	m_angularVelocity.m_pitchDegrees += pitch;

	UpdateGameCameraTestShapes3D();
	UpdateInputForCameraMovement( deltaSeconds );
	UpdatePauseQuitAndSlowMo();
	MoveRaycastInput( deltaSeconds );
	UpdateRaycastResult3D();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB3D::Render() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin Camera
	g_theRenderer->BeginCamera( m_worldCamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Create verts
	std::vector<Vertex_PCU> texturedVerts;
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> wireframeVerts;

	//----------------------------------------------------------------------------------------------------------------------
	// "GameModeRaycastVsAABB3D" title
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 textOrigin = Vec3(	 50.0f,  50.0f,  10.0f );
	Vec3 iBasis		= Vec3(	  0.0f,  -1.0f,   0.0f );
	Vec3 jBasis		= Vec3(	  0.0f,   0.0f,   1.0f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, textOrigin, iBasis, jBasis, 5.0f, "GameModeRaycastVsOBB3D!", Rgba8::GREEN );

//	AddVertsForOBB3D( verts, *m_obb3, Rgba8::MAGENTA );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Quad
	for ( int i = 0; i < m_obb3List.size(); i++ )
	{
		AddVertsForOBB3D( verts, *m_obb3List[i], Rgba8::DARK_CYAN );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render stationary world compass
	AddVertsForCompass( verts, Vec3::ZERO, 1.0f, 0.08f );

	//----------------------------------------------------------------------------------------------------------------------
	// Render player compass
	Mat44 playerMatrix  = m_worldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 playerForward	= playerMatrix.GetIBasis3D();
	float distFromCam	= 0.2f;

	Vec3 playerCenter		  = m_worldCamera.m_position + ( playerForward * distFromCam );
	float playerCompassLength = 0.01f;
	float playerAxisThickness = 0.001f;
	AddVertsForCompass( verts, playerCenter, playerCompassLength, playerAxisThickness );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Raycast arrow
	// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
	if ( m_didAABB3Impact )
	{
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_rayStartPos, m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_updatedImpactPos, ( m_arrowThickness * 3.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_updatedImpactPos, m_updatedImpactPos + ( m_updatedImpactNormal * 1.0f ), ( m_arrowThickness * 2.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_rayStartPos, m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for World	// Rendering spheres with test texture
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );	
	
	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for World	// Rendering spheres with test texture
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( m_testTexture );
	g_theRenderer->DrawVertexArray( static_cast<int>( texturedVerts.size() ), texturedVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for world Compass
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for wireFrame Objects (Sphere, AABB3, Cylinder)
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_NONE );
	g_theRenderer->DrawVertexArray( static_cast<int>( wireframeVerts.size() ), wireframeVerts.data() );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );

	//----------------------------------------------------------------------------------------------------------------------
	// End Camera
	g_theRenderer->EndCamera( m_worldCamera );

	RenderUIStuff();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB3D::Reshuffle() 
{
	//----------------------------------------------------------------------------------------------------------------------
	// Object variables
	// Quad
//	m_bounds = AABB3( Vec3( g_theRNG->RollRandomFloatInRange( 5.0f, 30.0f ), g_theRNG->RollRandomFloatInRange( 5.0f, 10.0f ), g_theRNG->RollRandomFloatInRange( 5.0f, 10.0f ) ), 
//					  Vec3( g_theRNG->RollRandomFloatInRange( 30.0f, 45.0f ), g_theRNG->RollRandomFloatInRange( 20.0f, 25.0f ), g_theRNG->RollRandomFloatInRange( 20.0f, 25.0f ) ) );

/*
	for ( int i = 0; i < m_aabb3List.size(); i++ )
	{
		float randX = g_theRNG->RollRandomFloatInRange( -50.0f, 50.0f );
		float randY = g_theRNG->RollRandomFloatInRange( -50.0f, 50.0f );
		float randZ = g_theRNG->RollRandomFloatInRange( -50.0f, 50.0f );
		m_aabb3List[i]->SetCenterXYZ( Vec3( randX, randY, randZ ) );
	}
*/
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB3D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB3D::UpdateGameCameraTestShapes3D()
{
	m_worldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 1000.0f  );
//	m_player->m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f ), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) );
	m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f ), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) );
	m_uiCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( SCREEN_SIZE_X, SCREEN_SIZE_Y ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB3D::UpdateInputForCameraMovement( float deltaSeconds)
{
	EulerAngles orientationPerFrame = EulerAngles( m_turnRate, m_turnRate, m_turnRate );

	Vec3 forward;
	Vec3 left;
	Vec3 up;

	m_worldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( forward, left, up );

	//----------------------------------------------------------------------------------------------------------------------
	// Forward
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
		m_worldCamera.m_position += ( m_speed * forward * deltaSeconds );
	}
	// Back
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_worldCamera.m_position -= ( m_speed * forward * deltaSeconds );
	}
	// Left
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_worldCamera.m_position += ( m_speed * left * deltaSeconds );
	}
	// Right
	if ( g_theInput->IsKeyDown( 'F' ) )
	{
		m_worldCamera.m_position -= ( m_speed * left * deltaSeconds );
	}
/*
	// Pitch up
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) )
	{
//		m_testShapesWorldCamera.m_orientation.m_pitchDegrees -= ( m_angularVelocity.m_pitchDegrees * deltaSeconds );
		m_angularVelocity.m_pitchDegrees -= ( orientationPerFrame.m_pitchDegrees * deltaSeconds );
	}

	// Pitch down
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) )
	{
//		m_testShapesWorldCamera.m_orientation.m_pitchDegrees += ( m_angularVelocity.m_pitchDegrees * deltaSeconds );
		m_angularVelocity.m_pitchDegrees += ( orientationPerFrame.m_pitchDegrees * deltaSeconds );
	}

	// Yaw left
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) )
	{
//		m_testShapesWorldCamera.m_orientation.m_yawDegrees += ( m_angularVelocity.m_yawDegrees * deltaSeconds );
		m_angularVelocity.m_yawDegrees += ( orientationPerFrame.m_yawDegrees * deltaSeconds );
	}

	// Yaw right 
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
	{
//		m_testShapesWorldCamera.m_orientation.m_yawDegrees -= ( m_angularVelocity.m_yawDegrees * deltaSeconds );
		m_angularVelocity.m_yawDegrees -= ( orientationPerFrame.m_yawDegrees * deltaSeconds );
	}
*/

	// Elevate
	if ( g_theInput->IsKeyDown( 'R' ) || g_theInput->IsKeyDown( 'Q' ) || g_theInput->IsKeyDown( 'A' ))
	{
		m_worldCamera.m_position.z += ( m_speed * deltaSeconds );
	}

	// De-Elevate
	if ( g_theInput->IsKeyDown( 'W' ) || g_theInput->IsKeyDown( 'Z' ) )
	{
		m_worldCamera.m_position.z -= ( m_speed * deltaSeconds );
	}

	// Speed up speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_speed = m_doublespeed;
	}
	// Speed up speed variable
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_speed = m_defaultSpeed;
	}

	// Slow down speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_CONTROL ) )
	{
		m_speed = m_halfspeed;
	}

	// Slow down speed variable
	if ( g_theInput->WasKeyJustReleased( KEYCODE_CONTROL ) )
	{
		m_speed = m_defaultSpeed;
	}

	// Slow-Mo
	if ( g_theInput->IsKeyDown( 'T' ) )
	{
		m_worldCamera.m_position.z -= ( m_elevateSpeed * deltaSeconds );
	}

//	m_orientationDegrees = m_angularVelocity;
//	m_orientationDegrees.m_pitchDegrees = GetClamped( m_orientationDegrees.m_pitchDegrees, -89.9f, 89.9f );

	m_worldCamera.m_orientation = m_angularVelocity;
	m_worldCamera.m_orientation.m_pitchDegrees = GetClamped( m_worldCamera.m_orientation.m_pitchDegrees, -89.9f, 89.9f );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB3D::RenderUIStuff() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_uiCamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;

	float cellHeight = 2.0f;
	AABB2 textbox1 = AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	AABB2 textbox2 = AABB2( textbox1.m_mins, Vec2( textbox1.m_maxs.x, textbox1.m_maxs.y - cellHeight ) );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, "Mode (F6/F7 for prev/next): Raycast Vs OBB 3D (3D)", Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, "F8 to randomize; ESDF = fly horizontal, WR/QZ/AZ = fly vertical, hold T slow", Rgba8::GREEN, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, "IJKLUO: Move entire ray, arrow keys + NM: move rayEndPos", Rgba8::GREEN, 0.75f, Vec2( 0.0f, 0.95f ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, Stringf( "camPos X: %0.2f, Y: %0.2f, Z: %0.2f", m_worldCamera.m_position.x, m_worldCamera.m_position.y, m_worldCamera.m_position.z ), Rgba8::GREEN, 0.75f, Vec2( 0.0f, 0.9f ), TextDrawMode::SHRINK_TO_FIT );

	//----------------------------------------------------------------------------------------------------------------------
	// Render playerCompass
	std::vector<Vertex_PCU> playerCompassVerts;

	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_uiCamera  );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for UI camera // Rendering texts
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for playerCompass 
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( playerCompassVerts.size() ), playerCompassVerts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB3D::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if ( g_theInput->WasKeyJustPressed( 'P' ) || g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::BUTTON_START ) )
	{
		SoundID testSound = g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
		g_theAudio->StartSound( testSound );			// Comment out this line of code to remove pause sound playing

		m_clock.TogglePause();
	}

	// Slow-Mo functionality
	m_isSlowMo = g_theInput->IsKeyDown( 'T' );
	if ( m_isSlowMo )
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
void GameModeRaycastVsOBB3D::AddVertsForCompass( std::vector<Vertex_PCU>& compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render stationary world compass
	// X
	Vec3 endPosX		 = startPosition + Vec3( axisLength, 0.0f, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosX, axisThickness, Rgba8::RED );
	// Y
	Vec3 endPosY		 = startPosition + Vec3( 0.0f, axisLength, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosY, axisThickness, Rgba8::GREEN );
	// Z
	Vec3 endPosZ		 = startPosition + Vec3( 0.0f, 0.0f, axisLength );
	AddVertsForArrow3D( compassVerts, startPosition, endPosZ, axisThickness, Rgba8::BLUE );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB3D::UpdateRaycastResult3D()
{
	m_didAABB3Impact = false;

	Vec3 startEndDisp	= m_rayEndPos - m_rayStartPos;
	float rayMaxLength	= startEndDisp.GetLength();
	Vec3 rayfwdNormal	= startEndDisp.GetNormalized();

	float superDist = 500.0f;
/*
	for ( int i = 0; i < m_obb3List.size(); i++ )
	{
		// Check if raycast impacted a lineSegment
		m_raycastVsAABB2Result3D = RaycastVsAABB3D( m_rayStartPos, rayfwdNormal, rayMaxLength, *m_aabb3List[i] );
		if ( m_raycastVsAABB2Result3D.m_didImpact )
		{
			Vec3 distFromCurrentLineToRay = m_raycastVsAABB2Result3D.m_impactPos - m_rayStartPos;
			if ( distFromCurrentLineToRay.GetLength() < superDist )
			{
				// Check for closest line segment to raycast 
				superDist				= distFromCurrentLineToRay.GetLength();
				m_currentLine			= i;
				m_didAABB3Impact		= true;
				// Use the updated values below ( impactPos and impactNormal ) for rendering raycast 
				m_updatedImpactPos		= m_raycastVsAABB2Result3D.m_impactPos;
				m_updatedImpactNormal	= m_raycastVsAABB2Result3D.m_impactNormal;
			}
		}
	}
*/
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeRaycastVsOBB3D::MoveRaycastInput( float deltaSeconds )
{
	float moveSpeed = 10.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Move entire raycast
	//----------------------------------------------------------------------------------------------------------------------
	// Move North
	if ( g_theInput->IsKeyDown( 'J' ) )
	{
		m_rayStartPos += Vec3( 0.0f, moveSpeed, 0.0f ) * deltaSeconds; 
		m_rayEndPos   += Vec3( 0.0f, moveSpeed, 0.0f ) * deltaSeconds;
	}
	// Move South
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		m_rayStartPos += Vec3( 0.0f, -moveSpeed, 0.0f ) * deltaSeconds;
		m_rayEndPos   += Vec3( 0.0f, -moveSpeed, 0.0f ) * deltaSeconds;
	}
	// Move East
	if ( g_theInput->IsKeyDown( 'I' ) )
	{
		m_rayStartPos += Vec3( moveSpeed, 0.0f, 0.0f ) * deltaSeconds;
		m_rayEndPos   += Vec3( moveSpeed, 0.0f, 0.0f ) * deltaSeconds;
	}
	// Move West
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		m_rayStartPos += Vec3( -moveSpeed, 0.0f, 0.0f ) * deltaSeconds;
		m_rayEndPos   += Vec3( -moveSpeed, 0.0f, 0.0f ) * deltaSeconds;
	}
	// Move Skyward
	if ( g_theInput->IsKeyDown( 'O' ) )
	{
		m_rayStartPos += Vec3( 0.0f, 0.0f, moveSpeed ) * deltaSeconds;
		m_rayEndPos   += Vec3( 0.0f, 0.0f, moveSpeed ) * deltaSeconds;
	}
	// Move Groundward
	if ( g_theInput->IsKeyDown( 'U' ) )
	{
		m_rayStartPos += Vec3( 0.0f, 0.0f, -moveSpeed ) * deltaSeconds;
		m_rayEndPos   += Vec3( 0.0f, 0.0f, -moveSpeed ) * deltaSeconds;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// East
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) )
	{
		m_rayEndPos += Vec3( moveSpeed, 0.0f, 0.0f ) * deltaSeconds; 
	}
	// West
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) )
	{
		m_rayEndPos += Vec3( -moveSpeed, 0.0f, 0.0f ) * deltaSeconds; 
	}
	// North
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) )
	{
		m_rayEndPos += Vec3( 0.0f, moveSpeed, 0.0f ) * deltaSeconds; 
	}
	// South
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
	{
		m_rayEndPos += Vec3( 0.0f, -moveSpeed, 0.0f ) * deltaSeconds; 
	}
	// Skyward
	if ( g_theInput->IsKeyDown( 'M' ) )
	{
		m_rayEndPos += Vec3( 0.0f, 0.0f, moveSpeed ) * deltaSeconds;
	}
	// Groundward
	if ( g_theInput->IsKeyDown( 'N' ) )
	{
		m_rayEndPos += Vec3( 0.0f, 0.0f, -moveSpeed ) * deltaSeconds;
	}
}
 
//----------------------------------------------------------------------------------------------------------------------
// code example for TGP squashing fish while boosting
//	float timeToMaxSquash = 3.f;
//	float driftTime = 0.f; //0-length of time you've been holding drift
//	
//	float currentSquashAmount = RangeMapClamped(driftTime, 0.f, timeToMaxSquash, -1.f, 1.f);
//	SetSquash(currentSquashAmount);
//	
//	// Squash amount is [-1.f, 1.f]
//	void SetSquash(float squashAmount)
//	{
//		m_fishSkeleton.SetMorphTarget("Squash", squashAmount);
//	} 