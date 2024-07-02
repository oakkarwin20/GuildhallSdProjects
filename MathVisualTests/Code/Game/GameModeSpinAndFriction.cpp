#include "Game/GameModeSpinAndFriction.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeSpinAndFriction::GameModeSpinAndFriction()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Create Disc Bumpers
//	for ( int i = 0; i < m_numBumpers; i++ )
//	{
//		DiscBumper_SpinAndFriction* discBumper				= new DiscBumper_SpinAndFriction();
//		discBumper->m_discBumperPos			= Vec2( g_theRNG->RollRandomFloatInRange( 5.0f, 195.0f ), g_theRNG->RollRandomFloatInRange( 5.0f, 95.0f ) );
//		discBumper->m_discBumperRadius		= g_theRNG->RollRandomFloatInRange( 1.0f, 6.0f );
//		discBumper->m_discBumperElasticity	= g_theRNG->RollRandomFloatInRange( 0.1f, 1.0f );
//		discBumper->m_discBumperColor		= Interpolate( Rgba8::RED, Rgba8::GREEN, discBumper->m_discBumperElasticity );
//		m_discBumperList.push_back( discBumper );
//	}
//	//----------------------------------------------------------------------------------------------------------------------
//	// Create Obb2 Bumpers
//	for ( int i = 0; i < m_numBumpers; i++ )
//	{
//		Obb2Bumper_SpinAndFriction* obb2Bumper					= new Obb2Bumper_SpinAndFriction();
//		obb2Bumper->m_obb2						= new OBB2D();
//		obb2Bumper->m_obb2->m_center			= Vec2( g_theRNG->RollRandomFloatInRange(  5.0f, 195.0f ), g_theRNG->RollRandomFloatInRange(  5.0f, 95.0f ) );
//		obb2Bumper->m_obb2->m_iBasisNormal		= Vec2( g_theRNG->RollRandomFloatInRange( -1.0f,   1.0f ), g_theRNG->RollRandomFloatInRange( -1.0f,  1.0f ) );
//		obb2Bumper->m_obb2->m_halfDimensions	= Vec2( g_theRNG->RollRandomFloatInRange(  2.0f,  10.0f ), g_theRNG->RollRandomFloatInRange(  2.0f, 10.0f ) );
//		obb2Bumper->m_obb2BumperElasticity		= g_theRNG->RollRandomFloatInRange( 0.1f, 1.0f );
//		obb2Bumper->m_Obb2BumperColor			= Interpolate( Rgba8::RED, Rgba8::GREEN, obb2Bumper->m_obb2BumperElasticity );
//		m_obb2BumperList.push_back( obb2Bumper );
//	}
//	//----------------------------------------------------------------------------------------------------------------------
//	// Create Capsule Bumpers
//	for ( int i = 0; i < m_numBumpers; i++ )
//	{
//		CapsuleBumper_SpinAndFriction* capsuleBumper				= new CapsuleBumper_SpinAndFriction();
//		capsuleBumper->m_capsuleBoneStart			= Vec2( g_theRNG->RollRandomFloatInRange(  5.0f, 195.0f ), g_theRNG->RollRandomFloatInRange(  5.0f, 95.0f ) );
//		capsuleBumper->m_capsuleOrientation			= g_theRNG->RollRandomFloatInRange( 0.0f, 360.0f );
//		capsuleBumper->m_capsuleLength				= g_theRNG->RollRandomFloatInRange( 1.0f,  15.0f );
//		capsuleBumper->m_capsuleDir					= Vec2::MakeFromPolarDegrees( capsuleBumper->m_capsuleOrientation, capsuleBumper->m_capsuleLength );
//		capsuleBumper->m_capsuleBoneEnd				= capsuleBumper->m_capsuleBoneStart + capsuleBumper->m_capsuleDir;
//		capsuleBumper->m_capsuleRadius				= g_theRNG->RollRandomFloatInRange( 1.0f, 6.0f );
//		capsuleBumper->m_capsuleBumperElasticity	= g_theRNG->RollRandomFloatInRange( 0.1f, 1.0f );
//		capsuleBumper->m_capsuleBumperColor			= Interpolate( Rgba8::RED, Rgba8::GREEN, capsuleBumper->m_capsuleBumperElasticity );
//		m_capsuleBumperList.push_back( capsuleBumper );
//	}

	// Initializing textures;
	m_testTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
}

//----------------------------------------------------------------------------------------------------------------------
GameModeSpinAndFriction::~GameModeSpinAndFriction()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::Update( float deltaSeconds )
{
	deltaSeconds = m_gameClock.GetDeltaSeconds();

	for ( int i = 0; i < m_billiardsList.size(); i++ )
	{
		DebuggerPrintf( "posX: %f, posY: %f, orientation: %f, angularVel: %f\n", m_billiardsList[i]->m_billiardPos.x, m_billiardsList[i]->m_billiardPos.y, m_billiardsList[i]->m_orientationDegrees, m_billiardsList[i]->m_angularVelocity );
	}

	if ( m_isVariableTimeStep )
	{
		ApplyGravityToBilliards( deltaSeconds );
		UpdateBallOrientation( deltaSeconds );
//		ComputeGarwinCollisions();
		UpdateBilliardCollisionAndClampToWorldBounds( deltaSeconds );
	}
	else
	{ 
		m_physicsUpdateDebt += deltaSeconds;
		while ( m_physicsUpdateDebt > m_physicsFixedTimeStep )
		{
			m_physicsUpdateDebt -= m_physicsFixedTimeStep;
			ApplyGravityToBilliards( m_physicsFixedTimeStep );
			UpdateBallOrientation( deltaSeconds );
//			ComputeGarwinCollisions();
			UpdateBilliardCollisionAndClampToWorldBounds( m_physicsFixedTimeStep );
		}
	}

	UpdateGameCameraPachinkoMachine();
	UpdateInput( deltaSeconds );
	UpdatePauseQuitAndSlowMo();
	UpdateSpawnBilliards();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::Reshuffle()
{
	// Remove billiards
	m_billiardsList.clear();

	// Reshuffle discBumper
	for ( int i = 0; i < m_discBumperList.size(); i++ )
	{
		DiscBumper_SpinAndFriction discBumper						= DiscBumper_SpinAndFriction();
		m_discBumperList[i]->m_discBumperPos		= Vec2( g_theRNG->RollRandomFloatInRange( 5.0f, 195.0f ), g_theRNG->RollRandomFloatInRange( 5.0f, 95.0f ) );
		m_discBumperList[i]->m_discBumperRadius		= g_theRNG->RollRandomFloatInRange( 1.0f, 6.0f );
		m_discBumperList[i]->m_discBumperElasticity = g_theRNG->RollRandomFloatInRange( 0.1f, 1.0f );
		
		Rgba8 lerpedBumperColor						= Interpolate( Rgba8::RED, Rgba8::GREEN, m_discBumperList[i]->m_discBumperElasticity );
		m_discBumperList[i]->m_discBumperColor		= lerpedBumperColor;
	}
	// Reshuffle obb2Bumper
	for ( int i = 0; i < m_obb2BumperList.size(); i++ )
	{
		m_obb2BumperList[i]->m_obb2->m_center			= Vec2( g_theRNG->RollRandomFloatInRange( 5.0f, 195.0f ), g_theRNG->RollRandomFloatInRange( 5.0f, 95.0f ) );
		m_obb2BumperList[i]->m_obb2->m_iBasisNormal		= Vec2( g_theRNG->RollRandomFloatInRange( 0.1f,   1.0f ), g_theRNG->RollRandomFloatInRange( 0.0f,  1.0f ) );
		m_obb2BumperList[i]->m_obb2->m_halfDimensions	= Vec2( g_theRNG->RollRandomFloatInRange( 2.0f,  10.0f ), g_theRNG->RollRandomFloatInRange( 2.0f, 10.0f ) );
		m_obb2BumperList[i]->m_obb2BumperElasticity		= g_theRNG->RollRandomFloatInRange( 0.1f, 1.0f );

		Rgba8 lerpedBumperColor							= Interpolate( Rgba8::RED, Rgba8::GREEN, m_obb2BumperList[i]->m_obb2BumperElasticity );
		m_obb2BumperList[i]->m_Obb2BumperColor			= lerpedBumperColor;
	}
	// Reshuffle discBumper
	for ( int i = 0; i < m_capsuleBumperList.size(); i++ )
	{
		m_capsuleBumperList[i]->m_capsuleBoneStart			= Vec2( g_theRNG->RollRandomFloatInRange( 5.0f, 195.0f ), g_theRNG->RollRandomFloatInRange( 5.0f, 95.0f ) );
		m_capsuleBumperList[i]->m_capsuleOrientation		= g_theRNG->RollRandomFloatInRange( 0.0f, 360.0f );
		m_capsuleBumperList[i]->m_capsuleLength				= g_theRNG->RollRandomFloatInRange( 1.0f, 15.0f );
		m_capsuleBumperList[i]->m_capsuleDir				= Vec2::MakeFromPolarDegrees( m_capsuleBumperList[i]->m_capsuleOrientation, m_capsuleBumperList[i]->m_capsuleLength );

		m_capsuleBumperList[i]->m_capsuleBoneEnd			= m_capsuleBumperList[i]->m_capsuleBoneStart + m_capsuleBumperList[i]->m_capsuleDir;
		m_capsuleBumperList[i]->m_capsuleRadius				= g_theRNG->RollRandomFloatInRange( 1.0f, 6.0f );
		m_capsuleBumperList[i]->m_capsuleBumperElasticity	= g_theRNG->RollRandomFloatInRange( 0.1f, 1.0f );

		Rgba8 lerpedBumperColor								= Interpolate( Rgba8::RED, Rgba8::GREEN, m_capsuleBumperList[i]->m_capsuleBumperElasticity );
		m_capsuleBumperList[i]->m_capsuleBumperColor		= lerpedBumperColor;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::UpdateGameCameraPachinkoMachine()
{
	m_raycastVsAABB2DWorldCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
	   m_raycastVsAABB2DUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	g_theRenderer->BeginCamera( m_raycastVsAABB2DWorldCamera );

	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> ballVerts;
	verts.reserve( 300 );

	//----------------------------------------------------------------------------------------------------------------------
	// Render wall borders
	if ( !m_isWarpOn )
	{
		AddVertsForLineSegment2D(verts,					Vec2::ZERO, Vec2( WORLD_SIZE_X,		    0.0f ), m_arrowThickness, Rgba8::CYAN );		// South border
	}
		AddVertsForLineSegment2D(verts,					Vec2::ZERO, Vec2(		  0.0f, WORLD_SIZE_Y ), m_arrowThickness, Rgba8::CYAN );		// West	 border
		AddVertsForLineSegment2D(verts, Vec2( WORLD_SIZE_X, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ), m_arrowThickness, Rgba8::CYAN );		// East	 border

	//----------------------------------------------------------------------------------------------------------------------
	// Render Bumpers
	// Render Disc bumpers
//	for ( int i = 0; i < m_discBumperList.size(); i++ )
//	{
//		AddVertsForDisc2D( verts, m_discBumperList[i]->m_discBumperPos, m_discBumperList[i]->m_discBumperRadius, m_discBumperList[i]->m_discBumperColor );
//	}
//	// Render Obb2 bumpers
//	for ( int i = 0; i < m_obb2BumperList.size(); i++ )
//	{
//		AddVertsForOBB2D( verts, *m_obb2BumperList[i]->m_obb2, m_obb2BumperList[i]->m_Obb2BumperColor );
//	}
//	// Render Capsule bumpers
//	for ( int i = 0; i < m_capsuleBumperList.size(); i++ )
//	{
//		AddVertsForCapsule2D( verts, m_capsuleBumperList[i]->m_capsuleBoneStart, m_capsuleBumperList[i]->m_capsuleBoneEnd, m_capsuleBumperList[i]->m_capsuleRadius, m_capsuleBumperList[i]->m_capsuleBumperColor );
//	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render billiards
	for ( int i = 0; i < m_billiardsList.size(); i++ )
	{
		AddVertsForDisc2D( ballVerts, m_billiardsList[i]->m_billiardPos, m_billiardsList[i]->m_billiardRadius, m_billiardsList[i]->m_billiardColor );
		Vec2 ballFwdNormal = m_billiardsList[i]->m_billiardPos.GetFowardNormal( m_billiardsList[i]->m_orientationDegrees );
		Vec2 ballEndPos = m_billiardsList[i]->m_billiardPos + ballFwdNormal;
		AddVertsForArrow2D( ballVerts, m_billiardsList[i]->m_billiardPos, ballEndPos, 5.0f, 2.0f, Rgba8::RED );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render ring at Raycast arrow startPos
	AddVertsForRing2D( verts, m_rayStartPos, m_billiardMinRadius, m_arrowThickness, Rgba8::LIGHTBLUE );
	AddVertsForRing2D( verts, m_rayStartPos, m_billiardMaxRadius, m_arrowThickness, Rgba8::LIGHTBLUE );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Raycast arrow
	// Ray start to end with no impact
	AddVertsForArrow2D( verts, m_rayStartPos, m_rayEndPos, m_arrowSize, m_arrowThickness, m_rayDefaultColor );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw call for Ball
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( ballVerts.size() ), ballVerts.data() );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw call for World
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );

	// End World Camera
	g_theRenderer->EndCamera( m_raycastVsAABB2DWorldCamera );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::RenderUIObjects() const
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
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, " Mode (F6/F7 for prev/next): Spin and Friction (2D)", Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	float			deltaSeconds = m_gameClock.GetDeltaSeconds();
	std::string		UIString;
//	if ( m_isWarpOn && m_isVariableTimeStep )
//	{
//		UIString = Stringf( " F8 to reset; LMB/RMB/ESDF/IJKL move spawn; T = slow, space(N) = billiard(s), B = warp, timestep = %.2fms (V, [, ]), dt = %.2fms", m_physicsFixedTimeStep * 1000.0f, deltaSeconds * 1000.0f );
//	}
	if ( !m_isWarpOn && !m_isVariableTimeStep  )
	{
		UIString = Stringf( " F8 to reset; LMB/RMB/ESDF/IJKL move spawn; T = slow, space(N) = billiard(s), B = bounce, timestep = %.2fms (V, [, ]), dt = %.2fms", m_physicsFixedTimeStep * 1000.0f, deltaSeconds * 1000.0f );
	}
	else if ( m_isWarpOn && !m_isVariableTimeStep )
	{
		UIString = Stringf( " F8 to reset; LMB/RMB/ESDF/IJKL move spawn; T = slow, space(N) = billiard(s), B = warp, timestep = %.2fms (V, [, ]), dt = %.2fms", m_physicsFixedTimeStep * 1000.0f, deltaSeconds * 1000.0f );
	}
	else if ( m_isWarpOn && m_isVariableTimeStep )
	{
		UIString = Stringf( " F8 to reset; LMB/RMB/ESDF/IJKL move spawn; T = slow, space(N) = billiard(s), B = warp, variable timestep, dt = %.2fms", deltaSeconds * 1000.0f );
	}
	else if ( !m_isWarpOn && m_isVariableTimeStep )
	{
		UIString = Stringf( " F8 to reset; LMB/RMB/ESDF/IJKL move spawn; T = slow, space(N) = billiard(s), B = bounce, variable timestep, dt = %.2fms", deltaSeconds * 1000.0f );
	}

	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, UIString, Rgba8::GREEN, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );	

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
void GameModeSpinAndFriction::UpdateInput( float deltaSeconds )
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
		m_rayEndPos	  += Vec2( moveSpeed, 0.0f ) * deltaSeconds;
	}

	// Toggle isFreeFall
	if ( g_theInput->WasKeyJustPressed( 'B' ) )
	{
		m_isWarpOn = !m_isWarpOn;
	}

	// Time step
	if ( g_theInput->WasKeyJustPressed( 'V' ) )
	{
		m_isVariableTimeStep = !m_isVariableTimeStep;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFTBRACKET ) )
	{
		m_physicsFixedTimeStep -= ( m_physicsFixedTimeStep * 0.1f );
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_RIGHTBRACKET ) )
	{
		m_physicsFixedTimeStep += ( m_physicsFixedTimeStep * 0.1f );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::UpdatePauseQuitAndSlowMo()
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
void GameModeSpinAndFriction::UpdateSpawnBilliards()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Shoot Billiards if 'SPACEBAR' is pressed
	if ( g_theInput->WasKeyJustPressed( ' ' ) )
	{
		// Create Billiard with velocity of raycast's magnitude
		PachinkoBilliards_SpinAndFriction* billiard		= new PachinkoBilliards_SpinAndFriction();
		billiard->m_billiardVelocity					= m_rayEndPos - m_rayStartPos;
		billiard->m_billiardPos							= m_rayStartPos;																		// Updating m_billiardPos to m_rayStartPos each frame to pass correct info when Rendering Billiard
		billiard->m_billiardRadius						= m_billiardMaxRadius;
		billiard->m_billiardColor						= Rgba8::WHITE;
//		billiard->m_angularVelocity						= 200.0f; // g_theRNG->RollRandomFloatInRange( -200, 200 );
		m_billiardsList.push_back( billiard );
	}
	// Spawn Billiards while 'N' is held down
	if ( g_theInput->IsKeyDown( 'N' ) )
	{
		// Create Billiard with velocity of raycast's magnitude
		PachinkoBilliards_SpinAndFriction* billiard		= new PachinkoBilliards_SpinAndFriction();
		billiard->m_billiardVelocity					= m_rayEndPos - m_rayStartPos;		
		billiard->m_billiardPos							= m_rayStartPos;																		// Updating m_billiardPos to m_rayStartPos each frame to pass correct info when Rendering Billiard
		billiard->m_billiardRadius						= g_theRNG->RollRandomFloatInRange( m_billiardMinRadius, m_billiardMaxRadius );
		billiard->m_billiardColor						= Rgba8::WHITE;
		m_billiardsList.push_back( billiard );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::UpdateBilliardCollisionAndClampToWorldBounds( float deltaSeconds )
{
	// loop through all billiards and update their velocities
	for ( int billiardIndex = 0; billiardIndex < m_billiardsList.size(); billiardIndex++ )
	{
		PachinkoBilliards_SpinAndFriction& currentBilliard = *m_billiardsList[billiardIndex];

		// Update each billiard position by adding velocity
		currentBilliard.m_billiardPos += ( currentBilliard.m_billiardVelocity * deltaSeconds );

		// Warp logic
		if ( m_isWarpOn )
		{
			// Check if billiard Y is lower than min threshold
			if ( currentBilliard.m_billiardPos.y < m_minYWarpThreshold )
			{
				// if true, set current billiard slightly higher than worldY
				currentBilliard.m_billiardPos.y = WORLD_SIZE_Y * 1.1f;						// Set billiard pos above world Y
			}
		}
		else if ( !m_isWarpOn )
		{
			if	( currentBilliard.m_billiardPos.y <= 0.0f + currentBilliard.m_billiardRadius )
			{
				//----------------------------------------------------------------------------------------------------------------------
				// Clamp to world pos and flip velocity when colliding with walls
				//----------------------------------------------------------------------------------------------------------------------
				currentBilliard.m_billiardPos.y		  = ( 0.0f + currentBilliard.m_billiardRadius );		// Clamp Billiard position to World.minY
				currentBilliard.m_billiardVelocity.y *= -m_wallElasticity;									// Reverse Billiard.y velocity to bounce off wall

				//----------------------------------------------------------------------------------------------------------------------
				// Apply Friction to slow down ball while in contact with the floor
				//---------------------------------------------------------------------------------------------------------------------- 
 				float floorLinearVelocityFriction		= 0.7f;
// 				float floorAngularVelocityFriction		= 0.99f;
 				float tangentialVelocityBeforeFriction	= currentBilliard.m_billiardVelocity.x;
 				currentBilliard.m_billiardVelocity.x	*= floorLinearVelocityFriction;						// Apply friction to ball's linear velocity while in contact with the ground
 				float tangentialVelocityAfterFriction	= currentBilliard.m_billiardVelocity.x;
 				float subtractedTangentialVelocity		= tangentialVelocityBeforeFriction - tangentialVelocityAfterFriction;
 
				if ( currentBilliard.m_billiardVelocity.x >= 0 )
				{
//					currentBilliard.m_angularVelocity = -ComputeGarwinAngularVelocity( ( currentBilliard.m_billiardVelocity.x * (1 + subtractedTangentialVelocity) ), currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
					currentBilliard.m_angularVelocity = -ComputeGarwinAngularVelocity( ( currentBilliard.m_billiardVelocity.GetLength() * (1 + subtractedTangentialVelocity) ), currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
				}
				else if ( currentBilliard.m_billiardVelocity.x < 0 )
				{
//					currentBilliard.m_angularVelocity = ComputeGarwinAngularVelocity( ( currentBilliard.m_billiardVelocity.x * ( 1 + subtractedTangentialVelocity ) ), currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
					currentBilliard.m_angularVelocity = ComputeGarwinAngularVelocity( ( -currentBilliard.m_billiardVelocity.GetLength() * ( 1 + subtractedTangentialVelocity ) ), currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
				}
//				currentBilliard.m_billiardVelocity.x = ComputeGarwinVelocityX( currentBilliard.m_billiardVelocity.x, currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );

			
 				//----------------------------------------------------------------------------------------------------------------------
 				// Apply angular velocity 
 				//----------------------------------------------------------------------------------------------------------------------				
 				// If billiard is traveling east, it should gain "positive" angular velocity (clockwise)
// 				if ( currentBilliard.m_billiardVelocity.x > 0 )
// 				{
//// //					currentBilliard.m_angularVelocity -= (subtractedTangentialVelocity * floorAngularVelocityFriction);
// 					currentBilliard.m_angularVelocity -= (tangentialVelocityAfterFriction * floorAngularVelocityFriction);
// 				}
 				// If billiard is traveling west, it should gain "negative" angular velocity (counter-clockwise)
// 				if ( currentBilliard.m_billiardVelocity.x < 0 )
// 				{
// //					currentBilliard.m_angularVelocity += (subtractedTangentialVelocity * floorAngularVelocityFriction);
// 					currentBilliard.m_angularVelocity += fabsf( (tangentialVelocityAfterFriction * floorAngularVelocityFriction) );
// 				}
 				//----------------------------------------------------------------------------------------------------------------------
 				// Reduce angular velocity with friction
 				//----------------------------------------------------------------------------------------------------------------------
// 				currentBilliard.m_angularVelocity *= floorAngularVelocityFriction;
			}
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Bounce this disc off every other disc // (current 'i' in the for-loop) 
		for ( int otherBilliardIndex = billiardIndex + 1; otherBilliardIndex < m_billiardsList.size(); otherBilliardIndex++ )
		{
				BounceDiscOffEachOther2D( m_billiardsList[billiardIndex]->m_billiardPos,		m_billiardsList[billiardIndex]->m_billiardRadius,			m_billiardsList[billiardIndex]->m_billiardVelocity,
										  m_billiardsList[otherBilliardIndex]->m_billiardPos,	m_billiardsList[otherBilliardIndex]->m_billiardRadius,		m_billiardsList[otherBilliardIndex]->m_billiardVelocity,
										  m_billiardsList[billiardIndex]->m_billiardElasticity, m_billiardsList[otherBilliardIndex]->m_billiardElasticity );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Check billiards collision against discBumpers
//		for ( int discBumperIndex = 0; discBumperIndex < m_discBumperList.size(); discBumperIndex++ )
//		{
//			// Reflect Billiard velocity off this Bumper 
//			BounceDiscOffFixedDiscBumper2D(	m_billiardsList[billiardIndex]->m_billiardPos,			   m_billiardsList[billiardIndex]->m_billiardRadius,			m_billiardsList[billiardIndex]->m_billiardVelocity,
//										   m_discBumperList[discBumperIndex]->m_discBumperPos,	    m_discBumperList[discBumperIndex]->m_discBumperRadius, 
//											m_billiardsList[billiardIndex]->m_billiardElasticity,   m_discBumperList[discBumperIndex]->m_discBumperElasticity );
//		}
		//----------------------------------------------------------------------------------------------------------------------
		// Check billiards collision against obb2Bumpers
// 		for ( int obb2Index = 0; obb2Index < m_obb2BumperList.size(); obb2Index++ )
// 		{
// 			// Get nearest point on Obb2
// 			Vec2 nearestPointOnObb2 = GetNearestPointOnOBB2D( m_billiardsList[billiardIndex]->m_billiardPos, *m_obb2BumperList[obb2Index]->m_obb2 );
// 			// Check if billiard is close enough to Obb2
// //			float distBilliardToObb2 = GetDistance2D( m_billiardsList[billiardIndex]->m_billiardPos, nearestPointOnObb2 );
// //			if ( distBilliardToObb2 < ( m_billiardsList[billiardIndex]->m_billiardRadius + m_nearestPointRadius ) )
// //			{
// 				// Reflect Billiard velocity off this discBumper 
// 				BounceDiscOffFixedDiscBumper2D(	m_billiardsList[billiardIndex]->m_billiardPos,			m_billiardsList[billiardIndex]->m_billiardRadius,	m_billiardsList[billiardIndex]->m_billiardVelocity,
// 																		   nearestPointOnObb2,									    m_nearestPointRadius, 
// 												m_billiardsList[billiardIndex]->m_billiardElasticity,   m_obb2BumperList[obb2Index]->m_obb2BumperElasticity );
// //			}
// 			
// 		}
		//----------------------------------------------------------------------------------------------------------------------
		// Check billiards collision against capsuleBumpers
// 		for ( int capsuleIndex = 0; capsuleIndex < m_capsuleBumperList.size(); capsuleIndex ++ )
// 		{
// 			// Get nearest point from billiard to Capsule
// 			Vec2 nearestPointOnCapusle = GetNearestPointOnCapsule2D( m_billiardsList[billiardIndex]->m_billiardPos, m_capsuleBumperList[capsuleIndex]->m_capsuleBoneStart, m_capsuleBumperList[capsuleIndex]->m_capsuleBoneEnd, m_capsuleBumperList[capsuleIndex]->m_capsuleRadius );
// 			// Check if billiard is too close to pointOnCapsule
// //			float distBilliardToCapsule = GetDistance2D( m_billiardsList[billiardIndex]->m_billiardPos, nearestPointOnCapusle );
// //			if ( distBilliardToCapsule < ( m_billiardsList[billiardIndex]->m_billiardRadius + m_nearestPointRadius ) )
// //			{
// 				// Bounce Billiard off this capsule capsuleBumper
// 				BounceDiscOffFixedDiscBumper2D( m_billiardsList[billiardIndex]->m_billiardPos,			m_billiardsList[billiardIndex]->m_billiardRadius,	m_billiardsList[billiardIndex]->m_billiardVelocity,
// 																		nearestPointOnCapusle,										m_nearestPointRadius,
// 												m_billiardsList[billiardIndex]->m_billiardElasticity,   m_capsuleBumperList[capsuleIndex]->m_capsuleBumperElasticity );
// 			}
//		}
		//----------------------------------------------------------------------------------------------------------------------
		// Clamping BilliardsPos to world bounds // Bounce Billiard by flipping x or y axis if pos > world bounds

		//----------------------------------------------------------------------------------------------------------------------
		// Handle logic for collisions against RIGHT wall
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_billiardsList[billiardIndex]->m_billiardPos.x >= ( WORLD_SIZE_X - m_billiardsList[billiardIndex]->m_billiardRadius ) )
		{
			m_billiardsList[billiardIndex]->m_billiardPos.x		  = ( WORLD_SIZE_X - m_billiardsList[billiardIndex]->m_billiardRadius );		// Clamp Billiard position to World.maxX
			m_billiardsList[billiardIndex]->m_billiardVelocity.x *= -m_wallElasticity;															// Reverse Billiard.x velocity to bounce off wall

//			currentBilliard.m_angularVelocity	 = -ComputeGarwinAngularVelocity( currentBilliard.m_billiardVelocity.x, currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
			currentBilliard.m_angularVelocity	 = -ComputeGarwinAngularVelocity( currentBilliard.m_billiardVelocity.GetLength(), currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
//			currentBilliard.m_billiardVelocity.x = ComputeGarwinVelocityX( currentBilliard.m_billiardVelocity.x, currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Handle logic for collisions against LEFT wall
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_billiardsList[billiardIndex]->m_billiardPos.x <= 0.0f + m_billiardsList[billiardIndex]->m_billiardRadius )
		{
			m_billiardsList[billiardIndex]->m_billiardPos.x		  = ( 0.0f + m_billiardsList[billiardIndex]->m_billiardRadius );				// Clamp Billiard position to World.minX
			m_billiardsList[billiardIndex]->m_billiardVelocity.x *= -m_wallElasticity;															// Reverse Billiard.x velocity to bounce off wall

//			currentBilliard.m_angularVelocity	 = -ComputeGarwinAngularVelocity( currentBilliard.m_billiardVelocity.x, currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
			currentBilliard.m_angularVelocity	 = -ComputeGarwinAngularVelocity( currentBilliard.m_billiardVelocity.GetLength(), currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
//			currentBilliard.m_billiardVelocity.x = ComputeGarwinVelocityX( currentBilliard.m_billiardVelocity.x, currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::ApplyGravityToBilliards( float deltaSeconds )
{
	for ( int i = 0; i < m_billiardsList.size(); i++ )
	{
		m_billiardsList[i]->m_billiardVelocity += Vec2( 0.0f, m_gravityWeight ) * deltaSeconds;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::UpdateBallOrientation( float deltaSeconds )
{
	for ( int i = 0; i < m_billiardsList.size(); i++ )
	{
		m_billiardsList[i]->m_orientationDegrees += m_billiardsList[i]->m_angularVelocity * deltaSeconds;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeSpinAndFriction::ComputeGarwinCollisions()
{
	for ( int i = 0; i < m_billiardsList.size(); i++ )
	{
		PachinkoBilliards_SpinAndFriction& currentBilliard = *m_billiardsList[i];

		//----------------------------------------------------------------------------------------------------------------------
		// Handle angular velocity logic when billiard is HIGHER than world max Y
		//----------------------------------------------------------------------------------------------------------------------
		if ( ( currentBilliard.m_billiardPos.y + currentBilliard.m_billiardRadius ) >= WORLD_SIZE_Y )
		{
			currentBilliard.m_angularVelocity	= -ComputeGarwinAngularVelocity( currentBilliard.m_billiardVelocity.x, currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
			currentBilliard.m_billiardVelocity.x = -ComputeGarwinVelocityX( currentBilliard.m_billiardVelocity.x, currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Handle angular velocity logic when billiard is TOO FAR RIGHT than world max X
		//----------------------------------------------------------------------------------------------------------------------
		if ( ( currentBilliard.m_billiardPos.x + currentBilliard.m_billiardRadius ) >= WORLD_SIZE_X )
		{
			currentBilliard.m_angularVelocity	= -ComputeGarwinAngularVelocity( currentBilliard.m_billiardVelocity.y, currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
			currentBilliard.m_billiardVelocity.x = -ComputeGarwinVelocityX( currentBilliard.m_billiardVelocity.x, currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Handle angular velocity logic when billiard is TOO FAR LEFT than world min X (0)
		//----------------------------------------------------------------------------------------------------------------------
		if ( ( currentBilliard.m_billiardPos.x - currentBilliard.m_billiardRadius ) <= 0 )
		{
			currentBilliard.m_angularVelocity	= -ComputeGarwinAngularVelocity( currentBilliard.m_billiardVelocity.y, currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
			currentBilliard.m_billiardVelocity.x = -ComputeGarwinVelocityX( currentBilliard.m_billiardVelocity.x, currentBilliard.m_angularVelocity, currentBilliard.m_billiardRadius );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
float GameModeSpinAndFriction::ComputeGarwinAngularVelocity( float velocityXorY, float angularVelocity, float radius )
{
	float velocityScaledByElasticity									= ( 1.0f + m_wallElasticity ) * velocityXorY;
	float garwinAlphaMinusElasticityScaledByRadiusTimesAngularVelocity	= ( GARWIN_ALPHA - m_wallElasticity ) * radius * angularVelocity;
	float radiusScaledByOnePlusGarwinAlpha								= radius * ( 1.0f + GARWIN_ALPHA );
	float newAngularVelocity											= ( velocityScaledByElasticity + garwinAlphaMinusElasticityScaledByRadiusTimesAngularVelocity ) / radiusScaledByOnePlusGarwinAlpha;
	return newAngularVelocity;

//	float velocityScaledByElasticity									= (1.0f + ELASTICITY_X) * velocityXorY;
//	float garwinAlphaMinusElasticityScaledByRadiusTimesAngularVelocity	= (GARWIN_ALPHA - ELASTICITY_X) * radius * angularVelocity;
//	float radiusScaledByOnePlusGarwinAlpha								= radius * (1.0f + GARWIN_ALPHA);
//	//	m_footballAngularVelocity										= ( (1.0 + ELASTICITY_X) * fwdDir ) + ( (GARWIN_ALPHA - ELASTICITY_X) * FOOTBALL_RADIUS * angularVelocity )SB / ( FOOTBALL_RADIUS * (1.0f + GARWIN_ALPHA) );
//	float newAngularVelocity											= ( velocityScaledByElasticity + garwinAlphaMinusElasticityScaledByRadiusTimesAngularVelocity ) / radiusScaledByOnePlusGarwinAlpha;
//	return newAngularVelocity;

}

//----------------------------------------------------------------------------------------------------------------------
float GameModeSpinAndFriction::ComputeGarwinVelocityX( float velocityX, float angularVelocity, float radius )
{
	UNUSED( velocityX );
//	float newVelocityX = ( (1 - GARWIN_ALPHA * ELASTICITY_X) + ( GARWIN_ALPHA * (1 + ELASTICITY_X) * radius * angularVelocity ) ) / ( 1 + GARWIN_ALPHA);
//	return newVelocityX;

	float oneMinusElasticityShrunkByGarwinAlpha  = 1 - GARWIN_ALPHA * ELASTICITY_X;
	float onePlusElasticityShrunkByGarwinAlpha   = GARWIN_ALPHA * (1 + ELASTICITY_X);
//	float newVelocityX							 = ( oneMinusElasticityShrunkByGarwinAlpha + ( onePlusElasticityShrunkByGarwinAlpha * radius * angularVelocity ) ) / ( 1 + GARWIN_ALPHA );
	float newVelocityX							 = ( oneMinusElasticityShrunkByGarwinAlpha * ( onePlusElasticityShrunkByGarwinAlpha * radius * angularVelocity ) ) / ( 1 + GARWIN_ALPHA );
	return newVelocityX;
}	

//----------------------------------------------------------------------------------------------------------------------
float GameModeSpinAndFriction::ComputeGarwinVelocityY( float velocityY )
{
	float newVelocityY = -(ELASTICITY_X * velocityY);
	return newVelocityY;
}

