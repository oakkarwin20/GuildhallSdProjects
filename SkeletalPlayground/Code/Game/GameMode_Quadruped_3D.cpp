#include "Game/App.hpp"
#include "Game/GameMode_Quadruped_3D.hpp"
#include "Game/Quadruped.hpp"
#include "Game/Map_GameMode3D.hpp"

#include "Engine/ThirdParty/Squirrel/Noise/SmoothNoise.hpp"
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
#include "Engine/SkeletalSystem/CreatureBase.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameMode_Quadruped_3D::GameMode_Quadruped_3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameMode_Quadruped_3D::~GameMode_Quadruped_3D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::Startup()
{
	g_debugFreeFly_F1		= true;
	g_debugRenderRaycast_F2	= false;
	g_debugAngles_F5		= false;
	g_debugFollowTarget_F11 = true;

	//----------------------------------------------------------------------------------------------------------------------
	// Init Map (blocks)
	//----------------------------------------------------------------------------------------------------------------------
	InitFloorAABB3();
	InitDynamicTerrain();

	//----------------------------------------------------------------------------------------------------------------------
	// Quadruped
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped = new Quadruped( this, Vec3( 0.0f, 0.0f, m_rootDefaultHeightZ ), 1.0f );
	m_quadruped->InitLimbs();
	m_quadruped->m_neck_FABRIK->m_target.m_goalPos_WS	 = Vec3( 50.0f, 0.0f, 20.0f );
	m_quadruped->m_neck_FABRIK->m_target.m_currentPos_WS = Vec3( 50.0f, 0.0f, 20.0f );
	m_quadruped->m_head_FABRIK->m_target.m_goalPos_WS	 = Vec3( 50.0f, 0.0f, 20.0f );
	m_quadruped->m_head_FABRIK->m_target.m_currentPos_WS = Vec3( 50.0f, 0.0f, 20.0f );
	// Update once on startup to pose leg targets at the correct positions
	float fakeDeltaSeconds = 0.016f;
	m_quadruped->PreUpdateChains ( fakeDeltaSeconds );
	m_quadruped->Update( fakeDeltaSeconds );
	m_quadruped->PostUpdateChains( fakeDeltaSeconds );
	m_quadruped->SetAllChainsCurTargetPosToTargetGoal();

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Player cam pos and orientation
	//----------------------------------------------------------------------------------------------------------------------
// 	m_gameMode3DWorldCamera.m_position						= Vec3( 0.0f, 50.0f, 10.0f ); 
	m_gameMode3DWorldCamera.m_position						= Vec3( 0.0f, 120.0f, 20.0f );		// Default Y position
 	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees		= -90.0f;
 	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees	=  5.0f;
//	m_gameMode3DWorldCamera.m_position						= Vec3( 30.0f, 18.0f, 7.0f );
//	m_gameMode3DWorldCamera.m_orientation					= EulerAngles( -90.0f, 5.0f, 0.0f );
	// Init targetManipulatedChain
	m_indexTargetChain		 = 6;
	m_targetManipulatedChain = m_quadruped->m_skeletalList[ m_indexTargetChain ];

	//----------------------------------------------------------------------------------------------------------------------
	// Set lighting
	//----------------------------------------------------------------------------------------------------------------------
	EulerAngles sunOrientation = EulerAngles( 0.0f, 65.0f, 0.0f );
	m_sunDirection = sunOrientation.GetForwardDir_XFwd_YLeft_ZUp();
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::Update( float deltaSeconds )
{	
	// Update core systems
	UpdatePauseQuitAndSlowMo();
	UpdateDebugKeys();
	UpdateCameraAndCreatureInput( deltaSeconds );

	//----------------------------------------------------------------------------------------------------------------------
	// Rotate endEffector orientation
	//----------------------------------------------------------------------------------------------------------------------
// 	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW  ) )
// 	{
// 		// Rotate "yaw" CCW
// 		m_leftFoot_FABRIK->m_target.m_fwdDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_fwdDir, m_leftFoot_FABRIK->m_target.m_upDir,     1.0f );
// 		m_leftFoot_FABRIK->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_fwdDir, m_leftFoot_FABRIK->m_target.m_upDir,    90.0f );
// 		m_leftFoot_FABRIK->m_target.m_upDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_fwdDir, m_leftFoot_FABRIK->m_target.m_leftDir, -90.0f );
// 	}
// 	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
// 	{
// 		// Rotate "yaw" CW
// 		m_leftFoot_FABRIK->m_target.m_fwdDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_fwdDir, m_leftFoot_FABRIK->m_target.m_upDir, -1.0f );
// 		m_leftFoot_FABRIK->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_fwdDir, m_leftFoot_FABRIK->m_target.m_upDir, 90.0f );
// 		m_leftFoot_FABRIK->m_target.m_upDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_fwdDir, m_leftFoot_FABRIK->m_target.m_leftDir, -90.0f );
// 	}
// 	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW  ) )
// 	{
// 		// Rotate positive "Pitch" 
// 		m_leftFoot_FABRIK->m_target.m_fwdDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_fwdDir, m_leftFoot_FABRIK->m_target.m_leftDir,   1.0f );
// 		m_leftFoot_FABRIK->m_target.m_upDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_fwdDir, m_leftFoot_FABRIK->m_target.m_leftDir, -90.0f );
// 		m_leftFoot_FABRIK->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_fwdDir, m_leftFoot_FABRIK->m_target.m_upDir,    90.0f );
// 	}
// 	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW    ) )
// 	{
// 		// Rotate negative "Pitch" 
// 		m_leftFoot_FABRIK->m_target.m_fwdDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_fwdDir, m_leftFoot_FABRIK->m_target.m_leftDir,  -1.0f );
// 		m_leftFoot_FABRIK->m_target.m_upDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_fwdDir, m_leftFoot_FABRIK->m_target.m_leftDir, -90.0f );
// 		m_leftFoot_FABRIK->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_fwdDir, m_leftFoot_FABRIK->m_target.m_upDir,    90.0f );
// 	}
// 	if ( g_theInput->IsKeyDown( KEYCODE_COMMA  ) )
// 	{
// 		// Rotate positive "Roll" 
// 		m_leftFoot_FABRIK->m_target.m_upDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_upDir,   m_leftFoot_FABRIK->m_target.m_fwdDir, -1.0f );
// 		m_leftFoot_FABRIK->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_leftDir, m_leftFoot_FABRIK->m_target.m_fwdDir, -1.0f );
// 	}
// 	if ( g_theInput->IsKeyDown( KEYCODE_PERIOD ) )
// 	{
// 		// Rotate positive "Roll" 
// 		m_leftFoot_FABRIK->m_target.m_upDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_upDir,   m_leftFoot_FABRIK->m_target.m_fwdDir,  1.0f );
// 		m_leftFoot_FABRIK->m_target.m_leftDir	= RotateVectorAboutArbitraryAxis( m_leftFoot_FABRIK->m_target.m_leftDir, m_leftFoot_FABRIK->m_target.m_fwdDir,  1.0f );
// 	}

	// Update map "elevators"
	if ( g_debugToggleFloor_F9 )
	{
		float time			= float( g_theApp->m_gameClock.GetTotalSeconds() );
		float sine			= SinDegrees( time * 50.0f ) * deltaSeconds * 2.0f;
		float cos			= CosDegrees( time * 50.0f ) * deltaSeconds * 2.0f;
		for ( int i = 1; i < m_blockList.size(); i++ )
		{
			int   mod			 = i % 2;
			float mathWaveToUse	 = sine;
			if ( mod == 0 )
			{
				mathWaveToUse	 = cos;
			}
			Block* curBlock  = m_blockList[i];
			Vec3 blockCenter = curBlock->m_aabb3.GetCenter();
			curBlock->m_aabb3.SetCenterXYZ( Vec3( blockCenter.x, blockCenter.y, blockCenter.z + mathWaveToUse ) );
		}

		UpdateDynamicTerrain( deltaSeconds );
	}

	UpdateTargetInput( deltaSeconds );
	// Quadruped
 	m_quadruped->PreUpdateChains( deltaSeconds );
 	m_quadruped->Update( deltaSeconds );
 	m_quadruped->PostUpdateChains( deltaSeconds );

	// Update Camera
	UpdateGameMode3DCamera();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::Shutdown()
{
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::UpdateDebugKeys()
{
	// Toggle common debug bools
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		g_debugFreeFly_F1 = !g_debugFreeFly_F1;
		m_quadruped->ToggleIsControlled();
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
		if ( !g_debugToggleConstraints_F8 )
		{
			m_quadruped->SetChainConstraints( m_quadruped->m_leftFoot_2BIK	);
			m_quadruped->SetChainConstraints( m_quadruped->m_rightFoot_2BIK	);
			m_quadruped->SetChainConstraints( m_quadruped->m_spine_CCD			);
			m_quadruped->SetChainConstraints( m_quadruped->m_leftArm_2BIK		);
			m_quadruped->SetChainConstraints( m_quadruped->m_rightArm_2BIK	);
			m_quadruped->SetChainConstraints( m_quadruped->m_tail_CCD			);
			m_quadruped->SetChainConstraints( m_quadruped->m_neck_FABRIK		);
			m_quadruped->SetChainConstraints( m_quadruped->m_head_FABRIK		);
			m_quadruped->SetChainConstraints( m_quadruped->m_tentacleLeft_CCD	);
			m_quadruped->SetChainConstraints( m_quadruped->m_tentacleRight_CCD	);
		}
		else
		{
			m_quadruped->m_leftFoot_2BIK	->ResetAllJointConstraints();
			m_quadruped->m_rightFoot_2BIK	->ResetAllJointConstraints();
			m_quadruped->m_spine_CCD		->ResetAllJointConstraints();
			m_quadruped->m_leftArm_2BIK		->ResetAllJointConstraints();
			m_quadruped->m_rightArm_2BIK	->ResetAllJointConstraints();
			m_quadruped->m_tail_CCD			->ResetAllJointConstraints();
			m_quadruped->m_neck_FABRIK		->ResetAllJointConstraints();
			m_quadruped->m_head_FABRIK		->ResetAllJointConstraints();
			m_quadruped->m_tentacleLeft_CCD	->ResetAllJointConstraints();
			m_quadruped->m_tentacleRight_CCD->ResetAllJointConstraints();
		}
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F9 ) )
	{
		g_debugToggleFloor_F9 = !g_debugToggleFloor_F9;
		InitFloorAABB3();
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F11 ) )
	{
		g_debugFollowTarget_F11 = !g_debugFollowTarget_F11;
	}

	// Toggle rendering between bones and mesh
	if ( g_theInput->WasKeyJustPressed( KEYCODE_BACKSPACE ) )
	{
		g_debugToggleMesh_Backspace = !g_debugToggleMesh_Backspace;
	}

	// Control camera dist from player
	if ( g_theInput->IsKeyDown( '9' ) )
	{
		m_distCamAwayFromPlayer += 10.0f;
	}
	if ( g_theInput->IsKeyDown( '0' ) )
	{
		m_distCamAwayFromPlayer -= 10.0f;
	}

	// Preset positions for target
	// X
	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		m_targetManipulatedChain->m_target.m_currentPos_WS = Vec3( 30.0f, 0.0f, 0.0f );
	}
	// Y
	if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		m_targetManipulatedChain->m_target.m_currentPos_WS = Vec3( 0.0f, 30.0f, 0.0f );
	}
	// Z
	if ( g_theInput->WasKeyJustPressed( '3' ) )
	{
		m_targetManipulatedChain->m_target.m_currentPos_WS = Vec3( 0.0f, 0.0f, 30.0f );
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
	if ( g_theInput->WasKeyJustPressed( 'C' ) )
	{
		// Look down -X-axis (YZ plane)
		m_gameMode3DWorldCamera.m_position	  = Vec3( -100.0f, 0.0f, 10.0f );
		m_gameMode3DWorldCamera.m_orientation = EulerAngles( 0.0f, 0.0f, 0.0f );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::UpdateGameMode3DCamera()
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
void GameMode_Quadruped_3D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->BeginCamera( m_gameMode3DWorldCamera );

	// Initialize common render variables
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> floorVerts;
	std::vector<Vertex_PCU> compassVerts;
	std::vector<Vertex_PCU> verts_BackFace;
	std::vector<Vertex_PCU> verts_testTexture;
	std::vector<Vertex_PCU> verts_ikChainBody;

	//----------------------------------------------------------------------------------------------------------------------
	// Render world compass
	//----------------------------------------------------------------------------------------------------------------------
//	AddVertsForCompass( compassVerts, Vec3( 0.0, 0.0f, -1.0f ), 10.0f, 0.5f );

	//----------------------------------------------------------------------------------------------------------------------
	// "ProtoMode" title
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 textOrigin = Vec3( 200.0f, 150.0f,  0.0f );
	Vec3 iBasis		= Vec3(	  0.0f,  -1.0f,  0.0f );
	Vec3 jBasis		= Vec3(	  0.0f,   0.0f,  1.0f );
//	g_theApp->m_textFont->AddVertsForText3D( textVerts, textOrigin, iBasis, jBasis, 25.0f, "Quadruped Test 3D!", Rgba8::GREEN );

	//----------------------------------------------------------------------------------------------------------------------
	// Render skybox
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> verts_skybox;
	AddVertsForSphere3D( verts_skybox, Vec3( 0.0f, 0.0f, 100.0f ), 1000.0f, 32.0f, 32.0f, Rgba8::WHITE );
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
	// Lighting
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 sunDir = m_sunDirection.GetNormalized();
	g_theRenderer->SetLightingConstants( sunDir, m_sunIntensity, m_ambientIntensity, m_gameMode3DWorldCamera.m_position, 0, 1, m_specularIntensity, m_specularPower );


	//----------------------------------------------------------------------------------------------------------------------
	// Render Dynamic Terrain
	//----------------------------------------------------------------------------------------------------------------------
	// Render plane
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), Rgba8::LIGHTBLUE );
	g_theRenderer->BindTexture( g_theApp->m_texture_MoonSurface );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexAndIndexBuffer( m_vbo, m_ibo, int( m_indexList.size() ) );
	g_theRenderer->BindTexture( nullptr );

	//----------------------------------------------------------------------------------------------------------------------
	// Render statc AABB3 Floor
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < m_blockList.size(); i++ )
	{
		AddVertsForAABB3D( floorVerts, m_blockList[i]->m_aabb3, Rgba8(80, 109, 80, 255) );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render Creature
	//----------------------------------------------------------------------------------------------------------------------
	RenderCreature( verts, verts_testTexture, textVerts, verts_BackFace, verts_ikChainBody, g_theApp->m_texture_GlowingRock );

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

	// World static AABB3 floor
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( g_theApp->m_texture_MoonSurface );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( floorVerts.size() ), floorVerts.data() );

	// Transformed ikChain model origin
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	Mat44 quadrupedRootMatrix = m_quadruped->m_modelRoot->m_eulerAnglesGoal_LS.GetAsMatrix_XFwd_YLeft_ZUp();
	quadrupedRootMatrix.SetTranslation3D( m_quadruped->m_modelRoot->m_jointPos_MS );
	g_theRenderer->SetModelConstants( quadrupedRootMatrix );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_ikChainBody.size() ), verts_ikChainBody.data() );

	// Textured objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theApp->m_texture_Basketball );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_testTexture.size() ), verts_testTexture.data() );

	// Backface objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_BackFace.size() ), verts_BackFace.data() );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );

	// Compass 
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( compassVerts.size() ), compassVerts.data() );

	// Text objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	// Reset binded texture
	g_theRenderer->BindTexture( nullptr );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_gameMode3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	Rgba8 textColor_darkRed		= Rgba8::GREEN;
	Rgba8 textColor_darkGreen	= Rgba8::DARK_GREEN;
	float cellHeight			= 2.0f;
	AABB2 textbox1				= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );	
	std::string gameInfo		= Stringf( "F1 (Toggle camera/player) Player || Mode (F6/F7 for prev/next) || Quadruped Test (3D)" );
	if ( g_debugFreeFly_F1 )
	{
		gameInfo = Stringf( "F1 (Toggle camera/player) Camera || Mode (F6/F7 for prev/next) || Quadruped Test (3D)" );
	}
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, gameInfo, textColor_darkRed, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	// Initialize and set UI variables
	Vec2 timeAlignment					= Vec2( 1.0f, 1.0f  );

	// Core Values
	float fps							= 1.0f / g_theApp->m_gameClock.GetDeltaSeconds();
	float scale							= g_theApp->m_gameClock.GetTimeScale();
	std::string cameraPosText			= Stringf( "Cam position:           %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_position.x,					m_gameMode3DWorldCamera.m_position.y,						m_gameMode3DWorldCamera.m_position.z );
	std::string cameraOrientationText	= Stringf( "Cam Orientation (YPR):  %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees,		m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees,		m_gameMode3DWorldCamera.m_orientation.m_rollDegrees );	
	std::string timeText				= Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.", g_theApp->m_gameClock.GetTotalSeconds(), fps, scale );
	std::string stepModeFABRIK 			= Stringf( "Step mode: FABRIK\n").c_str();
	if ( m_quadruped->m_leftFoot_2BIK->m_isSingleStep_Debug )
	{
		stepModeFABRIK = Stringf( "Step mode: Debug\n").c_str();
	}
	std::string targetManipulatedChain = "INVALID STRING";
	if ( m_targetManipulatedChain )
	{
		targetManipulatedChain = m_targetManipulatedChain->m_name.c_str();
	}
	std::string anchorState_LA = Stringf( "Left  Arm:  %s", m_quadruped->m_leftArm_2BIK  ->GetAnchorAsString().c_str() );
	std::string anchorState_RA = Stringf( "Right Arm:  %s", m_quadruped->m_rightArm_2BIK ->GetAnchorAsString().c_str() );
	std::string anchorState_LF = Stringf( "Left  Foot: %s", m_quadruped->m_leftFoot_2BIK ->GetAnchorAsString().c_str() );
	std::string anchorState_RF = Stringf( "Right Foot: %s", m_quadruped->m_rightFoot_2BIK->GetAnchorAsString().c_str() );

	float textAlignmentY	= 1.0f;
	float textHeight		= 0.03f;
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,				   timeText, textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY						), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			  cameraPosText, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight			), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,    cameraOrientationText, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight			), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			 stepModeFABRIK, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight			), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,	 targetManipulatedChain, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight			), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			 anchorState_LA, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight * 2.0f	), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			 anchorState_RA, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight			), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			 anchorState_LF, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight			), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,			 anchorState_RF, textColor_darkRed, 0.75f, Vec2( 0.0f, textAlignmentY -= textHeight			), TextDrawMode::SHRINK_TO_FIT );

	textAlignmentY = 0.97f;
	if ( g_debugText_F4 )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Creature root euler
		EulerAngles creatureRootEuler_WS = m_quadruped->m_modelRoot->m_eulerAnglesGoal_LS;
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
			Stringf( "creatureRoot YPR_WS: %0.2f, %0.2f, %0.2f", creatureRootEuler_WS.m_yawDegrees, creatureRootEuler_WS.m_pitchDegrees, creatureRootEuler_WS.m_rollDegrees ),
			textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ) );
		//----------------------------------------------------------------------------------------------------------------------
		// IK chain WS pos
		Vec3 const& modelRootPos_WS = m_quadruped->m_modelRoot->m_jointPos_MS;
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
			Stringf( "modelRootPos_WS: %0.2f, %0.2f, %0.2f", modelRootPos_WS.x, modelRootPos_WS.y, modelRootPos_WS.z ),
			textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ), TextDrawMode::SHRINK_TO_FIT );
		//----------------------------------------------------------------------------------------------------------------------
		// IK chain WS euler
		EulerAngles curEuler_WS = m_quadruped->m_leftFoot_2BIK->m_eulerAngles_WS;
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
			Stringf( "IK_Chain YPR_WS: %0.2f, %0.2f, %0.2f", curEuler_WS.m_yawDegrees, curEuler_WS.m_pitchDegrees, curEuler_WS.m_rollDegrees ),
			textColor_darkRed, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ) );
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 0 (root)
		EulerAngles curEuler_LS = m_quadruped->m_leftFoot_2BIK->m_firstJoint->m_eulerAnglesGoal_LS;
		g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
			Stringf( "Joint0_YPR: %0.2f, %0.2f, %0.2f", curEuler_LS.m_yawDegrees, curEuler_LS.m_pitchDegrees, curEuler_LS.m_rollDegrees ),	
			textColor_darkGreen, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ) );
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 1
		if ( m_quadruped->m_leftFoot_2BIK->m_jointList.size() >= 2 )
		{
			curEuler_LS = m_quadruped->m_leftFoot_2BIK->m_jointList[1]->m_eulerAnglesGoal_LS;
			g_theApp->m_textFont->AddVertsForTextInBox2D(	textVerts, textbox1, cellHeight, 
				Stringf( "Joint1_YPR: %0.2f, %0.2f, %0.2f", curEuler_LS.m_yawDegrees, curEuler_LS.m_pitchDegrees, curEuler_LS.m_rollDegrees ),
				textColor_darkGreen, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ) );
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 2
		if ( m_quadruped->m_leftFoot_2BIK->m_jointList.size() >= 3 )
		{
			curEuler_LS = m_quadruped->m_leftFoot_2BIK->m_jointList[ 2 ]->m_eulerAnglesGoal_LS;
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,
				Stringf( "Joint2_YPR: %0.2f, %0.2f, %0.2f", curEuler_LS.m_yawDegrees, curEuler_LS.m_pitchDegrees, curEuler_LS.m_rollDegrees ),
				textColor_darkGreen, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ) );
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 3
		if ( m_quadruped->m_leftFoot_2BIK->m_jointList.size() >= 4 )
		{
			curEuler_LS = m_quadruped->m_leftFoot_2BIK->m_jointList[ 3 ]->m_eulerAnglesGoal_LS;
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,
				Stringf( "Joint3_YPR: %0.2f, %0.2f, %0.2f", curEuler_LS.m_yawDegrees, curEuler_LS.m_pitchDegrees, curEuler_LS.m_rollDegrees ),
				textColor_darkGreen, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ) );
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 4
		if ( m_quadruped->m_leftFoot_2BIK->m_jointList.size() >= 5 )
		{
			curEuler_LS = m_quadruped->m_leftFoot_2BIK->m_jointList[ 4 ]->m_eulerAnglesGoal_LS;
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,
				Stringf( "Joint4_YPR: %0.2f, %0.2f, %0.2f", curEuler_LS.m_yawDegrees, curEuler_LS.m_pitchDegrees, curEuler_LS.m_rollDegrees ),
				textColor_darkGreen, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ) );
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 5
		if ( m_quadruped->m_leftFoot_2BIK->m_jointList.size() >= 6 )
		{
			curEuler_LS = m_quadruped->m_leftFoot_2BIK->m_jointList[ 5 ]->m_eulerAnglesGoal_LS;
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,
				Stringf( "Joint5_YPR: %0.2f, %0.2f, %0.2f", curEuler_LS.m_yawDegrees, curEuler_LS.m_pitchDegrees, curEuler_LS.m_rollDegrees ),
				textColor_darkGreen, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ) );
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Joint 6
		if ( m_quadruped->m_leftFoot_2BIK->m_jointList.size() >= 7 )
		{
			curEuler_LS = m_quadruped->m_leftFoot_2BIK->m_jointList[ 6 ]->m_eulerAnglesGoal_LS;
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,
				Stringf( "Joint6_YPR: %0.2f, %0.2f, %0.2f", curEuler_LS.m_yawDegrees, curEuler_LS.m_pitchDegrees, curEuler_LS.m_rollDegrees ),
				textColor_darkGreen, 0.75f, Vec2( 1.0f, textAlignmentY -= textHeight ) );
		}
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
void GameMode_Quadruped_3D::AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
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
void GameMode_Quadruped_3D::UpdateTargetInput( float deltaSeconds )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Toggle controlled targetManipulatedChain
	//----------------------------------------------------------------------------------------------------------------------
	// Debug single step
	if ( g_theInput->WasKeyJustPressed( KEYCODE_RIGHTBRACKET ) )
	{
//		m_specularIntensity++;
//		m_specularPower++;

		m_indexTargetChain++;
		if ( m_indexTargetChain >= m_quadruped->m_skeletalList.size() )
		{
			m_indexTargetChain = 0;
		}
		m_targetManipulatedChain = m_quadruped->m_skeletalList[ m_indexTargetChain ];
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFTBRACKET ) )
	{
//		m_specularIntensity--;
//		m_specularPower--;

		m_indexTargetChain--;
		if ( m_indexTargetChain < 0)
		{
			m_indexTargetChain = int( m_quadruped->m_skeletalList.size() - 1 );
		}
		m_targetManipulatedChain = m_quadruped->m_skeletalList[ m_indexTargetChain ];
	}

	if ( g_theInput->WasKeyJustPressed( '-' ) )
	{
	}
	if ( g_theInput->WasKeyJustPressed( '+' ) )
	{
	}
	

	// Note: if chain == head, change the targetPos from "current" to "goal"
	float stepAmount	= 50.0f;
	IK_Chain3D* chain	= m_targetManipulatedChain;
	Vec3& targetPos		= chain->m_target.m_currentPos_WS;
	if ( m_targetManipulatedChain == m_quadruped->m_head_FABRIK )
	{
		targetPos = chain->m_target.m_goalPos_WS;
 	}

	if ( g_debugFollowTarget_F11 )
	{
		float time			= g_theApp->m_gameClock.GetTotalSeconds();
		float sine			= SinDegrees( time * 10.0f ) * 75.0f;
		float cos			= CosDegrees( time * 10.0f ) * 75.0f;
		float perlinNoise	= Compute2dPerlinNoise( time * 0.5f, 0.0f ) * 65.0f;
		targetPos			= Vec3( 0.0f + cos, 0.0f + sine, 0.0f + perlinNoise );
		m_debugNoisePos		= chain->m_target.m_currentPos_WS;
	}
	else
	{
		if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
		{
			stepAmount *= 4.0f;
		}

		// East (+X)
		if ( g_theInput->IsKeyDown( 'I' ) )
		{
			targetPos += Vec3( stepAmount, 0.0f, 0.0f ) * deltaSeconds;
		}
		// West (-X)
		if ( g_theInput->IsKeyDown( 'K' ) )
		{
			targetPos += Vec3( -stepAmount, 0.0f, 0.0f ) * deltaSeconds;
		}
		// North (+Y)
		if ( g_theInput->IsKeyDown( 'J' ) )
		{
			targetPos += Vec3( 0.0f, stepAmount, 0.0f ) * deltaSeconds;
		}
		// South (-Y)
		if ( g_theInput->IsKeyDown( 'L' ) )
		{
			targetPos += Vec3( 0.0f, -stepAmount, 0.0f ) * deltaSeconds;
		}
		// Sky (+Z)
		if ( g_theInput->IsKeyDown( 'O' ) )
		{
			targetPos += Vec3( 0.0f, 0.0f, stepAmount ) * deltaSeconds;
		}
		// Ground (-Z)
		if ( g_theInput->IsKeyDown( 'U' ) )
		{
			targetPos += Vec3( 0.0f, 0.0f, -stepAmount ) * deltaSeconds;
		}

	}

	if ( chain == m_quadruped->m_head_FABRIK )
	{
		// Also update the neck to follow the same target if we are currently manipulating the head
		m_quadruped->m_neck_FABRIK->m_target.m_goalPos_WS = targetPos;
	}
	if ( m_targetManipulatedChain == m_quadruped->m_head_FABRIK )
	{
		chain->m_target.m_goalPos_WS = targetPos;
	}
	else
	{
		chain->m_target.m_currentPos_WS = targetPos;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::RenderCreature(	std::vector<Vertex_PCU>& verts, 
											std::vector<Vertex_PCU>& verts_textured, 
											std::vector<Vertex_PCU>& textVerts, 
											std::vector<Vertex_PCU>& verts_BackFace,
											std::vector<Vertex_PCU>& verts_ikChainBody, 
											Texture* creatureTexture 
										 ) const
{
	m_quadruped->RenderQuadruped( verts, verts_textured, textVerts, verts_BackFace, verts_ikChainBody, m_gameMode3DWorldCamera, creatureTexture );

	// Render target
	Vec3 targetPos	= m_targetManipulatedChain->m_target.m_currentPos_WS;
	float width		= 0.4f;
	AABB3 bounds	= AABB3( targetPos - Vec3(width, width, width), targetPos + Vec3(width, width, width) );
	AddVertsForAABB3D( verts, bounds, Rgba8::MAGENTA );
	m_targetManipulatedChain->DebugDrawTarget( verts_textured, 1.0f, Rgba8::BROWN, false, 8.0f );

	// Debug draw noise position
//	AddVertsForSphere3D( verts, m_debugNoisePos, 1.0f, 4.0f, 4.0f, Rgba8::MAGENTA );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug rendering rotation axis
	//----------------------------------------------------------------------------------------------------------------------
//	Vec3 end = m_quadruped->m_leftFoot_FABRIK->m_firstJoint->m_jointPos_LS + m_quadruped->m_leftFoot_FABRIK->m_debugVector_RotationAxis_MS * 2.0f;
//	AddVertsForArrow3D( verts, m_quadruped->m_leftFoot_FABRIK->m_firstJoint->m_jointPos_LS, end, 0.2f, Rgba8::MAGENTA );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::SetChainConstraints( IK_Chain3D* const currentChain )
{
	for ( int i = 0; i < currentChain->m_jointList.size(); i++ )
	{
		IK_Joint3D* currentSegment = currentChain->m_jointList[ i ];
		int remainder = i % 2;
		if ( remainder == 0 )
		{
			// Yaw only if even
			currentSegment->SetConstraints_YPR( FloatRange( -75.0f, 75.0f ), FloatRange( -0.0f, 0.0f ), FloatRange( -0.0f, 0.0f ) );
		}
		else
		{
			// Pitch only if odd
			currentSegment->SetConstraints_YPR( FloatRange( -0.0f, 0.0f ), FloatRange( -75.0f, 75.0f ), FloatRange( -0.0f, 0.0f ) );
		}
	}


// 	for ( int i = 0; i < currentChain->m_jointList.size(); i++ )
// 	{
// 		IK_Joint3D* currentSegment = currentChain->m_jointList[ i ];
// 		currentSegment->SetConstraints_YPR( FloatRange( -75.0f, 75.0f ), FloatRange( -75.0f, 75.0f ), FloatRange( -0.0f, 0.0f ) );
// 	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::UpdateCameraAndCreatureInput( float deltaSeconds )
{
	Vec3 iBasis, jBasis, kBasis;
	m_gameMode3DWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
	iBasis.z = 0.0f;
	jBasis.z = 0.0f;
	iBasis	 = iBasis.GetNormalized(); 
	jBasis	 = jBasis.GetNormalized(); 
	kBasis   = kBasis.GetNormalized();

	//----------------------------------------------------------------------------------------------------------------------
	// All directions are local
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_currentSpeed		= m_fasterSpeed;
		m_goalWalkLerpSpeed	= m_sprintLerpSpeed;
	}
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_currentSpeed		= m_defaultSpeed;
		m_goalWalkLerpSpeed	= m_walkLerpSpeed;
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
		// Forward
		if ( g_theInput->IsKeyDown( 'W' ) )
		{
			m_quadruped->m_modelRoot->m_jointPos_MS += ( Vec3::X_FWD * m_currentSpeed ) * deltaSeconds;
		}
		// Left
		if ( g_theInput->IsKeyDown( 'A' ) )
		{
			m_quadruped->m_modelRoot->m_jointPos_MS += ( Vec3::Y_LEFT * m_currentSpeed ) * deltaSeconds;
		}
		// Backwards
		if ( g_theInput->IsKeyDown( 'S' ) )
		{
			m_quadruped->m_modelRoot->m_jointPos_MS -= ( Vec3::X_FWD * m_currentSpeed ) * deltaSeconds;
		}
		// Right
		if ( g_theInput->IsKeyDown( 'D' ) )
		{
			m_quadruped->m_modelRoot->m_jointPos_MS -= ( Vec3::Y_LEFT * m_currentSpeed ) * deltaSeconds;
		}
		// Sky (+Z)
		if ( g_theInput->IsKeyDown( 'E' ) )
		{
			m_quadruped->m_modelRoot->m_jointPos_MS += ( Vec3::Z_UP * m_currentSpeed ) * deltaSeconds;
		}
		// Ground (-Z)
		if ( g_theInput->IsKeyDown( 'Q' ) )
		{
			m_quadruped->m_modelRoot->m_jointPos_MS -= ( Vec3::Z_UP * m_currentSpeed ) * deltaSeconds;
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Model controls (euler)
		//----------------------------------------------------------------------------------------------------------------------
		float rotateSpeed = m_currentSpeed * 2.5f;
		// +Yaw
		if ( g_theInput->IsKeyDown( 'V' ) )
		{
			m_quadruped->m_modelRoot->m_eulerAnglesGoal_LS.m_yawDegrees += rotateSpeed * deltaSeconds;
		}
		// -Yaw
		if ( g_theInput->IsKeyDown( 'B' ) )
		{
			m_quadruped->m_modelRoot->m_eulerAnglesGoal_LS.m_yawDegrees -= rotateSpeed * deltaSeconds;
		}
		// +Pitch
		if ( g_theInput->IsKeyDown( 'N' ) )
		{
			m_quadruped->m_modelRoot->m_eulerAnglesGoal_LS.m_pitchDegrees += rotateSpeed * deltaSeconds;
		}
		// -Pitch
		if ( g_theInput->IsKeyDown( 'M' ) )
		{
			m_quadruped->m_modelRoot->m_eulerAnglesGoal_LS.m_pitchDegrees -= rotateSpeed * deltaSeconds;
		}
		// +Roll
		if ( g_theInput->IsKeyDown( KEYCODE_COMMA ) )
		{
			m_quadruped->m_modelRoot->m_eulerAnglesGoal_LS.m_rollDegrees += rotateSpeed * deltaSeconds;
		}
		// -Roll
		if ( g_theInput->IsKeyDown( KEYCODE_PERIOD ) )
		{
			m_quadruped->m_modelRoot->m_eulerAnglesGoal_LS.m_rollDegrees -= rotateSpeed * deltaSeconds;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
std::vector<Block*> GameMode_Quadruped_3D::GetBlockListFromGameMode()
{
	return m_blockList;
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::InitFloorAABB3()
{
	m_blockList.clear();
	AABB3  bounds	= AABB3();
	Block* block	= nullptr;
// 	if ( g_debugToggleFloor_F9 )
// 	{
// 		m_blockList.reserve( 6 );
// 		float minDepth = -20.0f;
// 		float maxDepth =  -8.0f;
// 		// Create 4 moving floors
// 		// NE (+X,+Y)
// 		bounds	= AABB3( Vec3( 2.0f, 2.0f, minDepth ), Vec3( 100.0f, 100.0f, maxDepth ) );
// 		block	= new Block( bounds, true );
// 		m_blockList.push_back( block );
// 		// SE (+X,-Y)
// 		bounds	= AABB3( Vec3( 2.0f, -100.0f, minDepth ), Vec3( 100.0f, -2.0f, maxDepth ) );
// 		block	= new Block( bounds, true );
// 		m_blockList.push_back( block );
// 		// SW (-X,-Y)
// 		bounds	= AABB3( Vec3( -100.0f, -100.0f, minDepth ), Vec3( -2.0f, -2.0f, maxDepth ) );
// 		block	= new Block( bounds, true );
// 		m_blockList.push_back( block );
// 		// NW (-X,+Y)
// 		bounds	= AABB3( Vec3( -100.0f, 2.0f, minDepth ), Vec3( -2.0f, 100.0f, maxDepth ) );
// 		block	= new Block( bounds, true );
// 		m_blockList.push_back( block );
// 	}
// 	else
// 	{
// 		// Create static floor
// 		bounds	= AABB3( Vec3(-100.0f, -100.0f, -20.0f), Vec3(100.0f, 100.0f, -9.0f) );
// 		block   = new Block( bounds, true );
// 		m_blockList.push_back( block );
// 	}

	if ( !g_debugToggleFloor_F9 )
	{
		// Create static floor
		bounds	= AABB3( Vec3(-100.0f, -100.0f, -10.0f), Vec3(100.0f, 100.0f, -9.0f) );
		block   = new Block( bounds, true );
		m_blockList.push_back( block );
	}
	else
	{
		// Create 4 moving floors
		// NE (+X,+Y)
		bounds	= AABB3( Vec3( 2.0f, 2.0f, -10.0f ), Vec3( 100.0f, 100.0f, -9.0f ) );
		block	= new Block( bounds, true );
		m_blockList.push_back( block );
		// SE (+X,-Y)
		bounds	= AABB3( Vec3( 2.0f, -100.0f, -10.0f ), Vec3( 100.0f, -2.0f, -9.0f ) );
		block	= new Block( bounds, true );
		m_blockList.push_back( block );
		// SW (-X,-Y)
		bounds	= AABB3( Vec3( -100.0f, -100.0f, -10.0f ), Vec3( -2.0f, -2.0f, -9.0f ) );
		block	= new Block( bounds, true );
		m_blockList.push_back( block );
		// NW (-X,+Y)
		bounds	= AABB3( Vec3( -100.0f, 2.0f, -10.0f ), Vec3( -2.0f, 100.0f, -9.0f ) );
		block	= new Block( bounds, true );
		m_blockList.push_back( block );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::InitDynamicTerrain()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Initialize randomized Z plane
	//----------------------------------------------------------------------------------------------------------------------
	// Randomize floor height
	AddVertsForPlane( m_planeVerts, m_indexList, Vec3(-300.0f, -100.0f, -10.0f), 15, 13, 13 );
	m_vbo = g_theRenderer->CreateVertexBuffer( m_planeVerts.size(), sizeof(Vertex_PCU) );
	m_ibo = g_theRenderer->CreateIndexBuffer (  m_indexList.size() );
	g_theRenderer->Copy_CPU_To_GPU( m_planeVerts.data(), sizeof( Vertex_PCU )   * m_planeVerts.size(), m_vbo, sizeof( Vertex_PCU ) );
	g_theRenderer->Copy_CPU_To_GPU(  m_indexList.data(), sizeof( unsigned int ) *  m_indexList.size(), m_ibo );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode_Quadruped_3D::UpdateDynamicTerrain( float deltaSeconds )
{
	float noiseTimeScale = 20.0f;
	float waveScalar	 = 10.0f;
	m_noiseTime			+= deltaSeconds * noiseTimeScale;
	for ( int i = 0; i < m_planeVerts.size(); i++ )
	{
		Vertex_PCU& currentVert		= m_planeVerts[ i ];
		float rand					= Compute3dPerlinNoise( currentVert.m_position.x, currentVert.m_position.y, m_noiseTime, 50.0f, 2 );
		float offset				= rand * waveScalar;
		float baseHeight			= -10.f;
		float height				= baseHeight + offset;
		currentVert.m_position.z	= height;
	}
	g_theRenderer->Copy_CPU_To_GPU( m_planeVerts.data(), sizeof( Vertex_PCU )   * m_planeVerts.size(), m_vbo, sizeof( Vertex_PCU ) );
	g_theRenderer->Copy_CPU_To_GPU(  m_indexList.data(), sizeof( unsigned int ) *  m_indexList.size(), m_ibo );
}
