#include "Game/App.hpp"
#include "Game/GameModeFifaTest3D.hpp"
#include "Game/Player3D.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeFifaTest3D::GameModeFifaTest3D()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Create Football and set position in game
	//----------------------------------------------------------------------------------------------------------------------
	m_football						= new Football3D( this );
//	m_footballDefaultPos			= Vec3( PITCH_CENTER_X, PITCH_CENTER_Y, m_football->m_footballRadius );
//	m_footballDefaultPos			= Vec3( PITCH_CENTER_X, PITCH_CENTER_Y, 1.8f );	// Test code
	m_football->m_footballPosition	= m_footballDefaultPos;

	//----------------------------------------------------------------------------------------------------------------------
	// Create Football and set position in game
	//----------------------------------------------------------------------------------------------------------------------
	m_player1						 = new Player3D( m_football, this );
	m_player1->m_position			 = m_player1DefaultPos;
	m_player1->m_playerOrientation	 = 0.0f;
	m_player1->m_playerIsControlled	 = true;

	m_player2						= new Player3D( m_football, this );
	m_player2->m_position			= m_player2DefaultPos;
	m_player2->m_playerOrientation	= 0.0f;
	m_player2->m_color				= Rgba8::DARK_YELLOW;
	
	m_playerList.push_back(m_player1);
	m_playerList.push_back(m_player2);

	//----------------------------------------------------------------------------------------------------------------------
	// Set world orientation
	//----------------------------------------------------------------------------------------------------------------------
	m_fifaTest3DWorldCamera.m_position					 = m_cameraPerspectiveViewPos;
	m_fifaTest3DWorldCamera.m_orientation.m_yawDegrees	 = m_perspectiveYawDegrees;
	m_fifaTest3DWorldCamera.m_orientation.m_pitchDegrees = m_perspectivePitchDegrees;

	//----------------------------------------------------------------------------------------------------------------------
	// Texture
	//----------------------------------------------------------------------------------------------------------------------
	m_testTexture		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	m_footballTexture	= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Football.png" );
	m_fieldTexture		= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Pitch.png" );

	// Resize verts
//	verts.resize( 30000 );
}

//----------------------------------------------------------------------------------------------------------------------
GameModeFifaTest3D::~GameModeFifaTest3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::Update( float deltaSeconds )
{			
	// Debug code to be deleted later
//	DebuggerPrintf( Stringf("ballPos X: %0.2f, Y: %0.2f, Z: %0.2f  ||  ballVel X: %0.2f, Y: %0.2f, Z: %0.2f ||  ballAccel X: %0.2f, Y: %0.2f, Z: %0.2f ||  ballVelMagnitude: %0.2f, \n", 
//		m_football->m_footballPosition.x,	  m_football->m_footballPosition.y,		m_football->m_footballPosition.z, 
//		m_football->m_footballVelocity.x,	  m_football->m_footballVelocity.y,		m_football->m_footballVelocity.z,
//		m_football->m_footballAcceleration.x, m_football->m_footballAcceleration.y, m_football->m_footballAcceleration.z,
//		m_football->m_footballVelocity.GetLength() ).c_str() );

//	DebuggerPrintf( Stringf("ballAngularVelocity X: %0.2f, Y: %0.2f, Z: %0.2f || footballOrientation yaw: %0.2f, pitch: %0.2f, roll: %0.2f \n", 
//		m_football->m_footballAngularVelocity.x, m_football->m_footballAngularVelocity.y, m_football->m_footballAngularVelocity.z,
//		m_football->m_footballOrientation.m_yawDegrees, m_football->m_footballOrientation.m_pitchDegrees, m_football->m_footballOrientation.m_rollDegrees ).c_str() );

//	DebuggerPrintf( "BallAV X: %0.2f, Y: %0.2f, Z: %0.2f\n", m_football->m_footballAngularVelocity.x, m_football->m_footballAngularVelocity.y, m_football->m_footballAngularVelocity.z );

	//----------------------------------------------------------------------------------------------------------------------
	// Toggle actor possession
	//----------------------------------------------------------------------------------------------------------------------
	XboxController const& controller = g_theInput->GetController( 0 );
	if ( g_theInput->WasKeyJustPressed( 'B' ) || controller.GetButton( LEFT_SHOULDER ).m_wasPressedLastFrame )
	{
		PossessNextActor();
	}
	UpdateDebugKeysInput();
	
	//----------------------------------------------------------------------------------------------------------------------
	// Update using timestep		// Frame rate IN-DEPENDENT physics update = consistent results
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_isVariableTimeStep )
	{
		m_physicsUpdateDebt += deltaSeconds;
		while ( m_physicsUpdateDebt > m_physicsFixedTimeStep )
		{
			m_physicsUpdateDebt -= m_physicsFixedTimeStep;
			if ( !m_isCameraControlled )
			{
				UpdatePlayer( m_physicsFixedTimeStep );
			}

			PredictBallTrajectory( m_physicsFixedTimeStep );
			UpdatePlayerVsPlayerCollision();
			UpdatePlayerVsBallCollision();
			UpdateBallPoc();
			UpdateFootballPhysics( m_physicsFixedTimeStep );
		}
	}
	//----------------------------------------------------------------------------------------------------------------------
	// Update using deltaSeconds	// Frame rate DEPENDENT physics update = inconsistent results
	//----------------------------------------------------------------------------------------------------------------------
	else if ( !m_isVariableTimeStep )
	{
		if ( !m_isCameraControlled )
		{
			UpdatePlayer( deltaSeconds );
		}

		PredictBallTrajectory( deltaSeconds );
		UpdatePlayerVsPlayerCollision();
		UpdatePlayerVsBallCollision();
		UpdateBallPoc();
		UpdateFootballPhysics( deltaSeconds );
	}

	if ( m_isCameraControlled )
	{
		UpdateCameraInput();
	}
	UpdatePauseQuitAndSlowMo();
	UpdateFifaTest3DCamera();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::UpdateCameraInput()
{
	float deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();

	XboxController const& controller = g_theInput->GetController(0);
	EulerAngles orientationPerFrame = EulerAngles( m_turnRate, m_turnRate, m_turnRate );

	Vec3 forward;
	Vec3 left;
	Vec3 up;

	m_fifaTest3DWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( forward, left, up );
	forward.z	= 0.0f;
	left.z		= 0.0f;
	forward		= forward.GetNormalized();
	left		= left.GetNormalized();

	// Increase speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) || g_theInput->IsKeyDown( ' ' ) || controller.GetButton(BUTTON_A).m_isPressed )
	{
		m_currentSpeed = m_fasterSpeed;
	}
	// Reset speed variable to default
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) ||  g_theInput->WasKeyJustReleased( ' ' ) || ( controller.GetButton(BUTTON_A).m_wasPressedLastFrame ) )
	{
		m_currentSpeed = m_defaultSpeed;
	}

	// Decrease speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_CONTROL ) )
	{
		m_currentSpeed = m_slowerSpeed;
	}

	// Reset speed variable to default
	if ( g_theInput->WasKeyJustReleased( KEYCODE_CONTROL ) )
	{
		m_currentSpeed = m_defaultSpeed;
	}
	
	// Forward
	if ( g_theInput->IsKeyDown('W') || controller.GetLeftJoyStick().GetPosition().y > 0.0f )
	{
		m_fifaTest3DWorldCamera.m_position += ( m_currentSpeed * forward * deltaSeconds );
	}

	// Back
	if ( g_theInput->IsKeyDown( 'S' ) ||  controller.GetLeftJoyStick().GetPosition().y < 0.0f )
	{
		m_fifaTest3DWorldCamera.m_position -= ( m_currentSpeed * forward * deltaSeconds );
	}

	// Left
	if ( g_theInput->IsKeyDown( 'A' ) ||  controller.GetLeftJoyStick().GetPosition().x < 0.0f )
	{
		m_fifaTest3DWorldCamera.m_position += ( m_currentSpeed * left * deltaSeconds);
	}

	// Right
	if ( g_theInput->IsKeyDown( 'D' ) ||  controller.GetLeftJoyStick().GetPosition().x > 0.0f )
	{
		m_fifaTest3DWorldCamera.m_position -= ( m_currentSpeed * left * deltaSeconds );
	}

	// Pitch up
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) || controller.GetRightJoyStick().GetPosition().y > 0.0f )
	{
		m_fifaTest3DWorldCamera.m_orientation.m_pitchDegrees -= orientationPerFrame.m_pitchDegrees * deltaSeconds;
	}

	// Pitch down
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) || controller.GetRightJoyStick().GetPosition().y < 0.0f )
	{
		m_fifaTest3DWorldCamera.m_orientation.m_pitchDegrees += orientationPerFrame.m_pitchDegrees * deltaSeconds;

	}
	// Yaw left
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) || controller.GetRightJoyStick().GetPosition().x < 0.0f )
	{
		m_fifaTest3DWorldCamera.m_orientation.m_yawDegrees += orientationPerFrame.m_yawDegrees * deltaSeconds;
	}

	// Yaw right 
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW || controller.GetRightJoyStick().GetPosition().x > 0.0f ) )
	{
		m_fifaTest3DWorldCamera.m_orientation.m_yawDegrees -= orientationPerFrame.m_yawDegrees * deltaSeconds;
	}

	// Elevate
	if ( g_theInput->IsKeyDown( 'Z' )	|| 
		 g_theInput->IsKeyDown( 'Q' )	|| 
		controller.GetButton(LEFT_SHOULDER).m_isPressed  )
	{
		m_fifaTest3DWorldCamera.m_position.z += ( m_currentSpeed * deltaSeconds );
	}

	// De-Elevate
	if ( g_theInput->IsKeyDown( 'C' ) || 
		 g_theInput->IsKeyDown( 'E' ) || 
		 controller.GetButton(RIGHT_SHOULDER).m_isPressed )
	{
		m_fifaTest3DWorldCamera.m_position.z -= ( m_currentSpeed * deltaSeconds );
	}
	m_fifaTest3DWorldCamera.m_orientation.m_pitchDegrees = GetClamped( m_fifaTest3DWorldCamera.m_orientation.m_pitchDegrees, -85.0f, 85.0f );
	m_fifaTest3DWorldCamera.m_orientation.m_rollDegrees  = GetClamped(  m_fifaTest3DWorldCamera.m_orientation.m_rollDegrees, -45.0f, 45.0f );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::UpdateFifaTest3DCamera()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Camera follows mouse
	//----------------------------------------------------------------------------------------------------------------------
	// Implement displacement.x to yaw and displacement.y to pitch
	//----------------------------------------------------------------------------------------------------------------------

	if ( m_isCameraControlled )
	{
		Vec2 cursorClientDelta								  = g_theInput->GetCursorClientDelta();
		float mouseSpeed									  = 0.05f;
		float yaw											  = cursorClientDelta.x * mouseSpeed;
		float pitch											  = cursorClientDelta.y * mouseSpeed;
		m_fifaTest3DWorldCamera.m_orientation.m_yawDegrees	 -= yaw;
		m_fifaTest3DWorldCamera.m_orientation.m_pitchDegrees += pitch;

		m_fifaTest3DWorldCamera.SetTransform( m_fifaTest3DWorldCamera.m_position, m_fifaTest3DWorldCamera.m_orientation );
	}
	// Update camera as TOP-DOWN mode 
	else if ( m_isCameraTopDown )
	{
		m_fifaTest3DWorldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 10000.0f );
		m_fifaTest3DWorldCamera.SetRenderBasis( Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) );
		// Set X
		float clampedCameraPosX = m_football->m_footballPosition.x;
		clampedCameraPosX		= GetClamped( clampedCameraPosX, (PITCH_SIZE_X * 0.2f), (PITCH_SIZE_X * 0.8f) );

		// Set Y
		float clampedCameraPosY = m_football->m_footballPosition.y;
		clampedCameraPosY		= GetClamped( clampedCameraPosY, (PITCH_SIZE_Y * 0.2f), (PITCH_SIZE_Y * 0.8f) );
//		clampedCameraPosY		= m_football->m_footballPosition.y - m_cameraHeightYSouthOfBall;

		// Set Z
//		float clampedCameraPosZ = m_football->m_footballPosition.z + 30.0f;
		float clampedCameraPosZ = 30.0f;

		m_fifaTest3DWorldCamera.SetTransform( Vec3( clampedCameraPosX, clampedCameraPosY, clampedCameraPosZ), EulerAngles( m_topDownYaw, m_topDownPitch, m_topDownRoll ) );
//		m_fifaTest3DWorldCamera.SetTransform( m_cameraTopDownViewPos, EulerAngles( m_topDownYaw, m_topDownPitch, m_topDownRoll ) );
		m_fifaTest3DUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );	
	}
	else	// Update camera as perspective
	{
		m_fifaTest3DWorldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 10000.0f );
		m_fifaTest3DWorldCamera.SetRenderBasis( Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) );
		
		float clampedCameraPosX = m_football->m_footballPosition.x;
		clampedCameraPosX		= GetClamped( clampedCameraPosX, (PITCH_SIZE_X * 0.2f), (PITCH_SIZE_X * 0.8f) );
		float clampedCameraPosY = -10.0f;
		clampedCameraPosY		= m_football->m_footballPosition.y - m_cameraHeightYSouthOfBall;
		float clampedCameraPosZ = m_football->m_footballPosition.z + m_cameraHeightZAboveBall;
		m_fifaTest3DWorldCamera.SetTransform( Vec3( clampedCameraPosX, clampedCameraPosY, clampedCameraPosZ), EulerAngles( m_perspectiveYawDegrees, m_perspectivePitchDegrees, m_perspectiveRollDegrees ) );
//		m_fifaTest3DWorldCamera.SetTransform( Vec3( clampedCameraPosX, -10.0f, 650.0f), EulerAngles( m_perspectiveYawDegrees, m_perspectivePitchDegrees, m_perspectiveRollDegrees ) );
//		m_fifaTest3DWorldCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
		m_fifaTest3DUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );		
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->BeginCamera( m_fifaTest3DWorldCamera );

	// Initialize common render variables
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> playerVerts;
	std::vector<Vertex_PCU> fieldVerts;
	std::vector<Vertex_PCU> footballVerts;
	Rgba8 fieldBoundaryColor	= Rgba8::WHITE; 
	Rgba8 fieldColor			= Rgba8::WHITE; 
//	float thickness				= 0.5f;

	// Old code for playerCompass
//	float distFromCam			= 0.2f;
//	Mat44 cameraMatrix			= m_fifaTest3DWorldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
//	Vec3 cameraForward			= cameraMatrix.GetIBasis3D();
//	Vec3 cameraCenter			= m_fifaTest3DWorldCamera.m_position + ( cameraForward * distFromCam );
//	float playerCompassLength	= 0.01f;
//	float playerAxisThickness	= 0.001f;
//	AddVertsForCompass( verts, cameraCenter, playerCompassLength, playerAxisThickness );

	// Render world compass
	AddVertsForCompass( verts, Vec3::ZERO, 20.0f, 0.5f );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Field quad
//	AABB2 fieldBounds = AABB2( Vec2::ZERO, Vec2( PITCH_SIZE_X, PITCH_SIZE_Y ) );
//	fieldBounds.AddPadding( PITCH_SIZE_X * 0.01f, PITCH_SIZE_Y * 0.05f );
//	AddVertsForAABB2D( fieldVerts, fieldBounds, Rgba8::YELLOW );
	
	AABB3 fieldBounds = AABB3( Vec3( 0.0f, 0.0f, -1.0f ), Vec3( PITCH_SIZE_X, PITCH_SIZE_Y, -0.5f ) );
	AddVertsForAABB3D( fieldVerts, fieldBounds, fieldColor );

	//----------------------------------------------------------------------------------------------------------------------
	// Render white field boundary lines
//	float offsetX = 2.0f;
//	float offsetY = 2.0f;
//	Vec2 field_BL = Vec2( fieldBounds.m_mins.x + offsetX, fieldBounds.m_mins.y + offsetY );		// Bottom Left
//	Vec2 field_BR = Vec2( fieldBounds.m_maxs.x - offsetX, fieldBounds.m_mins.y + offsetY );		// Bottom Right
//	Vec2 field_TL = Vec2( fieldBounds.m_mins.x + offsetX, fieldBounds.m_maxs.y - offsetY );		// Top Left
//	Vec2 field_TR = Vec2( fieldBounds.m_maxs.x - offsetX, fieldBounds.m_maxs.y - offsetY );		// Top Right
//	Vec2 field_BM = field_BL + ( ( field_BR - field_BL ) * 0.5f );								// Bottom Middle
//	Vec2 field_TM = field_TL + ( ( field_TR - field_TL ) * 0.5f );								// Top Middle
//
//	AddVertsForLineSegment2D( verts, field_BL, field_BR, thickness, fieldBoundaryColor );		// BL to BR
//	AddVertsForLineSegment2D( verts, field_TL, field_TR, thickness, fieldBoundaryColor );		// TL to TR
//	AddVertsForLineSegment2D( verts, field_BL, field_TL, thickness, fieldBoundaryColor );		// BL to TL
//	AddVertsForLineSegment2D( verts, field_BR, field_TR, thickness, fieldBoundaryColor );		// BR to TR
//	AddVertsForLineSegment2D( verts, field_BR, field_TR, thickness, fieldBoundaryColor );		// BR to TR
//	AddVertsForLineSegment2D( verts, field_BM, field_TM, thickness, fieldBoundaryColor );		// BM to TM

	//----------------------------------------------------------------------------------------------------------------------
	// Render Ring in the middle
//	float radius = 5.0f;
//	AddVertsForRing2D( verts, Vec2( PITCH_CENTER_X, PITCH_CENTER_Y ), radius, thickness, fieldBoundaryColor );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Penalty Box
	//----------------------------------------------------------------------------------------------------------------------
//	AddVertsForLineSegment2D( verts, )

	//----------------------------------------------------------------------------------------------------------------------
	// Render Goals
	//----------------------------------------------------------------------------------------------------------------------
	// Left Goal
//	float goalBoxX				= 2.0f;
//	float goalBoxY				= goalBoxX * 4.0f;
//	AABB2 goalBoxBounds			= AABB2( field_BL, Vec2( field_BL.x + goalBoxX, field_BL.y + goalBoxY) );
//	Vec2  leftGoalCenterPoint	= field_BL + ( (field_TL - field_BL) * 0.5f );
//	goalBoxBounds.SetCenter( Vec2( leftGoalCenterPoint.x - (goalBoxX * 0.5f), leftGoalCenterPoint.y ) );
//	AddVertsForAABB2D( verts, goalBoxBounds, fieldBoundaryColor );
//	// Right Goal
//	Vec2 rightGoalCenterPoint	= field_BR + ( (field_TR - field_BR) * 0.5f );
//	goalBoxBounds.SetCenter( Vec2( rightGoalCenterPoint.x + (goalBoxX * 0.5f), rightGoalCenterPoint.y ) );
//	AddVertsForAABB2D( verts, goalBoxBounds, fieldBoundaryColor );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Football
	//----------------------------------------------------------------------------------------------------------------------
	m_football->Render( footballVerts );
	// Rendering ball fwd debugArrow with untextured verts since AddVertsForArrow3D does have UV code written yet!
	AddVertsForArrow3D( verts, m_football->m_debugFootballFwdDirStartPos, m_football->m_debugFootballFwdDirEndPos, FOOTBALL_RADIUS, m_football->m_debugFootballFwdDirColor, AABB2::ZERO_TO_ONE );
	// Render "shadow" below ball
	AddVertsForDisc2D( verts, Vec2( m_football->m_footballPosition.x, m_football->m_footballPosition.y ), m_football->m_debugFootballShadowRadius, m_football->m_debugFootballFwdDirColor );


	//----------------------------------------------------------------------------------------------------------------------
	// Render debug predicted ball trajectory
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < m_football->m_ballTrajectoryPositionList.size(); i++ )
	{
		AddVertsForSphere3D( verts, m_football->m_ballTrajectoryPositionList[i], m_football->m_ballTrajectoryRadius, m_football->m_numSlices, m_football->m_numStacks, m_football->m_ballTrajectoryColor );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render Player
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < m_playerList.size(); i++ )
	{
		m_playerList[i]->Render( playerVerts );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// End world camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->EndCamera( m_fifaTest3DWorldCamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw calls
	//----------------------------------------------------------------------------------------------------------------------
	// Draw for world objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( playerVerts.size() ), playerVerts.data() );

	// Draw for world objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );
	
	// Draw for football with texture
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindTexture( m_footballTexture );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( footballVerts.size() ), footballVerts.data() );

	// Draw for field with texture
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( m_fieldTexture );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( fieldVerts.size() ), fieldVerts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_fifaTest3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> discVerts;
	float cellHeight	= 2.0f;
	AABB2 textbox1		= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	Vec2 alignment		= Vec2( 0.0f, 1.0f );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, "F1 (Toggle camera/player controls) || Mode (F6/F7 for prev/next) || FIFA Test (3D)", Rgba8::YELLOW, 0.75f, alignment, TextDrawMode::SHRINK_TO_FIT );

	// Initialize and set UI variables
	Vec2 playerPos						= Vec2( 0.0f, 100.0f );
	Vec2 cameraPosAlignment				= Vec2( 0.0f, 0.97f );
	Vec2 cameraOrientationAlignment		= Vec2( 0.0f, 0.94f );
	Vec2 ballPosAlignment				= Vec2( 0.0f, 0.88f );
	Vec2 ballVelocityAlignment			= Vec2( 0.0f, 0.85f );
	Vec2 ballAngularVelocityAlignment	= Vec2( 0.0f, 0.82f );
	Vec2 ballOrientatioPosition			= Vec2( SCREEN_SIZE_X, ( SCREEN_SIZE_Y ) );
	Vec2 timeAlignment					= Vec2( 1.0f, 1.0f );
	float fps							= 1.0f / g_theApp->m_gameClock.GetDeltaSeconds();
	float scale							= g_theApp->m_gameClock.GetTimeScale();
	std::string cameraPosText			= Stringf( "Camera position:        %0.2f, %0.2f, %0.2f",		m_fifaTest3DWorldCamera.m_position.x,				m_fifaTest3DWorldCamera.m_position.y,					m_fifaTest3DWorldCamera.m_position.z );
	std::string cameraOrientationText	= Stringf( "Orientation (YPR):      %0.2f, %0.2f, %0.2f",		m_fifaTest3DWorldCamera.m_orientation.m_yawDegrees, m_fifaTest3DWorldCamera.m_orientation.m_pitchDegrees,	m_fifaTest3DWorldCamera.m_orientation.m_rollDegrees );
	std::string timeText				= Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.", g_theApp->m_gameClock.GetTotalSeconds(), fps, scale );
	
	std::string ballPosText					= Stringf( "Ball position:          %0.2f, %0.2f, %0.2f", m_football->m_footballPosition.x,					m_football->m_footballPosition.y,					m_football->m_footballPosition.z );
	std::string ballVelocityText			= Stringf( "Ball velocity:          %0.2f, %0.2f, %0.2f", m_football->m_footballVelocity.x,					m_football->m_footballVelocity.y,					m_football->m_footballVelocity.z );
	std::string ballAngularVelocityText		= Stringf( "Ball angularVelocity:   %0.2f, %0.2f, %0.2f", m_football->m_footballAngularVelocity.x,			m_football->m_footballAngularVelocity.y,			m_football->m_footballAngularVelocity.z );
	std::string ballOrientationText			= Stringf( "Ball orientation (YPR): %0.2f, %0.2f, %0.2f", m_football->m_footballOrientation.m_yawDegrees,	m_football->m_footballOrientation.m_pitchDegrees,	m_football->m_footballOrientation.m_rollDegrees );


	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			 cameraPosText, Rgba8::YELLOW, 0.75f,			cameraPosAlignment, TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			 	  timeText, Rgba8::YELLOW, 0.75f,  				 timeAlignment, TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			   ballPosText, Rgba8::YELLOW, 0.75f,			  ballPosAlignment, TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,		  ballVelocityText, Rgba8::YELLOW, 0.75f,		 ballVelocityAlignment, TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, ballAngularVelocityText, Rgba8::YELLOW, 0.75f, ballAngularVelocityAlignment, TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,	   ballOrientationText, Rgba8::YELLOW, 0.75f,		ballOrientatioPosition, TextDrawMode::SHRINK_TO_FIT );
	
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,  cameraOrientationText, Rgba8::YELLOW, 0.75f, cameraOrientationAlignment, TextDrawMode::SHRINK_TO_FIT );

	//----------------------------------------------------------------------------------------------------------------------
	// Render UI disc for POC			// POC == point of contact
	//----------------------------------------------------------------------------------------------------------------------
	// Background disc
	Rgba8 discBackgroundColor_UI		= Rgba8::MAGENTA;
	Rgba8 discBackgroundCenterColor_UI	= Rgba8::DARK_RED;
	AddVertsForDisc2D( discVerts, m_discBackgroundCenterPos_UI, m_discBackgroundRadius_UI, discBackgroundColor_UI );
	AddVertsForDisc2D( discVerts, m_discBackgroundCenterPos_UI, m_discBackgroundRadius_UI * 0.2f, discBackgroundCenterColor_UI );
	
	// POC disc
//	m_discPocCenterPos_UI	= discBackgroundCenterPos;
//	Vec2  discPocCenterPos	= Vec2( discPocCenterPos, SCREEN_SIZE_Y - ( discBackgroundRadius * 2.0f ) );
	Rgba8 discPocColor_UI	= Rgba8::WHITE;
	AddVertsForDisc2D( discVerts, m_discPocCenterPos_UI, m_discPocRadius_UI, discPocColor_UI );

	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_fifaTest3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw for UI camera text
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	// Draw for UI camera disc
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
//	g_theRenderer->BindTexture( m_footballTexture );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( discVerts.size() ), discVerts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if ( g_theInput->WasKeyJustPressed( 'P' ) || g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::BUTTON_START ) )
	{
		SoundID testSound = g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
		g_theAudio->StartSound( testSound );			// Comment out this line of code to remove pause sound playing

		g_theApp->m_gameClock.TogglePause();
	}

	// Slow-Mo functionality
	m_isSlowMo = g_theInput->IsKeyDown( 'T' );
	if ( m_isSlowMo )
	{
		g_theApp->m_gameClock.SetTimeScale( 0.1f );
	}
	if ( g_theInput->WasKeyJustReleased( 'T' ) )
	{
		g_theApp->m_gameClock.SetTimeScale( 1.0f );
	}

	// Fast-Mo functionality
	m_isSlowMo = g_theInput->IsKeyDown( 'Y' );
	if ( m_isSlowMo )
	{
		g_theApp->m_gameClock.SetTimeScale( 2.0f );
	}
	if ( g_theInput->WasKeyJustReleased( 'Y' ) )
	{
		g_theApp->m_gameClock.SetTimeScale( 1.0f );
	}

	// Step one frame
	if ( g_theInput->WasKeyJustPressed( 'O' ) )
	{
		g_theApp->m_gameClock.StepSingleFrame();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::PreCalculateBallTrajectory()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::UpdatePlayer( float deltaSeconds )
{
	for ( int i = 0; i < m_playerList.size(); i++ )
	{
		m_playerList[i]->Update( deltaSeconds );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::PossessNextActor()
{
	m_player1->m_playerIsControlled = !m_player1->m_playerIsControlled;
	m_player2->m_playerIsControlled = !m_player2->m_playerIsControlled;
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::UpdatePlayerVsPlayerCollision()
{
	for ( int i = 0; i < m_playerList.size(); i++ )
	{
		for ( int j = (i + 1); j < m_playerList.size(); j++ )
		{
			if ( i == j )
			{
				continue;	
			}

			Vec2 player1PosV2 = Vec2( m_playerList[i]->m_position.x, m_playerList[i]->m_position.y );
			Vec2 player2PosV2 = Vec2( m_playerList[j]->m_position.x, m_playerList[j]->m_position.y );
			PushDiscsOutOfEachOther2D( player1PosV2, m_playerList[i]->m_physicsRadius, player2PosV2, m_playerList[j]->m_physicsRadius );
			m_playerList[i]->m_position.x = player1PosV2.x;
			m_playerList[i]->m_position.y = player1PosV2.y;

			m_playerList[j]->m_position.x = player2PosV2.x;
			m_playerList[j]->m_position.y = player2PosV2.y;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::UpdatePlayerVsBallCollision()
{
	for ( int i = 0; i < m_playerList.size(); i++ )
	{
		// Check if player and ball overlaps on XY plane
		Vec2 player1PosV2	= Vec2( m_playerList[i]->m_position.x, m_playerList[i]->m_position.y );
		Vec2 footballPosV2	= Vec2( m_football->m_footballPosition.x, m_football->m_footballPosition.y );
		PushDiscsOutOfEachOther2D( player1PosV2, m_playerList[i]->m_physicsRadius, footballPosV2, m_football->m_footballRadius );
		
		// Check if player and ball overlaps on Z plane
		// Get ball Z pos
		// Get player Z pos
		FloatRange	playerHeightRange	= FloatRange( 0.0f, m_playerList[i]->m_height );
		float		ballMinZ			= m_football->m_footballPosition.z - m_football->m_footballRadius;
		float		ballMaxZ			= m_football->m_footballPosition.z + m_football->m_footballRadius;
		FloatRange	ballHeightRange		= FloatRange( ballMinZ, ballMaxZ );

		if ( playerHeightRange.IsOverlapping( ballHeightRange ) )
		{
			// Update positions correctly after collision
			m_playerList[i]->m_position.x = player1PosV2.x;
			m_playerList[i]->m_position.y = player1PosV2.y;

			m_football->m_footballPosition.x = footballPosV2.x;
			m_football->m_footballPosition.y = footballPosV2.y;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::UpdateFootballPhysics( float deltaSeconds )
{
	m_football->Update( deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
{
	// Render stationary world compass
	// X
	Vec3 endPosX = startPosition + Vec3( axisLength, 0.0f, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosX, axisThickness, Rgba8::RED );
	// Y
	Vec3 endPosY = startPosition + Vec3( 0.0f, axisLength, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosY, axisThickness, Rgba8::GREEN );
	// Z
	Vec3 endPosZ = startPosition + Vec3( 0.0f, 0.0f, axisLength );
	AddVertsForArrow3D( compassVerts, startPosition, endPosZ, axisThickness, Rgba8::BLUE );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::UpdateDebugKeysInput()
{
	// Toggle debug camera modes
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		m_isCameraControlled = !m_isCameraControlled;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F2 ) )
	{
		m_isCameraTopDown = !m_isCameraTopDown;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F3 ) )
	{
		g_debugBallCanMoveIndependently = !g_debugBallCanMoveIndependently;
	}
	// Reset actors to default position
	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		// Reset actor position, velocities, and orientation
		m_player1->m_position			= m_player1DefaultPos;
		m_player1->m_velocity			= Vec3::ZERO;
		m_player1->m_playerOrientation	= 0.0f;

		m_player2->m_position			= m_player2DefaultPos;
		m_player2->m_velocity			= Vec3::ZERO;
		m_player2->m_playerOrientation	= 0.0f;
	}
	// Reset camera to default position
	if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		// Reset Position
		m_fifaTest3DWorldCamera.m_position	= m_cameraPerspectiveViewPos;

		Vec3 forward	= Vec3( 1.0f, 0.0f, 0.0f );
		Vec3 left		= Vec3( 0.0f, 1.0f, 0.0f );
		Vec3 up			= Vec3( 0.0f, 0.0f, 1.0f );

		// Reset Orientation
		m_fifaTest3DWorldCamera.m_orientation.m_yawDegrees	 = m_perspectiveYawDegrees;
		m_fifaTest3DWorldCamera.m_orientation.m_pitchDegrees = m_perspectivePitchDegrees;
		m_fifaTest3DWorldCamera.m_orientation.m_rollDegrees	 = m_perspectiveRollDegrees;
	}
	// Reset ball to world center
	if ( g_theInput->WasKeyJustPressed( '3' ) )
	{
		// Reset Position
		m_football->m_footballPosition		= m_footballDefaultPos;
		m_football->m_footballOrientation	= EulerAngles();
		m_football->m_footballVelocity		= Vec3::ZERO;
	}
	// Toggle between frame rate DEPENDENT and INDEPENDENT physics updates 
	if ( g_theInput->WasKeyJustPressed( '4' ) )
	{
		m_isVariableTimeStep =! m_isVariableTimeStep;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::PredictBallTrajectory( float deltaSeconds )
{
	if ( m_football->m_footballVelocity.GetLength() > 1.0f )
	{
		// Clear to get rid of old data
		m_football->m_ballTrajectoryPositionList.clear();

		// Create a temp ball to simulate physics "x" num frames ahead
		Football3D* forecastedBall	= new Football3D( this );
		*forecastedBall				= *m_football;								// De-reference pointer to copy data by value instead of re-assigning pointers
		deltaSeconds				+= ( deltaSeconds * 3.0f );
		for ( int i = 0; i <= m_football->m_numFramesToCalculateTrajectory; i++ )
		{

			// Update position	
			forecastedBall->ApplyGravity( deltaSeconds );
			forecastedBall->ApplyCounterGravityForce( deltaSeconds );
			forecastedBall->UpdatePhysics( deltaSeconds );
			forecastedBall->UpdateBallToFloorCollision( deltaSeconds );
			forecastedBall->UpdateClampToWorldBounds();
			Vec3 updatedBallPosition = forecastedBall->m_footballPosition;

			// Save positions
			m_football->m_ballTrajectoryPositionList.push_back( updatedBallPosition );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::UpdateBallPoc()
{
	if ( g_theInput->IsKeyDown( 'I' ) )
	{
		m_discPocCenterPos_UI += Vec2( 0.0f, 0.1f );
	}
	if ( g_theInput->IsKeyDown( 'J' ) )
	{
		m_discPocCenterPos_UI += Vec2( -0.1f, 0.0f );
	}
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		m_discPocCenterPos_UI += Vec2( 0.0f, -0.1f );
	}
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		m_discPocCenterPos_UI += Vec2( 0.1f, 0.0f );
	}

	// Clamp max position from background center to be background radius length
	Vec2 dispPocToDiscBackgroundCenter	= m_discPocCenterPos_UI - m_discBackgroundCenterPos_UI;
	float magnitude						= dispPocToDiscBackgroundCenter.GetLength();
	// Clamp POC from going "out of bounds"
	if ( magnitude > m_discBackgroundRadius_UI )
	{
		Vec2 oppositeDirNormalShrunk = -dispPocToDiscBackgroundCenter.GetNormalized() * 0.3f; 
		m_discPocCenterPos_UI		+= oppositeDirNormalShrunk;
	}

//	Vec2 minX = Vec2( m_discBackgroundCenterPos_UI.x + m_discBackgroundRadius_UI, m_discBackgroundCenterPos_UI.y							); 
//	Vec2 maxX = Vec2(							  m_discBackgroundCenterPos_UI.x, m_discBackgroundCenterPos_UI.y + m_discBackgroundRadius_UI);
	
	float minX = m_discBackgroundCenterPos_UI.x - m_discBackgroundRadius_UI; 
	float maxX = m_discBackgroundCenterPos_UI.x + m_discBackgroundRadius_UI; 
//	float minY = m_discBackgroundCenterPos_UI.y - m_discBackgroundRadius_UI; 
//	float maxY = m_discBackgroundCenterPos_UI.y + m_discBackgroundRadius_UI; 
	float yawToApply   = RangeMapClamped( m_discPocCenterPos_UI.x, minX, maxX, -45.0f, 45.0f );
//	float pitchToApply = RangeMapClamped( m_discPocCenterPos_UI.y, minY, maxY, -45.0f, 45.0f );
//	float yawToApply   = RangeMapClamped( m_discPocCenterPos_UI.x, minX, maxX, -45.0f, 45.0f );
//	float pitchToApply = RangeMapClamped( m_discPocCenterPos_UI.y, minY, maxY, -45.0f, 45.0f );

	// Create "dead-zones" by claming yaw numbers if they are "low enough"
	if ( yawToApply >= -20.0f && yawToApply <= 20.0f )
	{
		yawToApply = 0.0f;
	}

//	m_football->m_angularVelocityToAddBasedOnPOC += Vec3( yawToApply, pitchToApply, 0.0f );
	m_football->m_angularVelocityToAddBasedOnPOC += Vec3( yawToApply, 0.0f, 0.0f );

//	DebuggerPrintf( "yawToApply %0.2f\n", yawToApply );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeFifaTest3D::CalculatePlayerInterception( Player3D const& currentPlayer, Football3D const& football )
{
	// Don't recalculate if input parameters have NOT changed
//	if ( m_calculationPerformed )
//	{
//		return;
//	}

	// Check if playerPos == ballPos 
	if ( currentPlayer.m_position == football.m_footballPosition )
	{
//		m_interceptionPossible  = true;
//		m_interceptionPosition  = currentPlayer.m_position;
//		m_timeToInterception	= 0.0f;
//		m_currentPlayerVelocity = 
	}

	// Check if player is moving
	if ( currentPlayer.m_velocity.GetLength() < 0.0f )
	{
		return;
	}

	Vec3 dispBallToPlayer		= currentPlayer.m_position - football.m_footballPosition;
//	float distToBall			= dispBallToPlayer.GetLength();
	float playerSpeedMagnitude	= currentPlayer.m_velocity.GetLength();

	// If player is NOT moving
	if ( playerSpeedMagnitude < 0.0f )
	{

	}
	else
	{

	}
}
