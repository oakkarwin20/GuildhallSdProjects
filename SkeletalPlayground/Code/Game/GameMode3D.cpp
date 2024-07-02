#include "Game/App.hpp"
#include "Game/GameMode3D.hpp"
#include "Game/Map_GameMode3D.hpp"
#include "Game/Quadruped.hpp"
#include "Game/SpiderBase.hpp"
#include "Game/FoodManager.hpp"

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
GameMode3D::GameMode3D()
{
}


//----------------------------------------------------------------------------------------------------------------------
GameMode3D::~GameMode3D()
{
	delete m_quadruped;
	delete m_map;
	m_quadruped = nullptr;
	m_map		= nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::Startup()
{
	g_debugFreeFly_F1		= true;
	g_debugBasis_F3			= false;
	g_debugRenderRaycast_F2	= false;
	g_debugText_F4			= false;
	g_debugAngles_F5		= false;

	// WalkableObjects (Floors, environmental proxies)
	InitializeEnvironment();
	// Init animals
	InitializeCreatures();

	// Init lighting
	EulerAngles sunOrientation = EulerAngles( 0.0f, 65.0f, 0.0f );
	m_sunDirection = sunOrientation.GetForwardDir_XFwd_YLeft_ZUp();

	// Init camera
	m_gameMode3DWorldCamera.m_position		= Vec3( -130.0f, 0.0f, 71.0f );
	m_gameMode3DWorldCamera.m_orientation	= EulerAngles( 0.0f, 30.0f, 0.0f );

	m_gameMode3DWorldCamera.m_position		= Vec3( -45.0f, -12.0f, 71.0f );
	m_gameMode3DWorldCamera.m_orientation	= EulerAngles( 54.0f, 35.0f, 0.0f );

}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::Update( float deltaSeconds )
{	
	// Update core systems
	UpdatePauseQuitAndSlowMo();
	UpdateDebugKeys();
	UpdateCameraInput( deltaSeconds );
	UpdateCameraAndCreatureInput(  deltaSeconds );
	
	// Update sunDir
	EulerAngles sunOrientation = EulerAngles( 0.0f, 90.0f, 0.0f );
	m_sunDirection = sunOrientation.GetForwardDir_XFwd_YLeft_ZUp();

	// Map
	m_map->Update( deltaSeconds );

	//----------------------------------------------------------------------------------------------------------------------
	// Sprinting lerp toggle
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_isSprinting )
	{
		m_goalWalkLerpSpeed = Interpolate( m_goalWalkLerpSpeed, m_sprintLerpSpeed, deltaSeconds );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped
	//----------------------------------------------------------------------------------------------------------------------
//	m_quadruped->m_neck_FABRIK->m_target.m_goalPos_WS = m_jellyfish->m_prevPosLastFrame_WS;
//	m_quadruped->m_head_FABRIK->m_target.m_goalPos_WS = m_jellyfish->m_prevPosLastFrame_WS;
	m_quadruped->m_neck_FABRIK->m_target.m_goalPos_WS = m_spiderBase->m_modelRoot->m_jointPos_MS;
	m_quadruped->m_head_FABRIK->m_target.m_goalPos_WS = m_spiderBase->m_modelRoot->m_jointPos_MS;
//	m_quadruped->m_neck_FABRIK->m_target.m_goalPos_WS = m_gameMode3DWorldCamera.m_position;				// Hack to make lizard follow the camera in freefly
//	m_quadruped->m_head_FABRIK->m_target.m_goalPos_WS = m_gameMode3DWorldCamera.m_position;				// Hack to make lizard follow the camera in freefly
	m_quadruped->m_maxDist_comeCloser = 75.0f;
	m_quadruped->m_minDist_moveAway	  = 65.0f;
	m_quadruped->PreUpdateChains( deltaSeconds );
	m_quadruped->Update( deltaSeconds );
	m_quadruped->PostUpdateChains( deltaSeconds );

	//----------------------------------------------------------------------------------------------------------------------
	// Update Jellyfish
	//----------------------------------------------------------------------------------------------------------------------
	m_jellyfish->UpdateJoints( deltaSeconds );

	//----------------------------------------------------------------------------------------------------------------------
	// Update Spider
	//----------------------------------------------------------------------------------------------------------------------
	m_spiderBase->m_neck_FABRIK->m_target.m_goalPos_WS = m_jellyfish->m_prevPosLastFrame_WS;
//	m_spiderBase->m_neck_FABRIK->m_target.m_goalPos_WS = m_gameMode3DWorldCamera.m_position;			// Hack to make lizard follow the camera in freefly
//	m_spiderBase->m_neck_FABRIK->m_target.m_goalPos_WS = m_quadruped->m_modelRoot->m_jointPos_MS;		
	m_spiderBase->PreUpdateChains( deltaSeconds );
	m_spiderBase->Update( deltaSeconds );
	m_spiderBase->PostUpdateChains( deltaSeconds );

	// Update Camera
	UpdateGameMode3DCamera();
	TurnCreatureTowardsCameraDir();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::Shutdown()
{
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateDebugKeys()
{
	// Toggle common debug bools
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		g_debugFreeFly_F1 = !g_debugFreeFly_F1;
		if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_QUADRUPED )
		{
			m_quadruped->ToggleIsControlled();
		}
		else if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_SPIDER )
		{
			m_spiderBase->ToggleIsControlled();
		}
		else if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_CAMERA_AI )
		{
			m_spiderBase->ToggleIsControlled();
			m_controlledCreatureType = ControlledCreatureType::CONTROLLED_SPIDER;
		}
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
// 		if ( !g_debugToggleConstraints_F8 )
// 		{
// 			m_quadruped->SetChainConstraints( m_quadruped->m_leftFoot_2BIK	);
// 			m_quadruped->SetChainConstraints( m_quadruped->m_rightFoot_2BIK	);
// 			m_quadruped->SetChainConstraints( m_quadruped->m_spine_CCD			);
// 			m_quadruped->SetChainConstraints( m_quadruped->m_leftArm_2BIK		);
// 			m_quadruped->SetChainConstraints( m_quadruped->m_rightArm_2BIK	);
// 			m_quadruped->SetChainConstraints( m_quadruped->m_tail_CCD			);
// 			m_quadruped->SetChainConstraints( m_quadruped->m_neck_FABRIK		);
// 			m_quadruped->SetChainConstraints( m_quadruped->m_head_FABRIK		);
// 			m_quadruped->SetChainConstraints( m_quadruped->m_tentacleLeft_CCD	);
// 			m_quadruped->SetChainConstraints( m_quadruped->m_tentacleRight_CCD	);
// 		}
// 		else
// 		{
// 			m_quadruped->m_leftFoot_2BIK	->ResetAllJointConstraints();
// 			m_quadruped->m_rightFoot_2BIK	->ResetAllJointConstraints();
// 			m_quadruped->m_spine_CCD		->ResetAllJointConstraints();
// 			m_quadruped->m_leftArm_2BIK		->ResetAllJointConstraints();
// 			m_quadruped->m_rightArm_2BIK	->ResetAllJointConstraints();
// 			m_quadruped->m_tail_CCD			->ResetAllJointConstraints();
// 			m_quadruped->m_neck_FABRIK		->ResetAllJointConstraints();
// 			m_quadruped->m_head_FABRIK		->ResetAllJointConstraints();
// 			m_quadruped->m_tentacleLeft_CCD	->ResetAllJointConstraints();
// 			m_quadruped->m_tentacleRight_CCD->ResetAllJointConstraints();
// 		}
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


	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		g_debugRenderBezier_1 = !g_debugRenderBezier_1;
	}
	if ( g_theInput->WasKeyJustPressed( '2' ) && !g_debugFreeFly_F1 )
	{
		if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_QUADRUPED )
		{
			m_controlledCreatureType = ControlledCreatureType::CONTROLLED_SPIDER;
		}
		else if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_SPIDER )
		{
			m_controlledCreatureType = ControlledCreatureType::CONTROLLED_QUADRUPED;
		}
		m_spiderBase->ToggleIsControlled();
		m_quadruped->ToggleIsControlled();
	}

	// Control Elevator
	// Sky (+Z)
// 	if ( g_theInput->IsKeyDown( 'O' ) )
// 	{
// 		Vec3 newPos = m_map->m_elevator_2->m_aabb3.GetCenter() + Vec3( 0.0f, 0.0f, 1.0f );
// 		m_map->m_elevator_2->m_aabb3.SetCenterXYZ( newPos );
// 	}
// 	// Ground (-Z)
// 	if ( g_theInput->IsKeyDown( 'U' ) )
// 	{
// 		Vec3 newPos = m_map->m_elevator_2->m_aabb3.GetCenter() + Vec3( 0.0f, 0.0f, -1.0f );
// 		m_map->m_elevator_2->m_aabb3.SetCenterXYZ( newPos );
// 	}

	// Control camera dist from player
	if ( g_theInput->IsKeyDown( '9' ) )
	{
		m_distCamAwayFromPlayer += 10.0f;
	}
	if ( g_theInput->IsKeyDown( '0' ) )
	{
		m_distCamAwayFromPlayer -= 10.0f;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateCameraInput( float deltaSeconds )
{
	if ( g_debugFreeFly_F1 == false )
	{
		return;
	}

	Vec3 iBasis, jBasis, kBasis;
	m_gameMode3DWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
	iBasis.z = 0.0f;
	jBasis.z = 0.0f;
	iBasis	 = iBasis.GetNormalized(); 
	jBasis	 = jBasis.GetNormalized(); 
	kBasis   = kBasis.GetNormalized();

	float cameraSpeedScalar = 3.0f;
	//----------------------------------------------------------------------------------------------------------------------
	// All directions are local
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_currentSpeed = m_fasterSpeed * cameraSpeedScalar;
	}
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_currentSpeed = m_defaultSpeed * cameraSpeedScalar;
	}
	// Forward
	if ( g_theInput->IsKeyDown( 'W' ) )
	{
		m_gameMode3DWorldCamera.m_position += ( iBasis * m_currentSpeed * cameraSpeedScalar ) * deltaSeconds;
	}
	// Left
	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		m_gameMode3DWorldCamera.m_position += ( jBasis * m_currentSpeed * cameraSpeedScalar ) * deltaSeconds;
	}
	// Backwards
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_gameMode3DWorldCamera.m_position -= ( iBasis * m_currentSpeed * cameraSpeedScalar ) * deltaSeconds;
	}
	// Right
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_gameMode3DWorldCamera.m_position -= ( jBasis * m_currentSpeed * cameraSpeedScalar ) * deltaSeconds;
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
		m_gameMode3DWorldCamera.m_position += ( Vec3( 0.0f, 0.0f, 1.0f ) * m_currentSpeed * cameraSpeedScalar ) * deltaSeconds;
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'Q' ) )
	{
		m_gameMode3DWorldCamera.m_position -= ( Vec3( 0.0f, 0.0f, 1.0f ) * m_currentSpeed * cameraSpeedScalar ) * deltaSeconds;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateGameMode3DCamera()
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
//	m_gameMode3DWorldCamera.SetTransform( m_gameMode3DWorldCamera.m_position, m_gameMode3DWorldCamera.m_orientation );
	
	m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees = GetClamped( m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees, -85.0f, 85.0f );
	m_gameMode3DWorldCamera.m_orientation.m_rollDegrees  = GetClamped(  m_gameMode3DWorldCamera.m_orientation.m_rollDegrees, -45.0f, 45.0f );

	if ( g_debugFreeFly_F1 )
	{
		// Set cameraPos roam "freely"
		m_gameMode3DWorldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 10000.0f );
		m_gameMode3DWorldCamera.SetRenderBasis( Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) );
	}
	else
	{
		// Set cameraPos to stay attached to playerPos
		Vec3 playerPos	= Vec3::ZERO;
		if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_QUADRUPED )
		{
			playerPos = m_quadruped->m_modelRoot->m_jointPos_MS - ( m_gameMode3DWorldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp() * m_distCamAwayFromPlayer );		// #QuadupedHack
		}
		else if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_SPIDER )
		{
			playerPos = m_spiderBase->m_modelRoot->m_jointPos_MS - ( m_gameMode3DWorldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp() * m_distCamAwayFromPlayer );		// #QuadupedHack
		}
		m_gameMode3DWorldCamera.SetTransform( playerPos, m_gameMode3DWorldCamera.m_orientation );
		m_gameMode3DWorldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 10000.0f );
		m_gameMode3DWorldCamera.SetRenderBasis( Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) );
	}

	// Update UI camera
	m_gameMode3DUICamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );		
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->BeginCamera( m_gameMode3DWorldCamera );
	DebugRenderWorld( m_gameMode3DWorldCamera );

	// Initialize common render variables
	std::vector<Vertex_PCU> verts_Text;
	std::vector<Vertex_PCU> verts_Creature;
	std::vector<Vertex_PCU> verts_NoTexture;
	std::vector<Vertex_PCU> verts_BackfaceCull;
	std::vector<Vertex_PCU> verts_ikChainBody;
	std::vector<Vertex_PCU> verts_textureGlowingRock;
	std::vector<Vertex_PCU> verts_textureGalaxy;

//	AddVertsForCompass( verts_NoTexture, Vec3::ZERO, 10.0f, 0.5f );
	//----------------------------------------------------------------------------------------------------------------------
	// "ProtoMode" title
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 textOrigin = Vec3( 200.0f, 130.0f, 100.0f );
	Vec3 iBasis		= Vec3(	  0.0f,  -1.0f,   0.0f );
	Vec3 jBasis		= Vec3(	  0.0f,   0.0f,   1.0f );
//	g_theApp->m_textFont->AddVertsForText3D( verts_Text, textOrigin, iBasis, jBasis, 25.0f, "ProtoMode3D!", Rgba8::GREEN );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Lighting (lizard)
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 sunDir = m_sunDirection.GetNormalized();
	g_theRenderer->SetLightingConstants( sunDir, m_sunIntensity, m_ambientIntensity, m_gameMode3DWorldCamera.m_position, 0, 1, m_specularIntensity, m_specularPower );

	//----------------------------------------------------------------------------------------------------------------------
	// Creature Render Functions
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped->RenderQuadruped( verts_NoTexture, verts_Creature, verts_Text, verts_BackfaceCull, verts_ikChainBody, m_gameMode3DWorldCamera, g_theApp->m_texture_GlowingRock );
	RenderCreature( verts_NoTexture, verts_Creature, verts_Text, verts_BackfaceCull, verts_ikChainBody );
	RenderRaycasts( verts_NoTexture );
	m_map->Render();

	//----------------------------------------------------------------------------------------------------------------------
	// Jellyfish rendering
	//----------------------------------------------------------------------------------------------------------------------
	m_jellyfish->RenderJellyfish( verts_textureGalaxy );


	//----------------------------------------------------------------------------------------------------------------------
	// Lighting (spider)
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->SetLightingConstants( sunDir, m_sunIntensity, m_ambientIntensity, m_gameMode3DWorldCamera.m_position, 0, 1, m_specularIntensity_spider, m_specularPower_spider );

	//----------------------------------------------------------------------------------------------------------------------
	// Spider rendering
	//----------------------------------------------------------------------------------------------------------------------
	m_spiderBase->RenderSpiderBase( verts_NoTexture, verts_Creature, verts_Text, verts_BackfaceCull, verts_ikChainBody, m_gameMode3DWorldCamera, g_theApp->m_texture_GlowingRock );
	// Render target
//	Vec3 targetPos	= m_spiderBase->m_neck_FABRIK->m_target.m_currentPos_WS;
//	float width		= 0.4f;
//	AABB3 bounds	= AABB3( targetPos - Vec3(width, width, width), targetPos + Vec3(width, width, width) );
//	AddVertsForAABB3D( verts_NoTexture, bounds, Rgba8::MAGENTA );
//	m_spiderBase->m_neck_FABRIK->DebugDrawTarget( verts_NoTexture, 1.0f, Rgba8::BROWN, true, 8.0f );

/*
	//----------------------------------------------------------------------------------------------------------------------
	// Render debug rayVsTri hit info
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 rayEnd				= m_rayVsTri.m_rayStartPosition + m_rayVsTri.m_rayFwdNormal * m_rayVsTri.m_rayMaxLength;
	Vec3 impactNormalEnd	= m_rayVsTri.m_impactPos + m_rayVsTri.m_impactNormal * 1.0f;
	AddVertsForArrow3D ( verts,	m_rayVsTri.m_rayStartPosition,				    rayEnd, 0.5f, Rgba8::DARK_YELLOW	);
	AddVertsForArrow3D ( verts, m_rayVsTri.m_rayStartPosition,	m_rayVsTri.m_impactPos, 0.1f, Rgba8::CYAN			);
	AddVertsForArrow3D ( verts,		   m_rayVsTri.m_impactPos,		   impactNormalEnd, 0.1f, Rgba8::BLUE			);
	AddVertsForSphere3D( verts,		   m_rayVsTri.m_impactPos,				0.5f, 6.0f, 4.0f, Rgba8::MAGENTA		);
*/

	//----------------------------------------------------------------------------------------------------------------------
	// End world camera
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->EndCamera( m_gameMode3DWorldCamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw calls 
	//----------------------------------------------------------------------------------------------------------------------
	// Backface culled objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_BackfaceCull.size() ), verts_BackfaceCull.data() );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );

	// Creature 
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theApp->m_texture_GlowingRock );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_Creature.size() ), verts_Creature.data() );

	// Jellyfish
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theApp->m_texture_Galaxy );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_textureGalaxy.size() ), verts_textureGalaxy.data() );
	
	// Transformed ikChain model origin
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	Mat44 ikChainMat = m_quadruped->m_leftFoot_2BIK->m_firstJoint->GetIkChainMatrix_ModelToWorld();
	g_theRenderer->SetModelConstants( ikChainMat );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_ikChainBody.size() ), verts_ikChainBody.data() );

	// Text objects
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_Text.size() ), verts_Text.data() );
	// Reset binded texture
	g_theRenderer->BindTexture( nullptr );

	// No Texture
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_NoTexture.size() ), verts_NoTexture.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_gameMode3DUICamera );
	DebugRenderScreen( m_gameMode3DUICamera );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	Rgba8 textColor_darkRed		= Rgba8::GREEN;
	Rgba8 textColor_darkGreen	= Rgba8::DARK_GREEN;
	float cellHeight	= 2.0f;
	AABB2 textbox1		= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	
	std::string gameInfo;
	if ( g_debugFreeFly_F1 )
	{
		if ( g_debugText_F4 )
		{
			gameInfo = Stringf( "F1 (Toggle camera/player) Camera || Mode (F6/F7 for prev/next) || GameMode (3D)" );
		}
	}						
	else
	{
		if ( g_debugText_F4 )
		{
			gameInfo = Stringf( "F1 (Toggle camera/player) Player || Mode (F6/F7 for prev/next) || GameMode (3D)" );
		}
	}
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, gameInfo, textColor_darkRed, 0.8f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );
																											   
	if ( g_debugText_F4 )
	{
		// Core Values
		float fps							 = 1.0f / g_theApp->m_gameClock.GetDeltaSeconds();
		float scale							 = g_theApp->m_gameClock.GetTimeScale();
		std::string cameraPosText			 = Stringf( "Cam position:           %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_position.x,				m_gameMode3DWorldCamera.m_position.y,					m_gameMode3DWorldCamera.m_position.z );
		std::string cameraOrientationText	 = Stringf( "Cam Orientation (YPR):  %0.2f, %0.2f, %0.2f",	m_gameMode3DWorldCamera.m_orientation.m_yawDegrees, m_gameMode3DWorldCamera.m_orientation.m_pitchDegrees,	m_gameMode3DWorldCamera.m_orientation.m_rollDegrees );
		std::string timeText				 = Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.", g_theApp->m_gameClock.GetTotalSeconds(), fps, scale );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			 	   timeText, textColor_darkRed, 0.75f,  Vec2( 1.0f, 1.0f  ), TextDrawMode::SHRINK_TO_FIT );	
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, 			  cameraPosText, textColor_darkRed, 0.75f,	Vec2( 0.0f, 0.97f ), TextDrawMode::SHRINK_TO_FIT );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight,    cameraOrientationText, textColor_darkRed, 0.75f,  Vec2( 0.0f, 0.94f ), TextDrawMode::SHRINK_TO_FIT );
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
void GameMode3D::AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
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
void GameMode3D::UpdateDebugTreePosInput()
{
	float stepAmount = 1.0f;

	// East (+X)
	if ( g_theInput->IsKeyDown( 'I' ) )
	{
		m_treeDebugTargetPos += Vec3( stepAmount, 0.0f, 0.0f );
	}
	// West (-X)
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		m_treeDebugTargetPos += Vec3( -stepAmount, 0.0f, 0.0f );
	}
	// North (+Y)
	if ( g_theInput->IsKeyDown( 'J' ) )
	{
		m_treeDebugTargetPos += Vec3( 0.0f, stepAmount, 0.0f );
	}
	// South (-Y)
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		m_treeDebugTargetPos += Vec3( 0.0f, -stepAmount, 0.0f );
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'O' ) )
	{
		m_treeDebugTargetPos += Vec3( 0.0f, 0.0f, stepAmount );
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'U' ) )
	{
		m_treeDebugTargetPos += Vec3( 0.0f, 0.0f, -stepAmount );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Start Pos
	//----------------------------------------------------------------------------------------------------------------------
	// East (+X)
	if ( g_theInput->IsKeyDown( 'T' ) )
	{
		m_treeBranch2->m_position_WS += Vec3( 1.0f, 0.0f, 0.0f );
	}
	// West (-X)
	if ( g_theInput->IsKeyDown( 'G' ) )
	{
		m_treeBranch2->m_position_WS += Vec3( -1.0f, 0.0f, 0.0f );
	}
	// North (+Y)
	if ( g_theInput->IsKeyDown( 'F' ) )
	{
		m_treeBranch2->m_position_WS += Vec3( 0.0f, 1.0f, 0.0f );
	}
	// South (-Y)
	if ( g_theInput->IsKeyDown( 'H' ) )
	{
		m_treeBranch2->m_position_WS += Vec3( 0.0f, -1.0f, 0.0f );
	}
}

	
//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::InitializeCreatures()
{
	float initZOffset = 5.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Quadruped
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped	= new Quadruped( this, Vec3( 0.0f, 0.0f, m_rootDefaultHeightZ ), 1.0f );
	m_quadruped->InitLimbs();
	m_quadruped->m_modelRoot->m_jointPos_MS = Vec3( -100.0f, 50.0f, m_rootDefaultHeightZ + initZOffset );

	//----------------------------------------------------------------------------------------------------------------------
	// Jellyfish
	//----------------------------------------------------------------------------------------------------------------------
	InitJellyfish();

	//----------------------------------------------------------------------------------------------------------------------
	// Spider
	//----------------------------------------------------------------------------------------------------------------------
	m_spiderBase = new SpiderBase( this, Vec3( 0.0f, 0.0f, m_rootDefaultHeightZ ), 1.0f );
	m_spiderBase->m_modelRoot->m_jointPos_MS = Vec3( 100.0f, 50.0f, m_rootDefaultHeightZ + initZOffset );
	m_spiderBase->InitLimbs();

	// Update once on startup to pose leg targets at the correct positions
	float fakeDeltaSeconds = 0.016f;
	m_quadruped ->PreUpdateChains ( fakeDeltaSeconds );
	m_spiderBase->PreUpdateChains ( fakeDeltaSeconds );
	m_quadruped ->PostUpdateChains( fakeDeltaSeconds );
	m_spiderBase->PostUpdateChains( fakeDeltaSeconds );
	m_quadruped ->SetAllChainsCurTargetPosToTargetGoal();
	m_spiderBase->SetAllChainsCurTargetPosToTargetGoal();
	
	// Set controls 
	if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_QUADRUPED )
	{
		m_quadruped->m_isControlled = false;
	}
	else if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_SPIDER )
	{
		m_spiderBase->m_isControlled = false;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::RenderCreature( std::vector<Vertex_PCU>& verts, 
								 std::vector<Vertex_PCU>& verts_texturedCreature, 
								 std::vector<Vertex_PCU>& textVerts, 
								 std::vector<Vertex_PCU>& verts_BackFace,
								 std::vector<Vertex_PCU>& verts_ikChainBody 
								) const
{
	UNUSED( verts_texturedCreature );

	//----------------------------------------------------------------------------------------------------------------------
	// Quadruped
	//----------------------------------------------------------------------------------------------------------------------	
	// Toggle rendering chains for debugging
	m_quadruped->m_leftFoot_2BIK		->m_shouldRender = true;
	m_quadruped->m_rightFoot_2BIK		->m_shouldRender = true;
	m_quadruped->m_leftArm_2BIK			->m_shouldRender = true;
	m_quadruped->m_rightArm_2BIK		->m_shouldRender = true;
	m_quadruped->m_neck_FABRIK			->m_shouldRender = false;
	m_quadruped->m_head_FABRIK			->m_shouldRender = false;
	m_quadruped->m_spine_CCD			->m_shouldRender = true;
	m_quadruped->m_tail_CCD				->m_shouldRender = false;
	m_quadruped->m_tentacleLeft_CCD		->m_shouldRender = false;
	m_quadruped->m_tentacleRight_CCD	->m_shouldRender = false;
//	m_quadruped->Render( verts_texturedCreature, Rgba8::WHITE, Rgba8::MAGENTA );
	// Root body WS
	AABB3 bounds = AABB3( -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f );
	AddVertsForAABB3D( verts_ikChainBody, bounds, Rgba8::BROWN );
	
	// Debug info
	if ( g_debugBasis_F3 )
	{
		// Joint basis
		m_quadruped->m_modelRoot			->RenderIJK_MS( verts, 20.0f );
		m_quadruped->m_leftFoot_2BIK		->DebugDrawJoints_IJK( verts );
		m_quadruped->m_rightFoot_2BIK		->DebugDrawJoints_IJK( verts );
		m_quadruped->m_leftArm_2BIK			->DebugDrawJoints_IJK( verts );
		m_quadruped->m_rightArm_2BIK		->DebugDrawJoints_IJK( verts );
		m_quadruped->m_neck_FABRIK			->DebugDrawJoints_IJK( verts );
		m_quadruped->m_head_FABRIK			->DebugDrawJoints_IJK( verts );
		m_quadruped->m_spine_CCD			->DebugDrawJoints_IJK( verts );
		m_quadruped->m_tail_CCD				->DebugDrawJoints_IJK( verts );
		m_quadruped->m_tentacleLeft_CCD		->DebugDrawJoints_IJK( verts );
		m_quadruped->m_tentacleRight_CCD	->DebugDrawJoints_IJK( verts );

		// Current position and target positions
		AddVertsForSphere3D( verts,  m_quadruped->m_leftArm_2BIK->m_target.m_currentPos_WS,	1.0f, 4.0f, 4.0f, Rgba8::WHITE );
		AddVertsForSphere3D( verts, m_quadruped->m_rightArm_2BIK->m_target.m_currentPos_WS,	1.0f, 4.0f, 4.0f, Rgba8::GRAY  );
		// Ideal position
		AddVertsForSphere3D( verts, m_quadruped->m_leftFoot_2BIK	->m_idealStepPos_WS, 1.0f, 4.0f, 4.0f, Rgba8::WHITE );
		AddVertsForSphere3D( verts, m_quadruped->m_rightFoot_2BIK	->m_idealStepPos_WS, 1.0f, 4.0f, 4.0f, Rgba8::GRAY  );
		AddVertsForSphere3D( verts, m_quadruped->m_leftArm_2BIK		->m_idealStepPos_WS, 1.0f, 4.0f, 4.0f, Rgba8::WHITE );
		AddVertsForSphere3D( verts, m_quadruped->m_rightArm_2BIK	->m_idealStepPos_WS, 1.0f, 4.0f, 4.0f, Rgba8::GRAY  );
		m_quadruped->m_leftArm_2BIK  ->RenderTarget_IJK( verts, 4.0f );
		m_quadruped->m_rightArm_2BIK ->RenderTarget_IJK( verts, 4.0f );
		m_quadruped->m_leftFoot_2BIK ->RenderTarget_IJK( verts, 4.0f );
		m_quadruped->m_rightFoot_2BIK->RenderTarget_IJK( verts, 4.0f );
	}
	if ( g_debugText_F4 )
	{
		float textHeight = 0.4f;
		Vec3  left		 = m_gameMode3DWorldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
		Vec3  up		 = m_gameMode3DWorldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
		 m_quadruped->m_leftArm_2BIK->DebugTextJointPos_WorldSpace( textVerts, textHeight, 2.0f, left, up, g_theApp->m_textFont );
		m_quadruped->m_rightArm_2BIK->DebugTextJointPos_WorldSpace( textVerts, textHeight, 2.0f, left, up, g_theApp->m_textFont );
	}
	if ( g_debugRenderBezier_1 )
	{
		// Debug Render Bezier
		m_quadruped->DebugDrawBezier( verts, m_quadruped->m_bezier_leftArm,   m_timer_LeftArm   );
		m_quadruped->DebugDrawBezier( verts, m_quadruped->m_bezier_rightArm,  m_timer_RightArm  );
		m_quadruped->DebugDrawBezier( verts, m_quadruped->m_bezier_leftFoot,  m_timer_LeftFoot  );
		m_quadruped->DebugDrawBezier( verts, m_quadruped->m_bezier_rightFoot, m_timer_RightFoot );
	}

	 // Debug info
	if ( g_debugBasis_F3 )
	{
	}	
	if ( g_debugAngles_F5 )
	{
		// Hip cone constraints
		m_quadruped->m_spine_CCD->DebugDrawConstraints_YPR( verts_BackFace, 2.0f );

		float constraintHeight = 2.5f;
		m_quadruped->m_leftArm_2BIK-> DebugDrawConstraints_YPR( verts_BackFace, constraintHeight );
		m_quadruped->m_rightArm_2BIK->DebugDrawConstraints_YPR( verts_BackFace, constraintHeight );
	}
	if ( g_debugRenderBezier_1 )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Test rendering for bezier curves 
		//----------------------------------------------------------------------------------------------------------------------
		// Right Arm
		float elaspedTime	= m_timer_RightArm.GetElapsedTime();
		Vec3 bezierPosAtT	= m_bezierCurve_RightArm.GetPointAtFraction( elaspedTime );
		AddVertsForSphere3D( verts, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );
		AddVertsForSphere3D( verts, m_bezierCurve_RightArm.m_startPos , 2.0f, 8.0f, 16.0f, Rgba8::RED	  );
		AddVertsForSphere3D( verts, m_bezierCurve_RightArm.m_guidePos1, 2.0f, 8.0f, 16.0f, Rgba8::GREEN   );
		AddVertsForSphere3D( verts, m_bezierCurve_RightArm.m_guidePos2, 2.0f, 8.0f, 16.0f, Rgba8::BLUE	  );
		AddVertsForSphere3D( verts, m_bezierCurve_RightArm.m_endPos   , 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA );
		// Left Arm
		elaspedTime			= m_timer_LeftArm.GetElapsedTime();
		bezierPosAtT		= m_bezierCurve_LeftArm.GetPointAtFraction( elaspedTime );
		AddVertsForSphere3D( verts, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );
		AddVertsForSphere3D( verts, m_bezierCurve_LeftArm.m_startPos , 2.0f, 8.0f, 16.0f, Rgba8::RED	  );
		AddVertsForSphere3D( verts, m_bezierCurve_LeftArm.m_guidePos1, 2.0f, 8.0f, 16.0f, Rgba8::GREEN    );
		AddVertsForSphere3D( verts, m_bezierCurve_LeftArm.m_guidePos2, 2.0f, 8.0f, 16.0f, Rgba8::BLUE	  );
		AddVertsForSphere3D( verts, m_bezierCurve_LeftArm.m_endPos   , 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA  );
		// Right Foot
		elaspedTime			= m_timer_RightFoot.GetElapsedTime();
		bezierPosAtT		= m_bezierCurve_RightFoot.GetPointAtFraction( elaspedTime );
		AddVertsForSphere3D( verts, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );
		AddVertsForSphere3D( verts, m_bezierCurve_RightFoot.m_startPos , 2.0f, 8.0f, 16.0f, Rgba8::RED	  );
		AddVertsForSphere3D( verts, m_bezierCurve_RightFoot.m_guidePos1, 2.0f, 8.0f, 16.0f, Rgba8::GREEN   );
		AddVertsForSphere3D( verts, m_bezierCurve_RightFoot.m_guidePos2, 2.0f, 8.0f, 16.0f, Rgba8::BLUE	  );
		AddVertsForSphere3D( verts, m_bezierCurve_RightFoot.m_endPos   , 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA );
		// Left Arm
		elaspedTime			= m_timer_LeftFoot.GetElapsedTime();
		bezierPosAtT		= m_bezierCurve_LeftFoot.GetPointAtFraction( elaspedTime );
		AddVertsForSphere3D( verts, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );
		AddVertsForSphere3D( verts, m_bezierCurve_LeftFoot.m_startPos , 2.0f, 8.0f, 16.0f, Rgba8::RED	  );
		AddVertsForSphere3D( verts, m_bezierCurve_LeftFoot.m_guidePos1, 2.0f, 8.0f, 16.0f, Rgba8::GREEN    );
		AddVertsForSphere3D( verts, m_bezierCurve_LeftFoot.m_guidePos2, 2.0f, 8.0f, 16.0f, Rgba8::BLUE	  );
		AddVertsForSphere3D( verts, m_bezierCurve_LeftFoot.m_endPos   , 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA  );

		// Left arm bezierCurve trail
		float thickness = 0.5f;
		Vec3 previousBezierDotPos = m_bezierCurve_LeftArm.m_startPos;
		int		m_numSubdivisions = 64;
		for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
		{
			// Calculate subdivisions
			float t						= (1.0f / static_cast<float>(m_numSubdivisions)) * (subdivisionIndex + 1);
			Vec3 currentBezierDotPos	= m_bezierCurve_LeftArm.GetPointAtFraction( t );
			// Render curved line graph
			AddVertsForLineSegment3D( verts, previousBezierDotPos, currentBezierDotPos, thickness, Rgba8::PURPLE );
			previousBezierDotPos = currentBezierDotPos;
		}
		// Right arm bezierCurve trail
		previousBezierDotPos = m_bezierCurve_RightArm.m_startPos;
		for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
		{
			// Calculate subdivisions
			float t						= (1.0f / static_cast<float>(m_numSubdivisions)) * (subdivisionIndex + 1);
			Vec3 currentBezierDotPos	= m_bezierCurve_RightArm.GetPointAtFraction( t );
			// Render curved line graph
			AddVertsForLineSegment3D( verts, previousBezierDotPos, currentBezierDotPos, thickness, Rgba8::PURPLE );
			previousBezierDotPos = currentBezierDotPos;
		}
		// Left Foot bezierCurve trail
		previousBezierDotPos = m_bezierCurve_RightFoot.m_startPos;
		m_numSubdivisions	 = 64;
		for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
		{
			// Calculate subdivisions
			float t						= (1.0f / static_cast<float>(m_numSubdivisions)) * (subdivisionIndex + 1);
			Vec3 currentBezierDotPos	= m_bezierCurve_RightFoot.GetPointAtFraction( t );
			// Render curved line graph
			AddVertsForLineSegment3D( verts, previousBezierDotPos, currentBezierDotPos, thickness, Rgba8::PURPLE );
			previousBezierDotPos = currentBezierDotPos;
		}
		// Right Foot bezierCurve trail
		previousBezierDotPos = m_bezierCurve_LeftFoot.m_startPos;
		for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
		{
			// Calculate subdivisions
			float t						= (1.0f / static_cast<float>(m_numSubdivisions)) * (subdivisionIndex + 1);
			Vec3 currentBezierDotPos	= m_bezierCurve_LeftFoot.GetPointAtFraction( t );
			// Render curved line graph
			AddVertsForLineSegment3D( verts, previousBezierDotPos, currentBezierDotPos, thickness, Rgba8::PURPLE );
			previousBezierDotPos = currentBezierDotPos;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::UpdateCameraAndCreatureInput( float deltaSeconds )
{
	if ( g_debugFreeFly_F1 == true )
	{
		return;
	}
	if ( m_isClimbing == true )
	{
		return;
	}

	Vec3 iBasis, jBasis, kBasis;
	m_gameMode3DWorldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
	iBasis.z = 0.0f;
	jBasis.z = 0.0f;
	iBasis	 = iBasis.GetNormalized(); 
	jBasis	 = jBasis.GetNormalized(); 
	kBasis   = kBasis.GetNormalized();

	Vec3 moveIntention = m_moveFwdDir;
	//----------------------------------------------------------------------------------------------------------------------
	// All directions are local
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_currentSpeed		= m_fasterSpeed;
		m_isSprinting		= true;
		m_goalWalkLerpSpeed	= m_sprintLerpSpeed;
	}
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_currentSpeed		= m_defaultSpeed;
		m_isSprinting		= false;
		m_goalWalkLerpSpeed	= m_walkLerpSpeed;
	}

	if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_QUADRUPED )
	{
		// Forward
		if ( g_theInput->IsKeyDown( 'W' ) )
		{
			m_quadruped->m_modelRoot->m_jointPos_MS += ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
			moveIntention							+= ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		// Left
		if ( g_theInput->IsKeyDown( 'A' ) )
		{
			m_quadruped->m_modelRoot->m_jointPos_MS += ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
			moveIntention							+= ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		// Backwards
		if ( g_theInput->IsKeyDown( 'S' ) )
		{
			m_quadruped->m_modelRoot->m_jointPos_MS	-= ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
			moveIntention							-= ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		// Right
		if ( g_theInput->IsKeyDown( 'D' ) )
		{
			m_quadruped->m_modelRoot->m_jointPos_MS	-= ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
			moveIntention							-= ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		// Sky (+Z)
		if ( g_theInput->IsKeyDown( 'E' ) )
		{
			m_quadruped->m_modelRoot->m_jointPos_MS	+= ( kBasis * m_currentSpeed ) * deltaSeconds;
			moveIntention							+= ( kBasis * m_currentSpeed ) * deltaSeconds;
		}
		// Ground (-Z)
		if ( g_theInput->IsKeyDown( 'Q' ) )
		{
			m_quadruped->m_modelRoot->m_jointPos_MS	-= ( kBasis * m_currentSpeed ) * deltaSeconds;
			moveIntention							-= ( kBasis * m_currentSpeed ) * deltaSeconds;
		}
	}
	else if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_SPIDER )
	{
		// Forward
		if ( g_theInput->IsKeyDown( 'W' ) )
		{
			m_spiderBase->m_modelRoot->m_jointPos_MS	+= ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
			moveIntention								+= ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		// Left
		if ( g_theInput->IsKeyDown( 'A' ) )
		{
			m_spiderBase->m_modelRoot->m_jointPos_MS	+= ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
			moveIntention								+= ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		// Backwards
		if ( g_theInput->IsKeyDown( 'S' ) )
		{
			m_spiderBase->m_modelRoot->m_jointPos_MS	-= ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
			moveIntention								-= ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		// Right
		if ( g_theInput->IsKeyDown( 'D' ) )
		{
			m_spiderBase->m_modelRoot->m_jointPos_MS	-= ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
			moveIntention								-= ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds;
		}
		// Sky (+Z)
		if ( g_theInput->IsKeyDown( 'E' ) )
		{
			m_spiderBase->m_modelRoot->m_jointPos_MS	+= ( kBasis * m_currentSpeed ) * deltaSeconds;
			moveIntention								+= ( kBasis * m_currentSpeed ) * deltaSeconds;
		}
		// Ground (-Z)
		if ( g_theInput->IsKeyDown( 'Q' ) )
		{
			m_spiderBase->m_modelRoot->m_jointPos_MS	-= ( kBasis * m_currentSpeed ) * deltaSeconds;
			moveIntention								-= ( kBasis * m_currentSpeed ) * deltaSeconds;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Set movementFwdDir
	//----------------------------------------------------------------------------------------------------------------------
	m_moveFwdDir = moveIntention.GetNormalized();
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::TurnCreatureTowardsCameraDir()
{
	if ( g_debugFreeFly_F1 )
	{
		return;
	} 

	Vec3  camFwdDir		 =  m_gameMode3DWorldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp();
	float goalDegrees	 = Atan2Degrees( m_moveFwdDir.y, m_moveFwdDir.x );
	float currentRootYaw = 0.0f;
	if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_QUADRUPED )
	{
		currentRootYaw												= m_quadruped->m_modelRoot->m_eulerAnglesGoal_LS.m_yawDegrees;
 		float newRootYawAngle										= GetTurnedTowardDegrees( currentRootYaw, goalDegrees, 1.5f );
		m_quadruped->m_modelRoot->m_eulerAnglesGoal_LS.m_yawDegrees = newRootYawAngle; 
	}
	else if ( m_controlledCreatureType == ControlledCreatureType::CONTROLLED_SPIDER )
	{
		currentRootYaw												 = m_spiderBase->m_modelRoot->m_eulerAnglesGoal_LS.m_yawDegrees;
 		float newRootYawAngle										 = GetTurnedTowardDegrees( currentRootYaw, goalDegrees, 1.5f );
		m_spiderBase->m_modelRoot->m_eulerAnglesGoal_LS.m_yawDegrees = newRootYawAngle; 
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::InitializeEnvironment()
{
	m_map = new Map_GameMode3D( this );
	m_map->m_foodManager = new FoodManager( 1, this );
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::RenderRaycasts( std::vector<Vertex_PCU>& verts ) const
{
	if ( !g_debugRenderRaycast_F2 )
	{
		return;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// DebugRenderRaycast for Quadruped Creature
	//----------------------------------------------------------------------------------------------------------------------
	m_quadruped->DebugDrawRaycasts( verts );


	//----------------------------------------------------------------------------------------------------------------------
	// Fwd raycast
	//----------------------------------------------------------------------------------------------------------------------
	// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
	if ( m_didRayImpact_FWD )
	{
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_rayStartPos_FWD, m_rayEndPos_FWD, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_updatedImpactPos_FWD, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_updatedImpactPos_FWD, m_updatedImpactPos_FWD + ( m_updatedImpactNormal_FWD * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else if ( m_didRayImpactClimbableObject_FWD )
	{
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_rayStartPos_FWD, m_rayEndPos_FWD, m_arrowThickness, m_rayImpactDistColor2 );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_updatedImpactPos_FWD, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_updatedImpactPos_FWD, m_updatedImpactPos_FWD + ( m_updatedImpactNormal_FWD * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );

	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_rayStartPos_FWD, m_rayEndPos_FWD, m_arrowThickness, m_rayDefaultColor );
	}
	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm
	//----------------------------------------------------------------------------------------------------------------------
	// RAD (Right Arm Down)
	if ( m_raycast_rightArmDown.m_didRayImpact )
	{
		// Check if raycast impacted a line		
		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_rightArmDown.m_rayStartPos, m_raycast_rightArmDown.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_rightArmDown.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_rightArmDown.m_updatedImpactPos, m_raycast_rightArmDown.m_updatedImpactPos + ( m_raycast_rightArmDown.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_rightArmDown.m_rayStartPos, m_raycast_rightArmDown.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	// RAF (Right Arm Forward)
	if ( m_raycast_rightArmFwd.m_didRayImpact )
	{
		// Check if raycast impacted a line		
		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_rightArmFwd.m_rayStartPos, m_raycast_rightArmFwd.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_rightArmFwd.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_rightArmFwd.m_updatedImpactPos, m_raycast_rightArmFwd.m_updatedImpactPos + ( m_raycast_rightArmFwd.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_rightArmFwd.m_rayStartPos, m_raycast_rightArmFwd.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	// NRAD (Next Right Arm Down)
	if ( m_raycast_NextRightArmDown.m_didRayImpact )
	{
		// Check if raycast impacted a line		
		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_NextRightArmDown.m_rayStartPos, m_raycast_NextRightArmDown.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_NextRightArmDown.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_NextRightArmDown.m_updatedImpactPos, m_raycast_NextRightArmDown.m_updatedImpactPos + ( m_raycast_NextRightArmDown.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_NextRightArmDown.m_rayStartPos, m_raycast_NextRightArmDown.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	
	//----------------------------------------------------------------------------------------------------------------------
	// Left Arm
	//----------------------------------------------------------------------------------------------------------------------
	// LAD (Left Arm Down)
	if ( m_raycast_LeftArmDown.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_LeftArmDown.m_rayStartPos, m_raycast_LeftArmDown.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_LeftArmDown.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_LeftArmDown.m_updatedImpactPos, m_raycast_LeftArmDown.m_updatedImpactPos + ( m_raycast_LeftArmDown.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_LeftArmDown.m_rayStartPos, m_raycast_LeftArmDown.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	// LAF (Left Arm Forward)
	if ( m_raycast_LeftArmFwd.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_LeftArmFwd.m_rayStartPos, m_raycast_LeftArmFwd.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_LeftArmFwd.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_LeftArmFwd.m_updatedImpactPos, m_raycast_LeftArmFwd.m_updatedImpactPos + ( m_raycast_LeftArmFwd.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_LeftArmFwd.m_rayStartPos, m_raycast_LeftArmFwd.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	// NLAD (Next Left Arm Down)
	if ( m_raycast_NextLeftArmDown.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_NextLeftArmDown.m_rayStartPos, m_raycast_NextLeftArmDown.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_NextLeftArmDown.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_NextLeftArmDown.m_updatedImpactPos, m_raycast_NextLeftArmDown.m_updatedImpactPos + ( m_raycast_NextLeftArmDown.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_NextLeftArmDown.m_rayStartPos, m_raycast_NextLeftArmDown.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Left Foot
	//----------------------------------------------------------------------------------------------------------------------
	// LFD (Left Foot Down)
	if ( m_raycast_leftFootDown.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_leftFootDown.m_rayStartPos, m_raycast_leftFootDown.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_leftFootDown.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_leftFootDown.m_updatedImpactPos, m_raycast_leftFootDown.m_updatedImpactPos+ ( m_raycast_leftFootDown.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_leftFootDown.m_rayStartPos, m_raycast_leftFootDown.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	// LAF (Left Foot Forward)
	if ( m_raycast_LeftFootFwd.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_LeftFootFwd.m_rayStartPos, m_raycast_LeftFootFwd.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_LeftFootFwd.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_LeftFootFwd.m_updatedImpactPos, m_raycast_LeftFootFwd.m_updatedImpactPos + ( m_raycast_LeftFootFwd.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_LeftFootFwd.m_rayStartPos, m_raycast_LeftFootFwd.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Right Foot
	//----------------------------------------------------------------------------------------------------------------------
	// RAD (Right Foot Down)
	if ( m_raycast_rightFootDown.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_rightFootDown.m_rayStartPos, m_raycast_rightFootDown.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_rightFootDown.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_rightFootDown.m_updatedImpactPos, m_raycast_rightFootDown.m_updatedImpactPos + ( m_raycast_rightFootDown.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_rightFootDown.m_rayStartPos, m_raycast_rightFootDown.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
	// RAF (Right Foot Forward)
	if ( m_raycast_rightFootFwd.m_didRayImpact )
	{
		// Check if raycast impacted a line		// If true, render raycast impactNormal, impactDist, and ray after hit 
		// Ray until impact			// RayStart to rayEndPoint
		AddVertsForArrow3D( verts, m_raycast_rightFootFwd.m_rayStartPos, m_raycast_rightFootFwd.m_rayEndPos, m_arrowThickness, m_rayImpactDistColor );
		// Disc at raycast impactPos 
		AddVertsForSphere3D( verts, m_raycast_rightFootFwd.m_updatedImpactPos, ( m_arrowThickness * 1.0f ), 8.0f, 8.0f, m_rayImpactDiscColor );
		// Impact normal			// raycast reflection off line 
		AddVertsForArrow3D( verts, m_raycast_rightFootFwd.m_updatedImpactPos, m_raycast_rightFootFwd.m_updatedImpactPos + ( m_raycast_rightFootFwd.m_updatedImpactNormal * 4.0f ), ( m_arrowThickness * 1.0f ), m_rayImpactNormalColor );
	}
	else
	{
		// Ray start to end with no impact
		AddVertsForArrow3D( verts, m_raycast_rightFootFwd.m_rayStartPos, m_raycast_rightFootFwd.m_rayEndPos, m_arrowThickness, m_rayDefaultColor );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::MoveRaycastInput( float deltaSeconds )
{
	float moveSpeed = 10.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Move entire raycast
	//----------------------------------------------------------------------------------------------------------------------
	// Move North
	if ( g_theInput->IsKeyDown( 'J' ) )
	{
		m_raycast_rightArmDown.m_rayStartPos += Vec3( 0.0f, moveSpeed, 0.0f ) * deltaSeconds; 
		m_raycast_rightArmDown.m_rayEndPos   += Vec3( 0.0f, moveSpeed, 0.0f ) * deltaSeconds;
	}
	// Move South
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		m_raycast_rightArmDown.m_rayStartPos += Vec3( 0.0f, -moveSpeed, 0.0f ) * deltaSeconds;
		m_raycast_rightArmDown.m_rayEndPos   += Vec3( 0.0f, -moveSpeed, 0.0f ) * deltaSeconds;
	}
	// Move East
	if ( g_theInput->IsKeyDown( 'I' ) )
	{
		m_raycast_rightArmDown.m_rayStartPos += Vec3( moveSpeed, 0.0f, 0.0f ) * deltaSeconds;
		m_raycast_rightArmDown.m_rayEndPos   += Vec3( moveSpeed, 0.0f, 0.0f ) * deltaSeconds;
	}
	// Move West
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		m_raycast_rightArmDown.m_rayStartPos += Vec3( -moveSpeed, 0.0f, 0.0f ) * deltaSeconds;
		m_raycast_rightArmDown.m_rayEndPos   += Vec3( -moveSpeed, 0.0f, 0.0f ) * deltaSeconds;
	}
	// Move Skyward
	if ( g_theInput->IsKeyDown( 'O' ) )
	{
		m_raycast_rightArmDown.m_rayStartPos += Vec3( 0.0f, 0.0f, moveSpeed ) * deltaSeconds;
		m_raycast_rightArmDown.m_rayEndPos   += Vec3( 0.0f, 0.0f, moveSpeed ) * deltaSeconds;
	}
	// Move Groundward
	if ( g_theInput->IsKeyDown( 'U' ) )
	{
		m_raycast_rightArmDown.m_rayStartPos += Vec3( 0.0f, 0.0f, -moveSpeed ) * deltaSeconds;
		m_raycast_rightArmDown.m_rayEndPos   += Vec3( 0.0f, 0.0f, -moveSpeed ) * deltaSeconds;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// East
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) )
	{
		m_raycast_rightArmDown.m_rayEndPos += Vec3( moveSpeed, 0.0f, 0.0f ) * deltaSeconds; 
	}
	// West
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) )
	{
		m_raycast_rightArmDown.m_rayEndPos += Vec3( -moveSpeed, 0.0f, 0.0f ) * deltaSeconds; 
	}
	// North
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) )
	{
		m_raycast_rightArmDown.m_rayEndPos += Vec3( 0.0f, moveSpeed, 0.0f ) * deltaSeconds; 
	}
	// South
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
	{
		m_raycast_rightArmDown.m_rayEndPos += Vec3( 0.0f, -moveSpeed, 0.0f ) * deltaSeconds; 
	}
	// Skyward
	if ( g_theInput->IsKeyDown( 'M' ) )
	{
		m_raycast_rightArmDown.m_rayEndPos += Vec3( 0.0f, 0.0f, moveSpeed ) * deltaSeconds;
	}
	// Groundward
	if ( g_theInput->IsKeyDown( 'N' ) )
	{
		m_raycast_rightArmDown.m_rayEndPos += Vec3( 0.0f, 0.0f, -moveSpeed ) * deltaSeconds;
	}
}


//----------------------------------------------------------------------------------------------------------------------
bool GameMode3D::DidRaycastHitTriangle( RaycastResult3D& raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal )
{
	bool  didImpact		= false;
	float t, u, v		= 0.0f;
	RaycastResult3D tempRayResult;
	// Increment in 3s to form triangles
	for ( int i = 0; i < m_map->m_indexList.size(); i += 3 )
	{
		// Calculate triangles from indexList
		int const& currentIndex_C = m_map->m_indexList[ i + 2];
		int const& currentIndex_A = m_map->m_indexList[ i + 0];
		int const& currentIndex_B = m_map->m_indexList[ i + 1];
		Vec3 vert0		= m_map->m_planeVerts[ currentIndex_A ].m_position;
		Vec3 vert1		= m_map->m_planeVerts[ currentIndex_B ].m_position;
		Vec3 vert2		= m_map->m_planeVerts[ currentIndex_C ].m_position;
		tempRayResult	= RaycastVsTriangle( rayStartPos, rayfwdNormal, rayLength, vert0, vert1, vert2, t, u, v );
		if ( tempRayResult.m_didImpact )
		{
			// If ray hit AND is closer
			if ( tempRayResult.m_impactDist < raycastResult.m_impactDist )
			{
				raycastResult		= tempRayResult;
				updatedImpactPos	= raycastResult.m_impactPos;
				updatedImpactNormal = raycastResult.m_impactNormal;
				didImpact			= true;
			}
		}
	}
	return didImpact;
}


//----------------------------------------------------------------------------------------------------------------------
void GameMode3D::InitJellyfish()
{
	m_jellyfish = new Jellyfish( this, Vec3::ZERO, 1.0f );
	m_jellyfish->InitJoints();
	m_jellyfish->InitMesh();
}