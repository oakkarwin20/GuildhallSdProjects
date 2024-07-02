#include "Game/GameModeBase.hpp"
#include "Game/GameMode_Quadruped_3D.hpp"
#include "Game/GameMode3D.hpp"
#include "Game/Quadruped.hpp"
#include "Game/Map_GameMode3D.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/ThirdParty/Squirrel/Noise/SmoothNoise.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Shader.hpp"

//----------------------------------------------------------------------------------------------------------------------
Quadruped::Quadruped( GameMode3D* game, Vec3 const& rootStartPos, float limbLength ) : CreatureBase( rootStartPos, limbLength )
{
	m_gameMode3D = game;
}


//----------------------------------------------------------------------------------------------------------------------
Quadruped::Quadruped( GameMode_Quadruped_3D* gameMode_Biped, Vec3 const& rootStartPos, float limbLength /*= FloatRange( 1.0f, 1.0f ) */ ) : CreatureBase( rootStartPos, limbLength )
{
	m_gameMode_Creature = gameMode_Biped;
	m_desiredRootHeightAboveLimbs = rootStartPos.z;
}


//----------------------------------------------------------------------------------------------------------------------
Quadruped::~Quadruped()
{
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::RenderQuadruped(	std::vector<Vertex_PCU>& verts, 
									std::vector<Vertex_PCU>& verts_textured, 
									std::vector<Vertex_PCU>& textVerts, 
									std::vector<Vertex_PCU>& verts_BackFace,
									std::vector<Vertex_PCU>& verts_ikChainBody,
									Camera const&			 worldCamera,
									Texture*				 creatureTexture 
								) const
{
	UNUSED( verts_textured );
	creatureTexture = g_theApp->m_texture_MoonSurface;
	//----------------------------------------------------------------------------------------------------------------------
	// Creature Root bone (hip)
	//----------------------------------------------------------------------------------------------------------------------
	// Toggle rendering chains for debugging
	m_leftFoot_2BIK		->m_shouldRender = true;
	m_rightFoot_2BIK	->m_shouldRender = true;
	m_leftArm_2BIK		->m_shouldRender = true;
	m_rightArm_2BIK		->m_shouldRender = true;
	m_neck_FABRIK		->m_shouldRender = true;
	m_head_FABRIK		->m_shouldRender = true;
	m_spine_CCD			->m_shouldRender = true;
	m_tail_CCD			->m_shouldRender = true;
	m_tentacleLeft_CCD	->m_shouldRender = false;
	m_tentacleRight_CCD	->m_shouldRender = false;
	if ( g_debugToggleMesh_Backspace )
	{
		RenderMesh( g_theRenderer, creatureTexture, g_litShader_PCUTBN );		// Render using meshs
	}
	else
	{
		m_modelRoot->RenderIJK_MS( verts, 7.0f, 0.2f );
		Render( verts_textured, Rgba8::WHITE, Rgba8::WHITE );					// Render bones/joints only
	}
	AddVertsForSphere3D( verts, m_modelRoot->m_jointPos_MS, 1.2f, 16.0f, 16.0f ); 
	
	// Root body WS
	AABB3 bounds = AABB3( -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f );
	AddVertsForAABB3D( verts_ikChainBody, bounds, Rgba8::BROWN );
	//----------------------------------------------------------------------------------------------------------------------
	// Chain targets, Current positions
	//----------------------------------------------------------------------------------------------------------------------
//	m_leftFoot_2BIK		->RenderTarget_CurrentPos( verts, 0.5f );
//	m_rightFoot_2BIK	->RenderTarget_CurrentPos( verts, 0.5f );
//	m_spine1_CCD		->RenderTarget_CurrentPos( verts, 0.5f );
// 	m_leftArm_2BIK		->RenderTarget_CurrentPos( verts, 0.5f );
// 	m_rightArm_2BIK		->RenderTarget_CurrentPos( verts, 0.5f );
//	m_tail_CCD			->RenderTarget_CurrentPos( verts, 0.5f );
//	m_neck_FABRIK		->RenderTarget_CurrentPos( verts, 0.5f );
//	m_head_FABRIK		->RenderTarget_CurrentPos( verts, 0.5f );
// 	m_tentacleLeft_CCD	->RenderTarget_CurrentPos( verts, 0.5f );
//	m_tentacleRight_CCD	->RenderTarget_CurrentPos( verts, 0.5f );	
	//----------------------------------------------------------------------------------------------------------------------
	// Chain targets, Goal position 
	//----------------------------------------------------------------------------------------------------------------------	
//	m_leftFoot_2BIK		->RenderTarget_GoalPos( verts, 0.5f, Rgba8::CYAN );
//	m_rightFoot_2BIK	->RenderTarget_GoalPos( verts, 0.5f, Rgba8::CYAN );
//	m_spine1_CCD		->RenderTarget_GoalPos( verts, 0.5f, Rgba8::CYAN );
//	m_leftArm_2BIK		->RenderTarget_GoalPos( verts, 0.5f, Rgba8::CYAN );
//	m_rightArm_2BIK		->RenderTarget_GoalPos( verts, 0.5f, Rgba8::CYAN );
//	m_tail_CCD			->RenderTarget_GoalPos( verts, 0.5f, Rgba8::CYAN );
//	m_neck_FABRIK		->RenderTarget_GoalPos( verts_textured, 0.5f, Rgba8::CYAN );
//	m_head_FABRIK		->RenderTarget_GoalPos( verts_textured, 0.5f, Rgba8::CYAN );

	float textHeight = 0.75f;
	if ( g_debugBasis_F3 )
	{
		m_head_FABRIK->m_shouldRender = true;

		float arrowLength = 7.0f;
//		m_leftFoot_2BIK		->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
//		m_rightFoot_2BIK	->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
// 		m_spine_CCD			->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
// 		m_leftArm_2BIK		->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
// 		m_rightArm_2BIK		->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
// 		m_tail_CCD			->DebugDrawJoints_IJK( verts, 0.1f, 5.0f );
// 		m_neck_FABRIK		->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
// 		m_head_FABRIK		->DebugDrawJoints_IJK( verts, 0.25f, 10.0f );
// 		m_tentacleLeft_CCD	->DebugDrawJoints_IJK( verts );
// 		m_tentacleRight_CCD	->DebugDrawJoints_IJK( verts );
		arrowLength = 3.0f;
	}
	if ( g_debugText_F4 )
	{
		Vec3 camLeft = worldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
		Vec3 camUp	 = worldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
// 		m_leftFoot_2BIK		->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
// 		m_rightFoot_2BIK	->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
//		m_spine1_CCD		->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
//		m_leftArm_2BIK		->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
//		m_rightArm_2BIK		->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
// 		m_tail_CCD			->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
		m_neck_FABRIK		->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
		m_head_FABRIK		->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
// 		m_leftFoot_FABRIK	->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
// 		m_rightFoot_FABRIK	->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
//		m_spine1_CCD		->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
// 		m_leftArm_FABRIK	->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
// 		m_rightArm_FABRIK	->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
//		m_tail_CCD			->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
		m_neck_FABRIK		->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
		m_head_FABRIK		->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
	}
	if ( g_debugAngles_F5 )
	{
//		m_leftFoot_2BIK		->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
//		m_rightFoot_FABRIK	->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
//		m_spine_CCD			->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
//		m_leftArm_FABRIK	->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
//		m_rightArm_FABRIK	->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
//		m_tail_CCD			->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
		m_neck_FABRIK		->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
		m_head_FABRIK		->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
//		m_tentacleLeft_CCD	->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
//		m_tentacleRight_CCD	->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
	}
	if ( g_debugRenderRaycast_F2 )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Ideal positions
		//----------------------------------------------------------------------------------------------------------------------
//		AddVertsForSphere3D( verts, m_leftFoot_2BIK		->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
//		AddVertsForSphere3D( verts, m_rightFoot_2BIK	->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
// 		AddVertsForSphere3D( verts, m_spine1_CCD		->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
//		AddVertsForSphere3D( verts, m_leftArm_2BIK		->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED );
//		AddVertsForSphere3D( verts, m_rightArm_2BIK		->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
// 		AddVertsForSphere3D( verts, m_tail_CCD			->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
 		AddVertsForSphere3D( verts, m_neck_FABRIK		->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
 		AddVertsForSphere3D( verts, m_head_FABRIK		->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
// 		AddVertsForSphere3D( verts, m_tentacleLeft_CCD	->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
// 		AddVertsForSphere3D( verts, m_tentacleRight_CCD	->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED );

		// Debug render raycast results BLOCKS
		DebugDrawRaycasts( verts );

		//----------------------------------------------------------------------------------------------------------------------
		// Max Dist from ideal Pos
		//----------------------------------------------------------------------------------------------------------------------
		AddVertsForDisc3D_XY( verts, m_leftFoot_2BIK ->m_idealStepPos_WS + Vec3( 0.0f, 0.0f, 0.01f ), m_maxDistFromIdealPosXY, Rgba8( 100, 0, 100 ) );
		AddVertsForDisc3D_XY( verts, m_rightFoot_2BIK->m_idealStepPos_WS + Vec3( 0.0f, 0.0f, 0.01f ), m_maxDistFromIdealPosXY, Rgba8( 100, 0, 100 ) );
		AddVertsForDisc3D_XY( verts, m_leftArm_2BIK	 ->m_idealStepPos_WS + Vec3( 0.0f, 0.0f, 0.01f ), m_maxDistFromIdealPosXY, Rgba8( 100, 0, 100 ) );
		AddVertsForDisc3D_XY( verts, m_rightArm_2BIK ->m_idealStepPos_WS + Vec3( 0.0f, 0.0f, 0.01f ), m_maxDistFromIdealPosXY, Rgba8( 100, 0, 100 ) );

		//----------------------------------------------------------------------------------------------------------------------
		// Max Min dist for "comeCloser" and "moveAway"
		//----------------------------------------------------------------------------------------------------------------------
		AddVertsForDisc3D_XY( verts, m_modelRoot->m_jointPos_MS, m_maxDist_comeCloser, Rgba8( 0, 0, 100, 35 ) );
		AddVertsForDisc3D_XY( verts, m_modelRoot->m_jointPos_MS + Vec3( 0.0f, 0.0f, 0.01f ), m_minDist_moveAway, Rgba8( 100, 0, 0, 35 ) );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Pole vector and rotation axis debug rendering
	//----------------------------------------------------------------------------------------------------------------------
// 	m_leftArm_2BIK		->DebugDrawPoleVectorInfo( verts, false );
// 	m_rightFoot_FABRIK	->DebugDrawPoleVectorInfo( verts, false );
// 	m_leftArm_FABRIK	->DebugDrawPoleVectorInfo( verts, false );
// 	m_rightArm_FABRIK	->DebugDrawPoleVectorInfo( verts, false );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug draw average feet position to clamp root
	//----------------------------------------------------------------------------------------------------------------------
// 	AddVertsForSphere3D ( verts, m_avgFeetPos, 2.0f, 16.0f, 16.0f, Rgba8::MAGENTA );
// 	AddVertsForDisc3D_XY( verts, Vec3( m_avgFeetPos.x, m_avgFeetPos.y, -8.0f ), m_clampedDistModelRootToAvgFeetPos );
// 	AddVertsForDisc3D_XY( verts, Vec3( m_avgFeetPos.x, m_avgFeetPos.y, -8.1f ), m_maxDistToClampRootFromAvgFeetPos, Rgba8::DARKER_GRAY );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug draw render Gait anchor states
	//----------------------------------------------------------------------------------------------------------------------
// 	float radius = 3.0f;
// 	m_leftArm_2BIK  ->DebugDrawAnchorStates( verts, m_leftArm_2BIK  ->m_lerpEulerToGoal, radius );
// 	m_rightArm_2BIK ->DebugDrawAnchorStates( verts, m_rightArm_2BIK ->m_lerpEulerToGoal, radius );
// 	m_leftFoot_2BIK ->DebugDrawAnchorStates( verts, m_leftFoot_2BIK ->m_lerpEulerToGoal, radius );
// 	m_rightFoot_2BIK->DebugDrawAnchorStates( verts, m_rightFoot_2BIK->m_lerpEulerToGoal, radius );
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::InitJointMesh()
{
	Rgba8 tint_skin		= Rgba8( 225, 153, 1, 255 );
	Rgba8 tint_ears		= Rgba8::CYAN;
	Rgba8 tint_eyes		= Rgba8::DARK_RED;
	Rgba8 tint_iris		= Rgba8::DARK_RED;
	Rgba8 tint_nostrils = Rgba8::BLACK;
	Vec2  oneOne		= Vec2(1.0f, 1.0f);

	//----------------------------------------------------------------------------------------------------------------------
	// Set Joint Mesh(s)
	//----------------------------------------------------------------------------------------------------------------------
	// Face
	float radius_mouthStart			= 1.75f;
	float radius_mouthStart_thin	= 1.75f;
	float radius_mouthEnd			= 0.6f;
	float radiust_mouthEnd_thin		= 0.6f;
	float xOffset					= 5.0f;
	float yOffset					= 1.05f;
	float yOffset_extra				= 1.75f;
	float zOffset					= 2.0f;
	m_head_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.0f,  yOffset_extra,  1.0f), Vec3(xOffset, 0.0f, zOffset - 0.1f),  radius_mouthStart_thin, radiust_mouthEnd_thin, oneOne, oneOne, tint_skin );	// Bottom Left  to mouthTip
	m_head_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.0f, -yOffset_extra,  1.0f), Vec3(xOffset, 0.0f, zOffset - 0.1f),  radius_mouthStart_thin, radiust_mouthEnd_thin, oneOne, oneOne, tint_skin );	// Bottom Right to mouthTip
	m_head_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.0f,  yOffset_extra,  1.5f), Vec3(xOffset, 0.0f, zOffset - 0.0f),  radius_mouthStart_thin, radiust_mouthEnd_thin, oneOne, oneOne, tint_skin );	// Lower Left  to mouthTip
	m_head_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.0f, -yOffset_extra,  1.5f), Vec3(xOffset, 0.0f, zOffset - 0.0f),  radius_mouthStart_thin, radiust_mouthEnd_thin, oneOne, oneOne, tint_skin );	// Lower Right to mouthTip
	m_head_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.0f,  yOffset,		 2.5f), Vec3(xOffset, 0.0f, zOffset + 0.05f), radius_mouthStart,	  radius_mouthEnd	   , oneOne, oneOne, tint_skin );	// Forehead_L to mouthTip
	m_head_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.0f, -yOffset,		 2.5f), Vec3(xOffset, 0.0f, zOffset + 0.05f), radius_mouthStart,	  radius_mouthEnd	   , oneOne, oneOne, tint_skin );	// Forehead_R to mouthTip
	m_head_FABRIK->m_firstJoint->InitMesh_Sphere( Vec3(xOffset, 0.0f, zOffset), radius_mouthEnd   + 0.1f , tint_skin );	// Nose tip
	m_head_FABRIK->m_firstJoint->InitMesh_Sphere( Vec3( 0.0f,  0.0f, 1.0f),		radius_mouthStart + 0.05f, tint_skin );	// Lower jaw?
	m_head_FABRIK->m_firstJoint->InitMesh_Sphere( Vec3(-1.0f,  0.0f, 2.0f),		radius_mouthStart + 0.25f, tint_skin );	// Skull (head top)
	m_head_FABRIK->m_firstJoint->InitMesh_Sphere( Vec3(0.0f,  2.15f, 0.5f),		radius_mouthStart - 0.5f , tint_skin );	// Left jowl
	m_head_FABRIK->m_firstJoint->InitMesh_Sphere( Vec3(0.0f, -2.15f, 0.5f),		radius_mouthStart - 0.5f , tint_skin );	// Right jowl
//	m_head_FABRIK->m_firstJoint->InitMesh_Sphere( Vec3(xOffset - 0.5f, -0.35f, zOffset + 0.75f), radiust_nostril, tint_nostrils );		// Nostril right
//	m_head_FABRIK->m_firstJoint->InitMesh_Sphere( Vec3(xOffset - 0.5f,  0.35f, zOffset + 0.75f), radiust_nostril, tint_nostrils );		// Nostril left



	// Eyeballs
	float eyeballRadius = 1.25f;
	float irisRadius	= 0.95f;
	m_head_FABRIK->m_firstJoint->InitMesh_Sphere( Vec3(0.0f,  2.25f, 2.5f), eyeballRadius, tint_eyes );		// Left  eyeball
	m_head_FABRIK->m_firstJoint->InitMesh_Sphere( Vec3(0.0f, -2.25f, 2.5f), eyeballRadius, tint_eyes );		// Right eyeball
	m_head_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.0f,  2.25f, 2.75f), Vec3(0.2f,  2.4f, 2.75f), irisRadius, irisRadius, Vec2(1.0f, 0.25f), Vec2(1.0f, 0.25f), tint_iris );		// Left  iris
	m_head_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.0f, -2.25f, 2.75f), Vec3(0.2f, -2.4f, 2.75f), irisRadius, irisRadius, Vec2(1.0f, 0.25f), Vec2(1.0f, 0.25f), tint_iris );		// Right iris
	// Eye sockets
//	m_head_FABRIK->m_firstJoint->InitMesh_CylinderTapered( Vec3(-2.0f,  1.25f, 2.0f), Vec3(0.5f,  2.5f, 2.5f), 1.0f, 1.25f );	// Left  eye socket
//	m_head_FABRIK->m_firstJoint->InitMesh_CylinderTapered( Vec3(-2.0f, -1.25f, 2.0f), Vec3(0.5f, -2.5f, 2.5f), 1.0f, 1.25f );	// Right eye socket
	// Connection to neck
	yOffset				= 4.5f;
	zOffset				= 2.5f;
	radius_mouthStart	= 1.75f;
	radius_mouthEnd		= 1.0f;
	// Ears
 	m_head_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3(-4.0f,  yOffset, 2.5f), Vec3(0.0f,  2.0f, zOffset), radius_mouthStart, radius_mouthEnd, oneOne, oneOne, tint_skin );		// Left  Ears Long
 	m_head_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3(-4.0f, -yOffset, 2.5f), Vec3(0.0f, -2.0f, zOffset), radius_mouthStart, radius_mouthEnd, oneOne, oneOne, tint_skin );		// Right Ears Long
	yOffset				= 2.0f;
	radius_mouthEnd		= 1.0f;
//	m_head_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3(-1.0f,  yOffset, 2.5f), Vec3(0.0f,  2.0f, zOffset), radius_mouthStart, radius_mouthEnd, oneOne, oneOne, tint_skin );		// Left  Ears Short
//	m_head_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3(-1.0f, -yOffset, 2.5f), Vec3(0.0f, -2.0f, zOffset), radius_mouthStart, radius_mouthEnd, oneOne, oneOne, tint_skin );		// Right Ears Short
	// Neck to head
	m_neck_FABRIK->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_neckLength, 0.0f, 2.0f), 2.9f, 1.5f, oneOne, oneOne, tint_skin );
	m_neck_FABRIK->m_firstJoint->InitMesh_Sphere( Vec3::ZERO, 2.9f, tint_skin );


	//----------------------------------------------------------------------------------------------------------------------
	// Spine mesh
	//----------------------------------------------------------------------------------------------------------------------
	float verticalSquish    = 0.85f;  
	float horizontalSquish  = 1.0f;	  
	float spineXOffset		= 0.0f;
	float spineScalar		= 0.0f;
	m_spine_CCD->m_firstJoint->  InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_spineLength + spineXOffset, 0.0f, 0.0f), 4.2f,  4.0f,  Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ), tint_skin );		// Spine 0 (Base/Butt)
	m_spine_CCD->m_jointList[1]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_spineLength + spineXOffset, 0.0f, 0.0f), 4.0f,  5.0f,  Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ), tint_skin );		// Spine 1
	m_spine_CCD->m_jointList[2]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_spineLength + spineXOffset, 0.0f, 0.0f), 5.0f,  4.75f, Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ), tint_skin );		// Spine 2
	m_spine_CCD->m_jointList[3]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_spineLength + spineXOffset, 0.0f, 0.0f), 4.75f, 4.2f,  Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ), tint_skin );		// Spine 3
	m_spine_CCD->m_jointList[4]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_spineLength + spineXOffset, 0.0f, 0.0f), 4.2f,  3.75f, Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ), tint_skin );		// Spine 4
	m_spine_CCD->m_jointList[5]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_spineLength + spineXOffset, 0.0f, 0.0f), 3.75f, 3.0f,  Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ), tint_skin );		// Spine 5
	m_spine_CCD->m_jointList[6]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(		   0.5f + spineXOffset, 0.0f, 0.0f), 3.0f,  2.5f,  Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ), tint_skin );		// Spine 6

	// old spine mesh version with segmented meshes
//  m_spine_CCD->m_firstJoint->  InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_spineLength + spineXOffset, 0.0f, 0.0f), 3.0f,  4.0f,  Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ) );		// Spine 0 (Base/Butt)
// 	m_spine_CCD->m_jointList[1]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_spineLength + spineXOffset, 0.0f, 0.0f), 4.0f,  5.0f,  Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ) );		// Spine 1
//  m_spine_CCD->m_jointList[2]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_spineLength + spineXOffset, 0.0f, 0.0f), 5.0f,  4.75f, Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ) );		// Spine 2
//  m_spine_CCD->m_jointList[3]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_spineLength + spineXOffset, 0.0f, 0.0f), 4.75f, 4.2f,  Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ) );		// Spine 3
//  m_spine_CCD->m_jointList[4]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_spineLength + spineXOffset, 0.0f, 0.0f), 4.2f,  3.75f, Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ) );		// Spine 4
//  m_spine_CCD->m_jointList[5]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_spineLength + spineXOffset, 0.0f, 0.0f), 3.75f, 3.0f,  Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ) );		// Spine 5
//  m_spine_CCD->m_jointList[6]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(		   0.5f + spineXOffset, 0.0f, 0.0f), 3.0f,  2.5f,  Vec2(verticalSquish, horizontalSquish), Vec2(verticalSquish + spineScalar, horizontalSquish + spineScalar ) );		// Spine 6

	//----------------------------------------------------------------------------------------------------------------------
	// Tail mesh
	//----------------------------------------------------------------------------------------------------------------------
	m_tail_CCD->m_firstJoint->  InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 3.0f,  3.2f , oneOne, oneOne, tint_skin );		// Tail base 0
	m_tail_CCD->m_jointList[1]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 3.2f,  3.0f , oneOne, oneOne, tint_skin );		// Tail part 1
	m_tail_CCD->m_jointList[2]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 3.0f,  2.5f , oneOne, oneOne, tint_skin );		// Tail part 2
	m_tail_CCD->m_jointList[3]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 2.5f,  2.0f , oneOne, oneOne, tint_skin );		// Tail part 3
	m_tail_CCD->m_jointList[4]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 2.0f,  1.75f, oneOne, oneOne, tint_skin );		// Tail part 4
	m_tail_CCD->m_jointList[5]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 1.75f, 1.15f, oneOne, oneOne, tint_skin );		// Tail part 5
	m_tail_CCD->m_jointList[6]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 1.15f, 0.75f, oneOne, oneOne, tint_skin );		// Tail part 6
	m_tail_CCD->m_jointList[7]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 0.75f, 0.3f , oneOne, oneOne, tint_skin );		// Tail part 6
	m_tail_CCD->m_finalJoint->  InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 0.3f,  0.0f , oneOne, oneOne, tint_skin );		// Tail tip  7
	// Hide tail gaps using spheres
//	m_tail_CCD->m_firstJoint->  InitMesh_Sphere( Vec3::ZERO, 3.0f  );		// Tail base 0
//	m_tail_CCD->m_jointList[1]->InitMesh_Sphere( Vec3::ZERO, 3.2f  );		// Tail part 1
//	m_tail_CCD->m_jointList[2]->InitMesh_Sphere( Vec3::ZERO, 3.0f  );		// Tail part 2
//	m_tail_CCD->m_jointList[3]->InitMesh_Sphere( Vec3::ZERO, 2.5f  );		// Tail part 3
//	m_tail_CCD->m_jointList[4]->InitMesh_Sphere( Vec3::ZERO, 2.0f );		// Tail part 4
//	m_tail_CCD->m_jointList[5]->InitMesh_Sphere( Vec3::ZERO, 1.75f );		// Tail part 5
//	m_tail_CCD->m_jointList[6]->InitMesh_Sphere( Vec3::ZERO, 1.15f );		// Tail part 6

	//----------------------------------------------------------------------------------------------------------------------
	// Cover gaps between spine and legs
	//----------------------------------------------------------------------------------------------------------------------
	// Hip to legs
	m_spine_CCD->m_firstJoint->		InitMesh_CylinderTapered( Vec3::ZERO, Vec3(0.0f,  m_spineLength, 0.0f), 3.25f, 2.25f, tint_skin );		// Spine to leftFoot (render hip)
	m_spine_CCD->m_firstJoint->		InitMesh_CylinderTapered( Vec3::ZERO, Vec3(0.0f, -m_spineLength, 0.0f), 3.25f, 2.25f, tint_skin );		// Spine to rightFoot(render hip)
	// Shoulder to arms
	m_spine_CCD->m_finalJoint->		InitMesh_CylinderTapered( Vec3::ZERO, Vec3(0.0f,  m_spineLength, 0.0f), 3.25f, 2.25f, tint_skin );		// Spine to leftArm  (render shoulders)
	m_spine_CCD->m_finalJoint->		InitMesh_CylinderTapered( Vec3::ZERO, Vec3(0.0f, -m_spineLength, 0.0f), 3.25f, 2.25f, tint_skin );		// Spine to rightArm (render shoulders)


	//----------------------------------------------------------------------------------------------------------------------
	// Init Legs/Arms mesh
	//----------------------------------------------------------------------------------------------------------------------
	float radius_bicep		= 2.25f;
	float increment_bicep	= 0.0f;
	float radius_foreArm 	= 1.6f;
	float decrement_foreArm = 0.95f;
	// Left leg
	m_leftFoot_2BIK->m_firstJoint->	  InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_legLength, 0.0f, 0.0f), radius_bicep, radius_foreArm + increment_bicep,	 oneOne, oneOne, tint_skin );		// Hip to knee
	m_leftFoot_2BIK->m_jointList[1]-> InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_legLength, 0.0f, 0.0f), radius_foreArm, radius_foreArm - decrement_foreArm, oneOne, oneOne, tint_skin );	// Knee to Ankle
	// Right leg
	m_rightFoot_2BIK->m_firstJoint->  InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_legLength, 0.0f, 0.0f), radius_bicep, radius_foreArm + increment_bicep,	 oneOne, oneOne, tint_skin );		// Hip to knee
	m_rightFoot_2BIK->m_jointList[1]->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_legLength, 0.0f, 0.0f), radius_foreArm, radius_foreArm - decrement_foreArm, oneOne, oneOne, tint_skin );	// Knee to Ankle
	// Left arm
	m_leftArm_2BIK->m_firstJoint->	  InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_legLength, 0.0f, 0.0f), radius_bicep, radius_foreArm + increment_bicep,	 oneOne, oneOne, tint_skin );		// Hip to knee
	m_leftArm_2BIK->m_jointList[1]->  InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_legLength, 0.0f, 0.0f), radius_foreArm, radius_foreArm - decrement_foreArm, oneOne, oneOne, tint_skin );	// Knee to Ankle
	// Right arm
	m_rightArm_2BIK->m_firstJoint->	  InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_legLength, 0.0f, 0.0f), radius_bicep, radius_foreArm + increment_bicep,	 oneOne, oneOne, tint_skin );		// Hip to knee
	m_rightArm_2BIK->m_jointList[1]-> InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(m_legLength, 0.0f, 0.0f), radius_foreArm, radius_foreArm - decrement_foreArm, oneOne, oneOne, tint_skin );	// Knee to Ankle

	//----------------------------------------------------------------------------------------------------------------------
	// Init palm mesh(s) hands/feet/fingers
	//----------------------------------------------------------------------------------------------------------------------
	float palmThickness_start = 0.75f;
	float palmThickness_end   = 0.1f;
	// MANIPULATING ARM/LEG CHAINS, end joint
	// Left Foot Toes
	m_leftFoot_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.25f, 0.0f, 0.0f), Vec3(3.0f,  0.0f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Middle finger
	m_leftFoot_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.25f, 0.0f, 0.0f), Vec3(3.0f, -2.0f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Index  finger (Mid Right)
	m_leftFoot_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.25f, 0.0f, 0.0f), Vec3(3.0f,  2.0f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Ring   finger (Mid Left)
	m_leftFoot_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.25f, 0.0f, 0.0f), Vec3(0.5f, -2.5f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Index  finger (Far Right)
	m_leftFoot_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.25f, 0.0f, 0.0f), Vec3(0.5f,  2.5f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Pinky  finger (Far Left)
	// left Foot tips
//	m_leftFoot_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(3.0f,  0.0f, 0.0f), palmThickness_end );		// Middle finger
//	m_leftFoot_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(3.0f, -2.0f, 0.0f), palmThickness_end );		// Index  finger (Mid Right)
//	m_leftFoot_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(3.0f,  2.0f, 0.0f), palmThickness_end );		// Ring   finger (Mid Left)
//	m_leftFoot_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(0.5f, -2.5f, 0.0f), palmThickness_end );		// Index  finger (Far Right)
//	m_leftFoot_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(0.5f,  2.5f, 0.0f), palmThickness_end );		// Pinky  finger (Far Left)
	// Right Foot Toes
	m_rightFoot_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.25f, 0.0f, 0.0f), Vec3(3.0f,  0.0f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Middle finger
	m_rightFoot_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.25f, 0.0f, 0.0f), Vec3(3.0f, -2.0f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Index  finger (Mid Right)
	m_rightFoot_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.25f, 0.0f, 0.0f), Vec3(3.0f,  2.0f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Ring   finger (Mid Left)
	m_rightFoot_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.25f, 0.0f, 0.0f), Vec3(0.5f, -2.5f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Index  finger (Far Right)
	m_rightFoot_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3(0.25f, 0.0f, 0.0f), Vec3(0.5f,  2.5f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Pinky  finger (Far Left)
	// Right Foot tips
//	m_rightFoot_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(3.0f,  0.0f, 0.0f), palmThickness_end );		// Middle finger
//	m_rightFoot_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(3.0f, -2.0f, 0.0f), palmThickness_end );		// Index  finger (Mid Right)
//	m_rightFoot_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(3.0f,  2.0f, 0.0f), palmThickness_end );		// Ring   finger (Mid Left)
//	m_rightFoot_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(0.5f, -2.5f, 0.0f), palmThickness_end );		// Index  finger (Far Right)
//	m_rightFoot_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(0.5f,  2.5f, 0.0f), palmThickness_end );		// Pinky  finger (Far Left)
	// Left Arm Fingers
	m_leftArm_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(3.5f,  0.0f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Middle finger
	m_leftArm_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(3.0f, -2.0f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Index  finger (Mid Right)
	m_leftArm_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(3.0f,  2.0f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Ring   finger (Mid Left)
	m_leftArm_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(0.5f, -2.5f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Index  finger (Far Right)
	m_leftArm_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(0.5f,  2.5f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Pinky  finger (Far Left)
	// Left Arm tips
//	m_leftArm_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(3.5f,  0.0f, 0.0f), palmThickness_end );		// Middle finger
//	m_leftArm_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(3.0f, -2.0f, 0.0f), palmThickness_end );		// Index  finger (Mid Right)
//	m_leftArm_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(3.0f,  2.0f, 0.0f), palmThickness_end );		// Ring   finger (Mid Left)
//	m_leftArm_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(0.5f, -2.5f, 0.0f), palmThickness_end );		// Index  finger (Far Right)
//	m_leftArm_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(0.5f,  2.5f, 0.0f), palmThickness_end );		// Pinky  finger (Far Left)
	// Right Arm Fingers
	m_rightArm_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(3.5f,  0.0f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Middle finger
	m_rightArm_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(3.0f, -2.0f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Index  finger (Mid Right)
	m_rightArm_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(3.0f,  2.0f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Ring   finger (Mid Left)
	m_rightArm_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(0.5f, -2.5f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Index  finger (Far Right)
	m_rightArm_2BIK->m_finalJoint->InitMesh_CapsuleTaperedEllipse( Vec3::ZERO, Vec3(0.5f,  2.5f, 0.0f), palmThickness_start, palmThickness_end, oneOne, oneOne, tint_skin );		// Pinky  finger (Far Left)
	// Right Arm tips
//	m_rightArm_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(3.5f,  0.0f, 0.0f), palmThickness_end );		// Middle finger
//	m_rightArm_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(3.0f, -2.0f, 0.0f), palmThickness_end );		// Index  finger (Mid Right)
//	m_rightArm_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(3.0f,  2.0f, 0.0f), palmThickness_end );		// Ring   finger (Mid Left)
//	m_rightArm_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(0.5f, -2.5f, 0.0f), palmThickness_end );		// Index  finger (Far Right)
//	m_rightArm_2BIK->m_finalJoint->InitMesh_Sphere( Vec3(0.5f,  2.5f, 0.0f), palmThickness_end );		// Pinky  finger (Far Left)
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::InitLimbs()
{
	// Create creature LeftFoot
	m_leftFoot_2BIK     = CreateChildChain( "m_leftFoot_2BIK", Vec3::ZERO );
	m_leftFoot_2BIK->AddNewJointToChain( Vec3( -m_distHipToRoot, m_distPelvisToRoot, 0.0f ), EulerAngles() );
	m_leftFoot_2BIK->AddNewJointToChain( Vec3(		m_legLength,			   0.0f, 0.0f ), EulerAngles() );
	m_leftFoot_2BIK->AddNewJointToChain( Vec3(		m_legLength,			   0.0f, 0.0f ), EulerAngles() );
	// Create creature RightFoot
	m_rightFoot_2BIK    = CreateChildChain( "m_rightFoot_2BIK", Vec3::ZERO );
	m_rightFoot_2BIK->AddNewJointToChain( Vec3( -m_distHipToRoot, -m_distPelvisToRoot, 0.0f ), EulerAngles() );
	m_rightFoot_2BIK->AddNewJointToChain( Vec3(		 m_legLength,				 0.0f, 0.0f ), EulerAngles() );
	m_rightFoot_2BIK->AddNewJointToChain( Vec3(		 m_legLength,				 0.0f, 0.0f ), EulerAngles() );
	// Create creature Spine
	m_spine_CCD			= CreateChildChain( "m_spine_CCD", Vec3::ZERO );
	m_spine_CCD->AddNewJointToChain( Vec3( -m_distHipToRoot, 0.0f, 0.0f ), EulerAngles() );		// 0
	m_spine_CCD->AddNewJointToChain( Vec3(	  m_spineLength, 0.0f, 0.0f ), EulerAngles() );		// 1
	m_spine_CCD->AddNewJointToChain( Vec3(	  m_spineLength, 0.0f, 0.0f ), EulerAngles() );		// 2
	m_spine_CCD->AddNewJointToChain( Vec3(	  m_spineLength, 0.0f, 0.0f ), EulerAngles() );		// 3
	m_spine_CCD->AddNewJointToChain( Vec3(	  m_spineLength, 0.0f, 0.0f ), EulerAngles() );		// 4
	m_spine_CCD->AddNewJointToChain( Vec3(	  m_spineLength, 0.0f, 0.0f ), EulerAngles() );		// 5
	m_spine_CCD->AddNewJointToChain( Vec3(	  m_spineLength, 0.0f, 0.0f ), EulerAngles() );		// 6
	// Create creature LeftArm													   
	m_leftArm_2BIK      = CreateChildChain( "m_leftArm_2BIK", Vec3::ZERO, m_spine_CCD->m_finalJoint );
	m_leftArm_2BIK->AddNewJointToChain( Vec3(		 0.0f, m_distPelvisToRoot, 0.0f ), EulerAngles() );
	m_leftArm_2BIK->AddNewJointToChain( Vec3( m_armLength,			     0.0f, 0.0f ), EulerAngles() );
	m_leftArm_2BIK->AddNewJointToChain( Vec3( m_armLength,			     0.0f, 0.0f ), EulerAngles() );
	// Create creature RightArm
	m_rightArm_2BIK     = CreateChildChain( "m_rightArm_2BIK", Vec3::ZERO, m_spine_CCD->m_finalJoint );
	m_rightArm_2BIK->AddNewJointToChain( Vec3(		  0.0f, -m_distPelvisToRoot, 0.0f ), EulerAngles() );
	m_rightArm_2BIK->AddNewJointToChain( Vec3( m_armLength,				   0.0f, 0.0f ), EulerAngles() );
	m_rightArm_2BIK->AddNewJointToChain( Vec3( m_armLength,				   0.0f, 0.0f ), EulerAngles() );
	// Create creature Neck
	m_neck_FABRIK	    = CreateChildChain( "m_neck_FABRIK", Vec3::ZERO, m_spine_CCD->m_finalJoint );
	m_neck_FABRIK->AddNewJointToChain( Vec3(  0.0f, 0.0f, 0.0f ), EulerAngles() );
	m_neck_FABRIK->AddNewJointToChain( Vec3( m_neckLength, 0.0f, 0.0f ), EulerAngles() );
	// Create creature Head
	m_head_FABRIK	    = CreateChildChain( "m_head_FABRIK", Vec3::ZERO, m_neck_FABRIK->m_finalJoint );
	m_head_FABRIK->AddNewJointToChain( Vec3(		 0.0f, 0.0f, 0.0f ), EulerAngles() );
	m_head_FABRIK->AddNewJointToChain( Vec3( m_headLength, 0.0f, 0.0f ), EulerAngles() );
	// Create creature Tail
	m_tail_CCD		    = CreateChildChain( "m_tail_CCD", Vec3::ZERO );
	m_tail_CCD->AddNewJointToChain( Vec3( -m_distHipToRoot, 0.0f, 0.0f ), EulerAngles( 90.0f, 0.0f, 0.0f ) );
	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 0
	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 1
	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 2
	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 3
	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 4
	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 5
	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 6
	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 7
	// Create creature tentacleLeft
	m_tentacleLeft_CCD  = CreateChildChain( "m_tentacleLeft_CCD", Vec3::ZERO, m_spine_CCD->m_finalJoint );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 0.0f, m_distPelvisToRoot, 0.0f ), EulerAngles( 0.0f, -90.0f, 0.0f ) );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	m_tentacleLeft_CCD->AddNewJointToChain( Vec3( 2.0f,				  0.0f, 0.0f ), EulerAngles() );
	// Create creature tentacleRight
	m_tentacleRight_CCD = CreateChildChain( "m_tentacleRight_CCD", Vec3::ZERO, m_spine_CCD->m_finalJoint );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 0.0f, -m_distPelvisToRoot, 0.0f ), EulerAngles( 0.0f, -90.0f, 0.0f ) );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );
	m_tentacleRight_CCD->AddNewJointToChain( Vec3( 2.0f,				0.0f, 0.0f ), EulerAngles() );	

	// Set render mode, euler goal NOT cur
	m_leftFoot_2BIK->	m_lerpEulerToGoal	= true;
	m_leftArm_2BIK->	m_lerpEulerToGoal	= true;
	m_rightFoot_2BIK->	m_lerpEulerToGoal	= true;
	m_rightArm_2BIK->	m_lerpEulerToGoal	= true;
	m_head_FABRIK->		m_lerpEulerToGoal	= false;
	m_neck_FABRIK->		m_lerpEulerToGoal	= false;
	m_tail_CCD->		m_lerpEulerToGoal	= false;

	// Set SolverTypes
	m_leftFoot_2BIK		->SetSolverType( CHAIN_SOLVER_2BONE  );
	m_rightFoot_2BIK	->SetSolverType( CHAIN_SOLVER_2BONE  );
	m_leftArm_2BIK		->SetSolverType( CHAIN_SOLVER_2BONE	 );
	m_rightArm_2BIK		->SetSolverType( CHAIN_SOLVER_2BONE	 );
	m_neck_FABRIK		->SetSolverType( CHAIN_SOLVER_FABRIK );
	m_head_FABRIK		->SetSolverType( CHAIN_SOLVER_FABRIK );
	m_spine_CCD			->SetSolverType( CHAIN_SOLVER_CCD	 );
	m_tail_CCD			->SetSolverType( CHAIN_SOLVER_CCD	 );
	m_tentacleLeft_CCD	->SetSolverType( CHAIN_SOLVER_CCD	 );
	m_tentacleRight_CCD	->SetSolverType( CHAIN_SOLVER_CCD	 );
	// Set Target Current positions
	m_leftFoot_2BIK		->m_target.m_currentPos_WS = Vec3(   0.0f, 0.0f, -40.0f );
	m_rightFoot_2BIK	->m_target.m_currentPos_WS = Vec3(   0.0f, 0.0f, -40.0f );
	m_spine_CCD			->m_target.m_currentPos_WS = Vec3(  40.0f, 0.0f,   0.0f );
	m_leftArm_2BIK		->m_target.m_currentPos_WS = Vec3(	 0.0f, 0.0f, -40.0f );
	m_rightArm_2BIK		->m_target.m_currentPos_WS = Vec3(	 0.0f, 0.0f, -40.0f );
	m_tail_CCD			->m_target.m_currentPos_WS = Vec3( -80.0f, 0.0f,   0.0f );
	m_neck_FABRIK		->m_target.m_currentPos_WS = Vec3(  50.0f, 0.0f,   0.0f );
	m_head_FABRIK		->m_target.m_currentPos_WS = Vec3(  50.0f, 0.0f,  50.0f );
	m_tentacleLeft_CCD	->m_target.m_currentPos_WS = Vec3(  40.0f, 0.0f,  20.0f );
	m_tentacleRight_CCD	->m_target.m_currentPos_WS = Vec3(  40.0f, 0.0f,  20.0f );
	// Set Target Goal positions
	m_leftFoot_2BIK		->m_target.m_goalPos_WS = Vec3(   0.0f, 0.0f, -40.0f );
	m_rightFoot_2BIK	->m_target.m_goalPos_WS = Vec3(   0.0f, 0.0f, -40.0f );
	m_spine_CCD			->m_target.m_goalPos_WS = Vec3(  40.0f, 0.0f,   0.0f );
	m_leftArm_2BIK		->m_target.m_goalPos_WS = Vec3(   0.0f, 0.0f, -40.0f );
	m_rightArm_2BIK		->m_target.m_goalPos_WS = Vec3(   0.0f, 0.0f, -40.0f );
	m_tail_CCD			->m_target.m_goalPos_WS = Vec3( -40.0f, 0.0f,   0.0f );
	m_neck_FABRIK		->m_target.m_goalPos_WS = Vec3(  50.0f, 0.0f,   0.0f );
	m_head_FABRIK		->m_target.m_goalPos_WS = Vec3(  50.0f, 0.0f,  50.0f );
	m_tentacleLeft_CCD	->m_target.m_goalPos_WS = Vec3(  40.0f, 0.0f,  20.0f );
	m_tentacleRight_CCD	->m_target.m_goalPos_WS = Vec3(  40.0f, 0.0f,  20.0f );
	// Set actual constraints parameters
//	SetChainConstraints( m_leftFoot_FABRIK	 );
//	SetChainConstraints( m_rightFoot_FABRIK	 );
	SetChainConstraints( m_spine_CCD		 );
//	SetChainConstraints( m_leftArm_FABRIK	 );
//	SetChainConstraints( m_rightArm_FABRIK	 );
//	SetChainConstraints( m_tail_CCD			 );
	SetChainConstraints( m_neck_FABRIK		 );
	SetChainConstraints( m_head_FABRIK		 );
	SetChainConstraints( m_tentacleLeft_CCD	 );
	SetChainConstraints( m_tentacleRight_CCD );
	// Angle constraints to prevent neck/head "bending too far"
	m_neck_FABRIK->m_finalJoint->SetConstraints_YPR( FloatRange( -0.0f,  0.0f), FloatRange( -0.0f,  0.0f), FloatRange(-0.0f, 0.0f) );		// Neck end
	m_head_FABRIK->m_firstJoint->SetConstraints_YPR( FloatRange(-45.0f, 45.0f), FloatRange(-15.0f, 15.0f), FloatRange(-0.0f, 0.0f) );		// Head base, attached to neck
	m_head_FABRIK->m_finalJoint->SetConstraints_YPR( FloatRange( -0.0f,  0.0f), FloatRange( -0.0f,  0.0f), FloatRange(-0.0f, 0.0f) );		// Head tip, nose

	// Set anchor states
 	m_leftArm_2BIK	->SetAnchor_Free();
 	m_rightArm_2BIK	->SetAnchor_Free();
	m_leftFoot_2BIK	->SetAnchor_Free();
	m_rightFoot_2BIK->SetAnchor_Free();
	// Set chain lerp speed
	m_leftFoot_2BIK		->m_turnRate = 22500.0f;
	m_rightFoot_2BIK	->m_turnRate = 22500.0f;
	m_spine_CCD			->m_turnRate = 22500.0f;
	m_leftArm_2BIK		->m_turnRate = 22500.0f;
	m_rightArm_2BIK		->m_turnRate = 22500.0f;
	m_tail_CCD			->m_turnRate = 22500.0f;
 	m_neck_FABRIK		->m_turnRate = 180.0f;		
 	m_head_FABRIK		->m_turnRate = 2762.5f;			// Turn rate: 90 degrees per second
 	m_neck_FABRIK		->m_turnRate = 180.0f;		
 	m_head_FABRIK		->m_turnRate = 360.0f;
	m_tentacleLeft_CCD	->m_turnRate = 22500.0f;
	m_tentacleRight_CCD	->m_turnRate = 22500.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Set MatrixToEuler preference
	//----------------------------------------------------------------------------------------------------------------------
	m_spine_CCD		->SetChainPreference_MatrixToEuler( YAW_PREFERRED	);
// 	m_leftFoot_2BIK	->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
// 	m_rightFoot_2BIK->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
// 	m_leftArm_2BIK	->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
// 	m_rightArm_2BIK	->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
// 	m_tail_CCD		->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
 	m_neck_FABRIK	->SetChainPreference_MatrixToEuler( YAW_PREFERRED	);
 	m_head_FABRIK	->SetChainPreference_MatrixToEuler( YAW_PREFERRED	);


	//----------------------------------------------------------------------------------------------------------------------
	// Set Joint Mesh(s)
	//----------------------------------------------------------------------------------------------------------------------
	InitJointMesh();

	UpdatePoleVector();
}


//----------------------------------------------------------------------------------------------------------------------	
void Quadruped::SetChainConstraints( IK_Chain3D* const currentChain )
{
// 	for ( int i = 0; i < currentChain->m_jointList.size(); i++ )
// 	{
// 		IK_Joint3D* currentSegment = currentChain->m_jointList[ i ];
// 		int remainder = i % 2;
// 		if ( remainder == 0 )
// 		{
// 			// Yaw only if even
// 			currentSegment->SetConstraints_YPR( FloatRange( -75.0f, 75.0f ), FloatRange( -0.0f, 0.0f ), FloatRange( -0.0f, 0.0f ) );
// 		}
// 		else
// 		{
// 			// Pitch only if odd
// 			currentSegment->SetConstraints_YPR( FloatRange( -0.0f, 0.0f ), FloatRange( -75.0f, 75.0f ), FloatRange( -0.0f, 0.0f ) );
// 		}
// 	}

 	for ( int i = 0; i < currentChain->m_jointList.size(); i++ )
 	{
 		IK_Joint3D* currentSegment = currentChain->m_jointList[ i ];
 		currentSegment->SetConstraints_YPR( FloatRange( -75.0f, 75.0f ), FloatRange( -75.0f, 75.0f ), FloatRange( -0.0f, 0.0f ) );

		if ( currentChain == m_leftFoot_2BIK )
		{
			currentSegment->SetConstraints_YPR( FloatRange( -35.0f, 35.0f ), FloatRange( -10.0f, 135.0f ), FloatRange( -0.0f, 0.0f ) );
		}
 	}
}


//----------------------------------------------------------------------------------------------------------------------
// Note: This function pre-updates all skeletal relevant game-logic 
// before the IK chains actually solve for targets
//----------------------------------------------------------------------------------------------------------------------
void Quadruped::PreUpdateChains( float deltaSeconds )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped Hip EndEffector ("keep spine pointing hip fwd")
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 rootFwd							= m_modelRoot->m_eulerAnglesGoal_LS.GetForwardDir_XFwd_YLeft_ZUp();
	float fwdDist							= m_spineLength * m_spine_CCD->m_jointList.size();
	m_spine_CCD->m_target.m_currentPos_WS	= m_modelRoot->m_jointPos_MS + ( rootFwd * fwdDist );


	//----------------------------------------------------------------------------------------------------------------------
	// Update pole vectors 
	//----------------------------------------------------------------------------------------------------------------------
	UpdatePoleVector();
		
	//----------------------------------------------------------------------------------------------------------------------
	// New walk logic with refactored generic gait management system
	// Update order for anchor toggle states: LeftFoot, RightFoot, leftArm, RightArm
	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	// LEFT FOOT, REFACTORED
	//----------------------------------------------------------------------------------------------------------------------
	ComputeIdealStepPosition( m_leftFoot_2BIK, m_modelRoot->m_eulerAnglesGoal_LS, m_modelRoot->m_jointPos_MS, -m_fwdNess, +m_leftNess, -m_defaultHeightZ );		// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	bool canMove					= TryFreeAnchor( m_leftFoot_2BIK, m_rightArm_2BIK );																		// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	bool shouldStep					= ShouldChainStep( m_leftFoot_2BIK, canMove, m_maxDistFromIdealPosXY, m_maxDistFromIdealPosZ );								// 3. Check if each chain should step
	TryInitBezierTimer( canMove, shouldStep, m_bezierTimer_leftFoot, m_leftFoot_2BIK, m_rightArm_2BIK );														// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	bool doesRaycastHitFloor_CurPos = DoesRayHitFloor_CurPos( m_leftFoot_2BIK, m_raycast_CurPos_LeftFoot );														// 5a. Raycast against floor (Cur Pos)
	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_leftFoot_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_LeftFoot );									// 5b. Raycast against floor (Ideal Pos)
	UpdateIdealStepPosition( doesRaycastHitFloor_CurPos, m_leftFoot_2BIK, m_rightArm_2BIK, m_bezierTimer_leftFoot );											// 5c. Update ideal pos if better than prev step
	UpdateBezierCurvePositions( m_bezier_leftFoot, m_leftFoot_2BIK, Vec3::Z_UP, m_bezierTimer_leftFoot );														// 6. Update bezier points for each chain's foot height
	UpdateBezierTimer( m_bezier_leftFoot, m_leftFoot_2BIK, m_rightArm_2BIK, m_bezierTimer_leftFoot );															// 7. Check if Bezier should be stopped


	//----------------------------------------------------------------------------------------------------------------------
	// LEFT ARM, REFACTORED
	//----------------------------------------------------------------------------------------------------------------------
	ComputeIdealStepPosition( m_leftArm_2BIK, m_modelRoot->m_eulerAnglesGoal_LS, m_modelRoot->m_jointPos_MS, +m_fwdNess, +m_leftNess, -m_defaultHeightZ );		// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	canMove							= TryFreeAnchor( m_leftArm_2BIK, m_rightFoot_2BIK );																			// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	shouldStep						= ShouldChainStep( m_leftArm_2BIK, canMove, m_maxDistFromIdealPosXY, m_maxDistFromIdealPosZ );								// 3. Check if each chain should step
	TryInitBezierTimer( canMove, shouldStep, m_bezierTimer_leftArm, m_leftArm_2BIK, m_rightFoot_2BIK );															// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	doesRaycastHitFloor_CurPos		= DoesRayHitFloor_CurPos( m_leftArm_2BIK, m_raycast_CurPos_LeftArm );														// 5a. Raycast against floor (Cur Pos)
	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_leftArm_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_LeftArm );										// 5b. Raycast against floor (Ideal Pos)
	UpdateIdealStepPosition( doesRaycastHitFloor_CurPos, m_leftArm_2BIK, m_rightFoot_2BIK, m_bezierTimer_leftArm );												// 5c. Update ideal pos if better than prev step
	UpdateBezierCurvePositions( m_bezier_leftArm, m_leftArm_2BIK, Vec3::Z_UP, m_bezierTimer_leftArm );															// 6. Update bezier points for each chain's foot height
	UpdateBezierTimer( m_bezier_leftArm, m_leftArm_2BIK, m_rightFoot_2BIK, m_bezierTimer_leftArm );																// 7. Check if Bezier should be stopped

																																								
	//----------------------------------------------------------------------------------------------------------------------
	// RIGHT FOOT, REFACTORED
	//----------------------------------------------------------------------------------------------------------------------
	ComputeIdealStepPosition( m_rightFoot_2BIK, m_modelRoot->m_eulerAnglesGoal_LS, m_modelRoot->m_jointPos_MS, -m_fwdNess, -m_leftNess, -m_defaultHeightZ );	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	canMove							= TryFreeAnchor( m_rightFoot_2BIK, m_leftArm_2BIK );																		// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	shouldStep						= ShouldChainStep( m_rightFoot_2BIK, canMove, m_maxDistFromIdealPosXY, m_maxDistFromIdealPosZ );							// 3. Check if each chain should step
	TryInitBezierTimer( canMove, shouldStep, m_bezierTimer_rightFoot, m_rightFoot_2BIK, m_leftArm_2BIK );														// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	doesRaycastHitFloor_CurPos		= DoesRayHitFloor_CurPos( m_rightFoot_2BIK, m_raycast_CurPos_RightFoot );													// 5a. Raycast against floor (Cur Pos)
	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_rightFoot_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_RightFoot );									// 5b. Raycast against floor (Ideal Pos)
	UpdateIdealStepPosition( doesRaycastHitFloor_CurPos, m_rightFoot_2BIK, m_leftArm_2BIK, m_bezierTimer_rightFoot );											// 5c. Update ideal pos if better than prev step
	UpdateBezierCurvePositions( m_bezier_rightFoot, m_rightFoot_2BIK, Vec3::Z_UP, m_bezierTimer_rightFoot );													// 6. Update bezier points for each chain's foot height
	UpdateBezierTimer( m_bezier_rightFoot, m_rightFoot_2BIK, m_leftArm_2BIK, m_bezierTimer_rightFoot );														// 7. Check if Bezier should be stopped


	//----------------------------------------------------------------------------------------------------------------------
	// RIGHT ARM, REFACTORED
	//----------------------------------------------------------------------------------------------------------------------
	ComputeIdealStepPosition( m_rightArm_2BIK, m_modelRoot->m_eulerAnglesGoal_LS, m_modelRoot->m_jointPos_MS, +m_fwdNess, -m_leftNess, -m_defaultHeightZ );		// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	canMove							= TryFreeAnchor( m_rightArm_2BIK, m_leftFoot_2BIK );																			// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	shouldStep						= ShouldChainStep( m_rightArm_2BIK, canMove, m_maxDistFromIdealPosXY, m_maxDistFromIdealPosZ );								// 3. Check if each chain should step
	TryInitBezierTimer( canMove, shouldStep, m_bezierTimer_rightArm, m_rightArm_2BIK, m_leftFoot_2BIK );															// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	doesRaycastHitFloor_CurPos		= DoesRayHitFloor_CurPos( m_rightArm_2BIK, m_raycast_CurPos_LeftArm );														// 5a. Raycast against floor (Cur Pos)
	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_rightArm_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_RightArm );									// 5b. Raycast against floor (Ideal Pos)
	UpdateIdealStepPosition( doesRaycastHitFloor_CurPos, m_rightArm_2BIK, m_leftFoot_2BIK, m_bezierTimer_rightArm );												// 5c. Update ideal pos if better than prev step
	UpdateBezierCurvePositions( m_bezier_rightArm, m_rightArm_2BIK, Vec3::Z_UP, m_bezierTimer_rightArm );														// 6. Update bezier points for each chain's foot height
	UpdateBezierTimer( m_bezier_rightArm, m_rightArm_2BIK, m_leftFoot_2BIK, m_bezierTimer_rightArm );															// 7. Check if Bezier should be stopped


// 	//----------------------------------------------------------------------------------------------------------------------
// 	// Old walk logic
// 	// Update order for anchor toggle states: LeftFoot, RightFoot, leftArm, RightArm
// 	//----------------------------------------------------------------------------------------------------------------------
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// LEFT FOOT
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
// 	Vec3 fwd, left, up;		
// 	m_modelRoot->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( fwd, left, up );
// 	Vec3 fwdOffset							= ( fwd  * m_distHipToRoot				);
// 	Vec3 leftOffset							= ( left * (m_distPelvisToRoot * 2.0f)  );
// 	Vec3 upOffset							= ( up	 * m_defaultHeightZ				);
// 	m_leftFoot_2BIK->m_idealStepPos_WS	= m_modelRoot->m_jointPos_MS - fwdOffset + leftOffset - upOffset; 
// 	// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
// 	bool canMove = m_leftFoot_2BIK->CanMove();
// 	if ( !canMove )
// 	{
// 		// 2b. Try freeing anchor if CAN'T move
// //		canMove = m_leftFoot_2BIK->TryFreeAnchor( m_rightArm_2BIK );
// 		canMove = m_leftFoot_2BIK->TryFreeAnchor( m_rightFoot_2BIK, m_leftArm_2BIK );
// 	}
// 	// 3. Check if each chain should step
// 	bool shouldStep = false;
// 	if ( canMove )
// 	{
// 		Mat44 localToWorldMatrix	= m_leftFoot_2BIK->m_finalJoint->GetMatrix_LocalToWorld();
// 		Vec3  leftFootEE_WS			= localToWorldMatrix.GetTranslation3D();
// 		shouldStep					= IsIdealPosTooFar( m_leftFoot_2BIK->m_idealStepPos_WS, leftFootEE_WS, m_maxDistFromIdealPosXY, m_maxDistFromIdealPosZ );
// 	}
// 	// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
// 	if ( canMove && shouldStep )
// 	{
// 		if ( m_bezierTimer_leftFoot.IsStopped() )
// 		{
// 			// 4a. Start Bezier timer
// 			m_bezierTimer_leftFoot.Start();
// 			// 4b. Toggle anchor states
// 			m_leftFoot_2BIK->SetAnchor_Moving();
// 			m_rightArm_2BIK->SetAnchor_Locked();
// 		}
// 	}
// 	// 5a. Raycast against floor (Cur Pos)
// 	bool doesRaycastHitFloor = DoesRaycastHitFloor( m_leftFoot_2BIK->m_target.m_currentPos_WS, m_raycast_CurPos_LeftFoot );
// 	if ( doesRaycastHitFloor )
// 	{
// 		// Update curPos 
// 		m_leftFoot_2BIK->m_target.m_currentPos_WS = m_raycast_CurPos_LeftFoot.m_raycastResult.m_impactPos;
// 	}
// 	// 5b. Raycast against floor (Ideal Pos)
// 	Vec3 prevIdealPosition	= m_leftFoot_2BIK->m_idealStepPos_WS;
// 	doesRaycastHitFloor		= DoesRaycastHitFloor( m_leftFoot_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_LeftFoot );
// 	if ( !doesRaycastHitFloor )
// 	{
// 		// Keep previous ideal position?
// 		m_leftFoot_2BIK->m_idealStepPos_WS = m_leftFoot_2BIK->m_prevIdealPos_WS;
// 		// If there was no new position found, then keep the old position, stop the timer, and free the leg anchors.
// 		// 5c. Start Bezier timer
// 		m_bezierTimer_leftFoot.Stop();
// 		// 5d. Toggle anchor states
// 		m_leftFoot_2BIK->SetAnchor_Free();
// 		m_rightArm_2BIK->SetAnchor_Free();
// 	}
// 	else
// 	{
// 		m_leftFoot_2BIK->m_prevIdealPos_WS = m_leftFoot_2BIK->m_idealStepPos_WS;
// 	}
// 	// 5c. Update idealPos.Z based on rayImpactPos.z
//  	m_leftFoot_2BIK->m_target.m_goalPos_WS = m_leftFoot_2BIK->m_idealStepPos_WS;
// 	// 6. Update bezier points for each chain's foot height
// 	UpdateBezierCurvePositions( m_bezier_leftFoot, m_leftFoot_2BIK, Vec3::Z_UP, m_bezierTimer_leftFoot );
// 	// 7. Check if Bezier should be stopped
// 	UpdateBezierTimer( m_bezier_leftFoot, m_leftFoot_2BIK, m_rightArm_2BIK, m_bezierTimer_leftFoot );
// 
// 
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// LEFT ARM
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
// 	fwdOffset								= ( fwd  * m_distHipToRoot				);
// 	leftOffset								= ( left * (m_distPelvisToRoot * 2.0f)  );
// 	upOffset								= ( up	 * m_defaultHeightZ				);
// 	m_leftArm_2BIK->m_idealStepPos_WS		= m_modelRoot->m_jointPos_MS + fwdOffset + leftOffset - upOffset; 
// 	// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
// 	canMove = m_leftArm_2BIK->CanMove();
// 	if ( !canMove )
// 	{
// 		// 2b. Try freeing anchor if CAN'T move
// //		canMove = m_leftArm_2BIK->TryFreeAnchor( m_rightFoot_2BIK );
// 		canMove = m_leftArm_2BIK->TryFreeAnchor( m_leftFoot_2BIK, m_rightArm_2BIK );
// 	}
// 	// 3. Check if each chain should step
// 	shouldStep = false;
// 	if ( canMove )
// 	{
// 		Mat44 localToWorldMatrix	= m_leftArm_2BIK->m_finalJoint->GetMatrix_LocalToWorld();
// 		Vec3  leftArmEE_WS			= localToWorldMatrix.GetTranslation3D();
// 		shouldStep					= IsIdealPosTooFar( m_leftArm_2BIK->m_idealStepPos_WS, leftArmEE_WS, m_maxDistFromIdealPosXY, m_maxDistFromIdealPosZ );
// 	}
// 	// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
// 	if ( canMove && shouldStep )
// 	{
// 		if ( m_bezierTimer_leftArm.IsStopped() )
// 		{
// 			// 4a. Start Bezier timer
// 			m_bezierTimer_leftArm.Start();
// 			// 4b. Toggle anchor states
// 			m_leftArm_2BIK  ->SetAnchor_Moving();
// 			m_rightFoot_2BIK->SetAnchor_Locked();
// 		}
// 	}
// 	// 5a. Raycast against floor (Cur Pos)
// 	doesRaycastHitFloor = DoesRaycastHitFloor( m_leftArm_2BIK->m_target.m_currentPos_WS, m_raycast_CurPos_LeftArm );
// 	if ( doesRaycastHitFloor )
// 	{
// 		// Update curPos 
// 		m_leftArm_2BIK->m_target.m_currentPos_WS = m_raycast_CurPos_LeftArm.m_raycastResult.m_impactPos;
// 	}
// 	// 5b. Raycast against floor (Ideal Pos)
// 	prevIdealPosition	 = m_leftArm_2BIK->m_idealStepPos_WS;
// 	doesRaycastHitFloor	 = DoesRaycastHitFloor( m_leftArm_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_LeftArm );
// 	if ( !doesRaycastHitFloor )
// 	{
// 		// 5c. Keep previous ideal position?
// 		m_leftArm_2BIK->m_idealStepPos_WS = m_leftArm_2BIK->m_prevIdealPos_WS;
// 		// 5ci.Try get nearestPointOnAABB2
// 		
// 
// 		// If there was no new position found, then keep the old position, stop the timer, and free the leg anchors.
// 		// 5d. Start Bezier timer
// 		m_bezierTimer_leftArm.Stop();
// 		// 5e. Toggle anchor states
// 		m_leftArm_2BIK  ->SetAnchor_Free();
// 		m_rightFoot_2BIK->SetAnchor_Free();
// 	}
// 	else
// 	{
// 		m_leftArm_2BIK->m_prevIdealPos_WS = m_leftArm_2BIK->m_idealStepPos_WS;
// 	}
// 	// 5c. Update idealPos.Z based on rayImpactPos.z
// 	m_leftArm_2BIK->m_target.m_goalPos_WS = m_leftArm_2BIK->m_idealStepPos_WS;
// 	// 6. Update bezier points for each chain's foot height
// 	UpdateBezierCurvePositions( m_bezier_leftArm, m_leftArm_2BIK, Vec3::Z_UP, m_bezierTimer_leftArm );
// 	// 7. Check if Bezier should be stopped
// 	UpdateBezierTimer( m_bezier_leftArm, m_leftArm_2BIK, m_rightFoot_2BIK, m_bezierTimer_leftArm );
// 
// 
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// RIGHT FOOT
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
// 	fwdOffset								= ( fwd  * m_distHipToRoot				);
// 	leftOffset								= ( left * (m_distPelvisToRoot * 2.0f)  );
// 	upOffset								= ( up	 * m_defaultHeightZ				);
// 	m_rightFoot_2BIK->m_idealStepPos_WS	= m_modelRoot->m_jointPos_MS - fwdOffset - leftOffset - upOffset; 
// 	// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
// 	canMove = m_rightFoot_2BIK->CanMove();
// 	if ( !canMove )
// 	{
// 		// 2b. Try freeing anchor if CAN'T move
// //		canMove = m_rightFoot_2BIK->TryFreeAnchor( m_leftArm_2BIK );
// 		canMove = m_rightFoot_2BIK->TryFreeAnchor( m_rightArm_2BIK, m_leftFoot_2BIK );
// 	}
// 	// 3. Check if each chain should step
// 	shouldStep = false;
// 	if ( canMove )
// 	{
// 		Mat44 localToWorldMatrix	= m_rightFoot_2BIK->m_finalJoint->GetMatrix_LocalToWorld();
// 		Vec3  rightFootEE_WS		= localToWorldMatrix.GetTranslation3D();
// 		shouldStep					= IsIdealPosTooFar( m_rightFoot_2BIK->m_idealStepPos_WS, rightFootEE_WS, m_maxDistFromIdealPosXY, m_maxDistFromIdealPosZ );
// 	}
// 	// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
// 	if ( canMove && shouldStep )
// 	{
// 		if ( m_bezierTimer_rightFoot.IsStopped() )
// 		{
// 			// 4a. Start Bezier timer
// 			m_bezierTimer_rightFoot.Start();
// 			// 4b. Toggle anchor states
// 			m_rightFoot_2BIK->SetAnchor_Moving();
// 			m_leftArm_2BIK  ->SetAnchor_Locked();
// 		}
// 	}
// 	// 5a. Raycast against floor (Cur Pos)
// 	doesRaycastHitFloor = DoesRaycastHitFloor( m_rightFoot_2BIK->m_target.m_currentPos_WS, m_raycast_CurPos_RightFoot );
// 	if ( doesRaycastHitFloor )
// 	{
// 		// Update curPos 
// 		m_rightFoot_2BIK->m_target.m_currentPos_WS = m_raycast_CurPos_RightFoot.m_raycastResult.m_impactPos;
// 	}
// 	// 5b. Raycast against floor (Ideal Pos)
// 	prevIdealPosition	 = m_rightFoot_2BIK->m_idealStepPos_WS;
// 	doesRaycastHitFloor	 = DoesRaycastHitFloor( m_rightFoot_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_RightFoot );
// 	if ( !doesRaycastHitFloor )
// 	{
// 		// 5a. Keep previous ideal position?
// 		m_rightFoot_2BIK->m_idealStepPos_WS = m_rightFoot_2BIK->m_prevIdealPos_WS;
// 		// If there was no new position found, then keep the old position, stop the timer, and free the leg anchors.
// 		// 5b. Start Bezier timer
// 		m_bezierTimer_rightFoot.Stop();
// 		// 5c. Toggle anchor states
// 		m_rightFoot_2BIK->SetAnchor_Free();
// 		m_leftArm_2BIK  ->SetAnchor_Free();
// 	}
// 	else
// 	{
// 		m_rightFoot_2BIK->m_prevIdealPos_WS = m_rightFoot_2BIK->m_idealStepPos_WS;
// 	}
// 	// 5c. Update idealPos.Z based on rayImpactPos.z
// 	m_rightFoot_2BIK->m_target.m_goalPos_WS = m_rightFoot_2BIK->m_idealStepPos_WS;
// 	// 6. Update bezier points for each chain's foot height
// 	UpdateBezierCurvePositions( m_bezier_rightFoot, m_rightFoot_2BIK, Vec3::Z_UP, m_bezierTimer_rightFoot );
// 	// 7. Check if Bezier should be stopped
// 	UpdateBezierTimer( m_bezier_rightFoot, m_rightFoot_2BIK, m_leftArm_2BIK, m_bezierTimer_rightFoot );
// 
// 
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// RIGHT ARM
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
// 	fwdOffset								= ( fwd  * m_distHipToRoot				);
// 	leftOffset								= ( left * (m_distPelvisToRoot * 2.0f)  );
// 	upOffset								= ( up	 * m_defaultHeightZ				);
// 	m_rightArm_2BIK->m_idealStepPos_WS	= m_modelRoot->m_jointPos_MS + fwdOffset - leftOffset - upOffset; 
// 	// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
// 	canMove = m_rightArm_2BIK->CanMove();
// 	if ( !canMove )
// 	{
// 		// 2b. Try freeing anchor if CAN'T move
// //		canMove = m_rightArm_2BIK->TryFreeAnchor( m_leftFoot_2BIK );
// 		canMove = m_rightArm_2BIK->TryFreeAnchor( m_leftArm_2BIK, m_rightFoot_2BIK );
// 	}
// 	// 3. Check if each chain should step
// 	shouldStep = false;
// 	if ( canMove )
// 	{
// 		Mat44 localToWorldMatrix	= m_rightArm_2BIK->m_finalJoint->GetMatrix_LocalToWorld();
// 		Vec3  rightArmEE_WS			= localToWorldMatrix.GetTranslation3D();
// 		shouldStep					= IsIdealPosTooFar( m_rightArm_2BIK->m_idealStepPos_WS, rightArmEE_WS, m_maxDistFromIdealPosXY, m_maxDistFromIdealPosZ );
// 	}
// 	// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
// 	if ( canMove && shouldStep )
// 	{
// 		if ( m_bezierTimer_rightArm.IsStopped() )
// 		{
// 			// 4a. Start Bezier timer
// 			m_bezierTimer_rightArm.Start();
// 			// 4b. Toggle anchor states
// 			m_rightArm_2BIK->SetAnchor_Moving();
// 			m_leftFoot_2BIK->SetAnchor_Locked();
// 		}
// 	}
// 	// 5a. Raycast against floor (Cur Pos)
// 	doesRaycastHitFloor = DoesRaycastHitFloor( m_rightArm_2BIK->m_target.m_currentPos_WS, m_raycast_CurPos_RightArm );
// 	if ( doesRaycastHitFloor )
// 	{
// 		// Update curPos 
// 		m_rightArm_2BIK->m_target.m_currentPos_WS = m_raycast_CurPos_RightArm.m_raycastResult.m_impactPos;
// 	}
// 	// 5b. Raycast against floor (Ideal Pos)
// 	prevIdealPosition	 = m_rightArm_2BIK->m_idealStepPos_WS;
// 	doesRaycastHitFloor	 = DoesRaycastHitFloor( m_rightArm_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_RightArm );
// 	if ( !doesRaycastHitFloor )
// 	{
// 		// 5a. Keep previous ideal position?
// 		m_rightArm_2BIK->m_idealStepPos_WS = m_rightArm_2BIK->m_prevIdealPos_WS;
// 		// If there was no new position found, then keep the old position, stop the timer, and free the leg anchors.
// 		// 5b. Start Bezier timer
// 		m_bezierTimer_rightArm.Stop();
// 		// 5c. Toggle anchor states
// 		m_rightArm_2BIK->SetAnchor_Free();
// 		m_leftFoot_2BIK->SetAnchor_Free();
// 	}
// 	else
// 	{
// 		m_rightArm_2BIK->m_prevIdealPos_WS = m_rightArm_2BIK->m_idealStepPos_WS;
// 	}
// 	// 5c. Update idealPos.Z based on rayImpactPos.z
// 	m_rightArm_2BIK->m_target.m_goalPos_WS = m_rightArm_2BIK->m_idealStepPos_WS;
// 	// 6. Update bezier points for each chain's foot height
// 	UpdateBezierCurvePositions( m_bezier_rightArm, m_rightArm_2BIK, Vec3::Z_UP, m_bezierTimer_rightArm );
// 	// 7. Check if Bezier should be stopped
// 	UpdateBezierTimer( m_bezier_rightArm, m_rightArm_2BIK, m_leftFoot_2BIK, m_bezierTimer_rightArm );


	//----------------------------------------------------------------------------------------------------------------------
	// Creature translation & rotation in response to the target
	//----------------------------------------------------------------------------------------------------------------------
	if ( !m_isControlled )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Translate root towards/away from target
		//----------------------------------------------------------------------------------------------------------------------
		// 1. Check distance between root and headTargetPos
		float maxMoveSpeed		= 300.0f;
		float minMoveSpeed		= 150.0f;
		float curMoveSpeed		= 0.0f;
		Vec3 dirRootToTarget	= ( m_head_FABRIK->m_target.m_goalPos_WS - m_modelRoot->m_jointPos_MS ).GetNormalized();
		Vec3 dirRootToTarget_XY	= Vec3( dirRootToTarget.x, dirRootToTarget.y, 0.0f ).GetNormalized();
		float distToTarget		= GetDistance3D( m_modelRoot->m_jointPos_MS, m_head_FABRIK->m_target.m_currentPos_WS );
		Vec3  targetVelocity	= Vec3::ZERO;
		Vec3  curVelocity	    = Vec3::ZERO;
		curMoveSpeed			= RangeMapClamped( distToTarget, m_minDist_moveAway, m_maxDist_comeCloser, maxMoveSpeed, minMoveSpeed ); 
		if ( distToTarget > m_maxDist_comeCloser )
		{
			targetVelocity = ( dirRootToTarget_XY * curMoveSpeed );
		}
		else if ( distToTarget < m_minDist_moveAway )
		{
			targetVelocity = ( -dirRootToTarget_XY * curMoveSpeed );
		}
		curVelocity					= Interpolate( curVelocity, targetVelocity, deltaSeconds * 5.0f );
		m_modelRoot->m_jointPos_MS += ( curVelocity * deltaSeconds );

		//----------------------------------------------------------------------------------------------------------------------
		// Rotate root towards target
		//----------------------------------------------------------------------------------------------------------------------
		// 1. Compute the signed angle between rootFwd and dirRootToTarget (Note: both vectors have to be projected onto the XY plane)
		rootFwd					= m_modelRoot->m_eulerAnglesGoal_LS.GetForwardDir_XFwd_YLeft_ZUp();
		Vec3 rootFwd_XY			= Vec3( rootFwd.x, rootFwd.y, 0.0f ).GetNormalized();
		dirRootToTarget_XY		= Vec3( dirRootToTarget.x, dirRootToTarget.y, 0.0f ).GetNormalized();
		float signedAngle		= GetSignedAngleDegreesBetweenVectors( rootFwd_XY, dirRootToTarget_XY, Vec3::Z_UP );
		signedAngle				= GetClamped( signedAngle, -60.0f, 60.0f );
		// 2. Rotate root by signed angle
		if ( signedAngle > m_maxAngle_turnBody )
		{
			// Rotate +degrees, CCW
			m_modelRoot->m_eulerAnglesGoal_LS.m_yawDegrees -= ( signedAngle * deltaSeconds );
		}
		else
		{
			// Rotate -degrees, CW
			m_modelRoot->m_eulerAnglesGoal_LS.m_yawDegrees += ( signedAngle * deltaSeconds );
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Lerp targetCurrent to targetGoal
	//----------------------------------------------------------------------------------------------------------------------
	float fractionTowardsEnd				= 0.05f + ( deltaSeconds );
	// Neck
	m_neck_FABRIK->m_target.m_currentPos_WS	= Interpolate( m_neck_FABRIK->m_target.m_currentPos_WS, m_neck_FABRIK->m_target.m_goalPos_WS, fractionTowardsEnd * 0.65f );
	// Head 
 	m_head_FABRIK->m_target.m_currentPos_WS	= Interpolate( m_head_FABRIK->m_target.m_currentPos_WS, m_head_FABRIK->m_target.m_goalPos_WS, fractionTowardsEnd * 0.35f ); 

 	//----------------------------------------------------------------------------------------------------------------------
 	// Update Quadruped Height
 	//----------------------------------------------------------------------------------------------------------------------
 	// Get Legs & Arms positions in WS
	Vec3  leftFootEePos_WS			=  m_leftFoot_2BIK->m_finalJoint->GetMatrix_LocalToWorld().GetTranslation3D();
	Vec3  rightFootEePos_WS			= m_rightFoot_2BIK->m_finalJoint->GetMatrix_LocalToWorld().GetTranslation3D();
	Vec3  leftArmEePos_WS			=   m_leftArm_2BIK->m_finalJoint->GetMatrix_LocalToWorld().GetTranslation3D();
	Vec3  rightArmEePos_WS			=  m_rightArm_2BIK->m_finalJoint->GetMatrix_LocalToWorld().GetTranslation3D();
 	// Compute averages of legs and arms Z height
	float legsAvgZHeight			= ( leftFootEePos_WS.z + rightFootEePos_WS.z ) * 0.5f;
 	float armsAvgZHeight			= (  leftArmEePos_WS.z +  rightArmEePos_WS.z ) * 0.5f;
	float combinedAvgZHeight		= (		legsAvgZHeight + armsAvgZHeight		 ) * 0.5f;
	// Lerp from curHeight to goalHeight
	float curCreatureHeight			= m_modelRoot->m_jointPos_MS.z;
	float goalCreatureHeight		= combinedAvgZHeight + m_defaultHeightZ;
	m_modelRoot->m_jointPos_MS.z	= Interpolate( curCreatureHeight, goalCreatureHeight, fractionTowardsEnd * 0.75f );


	//----------------------------------------------------------------------------------------------------------------------
	// Lerp from currentRootPos to goalPos (Breathing)
	//----------------------------------------------------------------------------------------------------------------------
	// Move root pos up and down approach
//  float currentTime				= float( GetCurrentTimeSeconds() );
//  float sine						= SinDegrees( currentTime * 60.0f );
//  float heightOffset				= 0.05f;
// 	goalCreatureHeight				= m_modelRoot->m_jointPos_MS.z + ( heightOffset * sine );
//  m_modelRoot->m_jointPos_MS.z	= Interpolate( curCreatureHeight, goalCreatureHeight, fractionTowardsEnd * 0.75f );
	// Pitch approach
	float currentTime				= float( GetCurrentTimeSeconds() );
	float sine						= SinDegrees( currentTime * 30.0f );
	float pitchOffset				= 5.0f;
	float curQuadrupedPitch			= m_modelRoot->m_eulerAnglesGoal_LS.m_pitchDegrees;
	float goalQuadrupedPitch		= curQuadrupedPitch + ( pitchOffset * -fabsf(sine) );
	m_modelRoot->m_eulerAnglesGoal_LS.m_pitchDegrees = Interpolate( curQuadrupedPitch, goalQuadrupedPitch, fractionTowardsEnd * 0.75f );



	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped model root Pitch
	//----------------------------------------------------------------------------------------------------------------------
	Vec3  armsAvgPos								= Interpolate(  leftArmEePos_WS,  rightArmEePos_WS, 0.5f );
	Vec3  legsAvgPos								= Interpolate( leftFootEePos_WS, rightFootEePos_WS, 0.5f );
	// Compute distLegsToArms by projecting onto XY
	Vec3  dispLegsToArms							= armsAvgPos - legsAvgPos;
	Vec3  dirLegsToArmsXY							= Vec3( dispLegsToArms.x, dispLegsToArms.y, 0.0f );
	dirLegsToArmsXY.Normalize();
	float distLegsToArms							= GetProjectedLength3D( dispLegsToArms, dirLegsToArmsXY );
	float goalPitchDegrees							= -Atan2Degrees( dispLegsToArms.z, distLegsToArms );				// Negate result to match pitch (up is negative, down is positive)
	float curPitchDegrees							= m_modelRoot->m_eulerAnglesGoal_LS.m_pitchDegrees;
	m_modelRoot->m_eulerAnglesGoal_LS.m_pitchDegrees	= Interpolate( curPitchDegrees, goalPitchDegrees, fractionTowardsEnd );


	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped tail wagging
	//----------------------------------------------------------------------------------------------------------------------
 	float cos							= CosDegrees( currentTime * 100.0f ) * 5.0f;
	// Compute tailStart_WS
	Vec3  tailStart_WS					= m_tail_CCD->m_firstJoint->GetMatrix_LocalToWorld().GetTranslation3D();
	float fwdNess						= 0.0f;
	sine								= SinDegrees( currentTime * 100.0f );
	float upNess						= sine;
	float leftNess						= sine * 5.0f;
	Vec3  modelFwdDir, modelLeftDir, modelUpDir; 
	m_modelRoot->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( modelFwdDir, modelLeftDir, modelUpDir );
 	Vec3  restTailPos						= tailStart_WS - ( modelFwdDir * 40.0f ) - ( modelUpDir * 5.0f );
	m_tail_CCD->m_target.m_goalPos_WS		= restTailPos - ( modelFwdDir * fwdNess ) + ( modelLeftDir * leftNess * cos ) - ( modelUpDir * upNess );
	m_tail_CCD->m_target.m_currentPos_WS	= Interpolate( m_tail_CCD->m_target.m_currentPos_WS, m_tail_CCD->m_target.m_goalPos_WS, fractionTowardsEnd * 0.75f );


	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped tentacle wagging (Left)
	//----------------------------------------------------------------------------------------------------------------------
	float perlinNoise								= Compute2dPerlinNoise( currentTime * 0.5f, 1.0f  ) * 3.0f;
	Vec3  newGoalPos								= tailStart_WS + ( modelLeftDir * m_distHipToRoot ) + ( modelUpDir * 10.0f * (perlinNoise * 0.5f) );
	m_tentacleLeft_CCD->m_target.m_currentPos_WS	= newGoalPos;
	//----------------------------------------------------------------------------------------------------------------------
	// Update Quadruped tentacle wagging (Right)
	//----------------------------------------------------------------------------------------------------------------------
	perlinNoise										= Compute2dPerlinNoise( currentTime * 0.5f, 1.0f ) * 3.0f;
	newGoalPos										= tailStart_WS + ( -modelLeftDir * m_distHipToRoot ) + ( modelUpDir * 10.0f * (perlinNoise * 0.5f) );
	m_tentacleRight_CCD->m_target.m_currentPos_WS	= newGoalPos;
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::PostUpdateChains( float deltaSeconds )
{
	// WORKING VERSION (MANIPULATING Arm/Leg CHAINS, end joints)
	//----------------------------------------------------------------------------------------------------------------------
	// Left foot palm
	//----------------------------------------------------------------------------------------------------------------------
	// Version 3: solve in WS, transform and update data in LS
  	Vec3 impactNormal_WS			= m_raycast_IdealPos_LeftFoot.m_raycastResult.m_impactNormal;
	Vec3 rotationAxis_WS			= m_modelRoot->m_eulerAnglesGoal_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
  	// Rotate impactNormal_LS 90 degrees
  	Vec3 palmFwd_WS					= RotateVectorAboutArbitraryAxis( impactNormal_WS, rotationAxis_WS, 90.0f );
  	palmFwd_WS.Normalize();
	// Compute palmLeft_WS
	Vec3 palmLeft_WS				= CrossProduct3D( impactNormal_WS, palmFwd_WS );
	palmLeft_WS.Normalize();
	// Transform new palmFwd WS to LS
	Mat44 wToL_Matrix				= m_leftFoot_2BIK->m_finalJoint->GetMatrix_LocalToWorld( true ).GetOrthoNormalInverse();
	Vec3 palmFwd_LS					= wToL_Matrix.TransformVectorQuantity3D( palmFwd_WS  );
	Vec3 palmLeft_LS				= wToL_Matrix.TransformVectorQuantity3D( palmLeft_WS );
	EulerAngles goalEuler			= m_leftFoot_2BIK->m_finalJoint->GetEulerFromFwdAndLeft( palmFwd_LS, palmLeft_LS );
	float curPitch					= m_yprLastFrame_leftFootPalm.m_pitchDegrees;
	float curYaw					= m_yprLastFrame_leftFootPalm.m_yawDegrees;
	float curRoll					= m_yprLastFrame_leftFootPalm.m_rollDegrees;
	float fractionTowardsEnd		= deltaSeconds * 50.0f;
	fractionTowardsEnd				= 1.0f;
	m_yprLastFrame_leftFootPalm.m_yawDegrees	= Interpolate( curYaw,	 goalEuler.m_yawDegrees,   fractionTowardsEnd );
	m_yprLastFrame_leftFootPalm.m_pitchDegrees	= Interpolate( curPitch, goalEuler.m_pitchDegrees, fractionTowardsEnd );
	m_yprLastFrame_leftFootPalm.m_rollDegrees	= Interpolate( curRoll,	 goalEuler.m_rollDegrees,  fractionTowardsEnd );
	m_leftFoot_2BIK->m_finalJoint->m_eulerAnglesGoal_LS		= m_yprLastFrame_leftFootPalm;
	m_leftFoot_2BIK->m_finalJoint->m_eulerAnglesCurrent_LS	= m_yprLastFrame_leftFootPalm;
	// Set debugDraw vectors
 	m_debugVec_ImpactNormal_WS_Palm = palmFwd_WS;
 	m_debugVec_RotationAxis_WS_Palm	= rotationAxis_WS;	

	//----------------------------------------------------------------------------------------------------------------------
	// Right foot palm
	//----------------------------------------------------------------------------------------------------------------------
	// Version 3: solve in WS, transform and update data in LS
  	impactNormal_WS					= m_raycast_IdealPos_RightFoot.m_raycastResult.m_impactNormal;
  	// Rotate impactNormal_LS 90 degrees
  	palmFwd_WS						= RotateVectorAboutArbitraryAxis( impactNormal_WS, rotationAxis_WS, 90.0f );
  	palmFwd_WS.Normalize();
	// Compute palmLeft_WS
	palmLeft_WS						= CrossProduct3D( impactNormal_WS, palmFwd_WS );
	palmLeft_WS.Normalize();
	// Transform new palmFwd WS to LS
	wToL_Matrix						= m_rightFoot_2BIK->m_finalJoint->GetMatrix_LocalToWorld( true ).GetOrthoNormalInverse();
	palmFwd_LS						= wToL_Matrix.TransformVectorQuantity3D( palmFwd_WS );
	palmLeft_LS						= wToL_Matrix.TransformVectorQuantity3D( palmLeft_WS );
	m_rightFoot_2BIK->m_finalJoint->m_eulerAnglesGoal_LS	= m_rightFoot_2BIK->m_finalJoint->GetEulerFromFwdAndLeft( palmFwd_LS, palmLeft_LS );
	m_rightFoot_2BIK->m_finalJoint->m_eulerAnglesCurrent_LS = m_rightFoot_2BIK->m_finalJoint->GetEulerFromFwdAndLeft( palmFwd_LS, palmLeft_LS );
	// Set debugDraw vectors
 	m_debugVec_ImpactNormal_WS_Palm = palmFwd_WS;
 	m_debugVec_RotationAxis_WS_Palm	= rotationAxis_WS;

	//----------------------------------------------------------------------------------------------------------------------
	// Left Arm palm
	//----------------------------------------------------------------------------------------------------------------------
	// Version 3: solve in WS, transform and update data in LS
  	impactNormal_WS					= m_raycast_IdealPos_LeftArm.m_raycastResult.m_impactNormal;
  	// Rotate impactNormal_LS 90 degrees
  	palmFwd_WS						= RotateVectorAboutArbitraryAxis( impactNormal_WS, rotationAxis_WS, 90.0f );
  	palmFwd_WS.Normalize();
	// Compute palmLeft_WS
	palmLeft_WS						= CrossProduct3D( impactNormal_WS, palmFwd_WS );
	palmLeft_WS.Normalize();
	// Transform new palmFwd WS to LS
	wToL_Matrix						= m_leftArm_2BIK->m_finalJoint->GetMatrix_LocalToWorld( true ).GetOrthoNormalInverse();
	palmFwd_LS						= wToL_Matrix.TransformVectorQuantity3D( palmFwd_WS );
	palmLeft_LS						= wToL_Matrix.TransformVectorQuantity3D( palmLeft_WS );
  	m_leftArm_2BIK->m_finalJoint->m_eulerAnglesGoal_LS	  = m_leftArm_2BIK->m_finalJoint->GetEulerFromFwdAndLeft( palmFwd_LS, palmLeft_LS );
  	m_leftArm_2BIK->m_finalJoint->m_eulerAnglesCurrent_LS = m_leftArm_2BIK->m_finalJoint->GetEulerFromFwdAndLeft( palmFwd_LS, palmLeft_LS );
	// Set debugDraw vectors
 	m_debugVec_ImpactNormal_WS_Palm = palmFwd_WS;
 	m_debugVec_RotationAxis_WS_Palm	= rotationAxis_WS;

	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm palm
	//----------------------------------------------------------------------------------------------------------------------
	// Version 3: solve in WS, transform and update data in LS
  	impactNormal_WS					= m_raycast_IdealPos_RightArm.m_raycastResult.m_impactNormal;
  	// Rotate impactNormal_LS 90 degrees
  	palmFwd_WS						= RotateVectorAboutArbitraryAxis( impactNormal_WS, rotationAxis_WS, 90.0f );
  	palmFwd_WS.Normalize();
	// Compute palmLeft_WS
	palmLeft_WS						= CrossProduct3D( impactNormal_WS, palmFwd_WS );
	palmLeft_WS.Normalize();
	// Transform new palmFwd WS to LS
	wToL_Matrix						= m_rightArm_2BIK->m_finalJoint->GetMatrix_LocalToWorld( true ).GetOrthoNormalInverse();
	palmFwd_LS						= wToL_Matrix.TransformVectorQuantity3D( palmFwd_WS );
	palmLeft_LS						= wToL_Matrix.TransformVectorQuantity3D( palmLeft_WS );
	m_rightArm_2BIK->m_finalJoint->m_eulerAnglesGoal_LS		= m_rightArm_2BIK->m_finalJoint->GetEulerFromFwdAndLeft( palmFwd_LS, palmLeft_LS );
	m_rightArm_2BIK->m_finalJoint->m_eulerAnglesCurrent_LS  = m_rightArm_2BIK->m_finalJoint->GetEulerFromFwdAndLeft( palmFwd_LS, palmLeft_LS );
	// Set debugDraw vectors
 	m_debugVec_ImpactNormal_WS_Palm = palmFwd_WS;
 	m_debugVec_RotationAxis_WS_Palm	= rotationAxis_WS;

	//----------------------------------------------------------------------------------------------------------------------
	// Clamping model root to avoid going out of map bounds
	//----------------------------------------------------------------------------------------------------------------------
	ClampModelRootToFeetAvgPos( deltaSeconds );
}


//----------------------------------------------------------------------------------------------------------------------
bool Quadruped::IsIdealPosTooFar( Vec3 const& idealPos, Vec3 posToCompare, float maxDistXY, float maxDistZ )
{
	float curDistXY = GetDistance2D( Vec2( idealPos.x, idealPos.y ), Vec2( posToCompare.x, posToCompare.y ) );
	if ( curDistXY > maxDistXY )
	{
		// Compare XY distance
		return true;
	}
	float curDistZ = idealPos.z - posToCompare.z;
	if ( curDistZ > maxDistZ )
	{
		// Compare  Z distance
//		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool Quadruped::DidRaycastHitWalkableBlock( RaycastResult3D& raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal, std::vector<Block*> blockList )
{
	float superDist_FWD = 500.0f;
	bool  didImpact		= false;
	RaycastResult3D tempRayResult;
	for ( int i = 0; i < blockList.size(); i++ )
	{
		// Check if raycast impacted a block
		tempRayResult = RaycastVsAABB3D( rayStartPos, rayfwdNormal, rayLength, blockList[i]->m_aabb3 );
		if ( tempRayResult.m_didImpact )
		{
			// And the block is walkable
			if ( blockList[i]->m_isWalkable )
			{
				// And the block is close enough
				Vec3 distFromCurrentLineToRay = tempRayResult.m_impactPos - rayStartPos;
				if ( distFromCurrentLineToRay.GetLength() < superDist_FWD )
				{
					// Save the raycastHitInfo 

					// Check for closest line segment to raycast 
					superDist_FWD			= distFromCurrentLineToRay.GetLength();
					didImpact				= true;
					raycastResult			= tempRayResult;
					// Use the updated values below ( impactPos and impactNormal ) for rendering raycast 
					updatedImpactPos		= raycastResult.m_impactPos;
					updatedImpactNormal		= raycastResult.m_impactNormal;
				}
			}
		}
	}
	return didImpact;
}


//----------------------------------------------------------------------------------------------------------------------
bool Quadruped::DidRaycastHitTriangle( RaycastResult3D& raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal, std::vector<unsigned int> triIndexList, std::vector<Vertex_PCU> planeVertList )
{
	bool  didImpact		 = false;
	float t, u, v		 = 0.0f;
	float nearestHitDist = 999999.9f;
	RaycastResult3D tempRayResult;
	// Increment in 3s to form triangles
	for ( int i = 0; i < triIndexList.size(); i += 3 )
	{
		// Calculate triangles from indexList
		int const& currentIndex_C	= triIndexList[ i + 2 ];
		int const& currentIndex_A	= triIndexList[ i + 0 ];
		int const& currentIndex_B	= triIndexList[ i + 1 ];
		Vec3 vert0					= planeVertList[ currentIndex_A ].m_position;
		Vec3 vert1					= planeVertList[ currentIndex_B ].m_position;
		Vec3 vert2					= planeVertList[ currentIndex_C ].m_position;
		tempRayResult				= RaycastVsTriangle( rayStartPos, rayfwdNormal, rayLength, vert0, vert1, vert2, t, u, v );
		if ( tempRayResult.m_didImpact )
		{
			// If ray hit AND is closer
			//			if ( tempRayResult.m_impactDist < raycastResult.m_impactDist )
			if ( tempRayResult.m_impactDist < nearestHitDist )
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
bool Quadruped::DoesRaycastHitFloor( Vec3& refPosition, Raycast& raycast )
{
	// Use raycast to ensure the ideal next step is "placed" on a walkable block
	RaycastResult3D& rayResult_Blocks	= raycast.m_raycastResult;
	RaycastResult3D& rayResult_Tri		= raycast.m_raycastResult;
	Vec3 rayStartPos					= refPosition + Vec3( 0.0f, 0.0f, 15.0f );
	Vec3 impactPos_Blocks				= Vec3::ZERO;
	Vec3 impactPos_Tri					= Vec3::ZERO;
	Vec3 impactNormal_Blocks			= Vec3::ZERO;
	Vec3 impactNormal_Tris				= Vec3::ZERO;
	bool didRayImpactBlock				= false;
	bool didRayImpactTri				= false;
	float maxRayLength					= 0.0f;
	std::vector<Block*>			blockList;
	std::vector<unsigned int>	triIndexList;
	std::vector<Vertex_PCU>		planeVertList;
	if ( m_gameMode3D )
	{
		maxRayLength	= m_gameMode3D->m_raylength_Long;
		blockList		= m_gameMode3D->m_map->m_blockList;
		triIndexList	= m_gameMode3D->m_map->m_indexList;
		planeVertList	= m_gameMode3D->m_map->m_planeVerts;
	}
	else if ( m_gameMode_Creature )
	{
		maxRayLength	= m_gameMode_Creature->m_raylength_Long;
		blockList		= m_gameMode_Creature->GetBlockListFromGameMode();
		triIndexList	= m_gameMode_Creature->m_indexList;
		planeVertList	= m_gameMode_Creature->m_planeVerts;
	}

	didRayImpactBlock	= DidRaycastHitWalkableBlock( rayResult_Blocks,	rayStartPos, Vec3::NEGATIVE_Z, maxRayLength, impactPos_Blocks, impactNormal_Blocks, blockList	 );
	didRayImpactTri		= DidRaycastHitTriangle	    ( rayResult_Tri,	rayStartPos, Vec3::NEGATIVE_Z, maxRayLength, impactPos_Tri,	   impactNormal_Tris,   triIndexList, planeVertList );

	m_rayResult_Blocks  = rayResult_Blocks;
	m_rayResult_Tri		= rayResult_Tri;

	// Raycast against all tri and blocks
	if ( didRayImpactBlock && didRayImpactTri )
	{ 
		// Choose closest hit (between ABB3 and Triangles) if BOTH hit
		if ( rayResult_Blocks.m_impactDist < rayResult_Tri.m_impactDist )
		{
			refPosition = impactPos_Blocks;
		}
		else if ( rayResult_Blocks.m_impactDist > rayResult_Tri.m_impactDist )
		{
			refPosition = impactPos_Tri;
		}
		return true;
	}
	else if ( didRayImpactBlock && !didRayImpactTri )
	{
		refPosition = impactPos_Blocks;
		return true;
	}
	else if ( !didRayImpactBlock && didRayImpactTri )
	{
		refPosition = impactPos_Tri;
		return true;
	}
	else
	{
		// No one hit
		return false;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::UpdateBezierCurvePositions( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const curChain, Vec3 const& refUpDir, Stopwatch& bezierTimer )
{
	UNUSED( bezierTimer );

	// Calculate curve positions 
	Vec3 const& start			= curChain->m_target.m_currentPos_WS;
	Vec3 const& end				= curChain->m_target.m_goalPos_WS;
	Vec3		dispCurrentGoal	= end - start;
	Vec3		fwdNess1		= ( dispCurrentGoal * 0.33f );
	Vec3		fwdNess2		= ( dispCurrentGoal * 0.66f );
	float		maxLength		= curChain->GetMaxChainLength();
	Vec3		ownerUpDir		= refUpDir;
	Vec3		upNess			= ( maxLength * 0.35f ) * ownerUpDir;						// #ToDo: Replace ( maxLength * 0.5f ) with values from raycast approach
	Vec3		guide1			= start + fwdNess1 + upNess;
	Vec3		guide2			= start + fwdNess2 + upNess; 
	// Set curve positions
	bezierCurve.m_startPos	= start;
	bezierCurve.m_guidePos1	= guide1;
	bezierCurve.m_guidePos2	= guide2;
	bezierCurve.m_endPos	= end;
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::UpdateBezierTimer( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const curChain, IK_Chain3D* const anchorLinkedChain, Stopwatch& bezierTimer )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Update Bezier Curve "t"
	//----------------------------------------------------------------------------------------------------------------------
	float elapsedFraction = bezierTimer.GetElapsedFraction();
	if ( elapsedFraction > 0.8f )
	{
		curChain->SetAnchor_Free();
		anchorLinkedChain->SetAnchor_Free();
	}
	if ( bezierTimer.HasDurationElapsed() )
	{
		// Stop Bezier if finished lerping 
		bezierTimer.Stop();
	}
	else if ( elapsedFraction > 0.0f )
	{
		// Update targetCurrentPos if Bezier is still lerping
//		float timeScalar				= 2.0f;
		float smoothStepFraction		= SmoothStep5( elapsedFraction );
		//----------------------------------------------------------------------------------------------------------------------
		// Attempt to polish walk anims by adding different easing types per leg
		// Pausing this feature temporarily for prioritization and time's sake
		//----------------------------------------------------------------------------------------------------------------------
//		float smoothStepFraction		= 0.0f;
// 		int   easingType				= g_theRNG->RollRandomIntInRange( 0, 5 );
// 		// Use SmoothStart
// 		if ( easingType == 0 )
// 		{
// 			smoothStepFraction			= SmoothStart2( elapsedFraction );
// 		}
// 		else if ( easingType == 1 )
// 		{
// 			smoothStepFraction			= SmoothStart4( elapsedFraction );
// 		}
// 		else if ( easingType == 2 )
// 		{
// 			smoothStepFraction			= SmoothStart6( elapsedFraction );
// 		}
// 		// Use Hesitate
// 		else if ( easingType == 3 )
// 		{
// 			smoothStepFraction			= Hesitate3( elapsedFraction );
// 		}
// 		// Use SmoothStop
// 		else if ( easingType == 4 )
// 		{
// 			smoothStepFraction			= SmoothStop3( elapsedFraction );
// 		}
// 		else if ( easingType == 5 )
// 		{
// 			smoothStepFraction			= SmoothStop5( elapsedFraction );
// 		}
		Vec3  bezierLerp					= bezierCurve.GetPointAtFraction( smoothStepFraction );
		curChain->m_target.m_currentPos_WS	= bezierLerp;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::UpdatePoleVector()
{
	// Update poleVectors in MS, relative to the creature modelRoot
	IK_Joint3D* modelRoot				= m_modelRoot;
	Vec3 fwdNess						= ( Vec3::X_FWD	 * 20.0f );
	Vec3 leftNess						= ( Vec3::Y_LEFT * m_distPelvisToRoot * 4.0f );
	Vec3 upNess							= ( Vec3::Z_UP	 * m_defaultHeightZ	);
	m_leftFoot_2BIK ->m_poleVector_MS	= modelRoot->m_jointPos_LS + fwdNess + leftNess - upNess;
	m_rightFoot_2BIK->m_poleVector_MS	= modelRoot->m_jointPos_LS + fwdNess - leftNess - upNess;
	m_leftArm_2BIK  ->m_poleVector_MS	= modelRoot->m_jointPos_LS - fwdNess + leftNess - upNess;
	m_rightArm_2BIK ->m_poleVector_MS	= modelRoot->m_jointPos_LS - fwdNess - leftNess - upNess;
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::ClampModelRootToFeetAvgPos( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	//----------------------------------------------------------------------------------------------------------------------
	// Computing average position to clamp model root
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 leftArmPos_WS    = m_raycast_IdealPos_LeftArm.  m_raycastResult.m_impactPos;
	Vec3 rightArmPos_WS   = m_raycast_IdealPos_RightArm. m_raycastResult.m_impactPos;
	Vec3 leftFootPos_WS   = m_raycast_IdealPos_LeftFoot. m_raycastResult.m_impactPos;
	Vec3 rightFootPos_WS  = m_raycast_IdealPos_RightFoot.m_raycastResult.m_impactPos;
	m_avgFeetPos		  = ( leftArmPos_WS + rightArmPos_WS + leftFootPos_WS + rightFootPos_WS ) / 4;
// 	// New approach, lerp to clamped pos
// 	Vec2 avgFeetPosXY	  = Vec2( m_avgFeetPos.x, m_avgFeetPos.y );
// 	// Clamp root position relative to averageFeetPosition
// 	Vec2  modelRootPosXY_WS					= Vec2( m_modelRoot->m_jointPos_MS.x, m_modelRoot->m_jointPos_MS.y);
// 	Vec2  dirModelRootToAvgFeetPos			= ( modelRootPosXY_WS - avgFeetPosXY ).GetNormalized();		// Model root's model space happens to be in world space
// 	float distModelRootToAverageFeetPos		= GetDistance2D( modelRootPosXY_WS, avgFeetPosXY );
// 	m_clampedDistModelRootToAvgFeetPos		= GetClamped( distModelRootToAverageFeetPos, 0.0f, m_maxDistToClampRootFromAvgFeetPos );
// 	Vec2 clampedPosXY						= avgFeetPo7sXY + ( dirModelRootToAvgFeetPos * m_clampedDistModelRootToAvgFeetPos );
// 	float fractionTowardsEnd				= deltaSeconds * 250.0f;
// 	modelRootPosXY_WS						= Interpolate( modelRootPosXY_WS, clampedPosXY, fractionTowardsEnd );
// 	m_modelRoot->m_jointPos_MS.x			= modelRootPosXY_WS.x;
// 	m_modelRoot->m_jointPos_MS.y			= modelRootPosXY_WS.y;


	// Old approach, clamp on XY dir
 	Vec2 avgFeetPosXY	  = Vec2( m_avgFeetPos.x, m_avgFeetPos.y );
 	// Clamp root position relative to averageFeetPosition
 	Vec2  modelRootPosXY_WS					= Vec2( m_modelRoot->m_jointPos_MS.x, m_modelRoot->m_jointPos_MS.y);
 	Vec2  dirModelRootToAvgFeetPos			= ( modelRootPosXY_WS - avgFeetPosXY ).GetNormalized();		// Model root's model space happens to be in world space
 	float distModelRootToAverageFeetPos		= GetDistance2D( modelRootPosXY_WS, avgFeetPosXY );
 	m_clampedDistModelRootToAvgFeetPos		= GetClamped( distModelRootToAverageFeetPos, 0.0f, m_maxDistToClampRootFromAvgFeetPos );
 	Vec2 clampedPosXY						= avgFeetPosXY + ( dirModelRootToAvgFeetPos * m_clampedDistModelRootToAvgFeetPos );
 	m_modelRoot->m_jointPos_MS.x			= clampedPosXY.x;
 	m_modelRoot->m_jointPos_MS.y			= clampedPosXY.y;
}



//----------------------------------------------------------------------------------------------------------------------
void Quadruped::DebugDrawBezier( std::vector<Vertex_PCU>& verts, CubicBezierCurve3D const& bezierCurve, Stopwatch const& timer )
{
	// Bezier points
	float elaspedTime	= timer.GetElapsedTime();
	Vec3 bezierPosAtT	= bezierCurve.GetPointAtFraction( elaspedTime );
	AddVertsForSphere3D( verts, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );
	AddVertsForSphere3D( verts, bezierCurve.m_startPos , 0.5f, 8.0f, 16.0f, Rgba8::RED	   );
	AddVertsForSphere3D( verts, bezierCurve.m_guidePos1, 0.5f, 8.0f, 16.0f, Rgba8::GREEN   );
	AddVertsForSphere3D( verts, bezierCurve.m_guidePos2, 0.5f, 8.0f, 16.0f, Rgba8::BLUE	   );
	AddVertsForSphere3D( verts, bezierCurve.m_endPos   , 0.5f, 8.0f, 16.0f, Rgba8::MAGENTA );
	// Bezier trail
	float thickness = 0.5f;
	Vec3 previousBezierDotPos = bezierCurve.m_startPos;
	int		m_numSubdivisions = 64;
	for ( int subdivisionIndex = 0; subdivisionIndex <= (m_numSubdivisions - 1); subdivisionIndex++ )
	{
		// Calculate subdivisions
		float t						= (1.0f / static_cast<float>(m_numSubdivisions)) * (subdivisionIndex + 1);
		Vec3 currentBezierDotPos	= bezierCurve.GetPointAtFraction( t );
		// Render curved line graph
		AddVertsForLineSegment3D( verts, previousBezierDotPos, currentBezierDotPos, thickness, Rgba8::PURPLE );
		previousBezierDotPos = currentBezierDotPos;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::DebugDrawRaycasts( std::vector<Vertex_PCU>& verts ) const
{
	std::vector<RaycastResult3D> rayResultList;
	// Ideal positions
	rayResultList.push_back( m_raycast_IdealPos_LeftArm.  m_raycastResult );
	rayResultList.push_back( m_raycast_IdealPos_RightArm. m_raycastResult );
	rayResultList.push_back( m_raycast_IdealPos_LeftFoot. m_raycastResult );
	rayResultList.push_back( m_raycast_IdealPos_RightFoot.m_raycastResult );
	// Cur positions
	rayResultList.push_back( m_raycast_CurPos_LeftArm.  m_raycastResult );
	rayResultList.push_back( m_raycast_CurPos_RightArm. m_raycastResult );
	rayResultList.push_back( m_raycast_CurPos_LeftFoot. m_raycastResult );
	rayResultList.push_back( m_raycast_CurPos_RightFoot.m_raycastResult );

	for ( int i = 0; i < rayResultList.size(); i++ )
	{
		RaycastResult3D rayResult = rayResultList[i];
		AddVertsForArrow3D ( verts, rayResult.m_rayStartPosition, rayResult.m_impactPos, 0.1f, Rgba8::WHITE );
		if ( rayResult .m_didImpact )
		{	 
			// Ray impact normal
			Vec3 normalEndPos = rayResult.m_impactPos + ( rayResult.m_impactNormal * 4.0f );
			AddVertsForArrow3D ( verts, rayResult.m_impactPos, normalEndPos, 0.2f, Rgba8::LIGHTBLUE );
			// Ray impactPos
			AddVertsForSphere3D( verts, rayResult.m_impactPos, 2.0f, 16.0f, 16.0f, Rgba8::DARK_BLUE );
		}
		else
		{
			Vec3 endPos = rayResult.m_rayStartPosition + ( rayResult.m_rayFwdNormal * rayResult.m_rayMaxLength );
			AddVertsForArrow3D( verts, rayResult.m_rayStartPosition, endPos, 0.5f, Rgba8::DARKER_RED );
		}
	}
	
	if ( m_rayResult_Tri.m_didImpact )
	{
		// Ray
		AddVertsForArrow3D ( verts, m_rayResult_Tri.m_rayStartPosition, m_rayResult_Tri.m_impactPos, 0.5f, Rgba8::GREEN );
		// Ray impact normal
		Vec3 normalEndPos = m_rayResult_Tri.m_impactPos + ( m_rayResult_Tri.m_impactNormal * 2.0f );
		AddVertsForArrow3D( verts, m_rayResult_Tri.m_impactPos, normalEndPos, 0.5f, Rgba8::LIGHTBLUE );
		// Ray impactPos
		AddVertsForSphere3D( verts, m_rayResult_Tri.m_impactPos, 2.0f, 4.0f, 4.0f, Rgba8::BLUE );
	}
	else
	{
		Vec3 endPos = m_rayResult_Tri.m_rayStartPosition + ( m_rayResult_Tri.m_rayFwdNormal * m_rayResult_Tri.m_rayMaxLength );
		AddVertsForArrow3D( verts, m_rayResult_Tri.m_rayStartPosition, endPos, 0.5f, Rgba8::DARKER_GREEN );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::ComputeIdealStepPosition( IK_Chain3D* chainToEdit, EulerAngles eulerAngles, Vec3 const& anchorPos, float fwdScalar, float leftScalar, float upScalar )
{
	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	Vec3 fwd, left, up;
	eulerAngles.GetAsVectors_XFwd_YLeft_ZUp( fwd, left, up );
	Vec3 fwdOffset					= ( fwd  * fwdScalar  );
	Vec3 leftOffset					= ( left * leftScalar );
	Vec3 upOffset					= ( up	 * upScalar	  );
	chainToEdit->m_idealStepPos_WS	= anchorPos + fwdOffset + leftOffset + upOffset; 
}


//----------------------------------------------------------------------------------------------------------------------
bool Quadruped::TryFreeAnchor( IK_Chain3D* chainToEdit, IK_Chain3D* anchorChain )
{
	// 2. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	bool canMove = chainToEdit->CanMove();
	if ( !canMove )
	{
		// 2b. Try freeing anchor if CAN'T move
		canMove = chainToEdit->TryFreeAnchor( anchorChain );
	}
	return canMove;
}


//----------------------------------------------------------------------------------------------------------------------
bool Quadruped::ShouldChainStep( IK_Chain3D* chainToEdit, bool canMove, float maxDistFromIdealPos_XY, float maxDistFromIdealPos_Z )
{
	// 3. Check if each chain should step
	bool shouldStep = false;
	if ( canMove )
	{
		Mat44 localToWorldMatrix	= chainToEdit->m_finalJoint->GetMatrix_LocalToWorld();
		Vec3  footEE_WS				= localToWorldMatrix.GetTranslation3D();
		shouldStep					= IsIdealPosTooFar( chainToEdit->m_idealStepPos_WS, footEE_WS, maxDistFromIdealPos_XY, maxDistFromIdealPos_Z );
	}
	return shouldStep;
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::TryInitBezierTimer( bool canMove, bool shouldStep, Stopwatch& bezierTimer, IK_Chain3D* chainToEdit, IK_Chain3D* anchorChain )
{
	// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	if ( canMove && shouldStep )
	{
		if ( bezierTimer.IsStopped() )
		{
			// 4a. Start Bezier timer
			bezierTimer.Start();
			// 4b. Toggle anchor states
			chainToEdit->SetAnchor_Moving();
			anchorChain->SetAnchor_Locked();
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
bool Quadruped::DoesRayHitFloor_CurPos( IK_Chain3D* chainToEdit, Raycast& raycast )
{
	// 5a. Raycast against floor (Cur Pos)
	bool doesRaycastHitFloor = DoesRaycastHitFloor( chainToEdit->m_target.m_currentPos_WS, raycast );
	if ( doesRaycastHitFloor )
	{
		// Update curPos 
		chainToEdit->m_target.m_currentPos_WS = raycast.m_raycastResult.m_impactPos;
	}
	return doesRaycastHitFloor;
}


//----------------------------------------------------------------------------------------------------------------------
void Quadruped::UpdateIdealStepPosition( bool doesRaycastHitFloor_CurPos, IK_Chain3D* chainToEdit, IK_Chain3D* anchorChain, Stopwatch& curChainTimer )
{
	// 5c. Update ideal pos if better than prev step
	if ( !doesRaycastHitFloor_CurPos )
	{
		// Keep previous ideal position?
		chainToEdit->m_idealStepPos_WS = chainToEdit->m_prevIdealPos_WS;
		// If there was no new position found, then keep the old position, stop the timer, and free the leg anchors.
		// 5c. Start Bezier timer
		curChainTimer.Stop();
		// 5d. Toggle anchor states
		chainToEdit->SetAnchor_Free();
		anchorChain->SetAnchor_Free();
	}
	else
	{
		chainToEdit->m_prevIdealPos_WS = chainToEdit->m_idealStepPos_WS;
	}
	// 5d. Update idealPos.Z based on rayImpactPos.z
	chainToEdit->m_target.m_goalPos_WS = chainToEdit->m_idealStepPos_WS;
}
