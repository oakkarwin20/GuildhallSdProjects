#include "Game/App.hpp"
#include "Game/GameMode_RobotArm_3D.hpp"
#include "Game/FoodManager.hpp"

#include "Engine/ThirdParty/Squirrel/Noise/SmoothNoise.hpp"
#include "Engine/SkeletalSystem/CreatureBase.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


//----------------------------------------------------------------------------------------------------------------------
GameMode_RobotArm_3D::GameMode_RobotArm_3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameMode_RobotArm_3D::~GameMode_RobotArm_3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::Startup()
{
	g_debugFreeFly_F1			= true;
	g_debugRenderRaycast_F2		= true;
	g_debugBasis_F3				= true;
	g_debugText_F4				= false;
	g_debugAngles_F5			= false;
	g_debugToggleConstraints_F8 = true;
	g_debugFollowTarget_F11		= false;
	g_debugAngles_F5			= true;

	InitializeIK_ChainCCD();

	m_gameMode3DWorldCamera.m_position	  = Vec3( 0.0f, 120.0f, 20.0f );
	m_gameMode3DWorldCamera.m_orientation = EulerAngles( -90.0f, 5.0f, 0.0f );

	m_ikChain_CCD->m_isSingleStep_Debug		= false;
	m_ikChain_CCD->m_target.m_currentPos_WS = Vec3( 50.0f, 0.0f, 25.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize food orbs
	//----------------------------------------------------------------------------------------------------------------------
	m_foodManager = new FoodManager( 1, this );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::Update( float deltaSeconds )
{	
	// Update core systems
	UpdatePauseQuitAndSlowMo();
	UpdateDebugKeys();
	UpdateCameraInput( deltaSeconds );

	// Update food manager
	if ( g_debugFollowTarget_F11 )
	{
		m_foodManager->Update( deltaSeconds );
		m_ikChain_CCD->m_target.m_currentPos_WS = m_foodManager->m_foodList[0].m_position;
	}
	else
	{
		// Update IK chain
		UpdateTargetInput( deltaSeconds );
	}
	UpdateCreature( deltaSeconds );

	// Clamp chainInfoIndex
	int numSolutions = int( m_ikChain_CCD->m_solutionsThisFrameList.size() - 1 );
	m_chainInfoIndex = GetClamped( m_chainInfoIndex, 0, numSolutions );
	if ( m_chainInfoIndex < 0 )
	{
		m_chainInfoIndex = 0;
	}

	// Update Camera
	UpdateGameMode3DCamera();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::Render() const
{
	RenderWorldObjects();
//	RenderUIObjects();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::Shutdown()
{
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::UpdateDebugKeys()
{
	// Toggle common debug bools
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		g_debugFreeFly_F1 = !g_debugFreeFly_F1;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F2 ) )
	{
		g_debugRenderRaycast_F2 = !g_debugRenderRaycast_F2;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F3 ) )
	{
		g_debugBasis_F3 = !g_debugBasis_F3;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F4 ) )
	{
		g_debugText_F4 = !g_debugText_F4;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F5 ) )
	{
		g_debugAngles_F5 = !g_debugAngles_F5;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F8 ) )
	{
		g_debugToggleConstraints_F8 = !g_debugToggleConstraints_F8;
		SetIK_ChainConstraints();
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F11 ) )
	{
		g_debugFollowTarget_F11 = !g_debugFollowTarget_F11;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_BACKSPACE ) )
	{
		m_ikChain_CCD->m_lerpEulerToGoal = !m_ikChain_CCD->m_lerpEulerToGoal;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_DELETE ) )
	{
		m_ikChain_CCD->m_attemptDeadlockSolutions = !m_ikChain_CCD->m_attemptDeadlockSolutions;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_CONTROL ) )
	{
		m_ikChain_CCD->m_solveRoll = !m_ikChain_CCD->m_solveRoll;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Camera debug controls
	//----------------------------------------------------------------------------------------------------------------------
	// Control camera dist from player
	if ( g_theInput->IsKeyDown( '9' ) )
	{
		m_distCamAwayFromPlayer += 10.0f;
	}
	if ( g_theInput->IsKeyDown( '0' ) )
	{
		m_distCamAwayFromPlayer -= 10.0f;
	}
	// Preset camera positions
	if ( g_theInput->WasKeyJustPressed( 'X' ) )
	{
		// Look down X-axis (YZ plane)
		m_gameMode3DWorldCamera.m_position	  = Vec3( 120.0f, 0.0f, 20.0f );
		m_gameMode3DWorldCamera.m_orientation = EulerAngles( 180.0f, 5.0f, 0.0f );
	}
	if ( g_theInput->WasKeyJustPressed( 'Y' ) )
	{
		// Look down Y-axis (XZ plane)
		m_gameMode3DWorldCamera.m_position	  = Vec3( 0.0f, 120.0f, 20.0f );
		m_gameMode3DWorldCamera.m_orientation = EulerAngles( -90.0f, 5.0f, 0.0f );
	}
	if ( g_theInput->WasKeyJustPressed( 'Z' ) )
	{
		// Look down Z-axis (XY plane)
		m_gameMode3DWorldCamera.m_position	  = Vec3( 2.0f, 0.0f, 120.0f );
		m_gameMode3DWorldCamera.m_orientation = EulerAngles( 0.0f, 89.9f, 0.0f );
	}

	if ( g_theInput->WasKeyJustPressed( 'R' ) )
	{
		 m_ikChain_CCD->ResetEulerForAllJoints();
	}


	//----------------------------------------------------------------------------------------------------------------------
	// Increase / decrease num solverIterations
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->WasKeyJustPressed( '9' ) )
	{
		// Decrease
		m_ikChain_CCD->m_numIterations--;
		m_ikChain_CCD->m_numIterations = GetClamped( m_ikChain_CCD->m_numIterations, 0, INT_MAX );
	}
	if ( g_theInput->WasKeyJustPressed( '0' ) )
	{
		// Decrease
		m_ikChain_CCD->m_numIterations++;
	}
	if ( g_theInput->WasKeyJustPressed( '5' ) )
	{
		// Decrease by 10
		m_ikChain_CCD->m_numIterations -= 100;
		m_ikChain_CCD->m_numIterations = GetClamped( m_ikChain_CCD->m_numIterations, 0, INT_MAX );
	}
	if ( g_theInput->WasKeyJustPressed( '6' ) )
	{
		// Decrease by 10
		m_ikChain_CCD->m_numIterations += 100;
	}
	if ( g_theInput->WasKeyJustPressed( '7' ) )
	{
		// Decrease by 10
		m_ikChain_CCD->m_numIterations -= 10;
		m_ikChain_CCD->m_numIterations  = GetClamped( m_ikChain_CCD->m_numIterations, 0, INT_MAX );
	}
	if ( g_theInput->WasKeyJustPressed( '8' ) )
	{
		// Decrease by 10
		m_ikChain_CCD->m_numIterations += 10;
	}


	//----------------------------------------------------------------------------------------------------------------------
	// Cycle chainInfoIndex
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		int numSolutions = int( m_ikChain_CCD->m_solutionsThisFrameList.size() - 1 );
		if ( g_theInput->IsKeyDown( KEYCODE_SEMICOLON ) )
		{
			m_chainInfoIndex--;
			m_chainInfoIndex = GetClamped( m_chainInfoIndex, 0, numSolutions );
		}
		if ( g_theInput->IsKeyDown( KEYCODE_SINGLEQUOTE ) )
		{
			m_chainInfoIndex++;
			m_chainInfoIndex = GetClamped( m_chainInfoIndex, 0, numSolutions );
			if ( m_chainInfoIndex < 0 )
			{
				m_chainInfoIndex = 0;
			}
		}
	}
	else
	{
		if ( g_theInput->WasKeyJustPressed( KEYCODE_SEMICOLON ) )
		{
			m_chainInfoIndex--;
		}
		if ( g_theInput->WasKeyJustPressed( KEYCODE_SINGLEQUOTE ) )
		{
			m_chainInfoIndex++;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::UpdateCameraInput( float deltaSeconds )
{
	Vec3 iBasis, jBasis, kBasis;
	m_gameMode3DWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
	iBasis.z = 0.0f;
	jBasis.z = 0.0f;
	iBasis.Normalize();
	jBasis.Normalize();
	kBasis.Normalize();

	//----------------------------------------------------------------------------------------------------------------------
	// All directions are local
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_currentSpeed = m_fasterSpeed;
	}
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_currentSpeed = m_defaultSpeed;
	}

	// Camera controls
	if ( g_debugFreeFly_F1 )
	{
		// Forward
		if ( g_theInput->IsKeyDown( 'W' ) )
		{
			m_gameMode3DWorldCamera.m_position += ( iBasis * m_currentSpeed ) * deltaSeconds;
		}
		// Left
		if ( g_theInput->IsKeyDown( 'A' ) )
		{
			m_gameMode3DWorldCamera.m_position += ( jBasis * m_currentSpeed ) * deltaSeconds;
		}
		// Backwards
		if ( g_theInput->IsKeyDown( 'S' ) )
		{
			m_gameMode3DWorldCamera.m_position -= ( iBasis * m_currentSpeed ) * deltaSeconds;
		}
		// Right
		if ( g_theInput->IsKeyDown( 'D' ) )
		{
			m_gameMode3DWorldCamera.m_position -= ( jBasis * m_currentSpeed ) * deltaSeconds;
		}
		// Sky (+Z)
		if ( g_theInput->IsKeyDown( 'E' ) )
		{
			m_gameMode3DWorldCamera.m_position += ( Vec3( 0.0f, 0.0f, 1.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		// Ground (-Z)
		if ( g_theInput->IsKeyDown( 'Q' ) )
		{
			m_gameMode3DWorldCamera.m_position -= ( Vec3( 0.0f, 0.0f, 1.0f ) * m_currentSpeed ) * deltaSeconds;
		}
	}
	else   
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Model controls (movement)
		//----------------------------------------------------------------------------------------------------------------------
		if ( g_theInput->IsKeyDown( 'W' ) )
		{
			m_creatureCCD->m_modelRoot->m_jointPos_MS += ( Vec3::X_FWD * m_currentSpeed ) * deltaSeconds;
		}
		// Left
		if ( g_theInput->IsKeyDown( 'A' ) )
		{
			m_creatureCCD->m_modelRoot->m_jointPos_MS += ( Vec3::Y_LEFT * m_currentSpeed ) * deltaSeconds;
		}
		// Backwards
		if ( g_theInput->IsKeyDown( 'S' ) )
		{
			m_creatureCCD->m_modelRoot->m_jointPos_MS -= ( Vec3::X_FWD * m_currentSpeed ) * deltaSeconds;
		}
		// Right
		if ( g_theInput->IsKeyDown( 'D' ) )
		{
			m_creatureCCD->m_modelRoot->m_jointPos_MS -= ( Vec3::Y_LEFT * m_currentSpeed ) * deltaSeconds;
		}
		// Sky (+Z)
		if ( g_theInput->IsKeyDown( 'E' ) )
		{
			m_creatureCCD->m_modelRoot->m_jointPos_MS += ( Vec3::Z_UP * m_currentSpeed ) * deltaSeconds;
		}
		// Ground (-Z)
		if ( g_theInput->IsKeyDown( 'Q' ) )
		{
			m_creatureCCD->m_modelRoot->m_jointPos_MS -= ( Vec3::Z_UP * m_currentSpeed ) * deltaSeconds;
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Model controls (euler)
		//----------------------------------------------------------------------------------------------------------------------
		// +Yaw
		if ( g_theInput->IsKeyDown( 'V' ) )
		{
			m_creatureCCD->m_modelRoot->m_eulerAnglesGoal_LS.m_yawDegrees += m_currentSpeed * deltaSeconds;
		}
		// -Yaw
		if ( g_theInput->IsKeyDown( 'B' ) )
		{
			m_creatureCCD->m_modelRoot->m_eulerAnglesGoal_LS.m_yawDegrees -= m_currentSpeed * deltaSeconds;
		}
		// +Pitch
		if ( g_theInput->IsKeyDown( 'N' ) )
		{
			m_creatureCCD->m_modelRoot->m_eulerAnglesGoal_LS.m_pitchDegrees += m_currentSpeed * deltaSeconds;
		}
		// -Pitch
		if ( g_theInput->IsKeyDown( 'M' ) )
		{
			m_creatureCCD->m_modelRoot->m_eulerAnglesGoal_LS.m_pitchDegrees -= m_currentSpeed * deltaSeconds;
		}
		// +Roll
		if ( g_theInput->IsKeyDown( KEYCODE_COMMA ) )
		{
			m_creatureCCD->m_modelRoot->m_eulerAnglesGoal_LS.m_rollDegrees += m_currentSpeed * deltaSeconds;
		}
		// -Roll
		if ( g_theInput->IsKeyDown( KEYCODE_PERIOD ) )
		{
			m_creatureCCD->m_modelRoot->m_eulerAnglesGoal_LS.m_rollDegrees -= m_currentSpeed * deltaSeconds;
		}
	}


	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees = GetClamped( m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees, -85.0f, 85.0f );
	m_gameMode3DWorldCamera.m_orientation.m_rollDegrees  = GetClamped( m_gameMode3DWorldCamera.m_orientation.m_rollDegrees,  -45.0f, 45.0f );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::UpdateGameMode3DCamera()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Camera follows mouse
	//----------------------------------------------------------------------------------------------------------------------
	// Implement displacement.x to yaw and displacement.y to pitch
	//----------------------------------------------------------------------------------------------------------------------

	// Update WORLD camera as perspective
	Vec2 cursorClientDelta								  = g_theInput->GetCursorClientDelta();
	float mouseSpeed									  = 0.05f;
	float yaw											  = cursorClientDelta.x * mouseSpeed;
	float pitch											  = cursorClientDelta.y * mouseSpeed;
	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees	 -= yaw;
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees += pitch;
	
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees = GetClamped( m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees, -85.0f, 85.0f );

	if ( g_debugFreeFly_F1 )
	{
		// Set cameraPos roam "freely"
		m_gameMode3DWorldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 10000.0f );
		m_gameMode3DWorldCamera.SetRenderBasis( Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) );
	}

	// Update UI camera
	m_gameMode3DUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );		
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->BeginCamera( m_gameMode3DWorldCamera );

	// Initialize common render variables
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> creatureVerts;
	std::vector<Vertex_PCU> creatureVerts2;
	std::vector<Vertex_PCU> compassVerts;
	std::vector<Vertex_PCU> verts_NoBackfaceCull;
	std::vector<Vertex_PCU> verts_model;

	//----------------------------------------------------------------------------------------------------------------------
	// Render world compass
	//----------------------------------------------------------------------------------------------------------------------
	AddVertsForCompass( compassVerts, Vec3( 0.0, 0.0f, 0.0f ), 100.0f, 0.1f );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Render skybox
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> verts_skybox;
	AddVertsForSphere3D( verts_skybox, Vec3(0.0f, 0.0f, 100.0f), 1000.0f, 32.0f, 32.0f );
	//	AddVertsForQuad3D( verts_skybox, Vec3(1000.0f, -1000.0f, -1000.0f), Vec3(-1000.0f, -1000.0f, -1000.0f), Vec3(-1000.0f, -1000.0f, 1000.0f), Vec3(1000.0f, -1000.0f, 1000.0f), Rgba8::WHITE, AABB2::ZERO_TO_ONE );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theApp->m_texture_Skybox );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_skybox.size() ), verts_skybox.data() );
	// Reset bindings
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );


	//----------------------------------------------------------------------------------------------------------------------
	// "ProtoMode" title
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 textOrigin = Vec3( 200.0f, 150.0f,  0.0f );
	Vec3 iBasis		= Vec3(	  0.0f,  -1.0f,  0.0f );
	Vec3 jBasis		= Vec3(	  0.0f,   0.0f,  1.0f );
	g_theApp->m_textFont->AddVertsForText3D( textVerts, textOrigin, iBasis, jBasis, 25.0f, "Robot Arm 3D!", Rgba8::GREEN );

	DebugRenderWorld ( m_gameMode3DWorldCamera );
	DebugRenderScreen( m_gameMode3DUICamera );

	// Render food manager
	if ( m_foodManager )
	{
		m_foodManager->Render( verts );
	}


	//----------------------------------------------------------------------------------------------------------------------
	// Render CCD creature
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_debugToggleMesh_Backspace )
	{
		m_creatureCCD->RenderMesh( g_theRenderer, g_theApp->m_texture_TestOpenGL );
	}
	{
		m_creatureCCD->Render( creatureVerts, Rgba8::WHITE, Rgba8::YELLOW );
	}
//	m_ikChain_CCD->DebugDrawTarget( verts, 0.5f, Rgba8::MAGENTA, true, 6.0f );
	m_ikChain_CCD->RenderTargetMesh( g_theRenderer );
	// Model body
	float length = 1.0f;
	AddVertsForAABB3D( verts_model, AABB3( Vec3( -length, -length, -length ), Vec3( length, length, length ) ), Rgba8::BROWN ); 

	if ( g_debugRenderRaycast_F2 )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Debug rendering chain solutions per iteration
		//----------------------------------------------------------------------------------------------------------------------	
		unsigned char brightness = 180;
		if ( m_ikChain_CCD->m_solutionsThisFrameList.size() > 0 && m_ikChain_CCD->m_isSingleStep_Debug )
		{
			ChainInfo& curChainInfo  = m_ikChain_CCD->m_solutionsThisFrameList[ m_chainInfoIndex ];
			for ( int jointInfoIndex = 0; jointInfoIndex < curChainInfo.m_jointInfoList.size(); jointInfoIndex++ )
			{
				JointInfo& curJointInfo = curChainInfo.m_jointInfoList[jointInfoIndex];
				curJointInfo.m_jointPos_MS;
				Vec3 fwd, left, up;
				curJointInfo.m_eulerAngles_MS.GetAsVectors_XFwd_YLeft_ZUp( fwd, left, up );
				// Render jointPos, fwd, left, up
				Vec3 const& jointPos = curJointInfo.m_jointPos_WS;
				AddVertsForSphere3D( verts, jointPos, 1.0f, 16.0f, 16.0f );		// JointPos
				float   arrowLength = 10.0f;
				Vec3	fwdEnd		= jointPos + ( curJointInfo.m_fwdDir_WS  * arrowLength );  
				arrowLength			= 5.0f;
				Vec3	leftEnd		= jointPos + ( curJointInfo.m_leftDir_WS * arrowLength );
				Vec3	upEnd   	= jointPos + ( curJointInfo.m_upDir_WS	 * arrowLength );
				AddVertsForArrow3D( verts, jointPos, fwdEnd,  0.5f, Rgba8(255, brightness, brightness, 200) );		// Fwd
				AddVertsForArrow3D( verts, jointPos, leftEnd, 0.5f, Rgba8(brightness, 255, brightness, 200) );		// Left
				AddVertsForArrow3D( verts, jointPos, upEnd,   0.5f, Rgba8(brightness, brightness, 255, 200) );		// Up
			}
		}
	}
	if ( g_debugBasis_F3 )
	{
		m_ikChain_CCD->DebugDrawJoints_IJK( verts, 0.2f, 5.0f );
	}
	if ( g_debugText_F4 )
	{
		Vec3 left = m_gameMode3DWorldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
		Vec3 up	  = m_gameMode3DWorldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
		m_ikChain_CCD->DebugTextJointPos_LocalSpace( textVerts, 0.75f, 2.0f, left, up, g_theApp->m_textFont );
	}
	if ( g_debugAngles_F5 )
	{
		m_ikChain_CCD->DebugDrawConstraints_YPR( verts_NoBackfaceCull, 4.0f );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Debug render debug noise position
	//----------------------------------------------------------------------------------------------------------------------
	AddVertsForSphere3D( verts, m_debugNoisePos, 0.5f, 16.0f, 16.0f, Rgba8::MAGENTA );

	//----------------------------------------------------------------------------------------------------------------------
	// Render garbage can
	//----------------------------------------------------------------------------------------------------------------------
	AddVertsForCylinder3D( verts, m_trashCanBottomPos, m_trashCanTopPos, 5.0f, Rgba8::MAGENTA ); 

	//----------------------------------------------------------------------------------------------------------------------
	// End world camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->EndCamera( m_gameMode3DWorldCamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw calls 
	//----------------------------------------------------------------------------------------------------------------------
	// World objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );
	
	// Creature verts
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theApp->m_texture_TestOpenGL );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( creatureVerts.size() ), creatureVerts.data() );

	// IK chain world pos verts
	Mat44 modelMat = m_ikChain_CCD->m_eulerAngles_WS.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMat.SetTranslation3D( m_ikChain_CCD->m_position_WS );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( modelMat );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_model.size() ), verts_model.data() );
	
	// Creature verts2
	creatureVerts2 = creatureVerts;
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	Mat44 mat;
	mat.SetIJK3D( -Vec3::X_FWD, -Vec3::Y_LEFT, Vec3::Z_UP );
	mat.SetTranslation3D( Vec3( -2.0f, 0.0f, 0.0f ) );
	g_theRenderer->SetModelConstants( mat );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
//	g_theRenderer->DrawVertexArray( static_cast<int>( creatureVerts2.size() ), creatureVerts2.data() );

	// World objects, no back face cull
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_NoBackfaceCull.size() ), verts_NoBackfaceCull.data() );
	// Reset cull mode to default
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	
	// Text objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	// Reset binded texture
	g_theRenderer->BindTexture( nullptr );

	// Compass 
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( compassVerts.size() ), compassVerts.data() );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_gameMode3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	Rgba8 textColor_darkRed		= Rgba8::GREEN;
	Rgba8 textColor_darkGreen	= Rgba8::GREEN;
	float cellHeight			= 2.0f;
	AABB2 textbox1				= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	Vec2 alignment				= Vec2( 0.0f, 1.0f );
	
	std::string gameInfo;
	if ( g_debugFreeFly_F1 )
	{
		gameInfo = Stringf( "F1 (Toggle camera/player) Camera || Mode (F6/F7 for prev/next) || Robot Arm (3D)" );
	}						
	else
	{
		gameInfo = Stringf( "F1 (Toggle camera/player) Player || Mode (F6/F7 for prev/next) || Robot Arm (3D)" );
	}
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, gameInfo, Rgba8::GREEN, 0.75f, alignment, TextDrawMode::SHRINK_TO_FIT );

	// Core Values
	float fps							= 1.0f / g_theApp->m_gameClock.GetDeltaSeconds();
	float scale							= g_theApp->m_gameClock.GetTimeScale();
	std::string cameraPosText			= Stringf( "Cam position:           %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_position.x,				m_gameMode3DWorldCamera.m_position.y,				  m_gameMode3DWorldCamera.m_position.z );
	std::string cameraOrientationText	= Stringf( "Cam Orientation (YPR):  %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees,	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees, m_gameMode3DWorldCamera.m_orientation.m_rollDegrees );
	std::string timeText				= Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.",			g_theApp->m_gameClock.GetTotalSeconds(), fps, scale );
	std::string numIterations_FABRIK	= Stringf( "numIterations_FABRIK:    %i",					m_ikChain_CCD->m_numIterations );
	std::string iterationsSolved_FABRIK	= Stringf( "IterationsSolved_FABRIK: %i",					m_ikChain_CCD->m_iterCount );
	std::string stepModeFABRIK 			= Stringf( "Step mode: FABRIK\n").c_str();
	if ( m_ikChain_CCD->m_isSingleStep_Debug )
	{
		stepModeFABRIK = Stringf( "Step mode: Debug\n").c_str();
	}
	std::string chainInfoIndex			= Stringf( "chainInfoIndex: %i\n", m_chainInfoIndex ).c_str();
	std::string isDeadlock				= Stringf( "IsDeadlock: FALSE\n" ).c_str();;
	if ( m_ikChain_CCD->m_isDeadlock )
	{
		isDeadlock = Stringf( "IsDeadlock: TRUE\n" ).c_str();
	}
	std::string solveRoll = Stringf( "Solve Roll: FALSE\n" ).c_str();;
	if ( m_ikChain_CCD->m_solveRoll )
	{
		solveRoll = Stringf( "Solve Roll: TRUE\n" ).c_str();
	}
	std::string isTargetTooFar = Stringf( "IsTargetTooFar: FALSE\n" ).c_str();;
	if ( m_ikChain_CCD->m_isTargetTooFar )
	{
		isTargetTooFar = Stringf( "IsTargetTooFar: TRUE\n" ).c_str();
	}
	std::string renderLerpedToGoalEuler = Stringf( "RenderLerpToGoalEuler: FALSE\n" ).c_str();
	if ( m_ikChain_CCD->m_lerpEulerToGoal )
	{		
		renderLerpedToGoalEuler = Stringf( "RenderLerpToGoalEuler: TRUE\n" ).c_str();
	}
	std::string attemptDeadlockSolutions = Stringf( "AttemptDeadlockSolutions: FALSE\n" ).c_str();
	if ( m_ikChain_CCD->m_attemptDeadlockSolutions )
	{
		attemptDeadlockSolutions = Stringf( "AttemptDeadlockSolutions: TRUE\n" ).c_str();
	}		

	float textAlignmentY	= 1.0f;
	float textHeight		= 0.03f;
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			 	   timeText, textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY ), TextDrawMode::SHRINK_TO_FIT );	
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			  cameraPosText, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,    cameraOrientationText, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,	   numIterations_FABRIK, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,	iterationsSolved_FABRIK, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			 stepModeFABRIK, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			 chainInfoIndex, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,				 isDeadlock, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,				  solveRoll, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			 isTargetTooFar, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,	renderLerpedToGoalEuler, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, attemptDeadlockSolutions, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );


	textAlignmentY = 0.97f;
	if ( g_debugText_F4 )
	{
		// Target YPR WS
		EulerAngles targetEuler = EulerAngles::GetAsEuler_XFwd_YLeft_ZUp( m_ikChain_CCD->m_target.m_fwdDir, m_ikChain_CCD->m_target.m_leftDir );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,
			Stringf( "Target YPR: %0.2f, %0.2f, %0.2f", targetEuler.m_yawDegrees, targetEuler.m_pitchDegrees, targetEuler.m_rollDegrees ),
			textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );

		// IK chain WS pos
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
														Stringf( "IK_Chain pos_WS: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_position_WS.x, 
																										 m_ikChain_CCD->m_position_WS.y, 
																										 m_ikChain_CCD->m_position_WS.z ),
			textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );

		// IK chain WS euler
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
			Stringf( "IK_Chain YPR_WS: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_eulerAngles_WS.m_yawDegrees, 
				m_ikChain_CCD->m_eulerAngles_WS.m_pitchDegrees, 
				m_ikChain_CCD->m_eulerAngles_WS.m_rollDegrees ),
			textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );

		//----------------------------------------------------------------------------------------------------------------------
		// Joint 0 (root)
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
			Stringf( "Joint0_YPR: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_firstJoint->m_eulerAnglesGoal_LS.m_yawDegrees,
				m_ikChain_CCD->m_firstJoint->m_eulerAnglesGoal_LS.m_pitchDegrees,
				m_ikChain_CCD->m_firstJoint->m_eulerAnglesGoal_LS.m_rollDegrees ),	
			textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight * 2.0f ), TextDrawMode::SHRINK_TO_FIT );
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 1
		if ( m_ikChain_CCD->m_jointList[1] )
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
				Stringf( "Joint1_YPR: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_jointList[1]->m_eulerAnglesGoal_LS.m_yawDegrees,
					m_ikChain_CCD->m_jointList[1]->m_eulerAnglesGoal_LS.m_pitchDegrees,
					m_ikChain_CCD->m_jointList[1]->m_eulerAnglesGoal_LS.m_rollDegrees ),	
				textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 2
		if ( m_ikChain_CCD->m_jointList.size() >= 3 )
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
															Stringf( "Joint2_YPR: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_jointList[2]->m_eulerAnglesGoal_LS.m_yawDegrees,
																										m_ikChain_CCD->m_jointList[2]->m_eulerAnglesGoal_LS.m_pitchDegrees,
																										m_ikChain_CCD->m_jointList[2]->m_eulerAnglesGoal_LS.m_rollDegrees ),	
				textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );		
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 3
		if ( m_ikChain_CCD->m_jointList.size() >= 4  )
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
				Stringf( "Joint3_YPR: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_jointList[3]->m_eulerAnglesGoal_LS.m_yawDegrees,
					m_ikChain_CCD->m_jointList[3]->m_eulerAnglesGoal_LS.m_pitchDegrees,
					m_ikChain_CCD->m_jointList[3]->m_eulerAnglesGoal_LS.m_rollDegrees ),	
				textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );		
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 4
		if ( m_ikChain_CCD->m_jointList.size() >= 5 )
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
				Stringf( "Joint4_YPR: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_jointList[4]->m_eulerAnglesGoal_LS.m_yawDegrees,
					m_ikChain_CCD->m_jointList[4]->m_eulerAnglesGoal_LS.m_pitchDegrees,
					m_ikChain_CCD->m_jointList[4]->m_eulerAnglesGoal_LS.m_rollDegrees ),	
				textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );		
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 5
		if ( m_ikChain_CCD->m_jointList.size() >= 6 )
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
				Stringf( "Joint5_YPR: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_jointList[5]->m_eulerAnglesGoal_LS.m_yawDegrees,
					m_ikChain_CCD->m_jointList[5]->m_eulerAnglesGoal_LS.m_pitchDegrees,
					m_ikChain_CCD->m_jointList[5]->m_eulerAnglesGoal_LS.m_rollDegrees ),	
				textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );		
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 6
		if ( m_ikChain_CCD->m_jointList.size() >= 7 )
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
															Stringf( "Joint6_YPR: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_jointList[6]->m_eulerAnglesGoal_LS.m_yawDegrees,
																										m_ikChain_CCD->m_jointList[6]->m_eulerAnglesGoal_LS.m_pitchDegrees,
																										m_ikChain_CCD->m_jointList[6]->m_eulerAnglesGoal_LS.m_rollDegrees ),	
				textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );		
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 7
		if ( m_ikChain_CCD->m_jointList.size() >= 8 )
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
				Stringf( "Joint7_YPR: %0.2f, %0.2f, %0.2f", m_ikChain_CCD->m_jointList[7]->m_eulerAnglesGoal_LS.m_yawDegrees,
					m_ikChain_CCD->m_jointList[7]->m_eulerAnglesGoal_LS.m_pitchDegrees,
					m_ikChain_CCD->m_jointList[7]->m_eulerAnglesGoal_LS.m_rollDegrees ),	
				textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );		
		}

		// DistEeToTarget 
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, Stringf( "DistEeToTarget:   %0.2f", m_ikChain_CCD->m_distEeToTarget ),
			textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );

		// EulerLastFrame
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
			Stringf( "Joint1 YPR last Frame: %0.2f, %0.2f, %0.2f", 
				m_ikChain_CCD->m_jointList[ 1 ]->m_eulerLS_LastFrame.m_yawDegrees,
				m_ikChain_CCD->m_jointList[ 1 ]->m_eulerLS_LastFrame.m_pitchDegrees,
				m_ikChain_CCD->m_jointList[ 1 ]->m_eulerLS_LastFrame.m_rollDegrees ),
			textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );


		//----------------------------------------------------------------------------------------------------------------------
		// Debug noise values (sine, cos, perlin)
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, Stringf( "Sine:   %0.2f", m_debugSine			), textColor_darkGreen, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, Stringf( "Cos:    %0.2f", m_debugCos			), textColor_darkGreen, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, Stringf( "Perlin: %0.2f", m_debugPerlinNoise	), textColor_darkGreen, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
	}

	
	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_gameMode3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw for UI camera text
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
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
void GameMode_RobotArm_3D::UpdateTargetInput( float deltaSeconds )
{
	if ( g_debugFollowTarget_F11 )
	{
		float time								= g_theApp->m_gameClock.GetTotalSeconds();
		float sine								= SinDegrees( time * 20.0f ) * 75.0f;
		float cos								= CosDegrees( time * 20.0f ) * 75.0f;
		float perlinNoise						= Compute2dPerlinNoise( time * 0.5f, 0.0f ) * 65.0f;
//		sine *= 65.0f;
//		cos  *= 65.0f;
		m_ikChain_CCD->m_target.m_currentPos_WS	= Vec3( 0.0f + cos, 0.0f + sine, 0.0f + perlinNoise );
		m_debugNoisePos							= m_ikChain_CCD->m_target.m_currentPos_WS;
//		m_ikChain_CCD->m_target.m_currentPos_WS	= Vec3( 0.0f + perlinNoise, 0.0f -fasterCos, 0.0f + cos );
		m_debugSine								= sine;
		m_debugCos								= cos;
		m_debugPerlinNoise						= perlinNoise;
//		m_ikChain_CCD->m_target.m_currentPos_WS.
	}
	else
	{
		float stepAmount = m_defaultSpeed;
		if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
		{
			stepAmount *= 2.0f;
		}

		// East (+X)
		if ( g_theInput->IsKeyDown( 'I' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos_WS += Vec3( stepAmount, 0.0f, 0.0f ) * deltaSeconds;
		}
		// West (-X)
		if ( g_theInput->IsKeyDown( 'K' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos_WS += Vec3( -stepAmount, 0.0f, 0.0f ) * deltaSeconds;
		}
		// North (+Y)
		if ( g_theInput->IsKeyDown( 'J' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos_WS += Vec3( 0.0f, stepAmount, 0.0f ) * deltaSeconds;
		}
		// South (-Y)
		if ( g_theInput->IsKeyDown( 'L' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos_WS += Vec3( 0.0f, -stepAmount, 0.0f ) * deltaSeconds;
		}
		// Sky (+Z)
		if ( g_theInput->IsKeyDown( 'O' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos_WS += Vec3( 0.0f, 0.0f, stepAmount ) * deltaSeconds;
		}
		// Ground (-Z)
		if ( g_theInput->IsKeyDown( 'U' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos_WS += Vec3( 0.0f, 0.0f, -stepAmount ) * deltaSeconds;
		}
		if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) )
		{
			// Negative roll
			m_ikChain_CCD->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_ikChain_CCD->m_target.m_upDir,   m_ikChain_CCD->m_target.m_fwdDir,  1.0f );
			m_ikChain_CCD->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_ikChain_CCD->m_target.m_leftDir, m_ikChain_CCD->m_target.m_fwdDir,  1.0f );

		}
		if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
		{
			// Positive roll
			m_ikChain_CCD->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_ikChain_CCD->m_target.m_upDir,   m_ikChain_CCD->m_target.m_fwdDir,  -1.0f );
			m_ikChain_CCD->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_ikChain_CCD->m_target.m_leftDir, m_ikChain_CCD->m_target.m_fwdDir,  -1.0f );
		}
		if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) )
		{
			// Negative pitch
			m_ikChain_CCD->m_target.m_fwdDir = RotateVectorAboutArbitraryAxis( m_ikChain_CCD->m_target.m_fwdDir, m_ikChain_CCD->m_target.m_leftDir, -1.0f );
			m_ikChain_CCD->m_target.m_upDir  = RotateVectorAboutArbitraryAxis( m_ikChain_CCD->m_target.m_upDir,  m_ikChain_CCD->m_target.m_leftDir, -1.0f );

		}
		if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) )
		{
			// Positive pitch
			m_ikChain_CCD->m_target.m_fwdDir = RotateVectorAboutArbitraryAxis( m_ikChain_CCD->m_target.m_fwdDir, m_ikChain_CCD->m_target.m_leftDir, 1.0f );
			m_ikChain_CCD->m_target.m_upDir  = RotateVectorAboutArbitraryAxis( m_ikChain_CCD->m_target.m_upDir,  m_ikChain_CCD->m_target.m_leftDir, 1.0f );
		}


		// Preset positions for target
		// X
		if ( g_theInput->WasKeyJustPressed( '1' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos_WS = Vec3( 30.0f, 0.0f, 0.0f );
		}
		// Y
		if ( g_theInput->WasKeyJustPressed( '2' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos_WS = Vec3( 0.0f, 30.0f, 0.0f );
		}
		// Z
		if ( g_theInput->WasKeyJustPressed( '3' ) )
		{
			m_ikChain_CCD->m_target.m_currentPos_WS = Vec3( 0.0f, 0.0f, 30.0f );
		}

	/*
		//----------------------------------------------------------------------------------------------------------------------
		// Rotate endEffector orientation
		//----------------------------------------------------------------------------------------------------------------------
		if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) )
		{
			// Rotate "yaw" CCW
			m_spine->m_target.m_fwdDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_upDir,	  1.0f  );
			m_spine->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_upDir,	  90.0f );
			m_spine->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_leftDir, -90.0f );
		}
		if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
		{
			// Rotate "yaw" CW
			m_spine->m_target.m_fwdDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_upDir,	  -1.0f );
			m_spine->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_upDir,	  90.0f );
			m_spine->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_leftDir, -90.0f );
		}
		if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) )
		{
			// Rotate positive "Pitch" 
			m_spine->m_target.m_fwdDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_leftDir,   1.0f );
			m_spine->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_leftDir, -90.0f );
			m_spine->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_upDir,    90.0f );
		}
		if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) )
		{
			// Rotate negative "Pitch" 
			m_spine->m_target.m_fwdDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_leftDir,  -1.0f );
			m_spine->m_target.m_upDir		= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_leftDir, -90.0f );
			m_spine->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_spine->m_target.m_fwdDir, m_spine->m_target.m_upDir,    90.0f );
		}
	*/
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::UpdateCreature( float deltaSeconds )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Only solve 1 frame when key is pressed
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->WasKeyJustPressed( KEYCODE_RIGHTBRACKET ) )
	{
		m_ikChain_CCD->m_isSingleStep_Debug = !m_ikChain_CCD->m_isSingleStep_Debug;
		if ( !m_ikChain_CCD->m_isSingleStep_Debug )
		{
			m_chainInfoIndex = 0;
		}
	}
	if ( m_ikChain_CCD->m_isSingleStep_Debug )
	{
		if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFTBRACKET ) )
		{
			m_creatureCCD->Update( deltaSeconds );
		}
	}
	else
	{
		m_creatureCCD->Update( deltaSeconds );
//		RobotCustomUpdate( deltaSeconds );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::InitializeIK_ChainCCD()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Creature
	//----------------------------------------------------------------------------------------------------------------------
	// Create creature root and arm
	m_creatureCCD						= new CreatureBase( Vec3( 0.0f, 0.0f, 0.0f ) );
	m_ikChain_CCD						= m_creatureCCD->CreateChildChain( "m_ikChain_CCD", Vec3::ZERO, nullptr, true );
	m_ikChain_CCD->m_solverType			= CHAIN_SOLVER_CCD;
	m_ikChain_CCD->AddNewJointToChain( Vec3(		 0.0f, 0.0f, 0.0f ), EulerAngles() );		// Root  0
	m_ikChain_CCD->AddNewJointToChain( Vec3( m_boneLength, 0.0f, 0.0f ), EulerAngles() );		// Child 1
	m_ikChain_CCD->AddNewJointToChain( Vec3( m_boneLength, 0.0f, 0.0f ), EulerAngles() );		// Child 2
   	m_ikChain_CCD->AddNewJointToChain( Vec3( m_boneLength, 0.0f, 0.0f ), EulerAngles() );		// Child 3
   	m_ikChain_CCD->AddNewJointToChain( Vec3( m_boneLength, 0.0f, 0.0f ), EulerAngles() );		// Child 4
 	// Actual parameters
	SetIK_ChainConstraints();
	m_ikChain_CCD->m_target.m_currentPos_WS = Vec3( 30.0f, 0.0f, 0.0f );
	m_ikChain_CCD->m_numIterations		    = 100;
	m_ikChain_CCD->m_solveRoll				= false;

	// Set solverTypes
	m_ikChain_CCD->SetSolverType( CHAIN_SOLVER_CCD );
	// Set matrix to euler preference 
//	m_ikChain_CCD->SetChainPreference_MatrixToEuler( FWD_TEMPORAL );
	m_ikChain_CCD->m_jointList[0]->m_matrixToEulerPreference = YAW_PREFERRED;
	m_ikChain_CCD->m_jointList[1]->m_matrixToEulerPreference = PITCH_PREFERRED;
	m_ikChain_CCD->m_jointList[2]->m_matrixToEulerPreference = PITCH_PREFERRED;
	m_ikChain_CCD->m_jointList[3]->m_matrixToEulerPreference = PITCH_PREFERRED;
	m_ikChain_CCD->m_jointList[4]->m_matrixToEulerPreference = FWD_TEMPORAL;
	// Set euler from fwd and left preference
//	m_ikChain_CCD->m_jointList[0]->m_matrixToEulerPreference = FWD_LEFT_TEMPORAL;
	// Deadlock solver type
	m_ikChain_CCD->m_deadlockSolverType = DEADLOCK_SOLVER_DEBT_ANGLE;
	// Disable deadlock solutions
	m_ikChain_CCD->m_attemptDeadlockSolutions = true; 
	// Set lerping turn rate
	m_ikChain_CCD->m_turnRate				= 624.0f; 
	m_ikChain_CCD->m_isSingleStep_Debug		= false;
	m_ikChain_CCD->m_target.m_currentPos_WS = Vec3( -66.0f, 0.0f, 25.0f );
	m_ikChain_CCD->m_lerpEulerToGoal		= true;

	//----------------------------------------------------------------------------------------------------------------------
	// Init jointMesh
	//----------------------------------------------------------------------------------------------------------------------
	float startRadius	= 1.0f;
	float endRadius		= 0.0f;
	m_creatureCCD->InitJointMeshes_Ellipsoid( m_ikChain_CCD, Vec3::ZERO, Vec3( m_boneLength, 0.0f, 0.0f ), startRadius, endRadius, Vec2(1.0f, 1.0f), Vec2(1.0f, 1.0f) );

	// Default solve once to get a pose
//	m_creatureCCD->Update( 0.016f );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::SetIK_ChainConstraints()
{
// 	m_ikChain_CCD->m_jointList[0]->SetConstraints_YPR( FloatRange( -135.0f, 135.0f ), FloatRange( -135.0f, 135.0f ), FloatRange( -15.0f, 15.0f ) );
// 	m_ikChain_CCD->m_jointList[1]->SetConstraints_YPR( FloatRange(   -0.0f,   0.0f ), FloatRange(  -15.0f, 165.0f ), FloatRange(  -0.0f,  0.0f ) );
// 	m_ikChain_CCD->m_jointList[2]->SetConstraints_YPR( FloatRange(  -15.0f, -15.0f ), FloatRange(  -45.0f,  85.0f ), FloatRange(  -0.0f,  0.0f ) );

	if ( g_debugToggleConstraints_F8 )
	{
		for ( int i = 0; i < m_ikChain_CCD->m_jointList.size(); i++ )
		{
			IK_Joint3D* currentJoint = m_ikChain_CCD->m_jointList[ i ];
			currentJoint->SetConstraints_YPR();
			// evens
//  		int remainder = i % 2;
//  		if ( remainder == 0 )
//  		{
//  			currentJoint->SetConstraints_YPR( FloatRange( -75.0f, 75.0f ), FloatRange( 0.0f, 0.0f ), FloatRange( -0.0f, 0.0f ) );
//  		}
//  		// odds
//  		else
//  		{
//  			currentJoint->SetConstraints_YPR( FloatRange( 0.0f, 0.0f ), FloatRange( -75.0f, 75.0f ), FloatRange( -0.0f, 0.0f ) );
//  		}

			// Cone constraints
			currentJoint->SetConstraints_YPR( FloatRange( -75.0f, 75.0f ), FloatRange( -75.0f, 75.0f ), FloatRange( -75.0f, 75.0f ) );

			// Custom robot arm constraints
// 			m_ikChain_CCD->m_jointList[0]->SetConstraints_YPR( FloatRange(-180.0f, 180.0f), FloatRange( -70.0f, -89.0f), FloatRange(0.0f, 0.0f) );
// 			m_ikChain_CCD->m_jointList[1]->SetConstraints_YPR( FloatRange(  -0.0f,   0.0f), FloatRange(-180.0f, 180.0f), FloatRange(0.0f, 0.0f) );
// 			m_ikChain_CCD->m_jointList[2]->SetConstraints_YPR( FloatRange(  -0.0f,   0.0f), FloatRange(-180.0f, 180.0f), FloatRange(0.0f, 0.0f) );
// 			m_ikChain_CCD->m_jointList[3]->SetConstraints_YPR( FloatRange(  -0.0f,   0.0f), FloatRange(-180.0f, 180.0f), FloatRange(0.0f, 0.0f) );
// 			m_ikChain_CCD->m_jointList[4]->SetConstraints_YPR( FloatRange(  -0.0f,   0.0f), FloatRange(  -0.0f,   0.0f), FloatRange(0.0f, 0.0f) );
		}
	}
	else
	{
 		for ( int i = 0; i < m_ikChain_CCD->m_jointList.size(); i++ )
 		{
 			IK_Joint3D* currentJoint = m_ikChain_CCD->m_jointList[ i ];
 			currentJoint->SetConstraints_YPR( FloatRange( -180.0f, 180.0f ), FloatRange( -180.0f, 180.0f ), FloatRange( -180.0f, 180.0f ) );
 		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_RobotArm_3D::RobotCustomUpdate( float deltaSeconds )
{
	//----------------------------------------------------------------------------------------------------------------------
	// 1. Rotate Yaw for joint0 till -kBasis aligns with dirToTarget
	//----------------------------------------------------------------------------------------------------------------------
 	// Compute dirToTargetXY_LS
 	IK_Joint3D* rootJoint				= m_ikChain_CCD->m_jointList[0];
 	Mat44		worldToLocal_Matrix		= rootJoint->GetIkChainMatrix_ModelToWorld().GetOrthoNormalInverse();
 	Vec3		rootJointPos_LS			= rootJoint->m_jointPos_LS;
 	Vec3		targetPos_LS			= worldToLocal_Matrix.TransformPosition3D( m_ikChain_CCD->m_target.m_currentPos_WS );
 	targetPos_LS						= Vec3( targetPos_LS.x, targetPos_LS.y, rootJointPos_LS.z );
 	Vec3		dirToTarget_LS			= ( targetPos_LS - rootJointPos_LS ).GetNormalized();
 	// Rotate Yaw till aligned with dirToTargetXY
 	Vec3  rootKBasis3D_LS				= rootJoint->m_eulerAnglesGoal_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
 	Vec3  rootIBasis3D_LS				= rootJoint->m_eulerAnglesGoal_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
 	Vec3  rootJBasis3D_LS				= rootJoint->m_eulerAnglesGoal_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
// 	Vec3  newIBasis						= RotateVectorAboutArbitraryAxis( dirToTarget_LS, rootJBasis3D_LS, -89.0f );
 	Vec3  newIBasis						= RotateVectorAboutArbitraryAxis( dirToTarget_LS, Vec3::Y_LEFT, -89.0f );
 	newIBasis.Normalize();
 	// Update joint data
 	rootJoint->m_eulerAnglesGoal_LS		= rootJoint->GetEulerFromFwdDir( newIBasis );
 	rootJoint->m_eulerAnglesCurrent_LS	= rootJoint->m_eulerAnglesGoal_LS;
 	// Check if -kbasis aligns with dirToTarget
//  float dotResult						= DotProduct3D( newIBasis, -rootKBasis3D_LS );
//  if ( dotResult > 0.9f )
//  {
 		m_creatureCCD->Update( deltaSeconds );	
// 	}

	//----------------------------------------------------------------------------------------------------------------------
	// 1. Rotate Yaw for joint0 till -kBasis aligns with dirToTarget
	//----------------------------------------------------------------------------------------------------------------------
// 	// Compute dirToTargetXY_LS
// 	IK_Joint3D* rootJoint				= m_ikChain_CCD->m_jointList[0];
// 	Mat44		worldToLocal_Matrix		= rootJoint->GetMatrix_LocalToWorld( true ).GetOrthoNormalInverse();
// 	Vec3		rootJointPos_LS			= rootJoint->m_jointPos_LS;
// 	Vec3		targetPos_LS			= worldToLocal_Matrix.TransformPosition3D( m_ikChain_CCD->m_target.m_currentPos_WS );
// 	targetPos_LS						= Vec3( targetPos_LS.x, targetPos_LS.y, rootJointPos_LS.z );
// 	Vec3		dirToTarget_LS			= ( targetPos_LS - rootJointPos_LS ).GetNormalized();
// 	// Rotate Yaw till aligned with dirToTargetXY
// 	Vec3  rootKBasis3D_LS				= rootJoint->m_eulerAnglesGoal_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
// 	Vec3  rootIBasis3D_LS				= rootJoint->m_eulerAnglesGoal_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
// 	float angleDisp						= GetSignedAngleDegreesBetweenVectors( dirToTarget_LS, rootKBasis3D_LS, rootIBasis3D_LS );
// //	float degreesToTurn					= Interpolate( 0.0f, angleDisp, deltaSeconds * 2.0f );
// 	Vec3  rotatedNegativeKbasis			= RotateVectorAboutArbitraryAxis( dirToTarget_LS, rootIBasis3D_LS, angleDisp );
// 	rotatedNegativeKbasis.Normalize();
// 	// Update joint data
// 	rootJoint->m_eulerAnglesGoal_LS		= rootJoint->GetEulerFromFwdAndLeft( rootIBasis3D_LS, -rotatedNegativeKbasis );
// 	rootJoint->m_eulerAnglesCurrent_LS	= rootJoint->m_eulerAnglesGoal_LS;
// 	// Check if -kbasis aligns with dirToTarget
// 	float dotResult						= DotProduct3D( rotatedNegativeKbasis, -rootKBasis3D_LS );
// 	if ( dotResult > 0.9f )
// 	{
// 		m_creatureCCD->Update( deltaSeconds );	
// 	}
}
