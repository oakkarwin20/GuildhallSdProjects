#include "Game/GameModeBase.hpp"
#include "Game/GameMode_Spider_3D.hpp"
#include "Game/GameMode3D.hpp"
#include "Game/SpiderBase.hpp"
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
SpiderBase::SpiderBase( GameMode3D* game, Vec3 const& rootStartPos, float limbLength ) : CreatureBase( rootStartPos, limbLength )
{
	m_gameMode3D = game;
}


//----------------------------------------------------------------------------------------------------------------------
SpiderBase::SpiderBase( GameMode_Spider_3D* gameMode_Spider, Vec3 const& rootStartPos, float limbLength /*= FloatRange( 1.0f, 1.0f ) */ ) : CreatureBase( rootStartPos, limbLength )
{
	m_gameMode_Spider = gameMode_Spider;
	m_desiredRootHeightAboveLimbs = rootStartPos.z;
}


//----------------------------------------------------------------------------------------------------------------------
SpiderBase::~SpiderBase()
{
}


//----------------------------------------------------------------------------------------------------------------------
void SpiderBase::RenderSpiderBase(	std::vector<Vertex_PCU>& verts, 
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
	m_spine_CCD			->m_shouldRender = true;
	m_leftLeg1_2BIK		->m_shouldRender = true;
	m_leftLeg2_2BIK		->m_shouldRender = true;
	m_leftLeg3_2BIK		->m_shouldRender = true;
	m_leftLeg4_2BIK		->m_shouldRender = true;
	m_rightLeg1_2BIK	->m_shouldRender = true;
	m_rightLeg2_2BIK	->m_shouldRender = true;
	m_rightLeg3_2BIK	->m_shouldRender = true;
	m_rightLeg4_2BIK	->m_shouldRender = true;
	m_neck_FABRIK 		->m_shouldRender = true;
//	m_tail_CCD			->m_shouldRender = true;
	AddVertsForSphere3D( verts, m_modelRoot->m_jointPos_MS, 1.2f, 16.0f, 16.0f ); 
	if ( g_debugToggleMesh_Backspace )
	{
		RenderMesh( g_theRenderer, creatureTexture, g_litShader_PCUTBN );		// Render using meshs
	}
	else
	{
		m_modelRoot->RenderIJK_MS( verts, 7.0f, 0.2f );
		Render( verts_textured, Rgba8::WHITE, Rgba8::WHITE );					// Render bones/joints only
	}

	// Root body WS
	AABB3 bounds = AABB3( -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f );
	AddVertsForAABB3D( verts_ikChainBody, bounds, Rgba8::BROWN );
	//----------------------------------------------------------------------------------------------------------------------
	// Chain targets, Current positions
	//----------------------------------------------------------------------------------------------------------------------
//	m_spine1_CCD		->RenderTarget_CurrentPos( verts, 0.5f );
// 	m_leftLeg1_2BIK		->RenderTarget_CurrentPos( verts, 0.5f );
// 	m_leftLeg2_2BIK		->RenderTarget_CurrentPos( verts, 0.5f );
// 	m_leftLeg3_2BIK		->RenderTarget_CurrentPos( verts, 0.5f );
// 	m_leftLeg4_2BIK		->RenderTarget_CurrentPos( verts, 0.5f );
// 	m_rightLeg1_2BIK	->RenderTarget_CurrentPos( verts, 0.5f );
// 	m_rightLeg2_2BIK	->RenderTarget_CurrentPos( verts, 0.5f );
// 	m_rightLeg3_2BIK	->RenderTarget_CurrentPos( verts, 0.5f );
// 	m_rightLeg4_2BIK	->RenderTarget_CurrentPos( verts, 0.5f );
//	m_tail_CCD			->RenderTarget_CurrentPos( verts, 0.5f );
//	m_neck_FABRIK		->RenderTarget_CurrentPos( verts, 0.5f );	
	//----------------------------------------------------------------------------------------------------------------------
	// Chain targets, Goal position 
	//----------------------------------------------------------------------------------------------------------------------	
//	m_leftLeg1_2BIK			->RenderTarget_GoalPos( verts, 0.45f, Rgba8::CYAN );
//	m_leftLeg2_2BIK			->RenderTarget_GoalPos( verts, 0.45f, Rgba8::CYAN );
//	m_leftLeg3_2BIK			->RenderTarget_GoalPos( verts, 0.45f, Rgba8::CYAN );
//	m_leftLeg4_2BIK			->RenderTarget_GoalPos( verts, 0.45f, Rgba8::CYAN );
//	m_rightLeg1_2BIK		->RenderTarget_GoalPos( verts, 0.45f, Rgba8::CYAN );
//	m_rightLeg2_2BIK		->RenderTarget_GoalPos( verts, 0.45f, Rgba8::CYAN );
//	m_rightLeg3_2BIK		->RenderTarget_GoalPos( verts, 0.45f, Rgba8::CYAN );
//	m_rightLeg4_2BIK		->RenderTarget_GoalPos( verts, 0.45f, Rgba8::CYAN );
//	m_spine1_CCD		->RenderTarget_GoalPos( verts, 0.5f, Rgba8::CYAN );
//	m_tail_CCD			->RenderTarget_GoalPos( verts, 0.5f, Rgba8::CYAN );
//	m_neck_FABRIK		->RenderTarget_GoalPos( verts, 0.5f, Rgba8::CYAN );

// 	// LeftFoot to tailStart
// 	Vec3 tailStart		= m_tail_CCD->m_firstJoint->GetMatrix_LocalToWorld().GetTranslation3D();
// 	Vec3 leftFootStart	= m_leftFoot_2BIK->m_firstJoint->GetMatrix_LocalToWorld().GetTranslation3D();
// 	AddVertsForCylinder3D( verts_textured, leftFootStart, tailStart, 1.0f );
// 	// RightFoot to tailStart
// 	Vec3 rightFootStart = m_rightFoot_2BIK->m_firstJoint->GetMatrix_LocalToWorld().GetTranslation3D();
// 	AddVertsForCylinder3D( verts_textured, rightFootStart, tailStart, 1.0f );

	float textHeight = 0.75f;
	if ( g_debugBasis_F3 )
	{
		float arrowLength = 7.0f;
		m_leftLeg1_2BIK		->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
//		m_leftLeg2_2BIK		->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
//		m_leftLeg3_2BIK		->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
//		m_leftLeg4_2BIK		->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
//		m_rightLeg1_2BIK	->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
//		m_rightLeg2_2BIK	->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
//		m_rightLeg3_2BIK	->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
//		m_rightLeg4_2BIK	->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
//		m_rightFoot_2BIK	->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
 		m_spine_CCD			->DebugDrawJoints_IJK( verts, 0.5f, 15.0f );
// 		m_leftArm_2BIK		->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
// 		m_rightArm_2BIK		->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
// 		m_tail_CCD			->DebugDrawJoints_IJK( verts, 0.1f, 5.0f );
 		m_neck_FABRIK		->DebugDrawJoints_IJK( verts, 0.1f, arrowLength );
		arrowLength = 3.0f;
	}
	if ( g_debugText_F4 )
	{
		Vec3 camLeft = worldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
		Vec3 camUp	 = worldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D();
 		m_leftLeg1_2BIK		->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
// 		m_rightFoot_2BIK	->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
//		m_spine1_CCD		->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
//		m_leftArm_2BIK		->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
//		m_rightArm_2BIK		->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
// 		m_tail_CCD			->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
//		m_neck_FABRIK		->DebugTextJointPos_WorldSpace( textVerts, textHeight, 6.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::RED );
// 		m_leftFoot_FABRIK	->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
// 		m_rightFoot_FABRIK	->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
//		m_spine1_CCD		->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
// 		m_leftArm_FABRIK	->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
// 		m_rightArm_FABRIK	->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
//		m_tail_CCD			->DebugTextJointPos_LocalSpace( textVerts, textHeight, 4.0f, camLeft, camUp, g_theApp->m_textFont, Rgba8::WHITE, false );
	}
	if ( g_debugAngles_F5 )
	{
		m_leftLeg1_2BIK		->DebugDrawConstraints_YPR( verts_BackFace, 5.0f );
//		m_rightFoot_FABRIK	->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
//		m_spine_CCD			->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
//		m_leftArm_FABRIK	->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
//		m_rightArm_FABRIK	->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
//		m_tail_CCD			->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
//		m_neck_FABRIK		->DebugDrawConstraints_YPR( verts_BackFace, 3.0f );
	}
	if ( g_debugRenderRaycast_F2 )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Ideal positions
		//----------------------------------------------------------------------------------------------------------------------
		AddVertsForSphere3D( verts, m_leftLeg4_2BIK		->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
		AddVertsForSphere3D( verts, m_rightLeg4_2BIK	->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
// 		AddVertsForSphere3D( verts, m_spine1_CCD		->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
		AddVertsForSphere3D( verts, m_leftLeg1_2BIK		->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED );
		AddVertsForSphere3D( verts, m_rightLeg1_2BIK	->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
// 		AddVertsForSphere3D( verts, m_tail_CCD			->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 
// 		AddVertsForSphere3D( verts, m_neck_FABRIK		->m_idealStepPos_WS, 1.0f, 16.0f, 16.0f, Rgba8::DARKER_RED ); 

		// Debug render raycast results BLOCKS
		DebugDrawRaycasts( verts );

		//----------------------------------------------------------------------------------------------------------------------
		// Max Dist from ideal Pos
		//----------------------------------------------------------------------------------------------------------------------
		AddVertsForDisc3D_XY( verts, m_leftLeg4_2BIK ->m_idealStepPos_WS + Vec3( 0.0f, 0.0f, 0.01f ), m_maxDistFromIdealPos_XY, Rgba8( 100, 0, 100 ) );
		AddVertsForDisc3D_XY( verts, m_rightLeg4_2BIK->m_idealStepPos_WS + Vec3( 0.0f, 0.0f, 0.01f ), m_maxDistFromIdealPos_XY, Rgba8( 100, 0, 100 ) );
		AddVertsForDisc3D_XY( verts, m_leftLeg1_2BIK ->m_idealStepPos_WS + Vec3( 0.0f, 0.0f, 0.01f ), m_maxDistFromIdealPos_XY, Rgba8( 100, 0, 100 ) );
		AddVertsForDisc3D_XY( verts, m_rightLeg1_2BIK->m_idealStepPos_WS + Vec3( 0.0f, 0.0f, 0.01f ), m_maxDistFromIdealPos_XY, Rgba8( 100, 0, 100 ) );

		//----------------------------------------------------------------------------------------------------------------------
		// Max Min dist for "comeCloser" and "moveAway"
		//----------------------------------------------------------------------------------------------------------------------
		AddVertsForDisc3D_XY( verts, m_modelRoot->m_jointPos_MS, m_maxDist_comeCloser, Rgba8( 0, 0, 100, 35 ) );
		AddVertsForDisc3D_XY( verts, m_modelRoot->m_jointPos_MS + Vec3( 0.0f, 0.0f, 0.01f ), m_minDist_moveAway, Rgba8( 100, 0, 0, 35 ) );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Pole vector and rotation axis debug rendering
	//----------------------------------------------------------------------------------------------------------------------
// 	m_leftLeg1_2BIK	->DebugDrawPoleVectorInfo( verts, false );
// 	m_leftLeg2_2BIK	->DebugDrawPoleVectorInfo( verts, false );
//	m_leftLeg3_2BIK	->DebugDrawPoleVectorInfo( verts, false );
// 	m_leftLeg4_2BIK	->DebugDrawPoleVectorInfo( verts, false );
//	m_rightLeg1_2BIK->DebugDrawPoleVectorInfo( verts, false );
//	m_rightLeg2_2BIK->DebugDrawPoleVectorInfo( verts, false );
//	m_rightLeg3_2BIK->DebugDrawPoleVectorInfo( verts, false );
//	m_rightLeg4_2BIK->DebugDrawPoleVectorInfo( verts, false );
 
	//----------------------------------------------------------------------------------------------------------------------
	// Debug rendering rotation axis for palm
	//----------------------------------------------------------------------------------------------------------------------
// 	Mat44 wToL_Matrix	 = m_leftFootPalm->m_firstJoint->GetMatrix_LocalToWorld().GetOrthoNormalInverse();
// 	Vec3 impactNormal_LS = wToL_Matrix.TransformVectorQuantity3D( m_raycast_IdealPos_LeftFoot.m_raycastResult.m_impactNormal );
// 	Vec3 start			 = m_leftFoot_2BIK->m_jointList[1].GetMatrix_LocalToWorld.GetTranslation3D();
// 	Vec3 end			 = start + impactNormal_LS;
// 	Vec3 start	= m_leftFoot_2BIK->m_jointList[1]->GetMatrix_LocalToWorld().GetTranslation3D();
//  	Vec3 end	= start + (m_debugVec_RotationAxis_WS_Palm * 10.f);
// 	AddVertsForArrow3D( verts, start, end, 0.75f, Rgba8::MAGENTA );
// 
// 	start	= m_leftFoot_2BIK->m_jointList[2]->GetMatrix_LocalToWorld().GetTranslation3D();
//  	end		= start + (m_debugVec_ImpactNormal_WS_Palm * 10.f);
// 	AddVertsForArrow3D( verts, start, end, 0.75f, Rgba8::CYAN );

	
	//----------------------------------------------------------------------------------------------------------------------
	// #TEST create normals from mesh list (PCU -> PCUTBN)
	//----------------------------------------------------------------------------------------------------------------------
//	std::vector<Vertex_PCUTBN> meshList_PCUTBN;
//	std::vector<Vertex_PCU> debugNormalsList;
// 	for ( int i = 0; i < m_spine_CCD->m_jointList.size(); i++ )
// 	{
// 		IK_Joint3D* curJoint = m_spine_CCD->m_jointList[1];
// 		for ( int j = 0; j < curJoint->m_meshList.size(); j += 3 )
// 		{
// 			Vertex_PCU& vertA = curJoint->m_meshList[j + 0];
// 			Vertex_PCU& vertB = curJoint->m_meshList[j + 1];
// 			Vertex_PCU& vertC = curJoint->m_meshList[j + 2];
// 
// 			// Compute disps
// 			Vec3 dispAB = vertB.m_position - vertA.m_position;
// 			Vec3 dispAC = vertC.m_position - vertA.m_position;
// 			// Compute normal per joint
// 			Vec3 normal = CrossProduct3D( dispAB, dispAC );
// 			normal.Normalize();
// 			// Make PCUTBN from PCU
// 			meshList_PCUTBN.emplace_back( vertA.m_position, vertA.m_color, vertA.m_uvTexCoords, Vec3::ZERO, Vec3::ZERO, normal );	// vertA 
// 			meshList_PCUTBN.emplace_back( vertB.m_position, vertB.m_color, vertB.m_uvTexCoords, Vec3::ZERO, Vec3::ZERO, normal );	// vertB 
// 			meshList_PCUTBN.emplace_back( vertC.m_position, vertC.m_color, vertC.m_uvTexCoords, Vec3::ZERO, Vec3::ZERO, normal );	// vertC 
// 
// 			// Save normals
// // 			Vec3 endA		= vertA.m_position + ( normal * 5.0f );
// // 			Vec3 endB		= vertB.m_position + ( normal * 5.0f );
// // 			Vec3 endC		= vertC.m_position + ( normal * 5.0f );
// // 			float radius	= 0.05f;
// // 			AddVertsForArrow3D( debugNormalsList, vertA.m_position, endA, radius, Rgba8::RED	);
// // 			AddVertsForArrow3D( debugNormalsList, vertB.m_position, endB, radius, Rgba8::GREEN  );
// // 			AddVertsForArrow3D( debugNormalsList, vertC.m_position, endC, radius, Rgba8::BLUE	);
// 		}
//	}
//	// Render per joint
//	g_theRenderer->SetModelConstants();
//	g_theRenderer->BindTexture( creatureTexture );
//	g_theRenderer->BindShader( g_litShader_PCUTBN );
//	g_theRenderer->DrawVertexArray( int( meshList_PCUTBN.size() ), meshList_PCUTBN.data() );
//	g_theRenderer->BindTexture( nullptr );
//	g_theRenderer->BindShader ( nullptr );
//	g_theRenderer->DrawVertexArray( int( debugNormalsList.size() ), debugNormalsList.data() );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug draw average feet position to clamp root
	//----------------------------------------------------------------------------------------------------------------------
// 	AddVertsForSphere3D( verts, m_avgFeetPos, 2.0f, 16.0f, 16.0f, Rgba8::MAGENTA );
// 	AddVertsForDisc3D_XY( verts, Vec3( m_avgFeetPos.x, m_avgFeetPos.y, -8.0f ), m_clampedDistModelRootToAvgFeetPos );
// 	AddVertsForDisc3D_XY( verts, Vec3( m_avgFeetPos.x, m_avgFeetPos.y, -8.1f ), m_maxDistToClampRootFromAvgFeetPos, Rgba8::DARKER_GRAY );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug draw Gait anchor states
	//----------------------------------------------------------------------------------------------------------------------
// 	float radius = 3.0f;
// 	m_leftLeg1_2BIK	->DebugDrawAnchorStates( verts, m_leftLeg1_2BIK	->m_lerpEulerToGoal, radius );
// 	m_leftLeg2_2BIK	->DebugDrawAnchorStates( verts, m_leftLeg2_2BIK	->m_lerpEulerToGoal, radius );
// 	m_leftLeg3_2BIK	->DebugDrawAnchorStates( verts, m_leftLeg3_2BIK	->m_lerpEulerToGoal, radius );
// 	m_leftLeg4_2BIK	->DebugDrawAnchorStates( verts, m_leftLeg4_2BIK	->m_lerpEulerToGoal, radius );
// 	m_rightLeg1_2BIK->DebugDrawAnchorStates( verts, m_rightLeg1_2BIK->m_lerpEulerToGoal, radius );
// 	m_rightLeg2_2BIK->DebugDrawAnchorStates( verts, m_rightLeg2_2BIK->m_lerpEulerToGoal, radius );
// 	m_rightLeg3_2BIK->DebugDrawAnchorStates( verts, m_rightLeg3_2BIK->m_lerpEulerToGoal, radius );
// 	m_rightLeg4_2BIK->DebugDrawAnchorStates( verts, m_rightLeg4_2BIK->m_lerpEulerToGoal, radius );
}


//----------------------------------------------------------------------------------------------------------------------
void SpiderBase::InitJointMesh()
{
	Rgba8 tint_skin = Rgba8( 35, 35, 35 );
	Rgba8 tint_eyes = Rgba8::RED;

	//----------------------------------------------------------------------------------------------------------------------
	// Hour glass on butt (bottom top)
	//----------------------------------------------------------------------------------------------------------------------
	m_spine_CCD->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3( -3.25f, 0.0f, 5.1f ), Vec3( -3.75f, 0.0f, 5.0f ), 0.05f, 0.75f, Vec2( 1.0f, 1.0f ), Vec2( 0.4f, 1.0f ), tint_eyes );	// bottom half of hour glass, closer to butt
	m_spine_CCD->m_firstJoint->InitMesh_CapsuleTaperedEllipse( Vec3( -2.75f, 0.0f, 5.1f ), Vec3( -3.275f, 0.0f, 5.1f ), 0.5f,  0.05f, Vec2( 0.4f, 1.0f ), Vec2( 1.0f, 1.0f ), tint_eyes );	// top half of hour glass, closer to head

	//----------------------------------------------------------------------------------------------------------------------
	// Spine mesh
	//----------------------------------------------------------------------------------------------------------------------
	m_spine_CCD->m_firstJoint->		InitMesh_CapsuleTaperedEllipse( Vec3(-1.5f, 0.0f, 1.0f), Vec3(m_distPelvisToRoot - 3.0f, 0.0f, 0.0f), 6.5f, 3.0f, Vec2( 0.6f,  1.0f ), Vec2( 0.85f, 1.0f ), tint_skin );		// Spine 0 (Base/Butt)
	m_spine_CCD->m_jointList[ 1 ]-> InitMesh_CapsuleTaperedEllipse( Vec3( 2.0f, 0.0f, 0.0f), Vec3(m_distPelvisToRoot,		 0.0f, 0.0f), 3.5f, 2.0f, Vec2( 0.75f, 1.0f ), Vec2( 0.9f,  1.0f ), tint_skin );		// Spine 0 (Base/Butt)
//	m_spine_CCD->m_firstJoint->InitMesh_CylinderTapered( Vec3( -1.0f, 0.0f, 0.0f ), Vec3( m_distPelvisToRoot, 0.0f, 0.0f ), 4.5f, 1.5f );		// Spine 0 (Base/Butt)
//	m_spine_CCD->m_firstJoint->InitMesh_Sphere( Vec3( -3.0f, 0.0f, 0.0f ), 5.0f );		// Spine 0 (Base/Butt)
//	m_spine_CCD->m_jointList[ 1 ]->InitMesh_Sphere( Vec3::ZERO, 3.5f );		// Spine 1 
//	m_spine_CCD->m_jointList[ 2 ]->InitMesh_Sphere( Vec3::ZERO, 3.0f );		// Spine 2 (Head)

	//----------------------------------------------------------------------------------------------------------------------
	// Spider eyes
	//----------------------------------------------------------------------------------------------------------------------
	m_spine_CCD->m_finalJoint->InitMesh_Sphere( Vec3(1.25f,  0.75f,  1.0f), 0.5f , tint_eyes );		// Center left
	m_spine_CCD->m_finalJoint->InitMesh_Sphere( Vec3(1.25f, -0.75f,  1.0f), 0.5f , tint_eyes );		// Center right
	m_spine_CCD->m_finalJoint->InitMesh_Sphere( Vec3(1.0f,   1.5f,  0.75f), 0.25f, tint_eyes );		// far left
	m_spine_CCD->m_finalJoint->InitMesh_Sphere( Vec3(1.0f,  -1.5f,  0.75f), 0.25f, tint_eyes );		// far right

	//----------------------------------------------------------------------------------------------------------------------
	// Spider mouth
	//----------------------------------------------------------------------------------------------------------------------
// 	m_spine_CCD->m_finalJoint->InitMesh_Sphere( Vec3(2.5f,  0.75f, 0.75f), 0.5f  );		// Center left
// 	m_spine_CCD->m_finalJoint->InitMesh_Sphere( Vec3(2.5f, -0.75f, 0.75f), 0.5f  );		// Center right
// 	m_spine_CCD->m_finalJoint->InitMesh_Sphere( Vec3(2.5f,  1.5f,  0.5f),  0.25f );		// far left
// 	m_spine_CCD->m_finalJoint->InitMesh_Sphere( Vec3(2.5f, -1.5f,  0.5f),  0.25f );		// far right

	//----------------------------------------------------------------------------------------------------------------------
	// Tail mesh
	//----------------------------------------------------------------------------------------------------------------------
// 	m_tail_CCD->m_firstJoint->  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 3.0f,  3.2f  );		// Tail base 0
// 	m_tail_CCD->m_jointList[1]->InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 3.2f,  3.0f  );		// Tail part 1
// 	m_tail_CCD->m_jointList[2]->InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 3.0f,  2.5f  );		// Tail part 2
// 	m_tail_CCD->m_jointList[3]->InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 2.5f,  2.0f  );		// Tail part 3
// 	m_tail_CCD->m_jointList[4]->InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 2.0f,  1.75f );		// Tail part 4
// 	m_tail_CCD->m_jointList[5]->InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 1.75f, 1.15f );		// Tail part 5
// 	m_tail_CCD->m_jointList[6]->InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 1.15f, 0.75f );		// Tail part 6
// 	m_tail_CCD->m_jointList[7]->InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 0.75f, 0.3f  );		// Tail part 6
// 	m_tail_CCD->m_finalJoint->  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_tailLength, 0.0f, 0.0f), 0.3f,  0.0f  );		// Tail tip  7
// 																															// Hide tail gaps using spheres
// 	m_tail_CCD->m_firstJoint->  InitMesh_Sphere( Vec3::ZERO, 3.0f  );		// Tail base 0
// 	m_tail_CCD->m_jointList[1]->InitMesh_Sphere( Vec3::ZERO, 3.2f  );		// Tail part 1
// 	m_tail_CCD->m_jointList[2]->InitMesh_Sphere( Vec3::ZERO, 3.0f  );		// Tail part 2
// 	m_tail_CCD->m_jointList[3]->InitMesh_Sphere( Vec3::ZERO, 2.5f  );		// Tail part 3
// 	m_tail_CCD->m_jointList[4]->InitMesh_Sphere( Vec3::ZERO, 2.0f );		// Tail part 4
// 	m_tail_CCD->m_jointList[5]->InitMesh_Sphere( Vec3::ZERO, 1.75f );		// Tail part 5
// 	m_tail_CCD->m_jointList[6]->InitMesh_Sphere( Vec3::ZERO, 1.15f );		// Tail part 6

	//----------------------------------------------------------------------------------------------------------------------
	// Init Legs/Arms mesh
	//----------------------------------------------------------------------------------------------------------------------
	float radius_bicep		= 1.0f;
	float radius_foreArm 	= 0.65f;
	float radius_foreArm2 	= 0.55f;
	float radius_wrist		= 0.25f;

	//----------------------------------------------------------------------------------------------------------------------
	// Right Legs (jagged mesh)
	//----------------------------------------------------------------------------------------------------------------------
	// Right Leg 1
	m_rightLeg1_2BIK->m_firstJoint->	InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength1, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_skin  );	// Hip to knee
	m_rightLeg1_2BIK->m_jointList[1]->  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength1, 0.0f, 0.0f), radius_foreArm2, radius_wrist, tint_skin );	// Knee to Ankle
	// Right Leg 2
	m_rightLeg2_2BIK->m_firstJoint->	InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength2, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_skin  );	// Hip to knee
	m_rightLeg2_2BIK->m_jointList[1]->  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength2, 0.0f, 0.0f), radius_foreArm2, radius_wrist, tint_skin );	// Knee to Ankle
	// Right Leg 3
	m_rightLeg3_2BIK->m_firstJoint->	InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength3, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_skin  );	// Hip to knee
	m_rightLeg3_2BIK->m_jointList[1]->  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength3, 0.0f, 0.0f), radius_foreArm2, radius_wrist, tint_skin );	// Knee to Ankle
	// Right leg 4
	m_rightLeg4_2BIK->m_firstJoint->  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength4, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_skin  );	// Hip to knee
	m_rightLeg4_2BIK->m_jointList[1]->InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength4, 0.0f, 0.0f), radius_foreArm2, radius_wrist, tint_skin );	// Knee to Ankle

	//----------------------------------------------------------------------------------------------------------------------
	// Right Legs (smooth mesh)
	//----------------------------------------------------------------------------------------------------------------------
	// Right Leg 1
// 	m_rightLeg1_2BIK->m_firstJoint->	InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength1, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_brown );	// Hip to knee
// 	m_rightLeg1_2BIK->m_jointList[1]->  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength1, 0.0f, 0.0f), radius_foreArm, radius_wrist, tint_brown );	// Knee to Ankle
// 	// Right Leg 2
// 	m_rightLeg2_2BIK->m_firstJoint->	InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength2, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_brown );	// Hip to knee
// 	m_rightLeg2_2BIK->m_jointList[1]->  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength2, 0.0f, 0.0f), radius_foreArm, radius_wrist, tint_brown );	// Knee to Ankle
// 	// Right Leg 3
// 	m_rightLeg3_2BIK->m_firstJoint->	InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength3, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_brown );	// Hip to knee
// 	m_rightLeg3_2BIK->m_jointList[1]->  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength3, 0.0f, 0.0f), radius_foreArm, radius_wrist, tint_brown );	// Knee to Ankle
// 	// Right leg 4
// 	m_rightLeg4_2BIK->m_firstJoint->  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength4, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_brown );		// Hip to knee
// 	m_rightLeg4_2BIK->m_jointList[1]->InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength4, 0.0f, 0.0f), radius_foreArm, radius_wrist, tint_brown );		// Knee to Ankle

	//----------------------------------------------------------------------------------------------------------------------
	// Left Legs (jagged mesh)
	//----------------------------------------------------------------------------------------------------------------------
	// Left Leg 1  	 
 	m_leftLeg1_2BIK->m_firstJoint	 ->InitMesh_CylinderTapered( Vec3::ZERO, Vec3( m_legLength1, 0.0f, 0.0f ), radius_bicep, radius_foreArm, tint_skin );	// Hip to knee
 	m_leftLeg1_2BIK->m_jointList[ 1 ]->InitMesh_CylinderTapered( Vec3::ZERO, Vec3( m_legLength1, 0.0f, 0.0f ), radius_foreArm2, radius_wrist, tint_skin );	// Knee to Ankle
 	// Left Leg 2
 	m_leftLeg2_2BIK->m_firstJoint->	  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength2, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_skin );		// Hip to knee
 	m_leftLeg2_2BIK->m_jointList[1]-> InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength2, 0.0f, 0.0f), radius_foreArm2, radius_wrist, tint_skin );	// Knee to Ankle
 	// Left Leg 3
 	m_leftLeg3_2BIK->m_firstJoint->	  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength3, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_skin );		// Hip to knee
 	m_leftLeg3_2BIK->m_jointList[1]-> InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength3, 0.0f, 0.0f), radius_foreArm2, radius_wrist, tint_skin );	// Knee to Ankle
 	// Left leg 4
 	m_leftLeg4_2BIK->m_firstJoint->	  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength4, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_skin );		// Hip to knee
 	m_leftLeg4_2BIK->m_jointList[1]-> InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength4, 0.0f, 0.0f), radius_foreArm2, radius_wrist, tint_skin );	// Knee to Ankle


	//----------------------------------------------------------------------------------------------------------------------
	// Left Legs (smooth mesh)
	//----------------------------------------------------------------------------------------------------------------------
	// Left Leg 1

	// Realistic version with multiple joints
//    	float startRadius	= radius_bicep;
//    	float scalar		= 0.2f;
//    	float endRadius		= radius_bicep - scalar;
	// Bicep
//   	m_leftLeg1_2BIK->m_firstJoint->	  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_leg1_segment1, 0.0f, 0.0f), startRadius, endRadius, tint_brown );	// Hip to knee
//   	 // Forearm
//   	startRadius	= endRadius;
//   	endRadius	= startRadius - scalar;
//   	m_leftLeg1_2BIK->m_jointList[1]-> InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_leg1_segment2, 0.0f, 0.0f), startRadius, endRadius, tint_brown );	// Knee to Ankle
//   	 
//   	startRadius	= endRadius;
//   	endRadius	= startRadius - scalar;
//   	m_leftLeg1_2BIK->m_jointList[2]-> InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_leg1_segment3, 0.0f, 0.0f), startRadius, endRadius, tint_brown );	// Knee to Ankle
//   	 
//   	startRadius	= endRadius;
//   	endRadius	= startRadius - scalar;
//   	m_leftLeg1_2BIK->m_jointList[3]-> InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_leg1_segment4, 0.0f, 0.0f), startRadius, endRadius, tint_brown );	// Knee to Ankle
  	 
	// Robot version with only 2 bones
// 	m_leftLeg1_2BIK->m_firstJoint	 ->InitMesh_CylinderTapered( Vec3::ZERO, Vec3( m_legLength1, 0.0f, 0.0f ), radius_bicep, radius_foreArm, tint_brown );	// Hip to knee
// 	m_leftLeg1_2BIK->m_jointList[ 1 ]->InitMesh_CylinderTapered( Vec3::ZERO, Vec3( m_legLength1, 0.0f, 0.0f ), radius_foreArm, radius_wrist, tint_brown );	// Knee to Ankle
// 	// Left Leg 2
// 	m_leftLeg2_2BIK->m_firstJoint->	  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength2, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_brown );		// Hip to knee
// 	m_leftLeg2_2BIK->m_jointList[1]-> InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength2, 0.0f, 0.0f), radius_foreArm, radius_wrist, tint_brown );		// Knee to Ankle
// 	// Left Leg 3
// 	m_leftLeg3_2BIK->m_firstJoint->	  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength3, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_brown );		// Hip to knee
// 	m_leftLeg3_2BIK->m_jointList[1]-> InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength3, 0.0f, 0.0f), radius_foreArm, radius_wrist, tint_brown );		// Knee to Ankle
// 	// Left leg 4
// 	m_leftLeg4_2BIK->m_firstJoint->	  InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength4, 0.0f, 0.0f), radius_bicep, radius_foreArm, tint_brown );		// Hip to knee
// 	m_leftLeg4_2BIK->m_jointList[1]-> InitMesh_CylinderTapered( Vec3::ZERO, Vec3(m_legLength4, 0.0f, 0.0f), radius_foreArm, radius_wrist, tint_brown );		// Knee to Ankle

	//----------------------------------------------------------------------------------------------------------------------
	// Connect body to leg start
	//----------------------------------------------------------------------------------------------------------------------
	float m_xOffset_leg2 = 3.0f;
	float m_xOffset_leg3 = 1.0f;
	m_spine_CCD->m_finalJoint->  InitMesh_Cylinder( Vec3(-m_distBetweenLegs, 0.0f, 0.0f), Vec3(				   0.0f,  m_distPelvisToRoot, 0.0f), radius_foreArm, tint_skin );		// Head to  leftLeg1
	m_spine_CCD->m_jointList[1]->InitMesh_Cylinder( Vec3(	 m_xOffset_leg2, 0.0f, 0.0f), Vec3(   m_distBetweenLegs,  m_distPelvisToRoot, 0.0f), radius_foreArm, tint_skin );		// Head to  leftLeg2
	m_spine_CCD->m_jointList[1]->InitMesh_Cylinder( Vec3(	 m_xOffset_leg3, 0.0f, 0.0f), Vec3(				   0.0f,  m_distPelvisToRoot, 0.0f), radius_foreArm, tint_skin );		// Head to  leftLeg3
	m_spine_CCD->m_jointList[1]->InitMesh_Cylinder( Vec3(			   0.0f, 0.0f, 0.0f), Vec3(-m_leg4Start_xOffset,  m_distPelvisToRoot, 0.0f), radius_foreArm, tint_skin );		// Head to  leftLeg4

	m_spine_CCD->m_finalJoint->	 InitMesh_Cylinder( Vec3(-m_distBetweenLegs, 0.0f, 0.0f), Vec3(				   0.0f, -m_distPelvisToRoot, 0.0f), radius_foreArm, tint_skin );		// Head to rightLeg1
	m_spine_CCD->m_jointList[1]->InitMesh_Cylinder( Vec3(	 m_xOffset_leg2, 0.0f, 0.0f), Vec3(	  m_distBetweenLegs, -m_distPelvisToRoot, 0.0f), radius_foreArm, tint_skin );		// Head to rightLeg2
	m_spine_CCD->m_jointList[1]->InitMesh_Cylinder( Vec3(	 m_xOffset_leg3, 0.0f, 0.0f), Vec3(				   0.0f, -m_distPelvisToRoot, 0.0f), radius_foreArm, tint_skin );		// Head to rightLeg3
	m_spine_CCD->m_jointList[1]->InitMesh_Cylinder( Vec3(			   0.0f, 0.0f, 0.0f), Vec3(-m_leg4Start_xOffset, -m_distPelvisToRoot, 0.0f), radius_foreArm, tint_skin );		// Head to rightLeg4
}


//----------------------------------------------------------------------------------------------------------------------
void SpiderBase::InitLimbs()
{
	// Create creature Spine
	m_spine_CCD			= CreateChildChain( "m_spine_CCD", Vec3::ZERO );
	m_spine_CCD->AddNewJointToChain( Vec3( -m_distHipToRoot, 0.0f, 0.0f ), EulerAngles() );		// 0
	m_spine_CCD->AddNewJointToChain( Vec3(	  m_spineLength, 0.0f, 0.0f ), EulerAngles() );		// 1
	m_spine_CCD->AddNewJointToChain( Vec3(	  m_spineLength, 0.0f, 0.0f ), EulerAngles() );		// 2
	
	// Create creature LeftLeg1
	// Robot version with only 2 bones
	m_leftLeg1_2BIK = CreateChildChain( "m_leftLeg1_2BIK", Vec3::ZERO, m_spine_CCD->m_finalJoint );
	m_leftLeg1_2BIK->AddNewJointToChain( Vec3(		   0.0f, m_distPelvisToRoot, 0.0f ), EulerAngles() );		// 0 (Root)
	m_leftLeg1_2BIK->AddNewJointToChain( Vec3( m_legLength1,			   0.0f, 0.0f ), EulerAngles() );		// 1  
	m_leftLeg1_2BIK->AddNewJointToChain( Vec3( m_legLength1,			   0.0f, 0.0f ), EulerAngles() );		// 4 (End effector)
	// Realistic version with multiple joints
// 	m_leftLeg1_2BIK      = CreateChildChain( "m_leftLeg1_2BIK", Vec3::ZERO, m_spine_CCD->m_finalJoint );
// 	m_leftLeg1_2BIK->AddNewJointToChain( Vec3(			  0.0f, m_distPelvisToRoot, 0.0f ), EulerAngles() );		// 0 (Root)
// 	m_leftLeg1_2BIK->AddNewJointToChain( Vec3( m_leg1_segment1, 0.0f, 0.0f ), EulerAngles() );		// 1  
// 	m_leftLeg1_2BIK->AddNewJointToChain( Vec3( m_leg1_segment2, 0.0f, 0.0f ), EulerAngles() );		// 2
// 	m_leftLeg1_2BIK->AddNewJointToChain( Vec3( m_leg1_segment3, 0.0f, 0.0f ), EulerAngles() );		// 3
// 	m_leftLeg1_2BIK->AddNewJointToChain( Vec3( m_leg1_segment4, 0.0f, 0.0f ), EulerAngles() );		// 4 (End effector)

	// Create creature LeftLeg2
	m_leftLeg2_2BIK      = CreateChildChain( "m_leftLeg2_2BIK", Vec3::ZERO, m_spine_CCD->m_jointList[1] );
	m_leftLeg2_2BIK->AddNewJointToChain( Vec3( m_distBetweenLegs, m_distPelvisToRoot, 0.0f ), EulerAngles() );
	m_leftLeg2_2BIK->AddNewJointToChain( Vec3(		m_legLength2,				0.0f, 0.0f ), EulerAngles() );
	m_leftLeg2_2BIK->AddNewJointToChain( Vec3(		m_legLength2,				0.0f, 0.0f ), EulerAngles() );
	// Create creature Left leg 3
	m_leftLeg3_2BIK      = CreateChildChain( "m_leftleg3_2BIK", Vec3::ZERO, m_spine_CCD->m_jointList[1] );
	m_leftLeg3_2BIK->AddNewJointToChain( Vec3(				 0.0f, m_distPelvisToRoot, 0.0f ), EulerAngles() );
	m_leftLeg3_2BIK->AddNewJointToChain( Vec3(		 m_legLength3,			     0.0f, 0.0f ), EulerAngles() );
	m_leftLeg3_2BIK->AddNewJointToChain( Vec3(		 m_legLength3,			     0.0f, 0.0f ), EulerAngles() );
	// Create creature Left leg 4
	m_leftLeg4_2BIK     = CreateChildChain( "m_leftLeg4_2BIK", Vec3::ZERO, m_spine_CCD->m_jointList[1] );
	m_leftLeg4_2BIK->AddNewJointToChain( Vec3( -m_leg4Start_xOffset, m_distPelvisToRoot, 0.0f ), EulerAngles() );
	m_leftLeg4_2BIK->AddNewJointToChain( Vec3(		   m_legLength4,			   0.0f, 0.0f ), EulerAngles() );
	m_leftLeg4_2BIK->AddNewJointToChain( Vec3(		   m_legLength4,			   0.0f, 0.0f ), EulerAngles() );

	// Create creature RightLeg 1
	m_rightLeg1_2BIK     = CreateChildChain( "m_rightLeg1_2BIK", Vec3::ZERO, m_spine_CCD->m_finalJoint );
	m_rightLeg1_2BIK->AddNewJointToChain( Vec3(		    0.0f, -m_distPelvisToRoot, 0.0f ), EulerAngles() );
	m_rightLeg1_2BIK->AddNewJointToChain( Vec3( m_legLength1,			     0.0f, 0.0f ), EulerAngles() );
	m_rightLeg1_2BIK->AddNewJointToChain( Vec3( m_legLength1,			     0.0f, 0.0f ), EulerAngles() );
	// Create creature RightLeg 2
	m_rightLeg2_2BIK     = CreateChildChain( "m_rightLeg2_2BIK", Vec3::ZERO, m_spine_CCD->m_jointList[1] );
	m_rightLeg2_2BIK->AddNewJointToChain( Vec3(	m_distBetweenLegs, -m_distPelvisToRoot, 0.0f ), EulerAngles() );
	m_rightLeg2_2BIK->AddNewJointToChain( Vec3(		 m_legLength2,				  0.0f, 0.0f ), EulerAngles() );
	m_rightLeg2_2BIK->AddNewJointToChain( Vec3(		 m_legLength2,				  0.0f, 0.0f ), EulerAngles() );
	// Create creature RightLeg 3
	m_rightLeg3_2BIK     = CreateChildChain( "m_rightLeg3_2BIK", Vec3::ZERO, m_spine_CCD->m_jointList[1] );
	m_rightLeg3_2BIK->AddNewJointToChain( Vec3(			   0.0f, -m_distPelvisToRoot, 0.0f ), EulerAngles() );
	m_rightLeg3_2BIK->AddNewJointToChain( Vec3(	   m_legLength3,				0.0f, 0.0f ), EulerAngles() );
	m_rightLeg3_2BIK->AddNewJointToChain( Vec3(	   m_legLength3,				0.0f, 0.0f ), EulerAngles() );
	// Create creature Right leg 4
	m_rightLeg4_2BIK    = CreateChildChain( "m_rightLeg4_2BIK", Vec3::ZERO, m_spine_CCD->m_jointList[1] );
	m_rightLeg4_2BIK->AddNewJointToChain( Vec3( -m_leg4Start_xOffset, -m_distPelvisToRoot, 0.0f ), EulerAngles() );
	m_rightLeg4_2BIK->AddNewJointToChain( Vec3(		    m_legLength4,				 0.0f, 0.0f ), EulerAngles() );
	m_rightLeg4_2BIK->AddNewJointToChain( Vec3(		    m_legLength4,				 0.0f, 0.0f ), EulerAngles() );

	// Create creature Neck
	m_neck_FABRIK	    = CreateChildChain( "m_neck_FABRIK", Vec3::ZERO, m_spine_CCD->m_finalJoint );
	m_neck_FABRIK->AddNewJointToChain( Vec3(		 0.0f, 0.0f, 0.0f ), EulerAngles() );
	m_neck_FABRIK->AddNewJointToChain( Vec3( m_neckLength, 0.0f, 0.0f ), EulerAngles() );
	// Create creature Tail
// 	m_tail_CCD		    = CreateChildChain( "m_tail_CCD", Vec3::ZERO );
// 	m_tail_CCD->AddNewJointToChain( Vec3( -m_distHipToRoot, 0.0f, 0.0f ), EulerAngles( 90.0f, 0.0f, 0.0f ) );
// 	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 0
// 	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 1
// 	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 2
// 	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 3
// 	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 4
// 	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 5
// 	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 6
// 	m_tail_CCD->AddNewJointToChain( Vec3(	  m_tailLength, 0.0f, 0.0f ), EulerAngles() );		// 7

	// Set render mode, euler goal NOT cur
	m_leftLeg1_2BIK->	m_lerpEulerToGoal	= false;
	m_leftLeg2_2BIK->	m_lerpEulerToGoal	= true;
	m_leftLeg3_2BIK->	m_lerpEulerToGoal	= true;
	m_leftLeg4_2BIK->	m_lerpEulerToGoal	= true;
	m_rightLeg1_2BIK->	m_lerpEulerToGoal	= true;
	m_rightLeg2_2BIK->	m_lerpEulerToGoal	= true;
	m_rightLeg3_2BIK->	m_lerpEulerToGoal	= true;
	m_rightLeg4_2BIK->	m_lerpEulerToGoal	= true;
	m_neck_FABRIK->		m_lerpEulerToGoal	= false;

	// Set SolverTypes
	m_leftLeg1_2BIK		->SetSolverType( CHAIN_SOLVER_2BONE  );
	m_leftLeg2_2BIK		->SetSolverType( CHAIN_SOLVER_2BONE	 );
	m_leftLeg3_2BIK		->SetSolverType( CHAIN_SOLVER_2BONE	 );
	m_leftLeg4_2BIK		->SetSolverType( CHAIN_SOLVER_2BONE  );
	m_rightLeg1_2BIK	->SetSolverType( CHAIN_SOLVER_2BONE	 );
	m_rightLeg2_2BIK	->SetSolverType( CHAIN_SOLVER_2BONE	 );
	m_rightLeg3_2BIK	->SetSolverType( CHAIN_SOLVER_2BONE	 );
	m_rightLeg4_2BIK	->SetSolverType( CHAIN_SOLVER_2BONE  );
	m_neck_FABRIK		->SetSolverType( CHAIN_SOLVER_FABRIK );
	m_spine_CCD			->SetSolverType( CHAIN_SOLVER_CCD	 );
//	m_tail_CCD			->SetSolverType( CHAIN_SOLVER_CCD	 );
	// Set Target Current positions
	m_leftLeg4_2BIK		->m_target.m_currentPos_WS = Vec3(   0.0f, 0.0f, -40.0f );
	m_rightLeg4_2BIK	->m_target.m_currentPos_WS = Vec3(   0.0f, 0.0f, -40.0f );
	m_spine_CCD			->m_target.m_currentPos_WS = Vec3(  40.0f, 0.0f,   0.0f );
	m_leftLeg1_2BIK		->m_target.m_currentPos_WS = Vec3(	 0.0f, 0.0f, -40.0f );
	m_rightLeg1_2BIK	->m_target.m_currentPos_WS = Vec3(	 0.0f, 0.0f, -40.0f );
//	m_tail_CCD			->m_target.m_currentPos_WS = Vec3( -80.0f, 0.0f,   0.0f );
	m_neck_FABRIK		->m_target.m_currentPos_WS = Vec3(  50.0f, 0.0f,   0.0f );
	// Set Target Goal positions
	m_leftLeg4_2BIK		->m_target.m_goalPos_WS = Vec3(   0.0f, 0.0f, -40.0f );
	m_rightLeg4_2BIK	->m_target.m_goalPos_WS = Vec3(   0.0f, 0.0f, -40.0f );
	m_spine_CCD			->m_target.m_goalPos_WS = Vec3(  40.0f, 0.0f,   0.0f );
	m_leftLeg1_2BIK		->m_target.m_goalPos_WS = Vec3(   0.0f, 0.0f, -40.0f );
	m_rightLeg1_2BIK	->m_target.m_goalPos_WS = Vec3(   0.0f, 0.0f, -40.0f );
//	m_tail_CCD			->m_target.m_goalPos_WS = Vec3( -40.0f, 0.0f,   0.0f );
	m_neck_FABRIK		->m_target.m_goalPos_WS = Vec3(  50.0f, 0.0f,   0.0f );
	// Set actual constraints parameters
	SetChainConstraints( m_leftLeg1_2BIK	 );
//	SetChainConstraints( m_rightFoot_FABRIK	 );
	SetChainConstraints( m_spine_CCD		 );
//	SetChainConstraints( m_leftArm_FABRIK	 );
//	SetChainConstraints( m_rightArm_FABRIK	 );
//	SetChainConstraints( m_tail_CCD			 );
	SetChainConstraints( m_neck_FABRIK		 );
	// Angle constraints to prevent neck/head "bending too far"
	m_neck_FABRIK->m_finalJoint->SetConstraints_YPR( FloatRange( -75.0f, 75.0f ), FloatRange( -35.0f, 35.0f ), FloatRange( -0.0f, 0.0f ) );

	//----------------------------------------------------------------------------------------------------------------------
	// Set anchor states
	//----------------------------------------------------------------------------------------------------------------------
 	m_leftLeg1_2BIK	->SetAnchor_Free();
 	m_leftLeg2_2BIK	->SetAnchor_Free();
 	m_leftLeg3_2BIK	->SetAnchor_Free();
	m_leftLeg4_2BIK	->SetAnchor_Free();
	m_rightLeg1_2BIK->SetAnchor_Free();
	m_rightLeg2_2BIK->SetAnchor_Free();
	m_rightLeg3_2BIK->SetAnchor_Free();
	m_rightLeg4_2BIK->SetAnchor_Free();

	//----------------------------------------------------------------------------------------------------------------------
	// Set chain lerp speed
	//----------------------------------------------------------------------------------------------------------------------
	m_leftLeg1_2BIK		->m_turnRate = 22500.0f;	// 437.0f;		// 994.0f;		// 2812.0f;
	m_leftLeg2_2BIK		->m_turnRate = 22500.0f;
	m_leftLeg3_2BIK		->m_turnRate = 22500.0f;
	m_leftLeg4_2BIK		->m_turnRate = 22500.0f;
	m_rightLeg1_2BIK	->m_turnRate = 22500.0f;
	m_rightLeg2_2BIK	->m_turnRate = 22500.0f;
	m_rightLeg3_2BIK	->m_turnRate = 22500.0f;
	m_rightLeg4_2BIK	->m_turnRate = 22500.0f;
	m_spine_CCD			->m_turnRate = 22500.0f;
//	m_tail_CCD			->m_turnRate = 22500.0f;
 	m_neck_FABRIK		->m_turnRate = 180.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Set MatrixToEuler preference
	//----------------------------------------------------------------------------------------------------------------------
	m_leftLeg1_2BIK	->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
	m_leftLeg2_2BIK	->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
	m_leftLeg3_2BIK	->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
	m_leftLeg4_2BIK	->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
	m_rightLeg1_2BIK->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
	m_rightLeg2_2BIK->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
	m_rightLeg3_2BIK->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
	m_rightLeg4_2BIK->SetChainPreference_MatrixToEuler( PITCH_PREFERRED );
//	m_neck_FABRIK	->SetChainPreference_MatrixToEuler( YAW_PREFERRED );
	m_spine_CCD		->SetChainPreference_MatrixToEuler( YAW_PREFERRED );
//	m_tail_CCD		->SetSolverType( CHAIN_SOLVER_CCD	 );


	// Set iterations
	m_leftLeg1_2BIK->m_numIterations = 100;


	//----------------------------------------------------------------------------------------------------------------------
	// Set Joint Mesh(s)
	//----------------------------------------------------------------------------------------------------------------------
	InitJointMesh();

	UpdatePoleVector();
}


//----------------------------------------------------------------------------------------------------------------------
void SpiderBase::SetChainConstraints( IK_Chain3D* const currentChain )
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

	if ( g_debugToggleConstraints_F8 )
	{
 		for ( int i = 0; i < currentChain->m_jointList.size(); i++ )
 		{
 			IK_Joint3D* currentSegment = currentChain->m_jointList[ i ];
// 			currentSegment->SetConstraints_YPR( FloatRange( -75.0f, 75.0f ), FloatRange( -75.0f, 75.0f ), FloatRange( -0.0f, 0.0f ) );			// 75 degree cone constraint
 			currentSegment->SetConstraints_YPR( FloatRange( -180.0f, 180.0f ), FloatRange( -180.0f, 180.0f ), FloatRange( -0.0f, 0.0f ) );		// Unconstrained
 		}
	}
	else
	{
//  		m_leftLeg1_2BIK->m_jointList[0]->SetConstraints_YPR( FloatRange(-45.0f, 135.0f), FloatRange(-55.0f, -45.0f), FloatRange(-0.0f, -0.0f) );
// 		m_leftLeg1_2BIK->m_jointList[1]->SetConstraints_YPR( FloatRange( -0.0f,   0.0f), FloatRange(  0.0f, 135.0f), FloatRange(-0.0f, -0.0f) );
// 		m_leftLeg1_2BIK->m_jointList[2]->SetConstraints_YPR( FloatRange( -0.0f,   0.0f), FloatRange(  0.0f, 135.0f), FloatRange(-0.0f, -0.0f) );
// 		m_leftLeg1_2BIK->m_jointList[3]->SetConstraints_YPR( FloatRange( -0.0f,   0.0f), FloatRange( 45.0f,  75.0f), FloatRange(-0.0f, -0.0f) );
// 		m_leftLeg1_2BIK->m_jointList[4]->SetConstraints_YPR( FloatRange( -0.0f,   0.0f), FloatRange(  0.0f,   0.0f), FloatRange(-0.0f, -0.0f) );
	}
}


//----------------------------------------------------------------------------------------------------------------------
// Note: This function pre-updates all skeletal relevant game-logic 
// before the IK chains actually solve for targets
//----------------------------------------------------------------------------------------------------------------------
void SpiderBase::PreUpdateChains( float deltaSeconds )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Update SpiderBase Hip EndEffector ("keep spine pointing hip fwd")
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 rootFwd							= m_modelRoot->m_eulerAnglesGoal_LS.GetForwardDir_XFwd_YLeft_ZUp();
	float fwdDist							= m_spineLength * m_spine_CCD->m_jointList.size();
	m_spine_CCD->m_target.m_currentPos_WS	= m_modelRoot->m_jointPos_MS + ( rootFwd * fwdDist );


	//----------------------------------------------------------------------------------------------------------------------
	// Update pole vectors 
	//----------------------------------------------------------------------------------------------------------------------
	UpdatePoleVector();

	//----------------------------------------------------------------------------------------------------------------------
	// New walk logic
	// Update order for anchor toggle states: LeftFoot, RightFoot, leftArm, RightArm
	//----------------------------------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------------------------------------
	// LEFT LEG 1, REFACTORED
	//----------------------------------------------------------------------------------------------------------------------
	ComputeIdealStepPosition( m_leftLeg1_2BIK, m_modelRoot->m_eulerAnglesGoal_LS, m_modelRoot->m_jointPos_MS, +m_fwdNess1, +m_leftNess1, -m_defaultHeightZ );	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	bool canMove					= TryFreeAnchor( m_leftLeg1_2BIK, m_rightLeg1_2BIK );																		// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	bool shouldStep					= ShouldChainStep( m_leftLeg1_2BIK, canMove, m_maxDistFromIdealPos_XY, m_maxDistFromIdealPos_Z );							// 3. Check if each chain should step
	TryInitBezierTimer( canMove, shouldStep, m_bezierTimer_leftFoot1, m_leftLeg1_2BIK, m_rightLeg1_2BIK );														// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	bool doesRaycastHitFloor_CurPos = DoesRayHitFloor_CurPos( m_leftLeg1_2BIK, m_raycast_CurPos_LeftFoot1 );													// 5a. Raycast against floor (Cur Pos)
	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_leftLeg1_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_LeftFoot1 );									// 5b. Raycast against floor (Ideal Pos)
	UpdateIdealStepPosition( doesRaycastHitFloor_CurPos, m_leftLeg1_2BIK, m_rightLeg1_2BIK, m_bezierTimer_leftFoot1 );											// 5c. Update ideal pos if better than prev step
	UpdateBezierCurvePositions( m_bezier_leftFoot1, m_leftLeg1_2BIK, Vec3::Z_UP, m_bezierTimer_leftFoot1 );														// 6. Update bezier points for each chain's foot height
	UpdateBezierTimer( m_bezier_leftFoot1, m_leftLeg1_2BIK, m_rightLeg1_2BIK, m_bezierTimer_leftFoot1 );														// 7. Check if Bezier should be stopped

	//----------------------------------------------------------------------------------------------------------------------
	// LEFT LEG 3, REFACTORED
	//----------------------------------------------------------------------------------------------------------------------
	ComputeIdealStepPosition( m_leftLeg3_2BIK, m_modelRoot->m_eulerAnglesGoal_LS, m_modelRoot->m_jointPos_MS, +m_fwdNess3, +m_leftNess3, -m_defaultHeightZ );	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	canMove							= TryFreeAnchor( m_leftLeg3_2BIK, m_rightLeg3_2BIK );																		// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	shouldStep						= ShouldChainStep( m_leftLeg3_2BIK, canMove, m_maxDistFromIdealPos_XY, m_maxDistFromIdealPos_Z );							// 3. Check if each chain should step
	TryInitBezierTimer( canMove, shouldStep, m_bezierTimer_leftFoot3, m_leftLeg3_2BIK, m_rightLeg3_2BIK );														// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	doesRaycastHitFloor_CurPos		= DoesRayHitFloor_CurPos( m_leftLeg3_2BIK, m_raycast_CurPos_LeftFoot3 );													// 5a. Raycast against floor (Cur Pos)
	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_leftLeg3_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_LeftFoot3 );									// 5b. Raycast against floor (Ideal Pos)
	UpdateIdealStepPosition( doesRaycastHitFloor_CurPos, m_leftLeg3_2BIK, m_rightLeg3_2BIK, m_bezierTimer_leftFoot3  );											// 5c. Update ideal pos if better than prev step
	UpdateBezierCurvePositions( m_bezier_leftFoot3, m_leftLeg3_2BIK, Vec3::Z_UP, m_bezierTimer_leftFoot3 );														// 6. Update bezier points for each chain's foot height
	UpdateBezierTimer( m_bezier_leftFoot3, m_leftLeg3_2BIK, m_rightLeg3_2BIK, m_bezierTimer_leftFoot3 );														// 7. Check if Bezier should be stopped

	//----------------------------------------------------------------------------------------------------------------------
	// LEFT LEG 2, REFACTORED
	//----------------------------------------------------------------------------------------------------------------------
	ComputeIdealStepPosition( m_leftLeg2_2BIK, m_modelRoot->m_eulerAnglesGoal_LS, m_modelRoot->m_jointPos_MS, +m_fwdNess2, +m_leftNess2, -m_defaultHeightZ );	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	canMove							= TryFreeAnchor( m_leftLeg2_2BIK, m_rightLeg2_2BIK );																		// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	shouldStep						= ShouldChainStep( m_leftLeg2_2BIK, canMove, m_maxDistFromIdealPos_XY, m_maxDistFromIdealPos_Z );							// 3. Check if each chain should step
	TryInitBezierTimer( canMove, shouldStep, m_bezierTimer_leftFoot2, m_leftLeg2_2BIK, m_rightLeg2_2BIK );														// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	doesRaycastHitFloor_CurPos		= DoesRayHitFloor_CurPos( m_leftLeg2_2BIK, m_raycast_CurPos_LeftFoot2 );													// 5a. Raycast against floor (Cur Pos)
	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_leftLeg2_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_LeftFoot2 );									// 5b. Raycast against floor (Ideal Pos)
	UpdateIdealStepPosition( doesRaycastHitFloor_CurPos, m_leftLeg2_2BIK, m_rightLeg2_2BIK, m_bezierTimer_leftFoot2  );											// 5c. Update ideal pos if better than prev step
	UpdateBezierCurvePositions( m_bezier_leftFoot2, m_leftLeg2_2BIK, Vec3::Z_UP, m_bezierTimer_leftFoot2 );														// 6. Update bezier points for each chain's foot height
	UpdateBezierTimer( m_bezier_leftFoot2, m_leftLeg2_2BIK, m_rightLeg2_2BIK, m_bezierTimer_leftFoot2 );														// 7. Check if Bezier should be stopped

	//----------------------------------------------------------------------------------------------------------------------
	// LEFT LEG 4, REFACTORED
	//----------------------------------------------------------------------------------------------------------------------
	ComputeIdealStepPosition( m_leftLeg4_2BIK, m_modelRoot->m_eulerAnglesGoal_LS, m_modelRoot->m_jointPos_MS, +m_fwdNess4, +m_leftNess4, -m_defaultHeightZ );	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	canMove							= TryFreeAnchor( m_leftLeg4_2BIK, m_rightLeg4_2BIK );																		// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	shouldStep						= ShouldChainStep( m_leftLeg4_2BIK, canMove, m_maxDistFromIdealPos_XY, m_maxDistFromIdealPos_Z );							// 3. Check if each chain should step
	TryInitBezierTimer( canMove, shouldStep, m_bezierTimer_leftFoot4, m_leftLeg4_2BIK, m_rightLeg4_2BIK );														// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	doesRaycastHitFloor_CurPos		= DoesRayHitFloor_CurPos( m_leftLeg4_2BIK, m_raycast_CurPos_LeftFoot4 );													// 5a. Raycast against floor (Cur Pos)
	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_leftLeg4_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_LeftFoot4 );									// 5b. Raycast against floor (Ideal Pos)
	UpdateIdealStepPosition( doesRaycastHitFloor_CurPos, m_leftLeg4_2BIK, m_rightLeg4_2BIK, m_bezierTimer_leftFoot4  );											// 5c. Update ideal pos if better than prev step
	UpdateBezierCurvePositions( m_bezier_leftFoot4, m_leftLeg4_2BIK, Vec3::Z_UP, m_bezierTimer_leftFoot4 );														// 6. Update bezier points for each chain's foot height
	UpdateBezierTimer( m_bezier_leftFoot4, m_leftLeg4_2BIK, m_rightLeg1_2BIK, m_bezierTimer_leftFoot4 );														// 7. Check if Bezier should be stopped
	
	//----------------------------------------------------------------------------------------------------------------------
	// RIGHT LEG 1, REFACTORED
	//----------------------------------------------------------------------------------------------------------------------
	ComputeIdealStepPosition( m_rightLeg1_2BIK, m_modelRoot->m_eulerAnglesGoal_LS, m_modelRoot->m_jointPos_MS, +m_fwdNess1, -m_leftNess1, -m_defaultHeightZ );	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	canMove							= TryFreeAnchor( m_rightLeg1_2BIK, m_leftLeg1_2BIK );																		// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	shouldStep						= ShouldChainStep( m_rightLeg1_2BIK, canMove, m_maxDistFromIdealPos_XY, m_maxDistFromIdealPos_Z );							// 3. Check if each chain should step
	TryInitBezierTimer( canMove, shouldStep, m_bezierTimer_rightFoot1, m_rightLeg1_2BIK, m_leftLeg1_2BIK );														// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	doesRaycastHitFloor_CurPos		= DoesRayHitFloor_CurPos( m_rightLeg1_2BIK, m_raycast_CurPos_RightFoot1 );													// 5a. Raycast against floor (Cur Pos)
	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_rightLeg1_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_RightFoot1 );								// 5b. Raycast against floor (Ideal Pos)
	UpdateIdealStepPosition( doesRaycastHitFloor_CurPos, m_rightLeg1_2BIK, m_leftLeg1_2BIK, m_bezierTimer_rightFoot1 );											// 5c. Update ideal pos if better than prev step
	UpdateBezierCurvePositions( m_bezier_rightFoot1, m_rightLeg1_2BIK, Vec3::Z_UP, m_bezierTimer_rightFoot1 );													// 6. Update bezier points for each chain's foot height
	UpdateBezierTimer( m_bezier_rightFoot1, m_rightLeg1_2BIK, m_leftLeg1_2BIK, m_bezierTimer_rightFoot1 );														// 7. Check if Bezier should be stopped

	//----------------------------------------------------------------------------------------------------------------------
	// RIGHT LEG 3, REFACTORED
	//----------------------------------------------------------------------------------------------------------------------
	ComputeIdealStepPosition( m_rightLeg3_2BIK, m_modelRoot->m_eulerAnglesGoal_LS, m_modelRoot->m_jointPos_MS, +m_fwdNess3, -m_leftNess3, -m_defaultHeightZ );	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	canMove							= TryFreeAnchor( m_rightLeg3_2BIK, m_leftLeg3_2BIK );																		// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	shouldStep						= ShouldChainStep( m_rightLeg3_2BIK, canMove, m_maxDistFromIdealPos_XY, m_maxDistFromIdealPos_Z );							// 3. Check if each chain should step
	TryInitBezierTimer( canMove, shouldStep, m_bezierTimer_rightFoot3, m_rightLeg3_2BIK, m_leftLeg3_2BIK );														// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	doesRaycastHitFloor_CurPos		= DoesRayHitFloor_CurPos( m_rightLeg3_2BIK, m_raycast_CurPos_RightFoot3 );													// 5a. Raycast against floor (Cur Pos)
	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_rightLeg3_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_RightFoot3 );								// 5b. Raycast against floor (Ideal Pos)
	UpdateIdealStepPosition( doesRaycastHitFloor_CurPos, m_rightLeg3_2BIK, m_leftLeg3_2BIK, m_bezierTimer_rightFoot3  );										// 5c. Update ideal pos if better than prev step
	UpdateBezierCurvePositions( m_bezier_rightFoot3, m_rightLeg3_2BIK, Vec3::Z_UP, m_bezierTimer_rightFoot3 );													// 6. Update bezier points for each chain's foot height
	UpdateBezierTimer( m_bezier_rightFoot3, m_rightLeg3_2BIK, m_leftLeg3_2BIK, m_bezierTimer_rightFoot3 );														// 7. Check if Bezier should be stopped

	//----------------------------------------------------------------------------------------------------------------------
	// RIGHT LEG 2, REFACTORED
	//----------------------------------------------------------------------------------------------------------------------
	ComputeIdealStepPosition( m_rightLeg2_2BIK, m_modelRoot->m_eulerAnglesGoal_LS, m_modelRoot->m_jointPos_MS, +m_fwdNess2, -m_leftNess2, -m_defaultHeightZ );	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	canMove							= TryFreeAnchor( m_rightLeg2_2BIK, m_leftLeg2_2BIK );																		// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	shouldStep						= ShouldChainStep( m_rightLeg2_2BIK, canMove, m_maxDistFromIdealPos_XY, m_maxDistFromIdealPos_Z );							// 3. Check if each chain should step
	TryInitBezierTimer( canMove, shouldStep, m_bezierTimer_rightFoot2, m_rightLeg2_2BIK, m_leftLeg2_2BIK );														// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	doesRaycastHitFloor_CurPos		= DoesRayHitFloor_CurPos( m_rightLeg2_2BIK, m_raycast_CurPos_RightFoot2 );													// 5a. Raycast against floor (Cur Pos)
	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_rightLeg2_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_RightFoot2 );								// 5b. Raycast against floor (Ideal Pos)
	UpdateIdealStepPosition( doesRaycastHitFloor_CurPos, m_rightLeg2_2BIK, m_leftLeg2_2BIK, m_bezierTimer_rightFoot2 );											// 5c. Update ideal pos if better than prev step
	UpdateBezierCurvePositions( m_bezier_rightFoot2, m_rightLeg2_2BIK, Vec3::Z_UP, m_bezierTimer_rightFoot2 );													// 6. Update bezier points for each chain's foot height
	UpdateBezierTimer( m_bezier_rightFoot2, m_rightLeg2_2BIK, m_leftLeg2_2BIK, m_bezierTimer_rightFoot2 );														// 7. Check if Bezier should be stopped

	//----------------------------------------------------------------------------------------------------------------------
	// RIGHT LEG 4, REFACTORED
	//----------------------------------------------------------------------------------------------------------------------
	ComputeIdealStepPosition( m_rightLeg4_2BIK, m_modelRoot->m_eulerAnglesGoal_LS, m_modelRoot->m_jointPos_MS, +m_fwdNess4, -m_leftNess4, -m_defaultHeightZ );	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
	canMove							= TryFreeAnchor( m_rightLeg4_2BIK, m_leftLeg4_2BIK );																		// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
	shouldStep						= ShouldChainStep( m_rightLeg4_2BIK, canMove, m_maxDistFromIdealPos_XY, m_maxDistFromIdealPos_Z );							// 3. Check if each chain should step
	TryInitBezierTimer( canMove, shouldStep, m_bezierTimer_rightFoot4, m_rightLeg4_2BIK, m_leftLeg4_2BIK );														// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
	doesRaycastHitFloor_CurPos		= DoesRayHitFloor_CurPos( m_rightLeg4_2BIK, m_raycast_CurPos_RightFoot4 );													// 5a. Raycast against floor (Cur Pos)
	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_rightLeg4_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_RightFoot4 );								// 5b. Raycast against floor (Ideal Pos)
	UpdateIdealStepPosition( doesRaycastHitFloor_CurPos, m_rightLeg4_2BIK, m_leftLeg4_2BIK, m_bezierTimer_rightFoot4 );											// 5c. Update ideal pos if better than prev step
	UpdateBezierCurvePositions( m_bezier_rightFoot4, m_rightLeg4_2BIK, Vec3::Z_UP, m_bezierTimer_rightFoot4 );													// 6. Update bezier points for each chain's foot height
	UpdateBezierTimer( m_bezier_rightFoot4, m_rightLeg4_2BIK, m_leftLeg4_2BIK, m_bezierTimer_rightFoot4 );														// 7. Check if Bezier should be stopped

// 	//----------------------------------------------------------------------------------------------------------------------
// 	// LEFT LEG 4
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
// 	float leftNess = 4.0f;
// 	Vec3 fwd, left, up;		
// 	m_modelRoot->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( fwd, left, up );
// 	Vec3 fwdOffset							= ( fwd  * m_distHipToRoot					);
// 	Vec3 leftOffset							= ( left * (m_distPelvisToRoot * leftNess)  );
// 	Vec3 upOffset							= ( up	 * m_defaultHeightZ					);
// 	m_leftLeg4_2BIK->m_idealStepPos_WS	= m_modelRoot->m_jointPos_MS - fwdOffset + leftOffset - upOffset; 
// 	// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
// 	bool canMove = m_leftLeg4_2BIK->CanMove();
// 	if ( !canMove )
// 	{
// 		// 2b. Try freeing anchor if CAN'T move
// 		canMove = m_leftLeg4_2BIK->TryFreeAnchor( m_rightLeg1_2BIK );
// 	}
// 	// 3. Check if each chain should step
// 	bool shouldStep = false;
// 	if ( canMove )
// 	{
// 		Mat44 localToWorldMatrix	= m_leftLeg4_2BIK->m_finalJoint->GetMatrix_LocalToWorld();
// 		Vec3  leftFootEE_WS			= localToWorldMatrix.GetTranslation3D();
// 		shouldStep					= IsIdealPosTooFar( m_leftLeg4_2BIK->m_idealStepPos_WS, leftFootEE_WS, m_maxDistFromIdealPos_XY, m_maxDistFromIdealPos_Z );
// 	}
// 	// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
// 	if ( canMove && shouldStep )
// 	{
// 		if ( m_bezierTimer_leftFoot.IsStopped() )
// 		{
// 			// 4a. Start Bezier timer
// 			m_bezierTimer_leftFoot.Start();
// 			// 4b. Toggle anchor states
// 			m_leftLeg4_2BIK->SetAnchor_Moving();
// 			m_rightLeg1_2BIK->SetAnchor_Locked();
// 		}
// 	}
// 	// 5a. Raycast against floor (Cur Pos)
// 	bool doesRaycastHitFloor_CurPos = DoesRaycastHitFloor( m_leftLeg4_2BIK->m_target.m_currentPos_WS, m_raycast_CurPos_LeftFoot );
// 	if ( doesRaycastHitFloor_CurPos )
// 	{
// 		// Update curPos 
// 		m_leftLeg4_2BIK->m_target.m_currentPos_WS = m_raycast_CurPos_LeftFoot.m_raycastResult.m_impactPos;
// 	}
// 	// 5b. Raycast against floor (Ideal Pos)
// 	Vec3 prevIdealPosition	= m_leftLeg4_2BIK->m_idealStepPos_WS;
// 	doesRaycastHitFloor_CurPos		= DoesRaycastHitFloor( m_leftLeg4_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_LeftFoot );
// 	if ( !doesRaycastHitFloor_CurPos )
// 	{
// 		// Keep previous ideal position?
// 		m_leftLeg4_2BIK->m_idealStepPos_WS = m_leftLeg4_2BIK->m_prevIdealPos_WS;
// 	}
// 	else
// 	{
// 		m_leftLeg4_2BIK->m_prevIdealPos_WS = m_leftLeg4_2BIK->m_idealStepPos_WS;
// 	}
// 	// 5c. Update idealPos.Z based on rayImpactPos.z
//  	m_leftLeg4_2BIK->m_target.m_goalPos_WS = m_leftLeg4_2BIK->m_idealStepPos_WS;
// 	// 6. Update bezier points for each chain's foot height
// 	UpdateBezierCurvePositions( m_bezier_leftFoot, m_leftLeg4_2BIK, Vec3::Z_UP, m_bezierTimer_leftFoot );
// 	// 7. Check if Bezier should be stopped
// 	UpdateBezierTimer( m_bezier_leftFoot, m_leftLeg4_2BIK, m_rightLeg1_2BIK, m_bezierTimer_leftFoot );
// 
// 
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// LEFT LEG 1
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
// 	fwdOffset								= ( fwd  * m_distHipToRoot					);
// 	leftOffset								= ( left * (m_distPelvisToRoot * leftNess ) );
// 	upOffset								= ( up	 * m_defaultHeightZ					);
// 	m_leftLeg1_2BIK->m_idealStepPos_WS		= m_modelRoot->m_jointPos_MS + fwdOffset + leftOffset - upOffset; 
// 	// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
// 	canMove = m_leftLeg1_2BIK->CanMove();
// 	if ( !canMove )
// 	{
// 		// 2b. Try freeing anchor if CAN'T move
// 		canMove = m_leftLeg1_2BIK->TryFreeAnchor( m_rightLeg4_2BIK );
// 	}
// 	// 3. Check if each chain should step
// 	shouldStep = false;
// 	if ( canMove )
// 	{
// 		Mat44 localToWorldMatrix	= m_leftLeg1_2BIK->m_finalJoint->GetMatrix_LocalToWorld();
// 		Vec3  leftArmEE_WS			= localToWorldMatrix.GetTranslation3D();
// 		shouldStep					= IsIdealPosTooFar( m_leftLeg1_2BIK->m_idealStepPos_WS, leftArmEE_WS, m_maxDistFromIdealPos_XY, m_maxDistFromIdealPos_Z );
// 	}
// 	// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
// 	if ( canMove && shouldStep )
// 	{
// 		if ( m_bezierTimer_leftArm.IsStopped() )
// 		{
// 			// 4a. Start Bezier timer
// 			m_bezierTimer_leftArm.Start();
// 			// 4b. Toggle anchor states
// 			m_leftLeg1_2BIK  ->SetAnchor_Moving();
// 			m_rightLeg4_2BIK->SetAnchor_Locked();
// 		}
// 	}
// 	// 5a. Raycast against floor (Cur Pos)
// 	doesRaycastHitFloor_CurPos = DoesRaycastHitFloor( m_leftLeg1_2BIK->m_target.m_currentPos_WS, m_raycast_CurPos_LeftArm );
// 	if ( doesRaycastHitFloor_CurPos )
// 	{
// 		// Update curPos 
// 		m_leftLeg1_2BIK->m_target.m_currentPos_WS = m_raycast_CurPos_LeftArm.m_raycastResult.m_impactPos;
// 	}
// 	// 5b. Raycast against floor (Ideal Pos)
// 	prevIdealPosition	 = m_leftLeg1_2BIK->m_idealStepPos_WS;
// 	doesRaycastHitFloor_CurPos	 = DoesRaycastHitFloor( m_leftLeg1_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_LeftArm );
// 	if ( !doesRaycastHitFloor_CurPos )
// 	{
// 		// Keep previous ideal position?
// 		m_leftLeg1_2BIK->m_idealStepPos_WS = m_leftLeg1_2BIK->m_prevIdealPos_WS;
// 	}
// 	else
// 	{
// 		m_leftLeg1_2BIK->m_prevIdealPos_WS = m_leftLeg1_2BIK->m_idealStepPos_WS;
// 	}
// 	// 5c. Update idealPos.Z based on rayImpactPos.z
// 	m_leftLeg1_2BIK->m_target.m_goalPos_WS = m_leftLeg1_2BIK->m_idealStepPos_WS;
// 	// 6. Update bezier points for each chain's foot height
// 	UpdateBezierCurvePositions( m_bezier_leftArm, m_leftLeg1_2BIK, Vec3::Z_UP, m_bezierTimer_leftArm );
// 	// 7. Check if Bezier should be stopped
// 	UpdateBezierTimer( m_bezier_leftArm, m_leftLeg1_2BIK, m_rightLeg4_2BIK, m_bezierTimer_leftArm );
// 
// 
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// RIGHT LEG 4
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
// 	fwdOffset								= ( fwd  * m_distHipToRoot					);
// 	leftOffset								= ( left * (m_distPelvisToRoot * leftNess ) );
// 	upOffset								= ( up	 * m_defaultHeightZ					);
// 	m_rightLeg4_2BIK->m_idealStepPos_WS	= m_modelRoot->m_jointPos_MS - fwdOffset - leftOffset - upOffset; 
// 	// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
// 	canMove = m_rightLeg4_2BIK->CanMove();
// 	if ( !canMove )
// 	{
// 		// 2b. Try freeing anchor if CAN'T move
// 		canMove = m_rightLeg4_2BIK->TryFreeAnchor( m_leftLeg1_2BIK );
// 	}
// 	// 3. Check if each chain should step
// 	shouldStep = false;
// 	if ( canMove )
// 	{
// 		Mat44 localToWorldMatrix	= m_rightLeg4_2BIK->m_finalJoint->GetMatrix_LocalToWorld();
// 		Vec3  rightFootEE_WS		= localToWorldMatrix.GetTranslation3D();
// 		shouldStep					= IsIdealPosTooFar( m_rightLeg4_2BIK->m_idealStepPos_WS, rightFootEE_WS, m_maxDistFromIdealPos_XY, m_maxDistFromIdealPos_Z );
// 	}
// 	// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
// 	if ( canMove && shouldStep )
// 	{
// 		if ( m_bezierTimer_rightFoot.IsStopped() )
// 		{
// 			// 4a. Start Bezier timer
// 			m_bezierTimer_rightFoot.Start();
// 			// 4b. Toggle anchor states
// 			m_rightLeg4_2BIK->SetAnchor_Moving();
// 			m_leftLeg1_2BIK  ->SetAnchor_Locked();
// 		}
// 	}
// 	// 5a. Raycast against floor (Cur Pos)
// 	doesRaycastHitFloor_CurPos = DoesRaycastHitFloor( m_rightLeg4_2BIK->m_target.m_currentPos_WS, m_raycast_CurPos_RightFoot );
// 	if ( doesRaycastHitFloor_CurPos )
// 	{
// 		// Update curPos 
// 		m_rightLeg4_2BIK->m_target.m_currentPos_WS = m_raycast_CurPos_RightFoot.m_raycastResult.m_impactPos;
// 	}
// 	// 5b. Raycast against floor (Ideal Pos)
// 	prevIdealPosition	 = m_rightLeg4_2BIK->m_idealStepPos_WS;
// 	doesRaycastHitFloor_CurPos	 = DoesRaycastHitFloor( m_rightLeg4_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_RightFoot );
// 	if ( !doesRaycastHitFloor_CurPos )
// 	{
// 		// Keep previous ideal position?
// 		m_rightLeg4_2BIK->m_idealStepPos_WS = m_rightLeg4_2BIK->m_prevIdealPos_WS;
// 	}
// 	else
// 	{
// 		m_rightLeg4_2BIK->m_prevIdealPos_WS = m_rightLeg4_2BIK->m_idealStepPos_WS;
// 	}
// 	// 5c. Update idealPos.Z based on rayImpactPos.z
// 	m_rightLeg4_2BIK->m_target.m_goalPos_WS = m_rightLeg4_2BIK->m_idealStepPos_WS;
// 	// 6. Update bezier points for each chain's foot height
// 	UpdateBezierCurvePositions( m_bezier_rightFoot, m_rightLeg4_2BIK, Vec3::Z_UP, m_bezierTimer_rightFoot );
// 	// 7. Check if Bezier should be stopped
// 	UpdateBezierTimer( m_bezier_rightFoot, m_rightLeg4_2BIK, m_leftLeg1_2BIK, m_bezierTimer_rightFoot );
// 
// 
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// RIGHT LEG 1
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 1. Update ideal step positions (as offsets from modelRoot or ownerJoint)
// 	fwdOffset								= ( fwd  * m_distHipToRoot					);
// 	leftOffset								= ( left * (m_distPelvisToRoot * leftNess ) );
// 	upOffset								= ( up	 * m_defaultHeightZ					);
// 	m_rightLeg1_2BIK->m_idealStepPos_WS	= m_modelRoot->m_jointPos_MS + fwdOffset - leftOffset - upOffset; 
// 	// 2a. Check if each chain canMove (false if "other" chain is moving) E.g., leftArm and rightFoot can NOT move at the same time
// 	canMove = m_rightLeg1_2BIK->CanMove();
// 	if ( !canMove )
// 	{
// 		// 2b. Try freeing anchor if CAN'T move
// 		canMove = m_rightLeg1_2BIK->TryFreeAnchor( m_leftLeg4_2BIK );
// 	}
// 	// 3. Check if each chain should step
// 	shouldStep = false;
// 	if ( canMove )
// 	{
// 		Mat44 localToWorldMatrix	= m_rightLeg1_2BIK->m_finalJoint->GetMatrix_LocalToWorld();
// 		Vec3  rightArmEE_WS			= localToWorldMatrix.GetTranslation3D();
// 		shouldStep					= IsIdealPosTooFar( m_rightLeg1_2BIK->m_idealStepPos_WS, rightArmEE_WS, m_maxDistFromIdealPos_XY, m_maxDistFromIdealPos_Z );
// 	}
// 	// 4. Init a bezierTimer ONLY IF it hasn't started AND canMove AND shouldStep 
// 	if ( canMove && shouldStep )
// 	{
// 		if ( m_bezierTimer_rightArm.IsStopped() )
// 		{
// 			// 4a. Start Bezier timer
// 			m_bezierTimer_rightArm.Start();
// 			// 4b. Toggle anchor states
// 			m_rightLeg1_2BIK->SetAnchor_Moving();
// 			m_leftLeg4_2BIK->SetAnchor_Locked();
// 		}
// 	}
// 	// 5a. Raycast against floor (Cur Pos)
// 	doesRaycastHitFloor_CurPos = DoesRaycastHitFloor( m_rightLeg1_2BIK->m_target.m_currentPos_WS, m_raycast_CurPos_RightArm );
// 	if ( doesRaycastHitFloor_CurPos )
// 	{
// 		// Update curPos 
// 		m_rightLeg1_2BIK->m_target.m_currentPos_WS = m_raycast_CurPos_RightArm.m_raycastResult.m_impactPos;
// 	}
// 	// 5b. Raycast against floor (Ideal Pos)
// 	prevIdealPosition	 = m_rightLeg1_2BIK->m_idealStepPos_WS;
// 	doesRaycastHitFloor_CurPos	 = DoesRaycastHitFloor( m_rightLeg1_2BIK->m_idealStepPos_WS, m_raycast_IdealPos_RightArm );
// 	if ( !doesRaycastHitFloor_CurPos )
// 	{
// 		// Keep previous ideal position?
// 		m_rightLeg1_2BIK->m_idealStepPos_WS = m_rightLeg1_2BIK->m_prevIdealPos_WS;
// 	}
// 	else
// 	{
// 		m_rightLeg1_2BIK->m_prevIdealPos_WS = m_rightLeg1_2BIK->m_idealStepPos_WS;
// 	}
// 	// 5c. Update idealPos.Z based on rayImpactPos.z
// 	m_rightLeg1_2BIK->m_target.m_goalPos_WS = m_rightLeg1_2BIK->m_idealStepPos_WS;
// 	// 6. Update bezier points for each chain's foot height
// 	UpdateBezierCurvePositions( m_bezier_rightArm, m_rightLeg1_2BIK, Vec3::Z_UP, m_bezierTimer_rightArm );
// 	// 7. Check if Bezier should be stopped
// 	UpdateBezierTimer( m_bezier_rightArm, m_rightLeg1_2BIK, m_leftLeg4_2BIK, m_bezierTimer_rightArm );


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
		Vec3 dirRootToTarget	= ( m_neck_FABRIK->m_target.m_currentPos_WS - m_modelRoot->m_jointPos_MS ).GetNormalized();
		Vec3 dirRootToTarget_XY	= Vec3( dirRootToTarget.x, dirRootToTarget.y, 0.0f ).GetNormalized();
		float distToTarget		= GetDistance3D( m_modelRoot->m_jointPos_MS, m_neck_FABRIK->m_target.m_currentPos_WS );
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
		float turnRate			= 175.0f;
		signedAngle				= GetClamped( signedAngle, -turnRate, turnRate );
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

 	//----------------------------------------------------------------------------------------------------------------------
 	// Update SpiderBase Height
 	//----------------------------------------------------------------------------------------------------------------------
 	// Get Legs & Arms positions in WS
	Vec3  leftFootEePos_WS			=   m_leftLeg4_2BIK->m_finalJoint->GetMatrix_LocalToWorld().GetTranslation3D();
	Vec3  rightFootEePos_WS			=  m_rightLeg4_2BIK->m_finalJoint->GetMatrix_LocalToWorld().GetTranslation3D();
	Vec3  leftArmEePos_WS			=   m_leftLeg1_2BIK->m_finalJoint->GetMatrix_LocalToWorld().GetTranslation3D();
	Vec3  rightArmEePos_WS			=  m_rightLeg1_2BIK->m_finalJoint->GetMatrix_LocalToWorld().GetTranslation3D();
 	// Compute averages of legs and arms Z height
	float legsAvgZHeight			= ( leftFootEePos_WS.z + rightFootEePos_WS.z ) * 0.5f;
 	float armsAvgZHeight			= (  leftArmEePos_WS.z +  rightArmEePos_WS.z ) * 0.5f;
	float combinedAvgZHeight		= (		legsAvgZHeight + armsAvgZHeight		 ) * 0.5f;
	// Lerp from curHeight to goalHeight
	float curCreatureHeight			= m_modelRoot->m_jointPos_MS.z;
	float goalCreatureHeight		= combinedAvgZHeight + m_defaultHeightZ;
	m_modelRoot->m_jointPos_MS.z	= Interpolate( curCreatureHeight, goalCreatureHeight, fractionTowardsEnd * 0.75f );


	// Lerp from currentRootPos to goalPos (Breathing)
//  	float currentTime				= float( GetCurrentTimeSeconds() );
//  	float sine						= SinDegrees( currentTime * 60.0f );
//  	float heightOffset				= 1.0f;
//  	float rootGoalHeightZ			= goalHeight + ( heightOffset * sine );
//  	fractionTowardsEnd				= 0.01f;
//  	fractionTowardsEnd				+= deltaSeconds * 8.0f;
// 	m_modelRoot->m_jointPos_MS.z	= Interpolate( curCreatureHeight, goalCreatureHeight, fractionTowardsEnd * 0.01f );
	

	//----------------------------------------------------------------------------------------------------------------------
	// Update SpiderBase model root Pitch
	//----------------------------------------------------------------------------------------------------------------------
	Vec3  armsAvgPos									= Interpolate(  leftArmEePos_WS,  rightArmEePos_WS, 0.5f );
	Vec3  legsAvgPos									= Interpolate( leftFootEePos_WS, rightFootEePos_WS, 0.5f );
	// Compute distLegsToArms by projecting onto XY
	Vec3  dispLegsToArms								= armsAvgPos - legsAvgPos;
	Vec3  dirLegsToArmsXY								= Vec3( dispLegsToArms.x, dispLegsToArms.y, 0.0f );
	dirLegsToArmsXY.Normalize();
	float distLegsToArms								= GetProjectedLength3D( dispLegsToArms, dirLegsToArmsXY );
	float goalPitchDegrees								= -Atan2Degrees( dispLegsToArms.z, distLegsToArms );				// Negate result to match pitch (up is negative, down is positive)
	float curPitchDegrees								= m_modelRoot->m_eulerAnglesGoal_LS.m_pitchDegrees;
	m_modelRoot->m_eulerAnglesGoal_LS.m_pitchDegrees	= Interpolate( curPitchDegrees, goalPitchDegrees, fractionTowardsEnd );


	//----------------------------------------------------------------------------------------------------------------------
	// Update SpiderBase tail wagging
	//----------------------------------------------------------------------------------------------------------------------
// 	float currentTime					= float( GetCurrentTimeSeconds() );
//  	float cos							= CosDegrees( currentTime * 100.0f ) * 5.0f;
// 	// Compute tailStart_WS
// 	Vec3  tailStart_WS					= m_tail_CCD->m_firstJoint->GetMatrix_LocalToWorld().GetTranslation3D();
// 	float fwdNess						= 0.0f;
// 	float sine							= SinDegrees( currentTime * 100.0f );
// 	float upNess						= sine;
// 	float leftNess						= sine * 5.0f;
// 	Vec3  modelFwdDir, modelLeftDir, modelUpDir; 
// 	m_modelRoot->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( modelFwdDir, modelLeftDir, modelUpDir );
//  	Vec3  restTailPos						= tailStart_WS - ( modelFwdDir * 40.0f ) - ( modelUpDir * 5.0f );
// 	m_tail_CCD->m_target.m_goalPos_WS		= restTailPos - ( modelFwdDir * fwdNess ) + ( modelLeftDir * leftNess * cos ) - ( modelUpDir * upNess );
// 	m_tail_CCD->m_target.m_currentPos_WS	= Interpolate( m_tail_CCD->m_target.m_currentPos_WS, m_tail_CCD->m_target.m_goalPos_WS, fractionTowardsEnd * 0.75f );
}


//----------------------------------------------------------------------------------------------------------------------
void SpiderBase::PostUpdateChains( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	// WORKING VERSION (MANIPULATING Arm/Leg CHAINS, end joints)
	//----------------------------------------------------------------------------------------------------------------------
	// Left foot palm
	//----------------------------------------------------------------------------------------------------------------------
	// Version 3: solve in WS, transform and update data in LS
  	Vec3 impactNormal_WS			= m_raycast_IdealPos_LeftFoot4.m_raycastResult.m_impactNormal;
	Vec3 rotationAxis_WS			= m_modelRoot->m_eulerAnglesGoal_LS.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
  	// Rotate impactNormal_LS 90 degrees
  	Vec3 palmFwd_WS					= RotateVectorAboutArbitraryAxis( impactNormal_WS, rotationAxis_WS, 90.0f );
  	palmFwd_WS.Normalize();
	// Compute palmLeft_WS
	Vec3 palmLeft_WS				= CrossProduct3D( impactNormal_WS, palmFwd_WS );
	palmLeft_WS.Normalize();
	// Transform new palmFwd WS to LS
	Mat44 wToL_Matrix				= m_leftLeg4_2BIK->m_finalJoint->GetMatrix_LocalToWorld().GetOrthoNormalInverse();
	Vec3 palmFwd_LS					= wToL_Matrix.TransformVectorQuantity3D( palmFwd_WS  );
	Vec3 palmLeft_LS				= wToL_Matrix.TransformVectorQuantity3D( palmLeft_WS );
	m_leftLeg4_2BIK->m_finalJoint->m_eulerAnglesGoal_LS = m_leftLeg4_2BIK->m_finalJoint->GetEulerFromFwdAndLeft( palmFwd_LS, palmLeft_LS );
	// Set debugDraw vectors
 	m_debugVec_ImpactNormal_WS_Palm = palmFwd_WS;
 	m_debugVec_RotationAxis_WS_Palm	= rotationAxis_WS;	

	//----------------------------------------------------------------------------------------------------------------------
	// Right foot palm
	//----------------------------------------------------------------------------------------------------------------------
	// Version 3: solve in WS, transform and update data in LS
  	impactNormal_WS					= m_raycast_IdealPos_RightFoot4.m_raycastResult.m_impactNormal;
  	// Rotate impactNormal_LS 90 degrees
  	palmFwd_WS						= RotateVectorAboutArbitraryAxis( impactNormal_WS, rotationAxis_WS, 90.0f );
  	palmFwd_WS.Normalize();
	// Compute palmLeft_WS
	palmLeft_WS						= CrossProduct3D( impactNormal_WS, palmFwd_WS );
	palmLeft_WS.Normalize();
	// Transform new palmFwd WS to LS
	wToL_Matrix						= m_rightLeg4_2BIK->m_finalJoint->GetMatrix_LocalToWorld().GetOrthoNormalInverse();
	palmFwd_LS						= wToL_Matrix.TransformVectorQuantity3D( palmFwd_WS );
	palmLeft_LS						= wToL_Matrix.TransformVectorQuantity3D( palmLeft_WS );
	m_rightLeg4_2BIK->m_finalJoint->m_eulerAnglesGoal_LS = m_rightLeg4_2BIK->m_finalJoint->GetEulerFromFwdAndLeft( palmFwd_LS, palmLeft_LS );
	// Set debugDraw vectors
 	m_debugVec_ImpactNormal_WS_Palm = palmFwd_WS;
 	m_debugVec_RotationAxis_WS_Palm	= rotationAxis_WS;

	//----------------------------------------------------------------------------------------------------------------------
	// Left Arm palm
	//----------------------------------------------------------------------------------------------------------------------
	// Version 3: solve in WS, transform and update data in LS
  	impactNormal_WS					= m_raycast_IdealPos_LeftFoot1.m_raycastResult.m_impactNormal;
  	// Rotate impactNormal_LS 90 degrees
  	palmFwd_WS						= RotateVectorAboutArbitraryAxis( impactNormal_WS, rotationAxis_WS, 90.0f );
  	palmFwd_WS.Normalize();
	// Compute palmLeft_WS
	palmLeft_WS						= CrossProduct3D( impactNormal_WS, palmFwd_WS );
	palmLeft_WS.Normalize();
	// Transform new palmFwd WS to LS
	wToL_Matrix						= m_leftLeg1_2BIK->m_finalJoint->GetMatrix_LocalToWorld().GetOrthoNormalInverse();
	palmFwd_LS						= wToL_Matrix.TransformVectorQuantity3D( palmFwd_WS );
	palmLeft_LS						= wToL_Matrix.TransformVectorQuantity3D( palmLeft_WS );
  	m_leftLeg1_2BIK->m_finalJoint->m_eulerAnglesGoal_LS = m_leftLeg1_2BIK->m_finalJoint->GetEulerFromFwdAndLeft( palmFwd_LS, palmLeft_LS );
	// Set debugDraw vectors
 	m_debugVec_ImpactNormal_WS_Palm = palmFwd_WS;
 	m_debugVec_RotationAxis_WS_Palm	= rotationAxis_WS;

	//----------------------------------------------------------------------------------------------------------------------
	// Right Arm palm
	//----------------------------------------------------------------------------------------------------------------------
	// Version 3: solve in WS, transform and update data in LS
  	impactNormal_WS					= m_raycast_IdealPos_RightFoot1.m_raycastResult.m_impactNormal;
  	// Rotate impactNormal_LS 90 degrees
  	palmFwd_WS						= RotateVectorAboutArbitraryAxis( impactNormal_WS, rotationAxis_WS, 90.0f );
  	palmFwd_WS.Normalize();
	// Compute palmLeft_WS
	palmLeft_WS						= CrossProduct3D( impactNormal_WS, palmFwd_WS );
	palmLeft_WS.Normalize();
	// Transform new palmFwd WS to LS
	wToL_Matrix						= m_rightLeg1_2BIK->m_finalJoint->GetMatrix_LocalToWorld().GetOrthoNormalInverse();
	palmFwd_LS						= wToL_Matrix.TransformVectorQuantity3D( palmFwd_WS );
	palmLeft_LS						= wToL_Matrix.TransformVectorQuantity3D( palmLeft_WS );
	m_rightLeg1_2BIK->m_finalJoint->m_eulerAnglesGoal_LS = m_rightLeg1_2BIK->m_finalJoint->GetEulerFromFwdAndLeft( palmFwd_LS, palmLeft_LS );
	// Set debugDraw vectors
 	m_debugVec_ImpactNormal_WS_Palm = palmFwd_WS;
 	m_debugVec_RotationAxis_WS_Palm	= rotationAxis_WS;

	//----------------------------------------------------------------------------------------------------------------------
	// Clamping model root to avoid going out of map bounds
	//----------------------------------------------------------------------------------------------------------------------
	ClampModelRootToFeetAvgPos();
}


//----------------------------------------------------------------------------------------------------------------------
bool SpiderBase::IsIdealPosTooFar( Vec3 const& idealPos, Vec3 posToCompare, float maxDistXY, float maxDistZ )
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
bool SpiderBase::DidRaycastHitWalkableBlock( RaycastResult3D& raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal, std::vector<Block*> blockList )
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
bool SpiderBase::DidRaycastHitTriangle( RaycastResult3D& raycastResult, Vec3& rayStartPos, Vec3& rayfwdNormal, float rayLength, Vec3& updatedImpactPos, Vec3& updatedImpactNormal, std::vector<unsigned int> triIndexList, std::vector<Vertex_PCU> planeVertList )
{
	bool  didImpact		 = false;
	float t				 = 0.0f;
	float u				 = 0.0f;
	float v				 = 0.0f;
	float nearestHitDist = 999999.9f;
	RaycastResult3D tempRayResult;
	// Increment in 3s to form triangles
	for ( int i = 0; i < triIndexList.size(); i += 3 )
	{
		// Calculate triangles from indexList
		int const& currentIndex_C	= triIndexList[ i + 2];
		int const& currentIndex_A	= triIndexList[ i + 0];
		int const& currentIndex_B	= triIndexList[ i + 1];
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
bool SpiderBase::DoesRaycastHitFloor( Vec3& refPosition, Raycast& raycast )
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
	else if ( m_gameMode_Spider )
	{
		// Static block list
		maxRayLength	= m_gameMode_Spider->m_raylength_Long;
		blockList		= m_gameMode_Spider->GetBlockListFromGameMode();
		// Dynamic Terrain
		maxRayLength	= m_gameMode_Spider->m_raylength_Long;
		blockList		= m_gameMode_Spider->m_blockList;
		triIndexList	= m_gameMode_Spider->m_indexList;
		planeVertList	= m_gameMode_Spider->m_planeVerts;
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
void SpiderBase::UpdateBezierCurvePositions( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const curChain, Vec3 const& refUpDir, Stopwatch& bezierTimer )
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
void SpiderBase::UpdateBezierTimer( CubicBezierCurve3D& bezierCurve, IK_Chain3D* const curChain, IK_Chain3D* const anchorLinkedChain, Stopwatch& bezierTimer )
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
void SpiderBase::UpdatePoleVector()
{
	// Update poleVectors in MS, relative to the creature modelRoot
	Vec3 modelFwd						= Vec3::X_FWD;
	Vec3 modelLeft						= Vec3::Y_LEFT;
	Vec3 modelUp						= Vec3::Z_UP;
	Vec3 fwdNess1						= ( modelFwd  * 10.0f );
	Vec3 fwdNess2						= ( modelFwd  *  5.0f );
	Vec3 fwdNess3						= ( modelFwd  *  5.0f );
	Vec3 fwdNess4						= ( modelFwd  * 10.0f );
	Vec3 leftNess1						= ( modelLeft * m_distPelvisToRoot * 3.0f );
	Vec3 leftNess2						= ( modelLeft * m_distPelvisToRoot * 5.0f );
	Vec3 leftNess3						= ( modelLeft * m_distPelvisToRoot * 4.0f );
	Vec3 leftNess4						= ( modelLeft * m_distPelvisToRoot * 4.0f );
	Vec3 upNess							= ( modelUp	  * m_defaultHeightZ   * 2.0f );
	// Left Leg
	m_leftLeg1_2BIK->m_poleVector_MS	= Vec3::ZERO + fwdNess1 + leftNess1 + upNess;
	m_leftLeg2_2BIK->m_poleVector_MS	= Vec3::ZERO + fwdNess2 + leftNess2 + upNess;
	m_leftLeg3_2BIK->m_poleVector_MS	= Vec3::ZERO - fwdNess3 + leftNess3 + upNess;
	m_leftLeg4_2BIK->m_poleVector_MS	= Vec3::ZERO - fwdNess4 + leftNess4 + upNess;
	// Right Leg
	m_rightLeg1_2BIK->m_poleVector_MS	= Vec3::ZERO + fwdNess1 - leftNess1 + upNess;
	m_rightLeg2_2BIK->m_poleVector_MS	= Vec3::ZERO + fwdNess2 - leftNess2 + upNess;
	m_rightLeg3_2BIK->m_poleVector_MS	= Vec3::ZERO - fwdNess3 - leftNess3 + upNess;
	m_rightLeg4_2BIK->m_poleVector_MS	= Vec3::ZERO - fwdNess4 - leftNess4 + upNess;

// 	// Left Leg
// 	m_leftLeg1_2BIK->m_poleVector_MS	= modelRoot->m_jointPos_LS + fwdNess1 + leftNess1 + upNess;
// 	m_leftLeg2_2BIK->m_poleVector_MS	= modelRoot->m_jointPos_LS + fwdNess2 + leftNess2 + upNess;
// 	m_leftLeg3_2BIK->m_poleVector_MS	= modelRoot->m_jointPos_LS - fwdNess3 + leftNess3 + upNess;
// 	m_leftLeg4_2BIK->m_poleVector_MS	= modelRoot->m_jointPos_LS - fwdNess4 + leftNess4 + upNess;
// 	// Right Leg
// 	m_rightLeg1_2BIK->m_poleVector_MS	= modelRoot->m_jointPos_LS + fwdNess1 - leftNess1 + upNess;
// 	m_rightLeg2_2BIK->m_poleVector_MS	= modelRoot->m_jointPos_LS + fwdNess2 - leftNess2 + upNess;
// 	m_rightLeg3_2BIK->m_poleVector_MS	= modelRoot->m_jointPos_LS - fwdNess3 - leftNess3 + upNess;
// 	m_rightLeg4_2BIK->m_poleVector_MS	= modelRoot->m_jointPos_LS - fwdNess4 - leftNess4 + upNess;
}


//----------------------------------------------------------------------------------------------------------------------
void SpiderBase::ClampModelRootToFeetAvgPos()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Computing average position to clamp model root
	//----------------------------------------------------------------------------------------------------------------------
	Vec3 leftFoot1Pos_WS	= m_raycast_IdealPos_LeftFoot1.m_raycastResult.m_impactPos;
	Vec3 leftFoot2Pos_WS	= m_raycast_IdealPos_LeftFoot2.m_raycastResult.m_impactPos;
	Vec3 leftFoot3Pos_WS	= m_raycast_IdealPos_LeftFoot3.m_raycastResult.m_impactPos;
	Vec3 leftFoot4Pos_WS	= m_raycast_IdealPos_LeftFoot4.m_raycastResult.m_impactPos;
	Vec3 rightFoot1Pos_WS	= m_raycast_IdealPos_RightFoot1.m_raycastResult.m_impactPos;
	Vec3 rightFoot2Pos_WS	= m_raycast_IdealPos_RightFoot2.m_raycastResult.m_impactPos;
	Vec3 rightFoot3Pos_WS	= m_raycast_IdealPos_RightFoot3.m_raycastResult.m_impactPos;
	Vec3 rightFoot4Pos_WS	= m_raycast_IdealPos_RightFoot4.m_raycastResult.m_impactPos;
	Vec3 leftFootSumPos_WS  = leftFoot1Pos_WS  + leftFoot2Pos_WS  + leftFoot3Pos_WS  + leftFoot4Pos_WS;
	Vec3 rightFootSumPos_WS = rightFoot1Pos_WS + rightFoot2Pos_WS + rightFoot3Pos_WS + rightFoot4Pos_WS;
	m_avgFeetPos			= ( leftFootSumPos_WS + rightFootSumPos_WS ) / 8.0f;
	Vec2 avgFeetPosXY		= Vec2( m_avgFeetPos.x, m_avgFeetPos.y );
	// Clamp root position relative to averageFeetPosition
	Vec2  modelRootPosXY_WS					= Vec2( m_modelRoot->m_jointPos_MS.x, m_modelRoot->m_jointPos_MS.y);
	Vec2  dirModelRootToAvgFeetPos			= ( modelRootPosXY_WS - avgFeetPosXY ).GetNormalized();		// Model root's model space happens to be in world space
	float distModelRootToAverageFeetPos		= GetDistance2D( modelRootPosXY_WS, avgFeetPosXY );
	m_clampedDistModelRootToAvgFeetPos		= GetClamped( distModelRootToAverageFeetPos, 0.0f, m_maxDistToClampRootFromAvgFeetPos );
	Vec2 clampedPosXY						= avgFeetPosXY + ( dirModelRootToAvgFeetPos * m_clampedDistModelRootToAvgFeetPos );
	m_modelRoot->m_jointPos_MS.x			= clampedPosXY.x;
	m_modelRoot->m_jointPos_MS.y			= clampedPosXY.y;

	// Old clamp logic, not clamping along XY
// 	// Clamp root position relative to averageFeetPosition
// 	Vec3 dirModelRootToAvgFeetPos			= ( m_modelRoot->m_jointPos_MS - m_avgFeetPos ).GetNormalized();		// Model root's model space happens to be in world space
// 	float distModelRootToAverageFeetPos		= GetDistance3D( m_modelRoot->m_jointPos_MS, m_avgFeetPos );
// 	float maxDistToClampRootFromAvgFeetPos	= 8.0f;
// 	m_clampedDistModelRootToAvgFeetPos		= GetClamped( distModelRootToAverageFeetPos, 0.0f, maxDistToClampRootFromAvgFeetPos );
// 	m_modelRoot->m_jointPos_MS				= m_avgFeetPos + ( dirModelRootToAvgFeetPos * m_clampedDistModelRootToAvgFeetPos );
}


//----------------------------------------------------------------------------------------------------------------------
void SpiderBase::DebugDrawBezier( std::vector<Vertex_PCU>& verts, CubicBezierCurve3D const& bezierCurve, Stopwatch const& timer )
{
	// Bezier points
	float elaspedTime	= timer.GetElapsedTime();
	Vec3 bezierPosAtT	= bezierCurve.GetPointAtFraction( elaspedTime );
	AddVertsForSphere3D( verts, bezierPosAtT, 2.0f, 8.0f, 16.0f, Rgba8::CYAN );
	AddVertsForSphere3D( verts, bezierCurve.m_startPos , 2.0f, 8.0f, 16.0f, Rgba8::RED	   );
	AddVertsForSphere3D( verts, bezierCurve.m_guidePos1, 2.0f, 8.0f, 16.0f, Rgba8::GREEN   );
	AddVertsForSphere3D( verts, bezierCurve.m_guidePos2, 2.0f, 8.0f, 16.0f, Rgba8::BLUE	   );
	AddVertsForSphere3D( verts, bezierCurve.m_endPos   , 2.0f, 8.0f, 16.0f, Rgba8::MAGENTA );
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
void SpiderBase::DebugDrawRaycasts( std::vector<Vertex_PCU>& verts ) const
{
	std::vector<RaycastResult3D> rayResultList;
	// Ideal positions
	rayResultList.push_back( m_raycast_IdealPos_LeftFoot1.  m_raycastResult );
	rayResultList.push_back( m_raycast_IdealPos_RightFoot1. m_raycastResult );
	rayResultList.push_back( m_raycast_IdealPos_LeftFoot4. m_raycastResult );
	rayResultList.push_back( m_raycast_IdealPos_RightFoot4.m_raycastResult );
	// Cur positions
	rayResultList.push_back( m_raycast_CurPos_LeftFoot1.  m_raycastResult );
	rayResultList.push_back( m_raycast_CurPos_RightFoot1. m_raycastResult );
	rayResultList.push_back( m_raycast_CurPos_LeftFoot4. m_raycastResult );
	rayResultList.push_back( m_raycast_CurPos_RightFoot4.m_raycastResult );

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
			AddVertsForSphere3D( verts, rayResult.m_impactPos, 1.0f, 16.0f, 16.0f, Rgba8::DARK_BLUE );
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
		AddVertsForSphere3D( verts, m_rayResult_Tri.m_impactPos, 1.0f, 4.0f, 4.0f, Rgba8::BLUE );
	}
	else
	{
		Vec3 endPos = m_rayResult_Tri.m_rayStartPosition + ( m_rayResult_Tri.m_rayFwdNormal * m_rayResult_Tri.m_rayMaxLength );
		AddVertsForArrow3D( verts, m_rayResult_Tri.m_rayStartPosition, endPos, 0.5f, Rgba8::DARKER_GREEN );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void SpiderBase::ComputeIdealStepPosition( IK_Chain3D* chainToEdit, EulerAngles eulerAngles, Vec3 const& anchorPos, float fwdScalar, float leftScalar, float upScalar )
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
bool SpiderBase::TryFreeAnchor( IK_Chain3D* chainToEdit, IK_Chain3D* anchorChain )
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
bool SpiderBase::ShouldChainStep( IK_Chain3D* chainToEdit, bool canMove, float maxDistFromIdealPos_XY, float maxDistFromIdealPos_Z )
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
void SpiderBase::TryInitBezierTimer( bool canMove, bool shouldStep, Stopwatch& bezierTimer, IK_Chain3D* chainToEdit, IK_Chain3D* anchorChain )
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
bool SpiderBase::DoesRayHitFloor_CurPos( IK_Chain3D* chainToEdit, Raycast& raycast )
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
void SpiderBase::UpdateIdealStepPosition( bool doesRaycastHitFloor_CurPos, IK_Chain3D* chainToEdit, IK_Chain3D* anchorChain, Stopwatch& curChainTimer )
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
