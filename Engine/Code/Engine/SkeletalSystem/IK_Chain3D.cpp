#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/SkeletalSystem/CreatureBase.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"


//----------------------------------------------------------------------------------------------------------------------
IK_Chain3D::IK_Chain3D( std::string name, Vec3 position_WS, IK_Joint3D* ownerSkeletonFirstJoint, CreatureBase* const creatureOwner, bool shouldReachInsteadOfDrag )
{
	m_name						= name;
	m_ownerSkeletonJoint		= ownerSkeletonFirstJoint;
	m_creatureOwner				= creatureOwner;
	m_position_WS				= position_WS;
	m_shouldReachInsteadOfDrag	= shouldReachInsteadOfDrag;
}


//----------------------------------------------------------------------------------------------------------------------
IK_Chain3D::~IK_Chain3D()
{
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Startup()
{
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Shutdown()
{
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Update( float deltaSeconds )
{
	if ( m_ownerSkeletonJoint )
	{
		// Update THIS chain's WS values (pos and euler) to reflect the owner joint
		if ( m_creatureOwner->m_modelRoot == m_ownerSkeletonJoint )
		{
			// If the ownerJoint is the ownerCreature's root, then we can just set the values since its in WS
			m_eulerAngles_WS = m_ownerSkeletonJoint->m_eulerAnglesGoal_LS;
			m_position_WS	 = m_ownerSkeletonJoint->m_jointPos_MS;
		}
		else 
		{
			// If the ownerJoint is a joint from a chain. (NOT the ownerCreature)
			// We need to transform the joint's local data into WS before saving its values
			Mat44 localToWorldMatrix = m_ownerSkeletonJoint->GetMatrix_LocalToWorld( m_ownerSkeletonJoint->m_ikChain->m_lerpEulerToGoal );
			Vec3  fwdDir_WS			 = localToWorldMatrix.GetIBasis3D();
			m_eulerAngles_WS		 = m_ownerSkeletonJoint->GetEulerFromFwdDir( fwdDir_WS );
			m_position_WS			 = localToWorldMatrix.GetTranslation3D();
		}
	}

	if ( m_shouldReachInsteadOfDrag )
	{
		if ( m_solverType == CHAIN_SOLVER_FABRIK )
		{
			Solve_FABRIK( m_target );
		}
		else if ( m_solverType == CHAIN_SOLVER_CCD )
		{
			Solve_CCD( m_target );
		}
		else if ( m_solverType == CHAIN_SOLVER_2BONE )
		{
			SolveTwoBoneIK_TriangulationMethod( m_target );
		}
		else if ( m_solverType == CHAIN_SOLVER_FABRIK_FWD_ONLY )
		{
			Solve_FABRIK_fwdOnly( m_target );
		}
		else if ( m_solverType == CHAIN_SOLVER_INVALID )
		{
			ERROR_RECOVERABLE( "Chain solver type was NOT set during initialization" );
		}
	}
	else
	{
		if ( m_solverType == CHAIN_SOLVER_FABRIK )
		{
			FABRIK_Forward( m_target );
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Lerp currentEuler to goalEuler
	//----------------------------------------------------------------------------------------------------------------------
//	turnRate = 22500.0f;			// eulerAngles will rotate 360.0f per frame, instantaneously.		// Try different turn rates, try half or quarter
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* curJoint = m_jointList[i];
		curJoint->LerpChainEuler_CurToGoal( deltaSeconds, m_turnRate, m_turnRate, m_turnRate );		
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::RenderTargetMesh( Renderer* g_theRenderer, Texture* texture ) const
{
	Mat44 targetMatrix = Mat44( m_target.m_fwdDir, m_target.m_leftDir, m_target.m_upDir, m_target.m_currentPos_WS );
	g_theRenderer->SetModelConstants( targetMatrix );
	g_theRenderer->BindTexture( texture );
	g_theRenderer->DrawVertexArray( static_cast<int>( m_target.m_meshList.size() ), m_target.m_meshList.data() );
	// Reset bindings
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::RenderMesh( Renderer* g_theRenderer, Texture* texture, Shader* const shader, Rgba8 const& color ) const
{
	// New version, rendering mesh at current euler lerping positions instead of joint positions to remove discontinuity between lerp and cur positions
	Vec3 parentPos_WS = Vec3::ZERO;
	for ( int jointIndex = 0; jointIndex < m_jointList.size(); jointIndex++ )
	{
		IK_Joint3D* curJoint = m_jointList[ jointIndex ];
		if ( curJoint->m_parent == nullptr )
		{
			Mat44 curJointMat = curJoint->GetMatrix_LocalToWorld( m_lerpEulerToGoal );
			g_theRenderer->SetModelConstants( curJointMat, color );
			if ( curJoint->m_renderMeshWithTexture )
			{
				g_theRenderer->BindTexture( texture );
			}
			else
			{
				g_theRenderer->BindTexture( nullptr );
			}
			g_theRenderer->BindShader ( shader );
			g_theRenderer->DrawVertexArray( int( curJoint->m_meshList_TBN.size() ), curJoint->m_meshList_TBN.data() );
			// Reset bindings and constants
			g_theRenderer->SetModelConstants();
			g_theRenderer->BindTexture( nullptr );
			g_theRenderer->BindShader ( nullptr );
			// Update parent pos
			Vec3 curJointFwd = curJointMat.GetIBasis3D();
			Vec3 curPos_WS	 = curJointMat.GetTranslation3D();
			parentPos_WS	 = curPos_WS + ( curJointFwd * curJoint->m_distToChild );		// Parent pos for child to use next iter
		}
		else	// if parent exists, use parent pos calculated last iteration
		{			
			// Get curJoint Matrix
			Mat44 curJointMat = curJoint->GetMatrix_LocalToWorld( m_lerpEulerToGoal );
			// Update matrix
			curJointMat.SetTranslation3D( parentPos_WS );
			// Transform mesh
			g_theRenderer->SetModelConstants( curJointMat, color );
			if ( curJoint->m_renderMeshWithTexture )
			{
				g_theRenderer->BindTexture( texture );
			}
			else
			{
				g_theRenderer->BindTexture( nullptr );
			}
			g_theRenderer->BindShader ( shader );
			g_theRenderer->DrawVertexArray( int( curJoint->m_meshList_TBN.size() ), curJoint->m_meshList_TBN.data() );
			// Reset bindings and constants
			g_theRenderer->SetModelConstants();
			g_theRenderer->BindTexture( nullptr );
			g_theRenderer->BindShader ( nullptr );
			// Update parent pos
			Vec3 curJointFwd = curJointMat.GetIBasis3D();
			Vec3 curPos_WS	 = curJointMat.GetTranslation3D();
			parentPos_WS	 = curPos_WS + ( curJointFwd * curJoint->m_distToChild );		// Parent pos for child to use next iter
		}
	}

	// New version, render curJointFwd towards child joint
// 	for ( int jointIndex = 0; jointIndex < m_jointList.size(); jointIndex++ )
// 	{
// 		IK_Joint3D* curJoint = m_jointList[ jointIndex ];
// 		if ( curJoint->m_child == nullptr )
// 		{
// 			Mat44 curJointMat = curJoint->GetMatrix_LocalToWorld( m_renderEulerAnglesGoal_notCur );
// 			g_theRenderer->SetModelConstants( curJointMat, color );
// 			g_theRenderer->BindTexture( texture );
// 			g_theRenderer->BindShader ( shader );
// 			g_theRenderer->DrawVertexArray( int( curJoint->m_meshList.size() ), curJoint->m_meshList.data() );
// 			// Reset bindings and constants
// 			g_theRenderer->SetModelConstants();
// 			g_theRenderer->BindTexture( nullptr );
// 			g_theRenderer->BindShader ( nullptr );
// 		}
// 		else
// 		{
// 			// Get childPos_WS
// 			Vec3 childPos_WS	= curJoint->m_child->GetMatrix_LocalToWorld().GetTranslation3D();
// 			// Get curPos_WS
// 			Mat44 curJointMat	= curJoint->GetMatrix_LocalToWorld( m_renderEulerAnglesGoal_notCur );
// 			Vec3  curPos_WS		= curJointMat.GetTranslation3D();
// 			// Get dispCurToChild
// 			Vec3 fwdDir			= ( childPos_WS - curPos_WS ).GetNormalized();
// 			Vec3 oldUp			= curJointMat.GetKBasis3D();
// 			Vec3 newLeft		= CrossProduct3D( oldUp, fwdDir );
// 			newLeft.Normalize();
// 			Vec3 newUp			= CrossProduct3D( fwdDir, newLeft );
// 			newUp.Normalize();
// 			Mat44 newMat		= Mat44( fwdDir, newLeft, newUp, curPos_WS );
// 			g_theRenderer->SetModelConstants( newMat, color );
// 			g_theRenderer->BindTexture( texture );
// 			g_theRenderer->BindShader ( shader );
// 			g_theRenderer->DrawVertexArray( int( curJoint->m_meshList.size() ), curJoint->m_meshList.data() );
// 			// Reset bindings and constants
// 			g_theRenderer->SetModelConstants();
// 			g_theRenderer->BindTexture( nullptr );
// 			g_theRenderer->BindShader ( nullptr );
// 		}
// 	}

	// Old version, render in joint fwd dir, not towards child joint
// 	for ( int jointIndex = 0; jointIndex < m_jointList.size(); jointIndex++ )
// 	{
// 		IK_Joint3D* curJoint = m_jointList[ jointIndex ];
// 		if ( curJoint->m_child == nullptr )
// 		{
// //			continue;
// 		}
// 
// 		Mat44 curJointMat = curJoint->GetMatrix_LocalToWorld( m_renderEulerAnglesGoal_notCur );
// 		g_theRenderer->SetModelConstants( curJointMat, color );
// 		g_theRenderer->BindTexture( texture );
// 		g_theRenderer->BindShader ( shader );
// 		g_theRenderer->DrawVertexArray( int( curJoint->m_meshList.size() ), curJoint->m_meshList.data() );
// 		// Reset bindings and constants
// 		g_theRenderer->SetModelConstants();
// 		g_theRenderer->BindTexture( nullptr );
// 		g_theRenderer->BindShader ( nullptr );
// 	}
}


//----------------------------------------------------------------------------------------------------------------------
// 'WS' means "world space"
// 'LS' means "local space"
//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Render( std::vector<Vertex_PCU>& verts, Rgba8 const& limbColor, Rgba8 const& jointColor, bool renderDebugJointBasis, bool renderDebugCurrentPos_EE ) const
{
	if ( m_solverType == CHAIN_SOLVER_FABRIK || m_solverType == CHAIN_SOLVER_2BONE )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Render Joints and limbs
		//----------------------------------------------------------------------------------------------------------------------
		Vec3 parentJointPos_WS = Vec3::ZERO;
		float distToParent = 0.0f;
		for ( int jointIndex = 0; jointIndex < m_jointList.size(); jointIndex++ )
		{
			// Lerping version, rendering using eulerAnglesCurrent_LS
			IK_Joint3D* curJoint = m_jointList[jointIndex];
			if ( jointIndex > 0 )
			{
				// Only render segments if currentJoint has a parent 				
				// Get curJointFwd_MS from curJoint.m_eulerAnglesCurrent_LS
				Mat44 lToW_Matrix;
				lToW_Matrix			= curJoint->m_parent->GetMatrix_LocalToWorld( m_lerpEulerToGoal );		// False means get matrix using eulerAnglesCurrent
				Vec3 curJointFwd_WS = lToW_Matrix.GetIBasis3D();
				Vec3 curJointPos_WS = parentJointPos_WS + ( curJointFwd_WS * distToParent );

				// Render joint and link
				AddVertsForCylinder3D( verts, parentJointPos_WS, curJointPos_WS, 0.25f, limbColor ); 
				AddVertsForSphere3D( verts, curJointPos_WS, 1.0f, 8.0f, 8.0f, jointColor );
				// Update data for rendering children
				parentJointPos_WS = curJointPos_WS;
				distToParent	  = curJoint->m_distToChild;
			}
			else
			{
				// Render root joint
				Mat44 modelToWorldMatrix;
				modelToWorldMatrix		= curJoint->GetMatrix_LocalToWorld( m_lerpEulerToGoal );
				Vec3  curJointPos_WS	 = modelToWorldMatrix.GetTranslation3D();
				AddVertsForSphere3D( verts, curJointPos_WS, 1.0f, 8.0f, 8.0f, jointColor );
				// Update data for rendering children
				parentJointPos_WS		 = curJointPos_WS;
				distToParent			 = curJoint->m_distToChild;
			}

			// Vanilla version, no lerping
// 			IK_Joint3D* currentJoint = m_jointList[jointIndex];
// 			if ( jointIndex > 0 )
// 			{
// 				Mat44 modelToWorldMatrix = currentJoint->GetIkChainMatrix_ModelToWorld();
// 				Vec3  curJointPos_WS	 = modelToWorldMatrix.TransformPosition3D( currentJoint->m_jointPos_MS );
// 				Vec3 parentJointPos_WS 	 = modelToWorldMatrix.TransformPosition3D( currentJoint->m_parent->m_jointPos_MS );
// 				// Only render segments if currentJoint has a parent 
// //				AddVertsForCylinder3D( verts, parentJointPos_WS, curJointPos_WS, 0.1f, limbColor );
// 				AddVertsForCone3D( verts, parentJointPos_WS, curJointPos_WS, 0.75f, limbColor, AABB2::ZERO_TO_ONE, 4 );
// 				// Render joints
// 				AddVertsForSphere3D( verts, curJointPos_WS, 1.0f, 8.0f, 8.0f, jointColor );
// 			}
// 			else
// 			{
// 				// Render root joint
// 				Mat44 modelToWorldMatrix = currentJoint->GetIkChainMatrix_ModelToWorld();
// 				Vec3  curJointPos_WS	 = modelToWorldMatrix.TransformPosition3D( currentJoint->m_jointPos_MS );
// 				AddVertsForSphere3D( verts, curJointPos_WS, 1.0f, 8.0f, 8.0f, jointColor );
// 			}
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Render Fwd, Left, Up basis
		//----------------------------------------------------------------------------------------------------------------------
		if ( renderDebugJointBasis )
		{
			DebugDrawJoints_IJK( verts );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// End Effectors
		//----------------------------------------------------------------------------------------------------------------------
		if ( renderDebugCurrentPos_EE )
		{
			RenderTarget_CurrentPos( verts );
	//		RenderTarget_EE_IJK_Vectors( verts, endPosLength )
		}
	}
	else if ( m_solverType == CHAIN_SOLVER_CCD )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Render limbs
		//----------------------------------------------------------------------------------------------------------------------
		Vec3  parentJointPos_WS = Vec3::ZERO;
		float distToParent		= 0.0f;
		for ( int jointIndex = 0; jointIndex < m_jointList.size(); jointIndex++ )
		{
			// Lerping version, rendering using eulerAnglesCurrent_LS
			IK_Joint3D* curJoint = m_jointList[jointIndex];
			if ( jointIndex > 0 )
			{
				// Only render segments if currentJoint has a parent 				
				// Get curJointFwd_MS from curJoint.m_eulerAnglesCurrent_LS
				Mat44 lToW_Matrix;
				if ( m_lerpEulerToGoal )
				{
					lToW_Matrix	= curJoint->m_parent->GetMatrix_LocalToWorld( m_lerpEulerToGoal );		// False means get matrix using eulerAnglesCurrent
				}
				else
				{
					lToW_Matrix	= curJoint->m_parent->GetMatrix_LocalToWorld( m_lerpEulerToGoal );
				}
				Vec3 curJointFwd_WS	= lToW_Matrix.GetIBasis3D();
				Vec3 curJointPos_WS	= parentJointPos_WS + ( curJointFwd_WS * distToParent );

				// Render joint and link
				AddVertsForCylinder3D( verts, parentJointPos_WS, curJointPos_WS, 0.25f, limbColor ); 
				AddVertsForSphere3D( verts, curJointPos_WS, 1.0f, 8.0f, 8.0f, jointColor );
				// Update data for rendering children
				parentJointPos_WS = curJointPos_WS;
				distToParent	  = curJoint->m_distToChild;
			}
			else
			{
				// Render root joint
				Mat44 modelToWorldMatrix;
				if ( m_lerpEulerToGoal )
				{
					modelToWorldMatrix = curJoint->GetMatrix_LocalToWorld( m_lerpEulerToGoal );
				}
				else
				{
					modelToWorldMatrix = curJoint->GetMatrix_LocalToWorld( m_lerpEulerToGoal );
				}
				Vec3  curJointPos_WS = modelToWorldMatrix.GetTranslation3D();
				AddVertsForSphere3D( verts, curJointPos_WS, 1.0f, 8.0f, 8.0f, jointColor );
				// Update data for rendering children
				parentJointPos_WS = curJointPos_WS;
				distToParent	  = curJoint->m_distToChild;
			}

			//----------------------------------------------------------------------------------------------------------------------
			// Vanilla version, no lerping
// 			IK_Joint3D* currentLimb		= m_jointList[limbIndex];
// 			Mat44 modelToWorldMatrix	= currentLimb->GetMatrix_LocalToWorld();
// 			Vec3  curJointPos_WS		= modelToWorldMatrix.GetTranslation3D();
// 			if ( limbIndex > 0 )
// 			{
// 				// Only render segments if currentJoint has a parent 
// 				AddVertsForCylinder3D( verts, parentJointPos_WS, curJointPos_WS, 0.1f, limbColor ); 
// 			}
// 			// Render joints
// 			AddVertsForSphere3D( verts, curJointPos_WS, 1.0f, 8.0f, 8.0f, jointColor );
// 			// Update variable for "next" joint to render as a cylinder
// 			parentJointPos_WS = curJointPos_WS;
		}
	}
	if ( m_solverType == CHAIN_SOLVER_FABRIK_FWD_ONLY )
	{
		Vec3 parentJointPos_WS = Vec3::ZERO;
		for ( int jointIndex = 0; jointIndex < m_jointList.size(); jointIndex++ )
		{
			// Lerping version, rendering using eulerAnglesCurrent_LS
			IK_Joint3D* curJoint = m_jointList[jointIndex];
			if ( jointIndex > 0 )
			{
				// Only render segments if currentJoint has a parent 				
				Vec3 curJointPos_MS = curJoint->m_jointPos_MS;
				Vec3 curJointPos_WS = m_position_WS + curJointPos_MS;

				// Render joint and link
				AddVertsForCylinder3D( verts, parentJointPos_WS, curJointPos_WS, 0.25f, limbColor ); 
				AddVertsForSphere3D( verts, curJointPos_WS, 1.0f, 8.0f, 8.0f, jointColor );
				// Update data for rendering children
				parentJointPos_WS = curJointPos_WS;
			}
			else
			{
				// Render root joint
				Vec3 curJointPos_WS		 = curJoint->m_jointPos_MS;
				AddVertsForSphere3D( verts, curJointPos_WS, 1.0f, 8.0f, 8.0f, jointColor );
				// Update data for rendering children
				parentJointPos_WS		 = curJointPos_WS;
			}
		}
	}
	else if ( m_solverType == CHAIN_SOLVER_INVALID )
	{
		ERROR_RECOVERABLE( "Attempting to render a chain with an INVALID solver type" );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::AddNewJointToChain( Vec3 const& position_localSpace, EulerAngles orientation_localSpace, FloatRange yawConstraints, 
																									  FloatRange pitchConstraints, 
																									  FloatRange rollConstraints )
{
	int	limbIndex				= int( m_jointList.size() );
	IK_Joint3D* newFinalJoint	= new IK_Joint3D(  
											limbIndex, 
										    position_localSpace, 
										    0,	
										    this, 
										    JOINT_CONSTRAINT_TYPE_EULER, 
										    orientation_localSpace, 
											yawConstraints, 
											pitchConstraints, 
											rollConstraints 
										  );

	// Update this IK_Chain's pointers for "firstJoint" and/or "finalJoint"
	if ( m_finalJoint != nullptr )
	{
		// Set newJoint's parent pointer to finalJoint
		// Note: At this point, "m_finalJoint" is still technically the final joint in 
		//		 the chain, but the "newJoint" is about get added into the "m_jointList"
		//		 and become the new "m_finalJoint"
		m_finalJoint->m_child		= newFinalJoint;
		newFinalJoint->m_parent		= m_finalJoint;									

		// Update "m_distToChild" for "finalJoint"
		float distToChild			= position_localSpace.GetLength();
		m_finalJoint->m_distToChild = distToChild;
	}
	else
	{
		// Logic for "firstJoint" 
		// If this is the "firstJoint", there is NO "finalJoint"
		m_firstJoint = newFinalJoint;
	}

	// Update finalJoint to the "newJoint" just created
	m_jointList.push_back( newFinalJoint );
	m_finalJoint = newFinalJoint;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::RenderTarget_IJK( std::vector<Vertex_PCU>& verts, float endPosLength ) const
{
	// Render IJK basis for each joint
	// This version uses the m_Left and m_Up vectors
	Vec3 fwdPos					= m_target.m_currentPos_WS + ( m_target.m_fwdDir  * endPosLength );
	Vec3 leftPos				= m_target.m_currentPos_WS + ( m_target.m_leftDir * endPosLength );
	Vec3 upPos					= m_target.m_currentPos_WS + ( m_target.m_upDir   * endPosLength );
	AddVertsForArrow3D( verts, m_target.m_currentPos_WS,  fwdPos, 0.1f, Rgba8::RED	 );		// Fwd
	AddVertsForArrow3D( verts, m_target.m_currentPos_WS, leftPos, 0.1f, Rgba8::GREEN );		// Left
	AddVertsForArrow3D( verts, m_target.m_currentPos_WS,   upPos, 0.1f, Rgba8::BLUE	 );		// Up
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::RenderTarget_CurrentPos( std::vector<Vertex_PCU>& verts, float radius /*= 2.0f*/, Rgba8 const& color) const
{
	AddVertsForSphere3D( verts, m_target.m_currentPos_WS, radius, 8.0f, 8.0f, color );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::RenderTarget_GoalPos( std::vector<Vertex_PCU>& verts, float radius, Rgba8 const& color ) const
{
	AddVertsForSphere3D( verts, m_target.m_goalPos_WS, radius, 8.0f, 8.0f, color );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugDrawJoints_IJK( std::vector<Vertex_PCU>& verts, float arrowThickness /*= 0.1f*/, float endPosLength /*= 3.0f*/ ) const
{
	if ( !m_shouldRender )
	{
		return;
	}

	// Render IJK basis for each joint
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* curJoint	= m_jointList[i];
		if ( m_solverType == CHAIN_SOLVER_FABRIK || m_solverType == CHAIN_SOLVER_2BONE )
		{
			// Get jointPos_LS and transform to WS
// 			Mat44 modelToWorldMatrix	= currentJoint->GetMatrix_ModelToWorld();
// 			Vec3  jointPos_WS			= modelToWorldMatrix.GetTranslation3D();
// 			Vec3  fwdPos				= jointPos_WS + ( modelToWorldMatrix.GetIBasis3D() * endPosLength );
// 			Vec3  leftPos				= jointPos_WS + ( modelToWorldMatrix.GetJBasis3D() * endPosLength );
// 			Vec3  upPos					= jointPos_WS + ( modelToWorldMatrix.GetKBasis3D() * endPosLength );
// 			AddVertsForArrow3D( verts, jointPos_WS, fwdPos,  arrowThickness, Rgba8::RED		);		// Fwd
// 			AddVertsForArrow3D( verts, jointPos_WS, leftPos, arrowThickness, Rgba8::GREEN   );		// Left
// 			AddVertsForArrow3D( verts, jointPos_WS, upPos,   arrowThickness, Rgba8::BLUE	);		// Up

			Mat44 modelToWorldMatrix	= curJoint->GetMatrix_LocalToWorld( m_lerpEulerToGoal );		// False means get matrix using eulerAnglesCurrent
// 			if ( m_lerpEulerToGoal )
// 			{
// 				modelToWorldMatrix = curJoint->GetMatrix_LocalToWorld( m_renderEulerAnglesGoal_notCur );		// False means get matrix using eulerAnglesCurrent
// 			}
// 			else
// 			{
// 				modelToWorldMatrix = curJoint->GetMatrix_LocalToWorld();
// 			}
			Vec3 jointPos_WS			= modelToWorldMatrix.GetTranslation3D();
			// This version uses the m_Left and m_Up vectors
			Vec3 fwdPos	 = jointPos_WS + ( modelToWorldMatrix.GetIBasis3D() * endPosLength );
			Vec3 leftPos = jointPos_WS + ( modelToWorldMatrix.GetJBasis3D()	* endPosLength );
			Vec3 upPos	 = jointPos_WS + ( modelToWorldMatrix.GetKBasis3D()	* endPosLength );

			AddVertsForArrow3D( verts, jointPos_WS, fwdPos,  arrowThickness, Rgba8::RED		);		// Fwd
			AddVertsForArrow3D( verts, jointPos_WS, leftPos, arrowThickness, Rgba8::GREEN   );		// Left
			AddVertsForArrow3D( verts, jointPos_WS, upPos,   arrowThickness, Rgba8::BLUE	);		// Up

			// Render spheres at basis end positions
// 			AddVertsForSphere3D( verts, fwdPos,  0.5f, 16.0f, 16.0f, Rgba8::RED		);
// 			AddVertsForSphere3D( verts, leftPos, 0.5f, 16.0f, 16.0f, Rgba8::GREEN   );
// 			AddVertsForSphere3D( verts, upPos,   0.5f, 16.0f, 16.0f, Rgba8::BLUE	);
		}
		else if ( m_solverType == CHAIN_SOLVER_CCD )
		{
			// Lerping version, rendering using eulerAnglesCurrent_LS
			Mat44 modelToWorldMatrix = curJoint->GetMatrix_LocalToWorld( m_lerpEulerToGoal );		// False means get matrix using eulerAnglesCurrent
// 			if ( m_lerpEulerToGoal )
// 			{
// 				modelToWorldMatrix = curJoint->GetMatrix_LocalToWorld( m_renderEulerAnglesGoal_notCur );		// False means get matrix using eulerAnglesCurrent
// 			}
// 			else
// 			{
// 				modelToWorldMatrix = curJoint->GetMatrix_LocalToWorld();
// 			}
			Vec3 jointPos_WS = modelToWorldMatrix.GetTranslation3D();
			// This version uses the m_Left and m_Up vectors
			Vec3 fwdPos	 = jointPos_WS + ( modelToWorldMatrix.GetIBasis3D() * endPosLength );
			Vec3 leftPos = jointPos_WS + ( modelToWorldMatrix.GetJBasis3D()	* endPosLength );
			Vec3 upPos	 = jointPos_WS + ( modelToWorldMatrix.GetKBasis3D()	* endPosLength );

			AddVertsForArrow3D( verts, jointPos_WS, fwdPos,  arrowThickness, Rgba8::RED		);		// Fwd
			AddVertsForArrow3D( verts, jointPos_WS, leftPos, arrowThickness, Rgba8::GREEN   );		// Left
			AddVertsForArrow3D( verts, jointPos_WS, upPos,   arrowThickness, Rgba8::BLUE	);		// Up
			
			// Render spheres at basis end positions
//			AddVertsForSphere3D( verts, fwdPos,  0.5f, 16.0f, 16.0f, Rgba8::RED		);
//			AddVertsForSphere3D( verts, leftPos, 0.5f, 16.0f, 16.0f, Rgba8::GREEN   );
//			AddVertsForSphere3D( verts, upPos,   0.5f, 16.0f, 16.0f, Rgba8::BLUE	);

			//----------------------------------------------------------------------------------------------------------------------
			// Vanilla version, NO LERPING
//			// Get jointPos_LS and transform to WS
// 			Mat44 modelToWorldMatrix	= curJoint->GetMatrix_LocalToWorld();
// 			Vec3  jointPos_WS			= modelToWorldMatrix.GetTranslation3D();
// 			// This version uses the m_Left and m_Up vectors
// 			Vec3 fwdPos					= jointPos_WS + ( modelToWorldMatrix.GetIBasis3D()  * endPosLength );
// 			Vec3 leftPos				= jointPos_WS + ( modelToWorldMatrix.GetJBasis3D()	* endPosLength );
// 			Vec3 upPos					= jointPos_WS + ( modelToWorldMatrix.GetKBasis3D()	* endPosLength );
// 			
// 			// Try rendering as line segments instead of arrow
// 
// 			AddVertsForArrow3D( verts, jointPos_WS, fwdPos,  arrowThickness, Rgba8::RED		);		// Fwd
// 			AddVertsForArrow3D( verts, jointPos_WS, leftPos, arrowThickness, Rgba8::GREEN   );		// Left
// 			AddVertsForArrow3D( verts, jointPos_WS, upPos,   arrowThickness, Rgba8::BLUE	);		// Up
// 
// 			// Render spheres at basis end positions
// 			AddVertsForSphere3D( verts, fwdPos,  0.5f, 16.0f, 16.0f, Rgba8::RED		 );
// 			AddVertsForSphere3D( verts, leftPos, 0.5f, 16.0f, 16.0f, Rgba8::GREEN    );
// 			AddVertsForSphere3D( verts, upPos,   0.5f, 16.0f, 16.0f, Rgba8::BLUE	 );

//			Vec3 axisPos				= jointPos_WS + ( currentJoint->m_axisOfRotation	* endPosLength * 0.5f );
//			AddVertsForArrow3D( verts, currentLimb->m_startPos,	axisPos, 0.6f, Rgba8::MAGENTA  );	// Axis of Rotation
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugDrawConstraints_YPR( std::vector<Vertex_PCU>& verts, float length ) const
{
	Vec3 parentFwd_WS = Vec3::ZERO;
	Vec3 parenLeft_WS = Vec3::ZERO;
	Vec3 parentUp_WS  = Vec3::ZERO;
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		// Lerping version, rendering using eulerAnglesCurrent_LS
		IK_Joint3D* currentJoint = m_jointList[i];
		if ( currentJoint->m_parent != nullptr )
		{
			// Logic for children
			Mat44 modelToWorldMatrix_curJoint		= currentJoint->GetMatrix_LocalToWorld( m_lerpEulerToGoal );		// False means get matrix using eulerAnglesCurrent
			Vec3  curJointPos_WS					= modelToWorldMatrix_curJoint.GetTranslation3D();
			Mat44 fwdMatrix							= Mat44( parentFwd_WS,  parenLeft_WS,  parentUp_WS,  curJointPos_WS );
			Mat44 leftMatrix						= Mat44( parentFwd_WS, -parentUp_WS,   parenLeft_WS, curJointPos_WS );
			Mat44 upMatrix							= Mat44( parenLeft_WS, -parentUp_WS,  -parentFwd_WS, curJointPos_WS );
			AddVertsForArc3D_Yaw  ( verts, fwdMatrix,  FloatRange( currentJoint->m_yawConstraints_LS.m_min,   currentJoint->m_yawConstraints_LS.m_max   ), length, Rgba8(100,   0,   0) );
			AddVertsForArc3D_Pitch( verts, leftMatrix, FloatRange( currentJoint->m_pitchConstraints_LS.m_min, currentJoint->m_pitchConstraints_LS.m_max ), length, Rgba8(  0, 100,   0) );
			AddVertsForArc3D_Roll ( verts, upMatrix,   FloatRange( currentJoint->m_rollConstraints_LS.m_min,  currentJoint->m_rollConstraints_LS.m_max  ), length, Rgba8(  0,   0, 100) );

			// Update parent dir
			parentFwd_WS = modelToWorldMatrix_curJoint.GetIBasis3D();
			parenLeft_WS = modelToWorldMatrix_curJoint.GetJBasis3D();
			parentUp_WS	 = modelToWorldMatrix_curJoint.GetKBasis3D();
		}
		else // Logic for Parent
		{
			Mat44 fwdMatrix;
			Mat44 leftMatrix;
			Mat44 upMatrix;
			if ( m_solverType == CHAIN_SOLVER_FABRIK || m_solverType == CHAIN_SOLVER_2BONE || m_solverType == CHAIN_SOLVER_CCD )
			{
				Vec3 fwd, left, up;
				m_eulerAngles_WS.GetAsVectors_XFwd_YLeft_ZUp( fwd, left, up );
				Vec3 rootPos_WS = currentJoint->GetMatrix_LocalToWorld( m_lerpEulerToGoal ).GetTranslation3D();
				fwdMatrix .SetIJKT3D( fwd, left,   up, rootPos_WS );
				leftMatrix.SetIJKT3D( fwd,  -up, left, rootPos_WS );
				upMatrix  .SetIJKT3D( left, -up, -fwd, rootPos_WS );
				// Update parent basis for rendering child
				parentFwd_WS = fwd;
				parenLeft_WS = left;
				parentUp_WS  = up;
			}
			AddVertsForArc3D_Yaw  ( verts, fwdMatrix,  FloatRange( currentJoint->m_yawConstraints_LS.m_min,   currentJoint->m_yawConstraints_LS.m_max   ), length, Rgba8( 100,   0,   0 ) );
			AddVertsForArc3D_Pitch( verts, leftMatrix, FloatRange( currentJoint->m_pitchConstraints_LS.m_min, currentJoint->m_pitchConstraints_LS.m_max ), length, Rgba8(   0, 100,   0 ) );
			AddVertsForArc3D_Roll ( verts, upMatrix,   FloatRange( currentJoint->m_rollConstraints_LS.m_min,  currentJoint->m_rollConstraints_LS.m_max  ), length, Rgba8(   0,   0, 100 ) );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Vanilla version with no lerping
		//----------------------------------------------------------------------------------------------------------------------
// 		IK_Joint3D* currentJoint = m_jointList[i];
// 		if ( currentJoint->m_parent != nullptr )
// 		{
// 			// Logic for children
// 			Mat44 fwdMatrix;
// 			Mat44 leftMatrix;
// 			Mat44 upMatrix;
// 
// 			Mat44 modelToWorldMatrix_curJoint;
// 			modelToWorldMatrix_curJoint				= currentJoint->GetMatrix_LocalToWorld( m_lerpEulerToGoal );		// False means get matrix using eulerAnglesCurrent
// 			Vec3  curJointPos_WS					= modelToWorldMatrix_curJoint.GetTranslation3D();
// 			Mat44 modelToWorldMatrix_parentJoint	= currentJoint->m_parent->GetMatrix_LocalToWorld( m_lerpEulerToGoal );
// 			Vec3  parentFwd							= modelToWorldMatrix_parentJoint.GetIBasis3D();
// 			Vec3  parenLeft							= modelToWorldMatrix_parentJoint.GetJBasis3D();
// 			Vec3  parentUp							= modelToWorldMatrix_parentJoint.GetKBasis3D();
// 			fwdMatrix .SetIJKT3D( parentFwd,  parenLeft, parentUp,  curJointPos_WS );
// 			leftMatrix.SetIJKT3D( parentFwd, -parentUp,  parenLeft, curJointPos_WS );
// 			upMatrix  .SetIJKT3D( parenLeft, -parentUp, -parentFwd, curJointPos_WS );
// 
// 			AddVertsForArc3D_Yaw  ( verts, fwdMatrix,  FloatRange( currentJoint->m_yawConstraints_LS.m_min,   currentJoint->m_yawConstraints_LS.m_max   ), length, Rgba8(100,   0,   0) );
// 			AddVertsForArc3D_Pitch( verts, leftMatrix, FloatRange( currentJoint->m_pitchConstraints_LS.m_min, currentJoint->m_pitchConstraints_LS.m_max ), length, Rgba8(  0, 100,   0) );
// 			AddVertsForArc3D_Roll ( verts, upMatrix,   FloatRange( currentJoint->m_rollConstraints_LS.m_min,  currentJoint->m_rollConstraints_LS.m_max  ), length, Rgba8(  0,   0, 100) );
// 		}
// 		else // Logic for Parent
// 		{
// 			Mat44 fwdMatrix;
// 			Mat44 leftMatrix;
// 			Mat44 upMatrix;
// 			if ( m_solverType == CHAIN_SOLVER_FABRIK || m_solverType == CHAIN_SOLVER_2BONE )
// 			{
// 				Vec3 fwd, left, up; 
// 				m_eulerAngles_WS.GetAsVectors_XFwd_YLeft_ZUp( fwd, left, up );
// 				Vec3 rootPos_WS = currentJoint->GetMatrix_LocalToWorld( m_lerpEulerToGoal ).GetTranslation3D();
// 				fwdMatrix.SetIJKT3D	( fwd,   left,    up, rootPos_WS );
// 				leftMatrix.SetIJKT3D( fwd,    -up,	left, rootPos_WS );
// 				upMatrix.SetIJKT3D	( left,   -up,  -fwd, rootPos_WS );
// 			}
// 			else if ( m_solverType == CHAIN_SOLVER_CCD )
// 			{
// 				Vec3 fwd, left, up; 
// 				m_eulerAngles_WS.GetAsVectors_XFwd_YLeft_ZUp( fwd, left, up );
// 				Vec3 rootPos_WS = currentJoint->GetMatrix_LocalToWorld( m_lerpEulerToGoal ).GetTranslation3D();
// 				fwdMatrix.SetIJKT3D	( fwd,   left,    up, rootPos_WS );
// 				leftMatrix.SetIJKT3D( fwd,    -up,	left, rootPos_WS );
// 				upMatrix.SetIJKT3D	( left,   -up,  -fwd, rootPos_WS );
// 			}
// 			AddVertsForArc3D_Yaw  ( verts, fwdMatrix,  FloatRange( currentJoint->m_yawConstraints_LS.m_min,   currentJoint->m_yawConstraints_LS.m_max   ), length, Rgba8( 100,   0,   0 ) );
// 			AddVertsForArc3D_Pitch( verts, leftMatrix, FloatRange( currentJoint->m_pitchConstraints_LS.m_min, currentJoint->m_pitchConstraints_LS.m_max ), length, Rgba8(   0, 100,   0 ) );
// 			AddVertsForArc3D_Roll ( verts, upMatrix,   FloatRange( currentJoint->m_rollConstraints_LS.m_min,  currentJoint->m_rollConstraints_LS.m_max  ), length, Rgba8(   0,   0, 100 ) );
// 		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
// void IK_Chain3D::DebugDrawParentBasis( std::vector<Vertex_PCU>& verts, float length /*= 4.0f */) const
// {
// 	for ( int i = 0; i < m_jointList.size(); i++ )
// 	{
// 		IK_Joint3D* currentJoint = m_jointList[i];
// 		if ( m_solverType == CHAIN_SOLVER_FABRIK || m_solverType == CHAIN_SOLVER_2BONE )
// 		{
// 			if ( currentJoint->m_parent != nullptr )
// 			{
// 				// Logic for children
// 				Vec3 startPos	= currentJoint->m_parent->m_endPos;
// 				Vec3 fwdEnd		= startPos + ( currentJoint->m_parent->m_fwdDir  * 4.0f );
// 				Vec3 leftEnd	= startPos + ( currentJoint->m_parent->m_leftDir * 4.0f );
// 				Vec3 upEnd		= startPos + ( currentJoint->m_parent->m_upDir	* 4.0f );
// 				AddVertsForArrow3D( verts, startPos, fwdEnd,  0.2f, Rgba8::MAGENTA   );
// 				AddVertsForArrow3D( verts, startPos, leftEnd, 0.2f, Rgba8::CYAN		 );
// 				AddVertsForArrow3D( verts, startPos, upEnd,   0.2f, Rgba8::LIGHTBLUE );
// 			}
// 			else // Logic for Parent
// 			{
// 				Vec3 startPos	= m_position_WS;
// 				Vec3 fwdEnd		= startPos + ( m_creatureOwner->m_modelRoot->m_fwdDir  * length );
// 				Vec3 leftEnd	= startPos + ( m_creatureOwner->m_modelRoot->m_leftDir * length );
// 				Vec3 upEnd		= startPos + ( m_creatureOwner->m_modelRoot->m_upDir	  * length );
// 				AddVertsForArrow3D( verts, startPos, fwdEnd,  0.2f, Rgba8::MAGENTA   );
// 				AddVertsForArrow3D( verts, startPos, leftEnd, 0.2f, Rgba8::CYAN		 );
// 				AddVertsForArrow3D( verts, startPos, upEnd,   0.2f, Rgba8::LIGHTBLUE );
// 			}
// 		}
// 		else if ( m_solverType == CHAIN_SOLVER_CCD )
// 		{
// 			if ( currentJoint->m_parent != nullptr )
// 			{
// 				// Logic for children
// 
// 				// Transform jointPos_LS to WS
// 				Mat44 localToModelMatrix = currentJoint->GetMatrix_LocalToModel();
// 				Vec3 startPos			 = localToModelMatrix.GetTranslation3D();
// 				Vec3 fwdEnd				 = startPos + ( currentJoint->m_parent->m_fwdDir  * 4.0f );
// 				Vec3 leftEnd			 = startPos + ( currentJoint->m_parent->m_leftDir * 4.0f );
// 				Vec3 upEnd				 = startPos + ( currentJoint->m_parent->m_upDir	 * 4.0f );
// 				AddVertsForArrow3D( verts, startPos, fwdEnd,  0.2f, Rgba8::MAGENTA   );
// 				AddVertsForArrow3D( verts, startPos, leftEnd, 0.2f, Rgba8::CYAN		 );
// 				AddVertsForArrow3D( verts, startPos, upEnd,   0.2f, Rgba8::LIGHTBLUE );
// 			}
// 			else // Logic for Parent
// 			{
// 				Vec3 startPos	= m_position_WS;
// 				Vec3 fwdEnd		= startPos + ( m_creatureOwner->m_modelRoot->m_fwdDir  * length );
// 				Vec3 leftEnd	= startPos + ( m_creatureOwner->m_modelRoot->m_leftDir * length );
// 				Vec3 upEnd		= startPos + ( m_creatureOwner->m_modelRoot->m_upDir	  * length );
// 				AddVertsForArrow3D( verts, startPos, fwdEnd,  0.2f, Rgba8::MAGENTA   );
// 				AddVertsForArrow3D( verts, startPos, leftEnd, 0.2f, Rgba8::CYAN		 );
// 				AddVertsForArrow3D( verts, startPos, upEnd,   0.2f, Rgba8::LIGHTBLUE );
// 			}
// 		}
// 	}
// }


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugDrawPoleVectorInfo( std::vector<Vertex_PCU>& verts, bool renderDebugAxis ) const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Pole vector and rotation axis debug rendering
	//----------------------------------------------------------------------------------------------------------------------
	// Pole vector
	Mat44 MToW_Matrix					= m_firstJoint->GetModelRootMatrix_ModelToWorld();
	Vec3  poleVector_WS					= MToW_Matrix.TransformPosition3D( m_poleVector_MS );
	AddVertsForSphere3D( verts, poleVector_WS, 1.0f, 4.0f, 4.0f, Rgba8::ORANGE );
	if ( renderDebugAxis )
	{
		// Rotation axis
		Vec3 jointPos_WS					= m_firstJoint->GetMatrix_LocalToWorld().GetTranslation3D();
		Vec3 debugVector_RotationAxis_MS	= MToW_Matrix.TransformVectorQuantity3D( m_debugVector_RotationAxis_MS );
		Vec3 end							= jointPos_WS + debugVector_RotationAxis_MS * 10.0f;
		AddVertsForArrow3D( verts, jointPos_WS, end, 0.2f, Rgba8::MAGENTA );
		// J0 to pole vector
		Vec3 debugVector_J0ToPV_WS			= MToW_Matrix.TransformVectorQuantity3D( m_debugVector_rootToPV_MS );
		end									= jointPos_WS + debugVector_J0ToPV_WS * 10.0f;
		AddVertsForArrow3D( verts, jointPos_WS, end, 0.2f, Rgba8( 0, 200, 50 ) );
		// J0 to target
		Vec3 debugVector_J0ToTarget_WS		= MToW_Matrix.TransformVectorQuantity3D( m_debugVector_rootToTarget_MS );
		end									= jointPos_WS + debugVector_J0ToTarget_WS * 10.0f;
		AddVertsForArrow3D( verts, jointPos_WS, end, 0.2f, Rgba8( 0, 50, 200 ) );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugDrawAnchorStates( std::vector<Vertex_PCU>& verts, bool renderAtGoalOrientation, float radius ) const
{
	Vec3 endJointPos_WS = m_finalJoint->GetMatrix_LocalToWorld( renderAtGoalOrientation ).GetTranslation3D();
	float stacks = 8.0f;
	float slices = 8.0f;
	if ( m_anchorState == ANCHOR_STATE_LOCKED )
	{
		AddVertsForSphere3D( verts, endJointPos_WS, radius, slices, stacks, Rgba8::RED );
	}
	else if ( m_anchorState == ANCHOR_STATE_FREE )
	{
		AddVertsForSphere3D( verts, endJointPos_WS, radius, slices, stacks, Rgba8::GREEN );
	}
	else if ( m_anchorState == ANCHOR_STATE_MOVING )
	{
		AddVertsForSphere3D( verts, endJointPos_WS, radius, slices, stacks, Rgba8::BLUE );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugDrawTarget( std::vector<Vertex_PCU>& verts, float radius, Rgba8 const& color /*= Rgba8::WHITE*/, bool renderBasis /*= false */, float basisLength /*= 1.0f*/ ) const
{
	AddVertsForSphere3D( verts, m_target.m_currentPos_WS, radius, 8.0f, 8.0f, color );
	if ( renderBasis )
	{
		RenderTarget_IJK( verts, basisLength );	 
	}
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugTextJointPos_WorldSpace( std::vector<Vertex_PCU>& verts, float textHeight, float heightOffset, Vec3 const& camera_jBasis, Vec3 const& camera_kBasis, BitmapFont* textFont, Rgba8 const& color ) const
{
	Vec3  fwd			= -camera_jBasis;
	Vec3  left			=  camera_kBasis;
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentJoint = m_jointList[i];

		if ( m_solverType == CHAIN_SOLVER_FABRIK || m_solverType == CHAIN_SOLVER_2BONE )
		{
// 			Mat44 modelToWorldMatrix = currentJoint->GetIkChainMatrix_ModelToWorld();
			Mat44 modelToWorldMatrix;
			if ( m_lerpEulerToGoal )
			{
				modelToWorldMatrix = currentJoint->GetMatrix_LocalToWorld( false );
			}
			else
			{
				modelToWorldMatrix = currentJoint->GetMatrix_LocalToWorld();
			}
//			Vec3 jointPos_WS = modelToWorldMatrix.TransformPosition3D( currentJoint->m_jointPos_MS );
// 			Vec3 jointPos_MS = localToModelMatrix.GetTranslation3D();
// 			Vec3 jointPos_WS = modelToWorldMatrix.TransformPosition3D( jointPos_MS );
			Vec3 jointPos_WS = modelToWorldMatrix.GetTranslation3D();
			textFont->AddVertsForText3D( verts, Vec3( jointPos_WS.x, jointPos_WS.y, jointPos_WS.z + heightOffset ),
				fwd, left, textHeight, Stringf( "%0.1f, %0.1f, %0.1f",	jointPos_WS.x, jointPos_WS.y, jointPos_WS.z ).c_str(), color );
		}
		else if ( m_solverType == CHAIN_SOLVER_CCD )
		{
			Mat44 localToModelMatrix;
			if ( m_lerpEulerToGoal )
			{
				localToModelMatrix = currentJoint->GetMatrix_LocalToWorld( false );
			}
			else
			{
				localToModelMatrix = currentJoint->GetMatrix_LocalToWorld();
			}
			Vec3 startPos = localToModelMatrix.GetTranslation3D();
			textFont->AddVertsForText3D( verts, Vec3( startPos.x, startPos.y, startPos.z + heightOffset ),
				fwd, left, textHeight, Stringf( "X: %0.1f, Y: %0.1f, Z: %0.1f", startPos.x, startPos.y, startPos.z ).c_str(), color );
		}

	}
	textFont->AddVertsForText3D( verts, Vec3( m_target.m_currentPos_WS.x, m_target.m_currentPos_WS.y, m_target.m_currentPos_WS.z + (heightOffset + (textHeight * 2.0f) ) ), fwd, left, textHeight, 
		Stringf( "X: %0.1f, Y: %0.1f, Z: %0.1f", m_target.m_currentPos_WS.x, m_target.m_currentPos_WS.y, m_target.m_currentPos_WS.z ).c_str(), color );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugTextJointPos_LocalSpace( std::vector<Vertex_PCU>& verts, float textHeight, float heightOffset, Vec3 const& camera_jBasis, Vec3 const& camera_kBasis, BitmapFont* textFont, Rgba8 const& color, bool renderTargetPos ) const
{
	Vec3  fwd			= -camera_jBasis;
	Vec3  left			=  camera_kBasis;
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentJoint = m_jointList[i];
		if ( m_solverType == CHAIN_SOLVER_FABRIK || m_solverType == CHAIN_SOLVER_2BONE )
		{
			// Lerped version
			Mat44 modelToWorldMatrix;
			if ( m_lerpEulerToGoal )
			{
				modelToWorldMatrix = currentJoint->GetMatrix_LocalToWorld( false );
			}
			else
			{
				modelToWorldMatrix = currentJoint->GetMatrix_LocalToWorld();
			}
			Vec3 jointPos_WS			= modelToWorldMatrix.GetTranslation3D();
			textFont->AddVertsForText3D( verts, Vec3( jointPos_WS.x, jointPos_WS.y, jointPos_WS.z + heightOffset ),
			fwd, left, textHeight, Stringf( "%0.1f, %0.1f, %0.1f",	currentJoint->m_jointPos_LS.x,
																	currentJoint->m_jointPos_LS.y, 
																	currentJoint->m_jointPos_LS.z ).c_str() );
			
			//----------------------------------------------------------------------------------------------------------------------
			// Vanilla version, NO LERPING
// 			Mat44 modelToWorldMatrix	= currentJoint->GetMatrix_LocalToWorld();
// 			Vec3 jointPos_WS			= modelToWorldMatrix.GetTranslation3D();
// 			textFont->AddVertsForText3D( verts, Vec3( jointPos_WS.x, jointPos_WS.y, jointPos_WS.z + heightOffset ),
// 				fwd, left, textHeight, Stringf( "%0.1f, %0.1f, %0.1f",	currentJoint->m_jointPos_LS.x,
// 																		currentJoint->m_jointPos_LS.y, 
// 																		currentJoint->m_jointPos_LS.z ).c_str() );
		}
		else if ( m_solverType == CHAIN_SOLVER_CCD )
		{
			// Lerped version
			Mat44 modelToWorldMatrix;
			if ( m_lerpEulerToGoal )
			{
				modelToWorldMatrix = currentJoint->GetMatrix_LocalToWorld( false );
			}
			else
			{
				modelToWorldMatrix = currentJoint->GetMatrix_LocalToWorld();
			}
			Vec3 jointPos_WS			= modelToWorldMatrix.GetTranslation3D();
			textFont->AddVertsForText3D( verts, Vec3( jointPos_WS.x, jointPos_WS.y, jointPos_WS.z + heightOffset ),
				fwd, left, textHeight, Stringf( "%0.1f, %0.1f, %0.1f",	currentJoint->m_jointPos_LS.x,
																		currentJoint->m_jointPos_LS.y, 
																		currentJoint->m_jointPos_LS.z ).c_str(), color );


			//----------------------------------------------------------------------------------------------------------------------
			// Vanilla version, NO LERPING
// 			Mat44 modelToWorldMatrix	= currentJoint->GetMatrix_LocalToWorld();
// 			Vec3 jointPos_WS			= modelToWorldMatrix.GetTranslation3D();
// 			textFont->AddVertsForText3D( verts, Vec3( jointPos_WS.x, jointPos_WS.y, jointPos_WS.z + heightOffset ),
// 				fwd, left, textHeight, Stringf( "%0.1f, %0.1f, %0.1f",	currentJoint->m_jointPos_LS.x,
// 																		currentJoint->m_jointPos_LS.y, 
// 																		currentJoint->m_jointPos_LS.z ).c_str(), color );
		}
	}
	if ( renderTargetPos )
	{
		textFont->AddVertsForText3D( verts, Vec3( m_target.m_currentPos_WS.x, m_target.m_currentPos_WS.y, m_target.m_currentPos_WS.z + (heightOffset + (textHeight * 2.0f) ) ), fwd, left, textHeight, 
			Stringf( "%0.1f, %0.1f, %0.1f", m_target.m_currentPos_WS.x, m_target.m_currentPos_WS.y, m_target.m_currentPos_WS.z ).c_str() );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DebugTextConstraints_YPR( std::vector<Vertex_PCU>& verts, float textHeight, float lengthToTextPosFromStart, float heightOffset, Vec3 const& jBasis, Vec3 const& kBasis, BitmapFont* textFont ) const
{
	Vec3			fwd			= -jBasis;
	Vec3			left		=  kBasis;
	unsigned char	colorTint	= 255;
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* curJoint	= m_jointList[i];
		IK_Joint3D* parentJoint = curJoint->m_parent;
		// Logic for children
		if ( curJoint->m_parent != nullptr )
		{
			Vec3 parentFwd, parentLeft, parentUp;
			parentJoint->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( parentFwd, parentLeft, parentUp );
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Compute end points for YAW min and max
			//----------------------------------------------------------------------------------------------------------------------
			Vec3 start			= parentJoint->m_jointPos_MS;
			Vec3 end			= start + ( parentFwd * lengthToTextPosFromStart );
			Vec3 vecToRotate	= end - start;
			Vec3 minTextPos		= RotateVectorAboutArbitraryAxis( vecToRotate, parentUp, curJoint->m_yawConstraints_LS.m_min );
			Vec3 maxTextPos		= RotateVectorAboutArbitraryAxis( vecToRotate, parentUp, curJoint->m_yawConstraints_LS.m_max );
			// Add height offsets
			minTextPos			= start + minTextPos + ( parentUp * heightOffset );
			maxTextPos			= start + maxTextPos + ( parentUp * heightOffset );
			// Yaw Min
			textFont->AddVertsForText3D( verts, minTextPos, fwd, left, textHeight, 
										Stringf( "Min%0.1d: %0.1f", i, curJoint->m_yawConstraints_LS.m_min ).c_str(), Rgba8( colorTint, 0, 0 ) );
			// Yaw Max
			textFont->AddVertsForText3D( verts, maxTextPos, fwd, left, textHeight,
										 Stringf( "Max%0.1d: %0.1f", i, curJoint->m_yawConstraints_LS.m_max ).c_str(), Rgba8( colorTint, 0, 0 ) );	
			//----------------------------------------------------------------------------------------------------------------------
			// 2. Compute end points for PITCH min and max
			//----------------------------------------------------------------------------------------------------------------------
			minTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, parentLeft, curJoint->m_pitchConstraints_LS.m_min );
			maxTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, parentLeft, curJoint->m_pitchConstraints_LS.m_max );
			minTextPos			= start + minTextPos + ( parentUp * heightOffset );
			maxTextPos			= start + maxTextPos + ( parentUp * -heightOffset );
			// Pitch Min
			textFont->AddVertsForText3D( verts, minTextPos, fwd, left, textHeight,
				Stringf( "Min%0.1d: %0.1f", i, curJoint->m_pitchConstraints_LS.m_min ).c_str(), Rgba8( 0, colorTint, 0 ) );
			// Pitch Max
			textFont->AddVertsForText3D( verts, maxTextPos, fwd, left, textHeight,
				Stringf( "Max%0.1d: %0.1f", i, curJoint->m_pitchConstraints_LS.m_max ).c_str(), Rgba8( 0, colorTint, 0 ) );
			//----------------------------------------------------------------------------------------------------------------------
			// 3. Compute end points for ROLL min and max
			//----------------------------------------------------------------------------------------------------------------------
			end					= start + ( parentLeft * lengthToTextPosFromStart );
			vecToRotate			= end - start;
			minTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, parentFwd, curJoint->m_rollConstraints_LS.m_min );
			maxTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, parentFwd, curJoint->m_rollConstraints_LS.m_max );
			minTextPos			= start + minTextPos + ( parentUp * -heightOffset );
			maxTextPos			= start + maxTextPos + ( parentUp *  heightOffset );
			// Roll Min
			textFont->AddVertsForText3D( verts, minTextPos, fwd, left, textHeight,
										Stringf( "Min%0.1d: %0.1f", i, curJoint->m_rollConstraints_LS.m_min ).c_str(), Rgba8( 0, 0, colorTint ) );
			// Roll Max
			textFont->AddVertsForText3D( verts, maxTextPos, fwd, left, textHeight,
										Stringf( "Max%0.1d: %0.1f", i, curJoint->m_rollConstraints_LS.m_max ).c_str(), Rgba8( 0, 0, colorTint ) );
		}
		else // Logic for Parent
		{
			Vec3 modelFwd, modelLeft, modelUp;
			m_creatureOwner->m_modelRoot->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( modelFwd, modelLeft, modelUp );
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Compute end points for YAW min and max
			//----------------------------------------------------------------------------------------------------------------------
			Vec3 start			= curJoint->m_jointPos_LS;
			Vec3 end			= start + ( modelFwd * lengthToTextPosFromStart );
			Vec3 vecToRotate	= end - start;
			Vec3 minTextPos		= RotateVectorAboutArbitraryAxis( vecToRotate, modelUp, curJoint->m_yawConstraints_LS.m_min );
			Vec3 maxTextPos		= RotateVectorAboutArbitraryAxis( vecToRotate, modelUp, curJoint->m_yawConstraints_LS.m_max );
			// Yaw Min
			textFont->AddVertsForText3D( verts, start + Vec3( minTextPos.x, minTextPos.y, minTextPos.z + heightOffset ), fwd, left, textHeight, 
											Stringf( "Min%0.1d: %0.1f", i, curJoint->m_yawConstraints_LS.m_min ).c_str(), Rgba8( colorTint, 0, 0 ) );
			// Yaw Max
			textFont->AddVertsForText3D( verts, start + Vec3( maxTextPos.x, maxTextPos.y, maxTextPos.z + heightOffset ), fwd, left, textHeight,
											Stringf( "Max%0.1d: %0.1f", i, curJoint->m_yawConstraints_LS.m_max ).c_str(), Rgba8( colorTint, 0, 0 ) );
			//----------------------------------------------------------------------------------------------------------------------
			// 2. Compute end points for PITCH min and max
			//----------------------------------------------------------------------------------------------------------------------
			minTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, modelLeft, curJoint->m_pitchConstraints_LS.m_min );
			maxTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, modelLeft, curJoint->m_pitchConstraints_LS.m_max );
			minTextPos			= start + minTextPos + ( modelUp *  heightOffset );
			maxTextPos			= start + maxTextPos + ( modelUp * -heightOffset );
			// Pitch Min
			textFont->AddVertsForText3D( verts, minTextPos, fwd, left, textHeight,
											Stringf( "Min%0.1d: %0.1f", i, curJoint->m_pitchConstraints_LS.m_min ).c_str(), Rgba8( 0, colorTint, 0 ) );
			// Pitch Max
			textFont->AddVertsForText3D( verts, maxTextPos, fwd, left, textHeight,
											Stringf( "Max%0.1d: %0.1f", i, curJoint->m_pitchConstraints_LS.m_max ).c_str(), Rgba8( 0, colorTint, 0 ) );
			//----------------------------------------------------------------------------------------------------------------------
			// 3. Compute end points for ROLL min and max
			//----------------------------------------------------------------------------------------------------------------------
			end					= start + ( modelLeft * lengthToTextPosFromStart );
			vecToRotate			= end - start; 
			minTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, modelFwd, curJoint->m_rollConstraints_LS.m_min );
			maxTextPos			= RotateVectorAboutArbitraryAxis( vecToRotate, modelFwd, curJoint->m_rollConstraints_LS.m_max );
			minTextPos			= start + minTextPos + ( modelUp * -heightOffset );
			maxTextPos			= start + maxTextPos + ( modelUp *  heightOffset );
			// Roll Min
			textFont->AddVertsForText3D( verts, minTextPos, fwd, left, textHeight,
											Stringf( "Min%0.1d: %0.1f", i, curJoint->m_rollConstraints_LS.m_min ).c_str(), Rgba8( 0, 0, colorTint ) );
			// Roll Max
			textFont->AddVertsForText3D( verts, maxTextPos, fwd, left, textHeight,
											Stringf( "Max%0.1d: %0.1f", i, curJoint->m_rollConstraints_LS.m_max ).c_str(), Rgba8( 0, 0, colorTint ) );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Solve_CCD( Target target )
{
	float distTargetPrevToCurFrame = GetDistanceSquared3D( m_targetPos_LastFrame, target.m_currentPos_WS );
	if ( CompareIfFloatsAreEqual( distTargetPrevToCurFrame, 0.0f, 0.0001f ) )
	{
		// target hasn't moved, don't bother solving
//		return;
	}

	// Reset variables every frame
	m_solutionsThisFrameList.clear();
	float curIterDistToTarget	= 0.0f;
	float toleranceDist			= 0.01f;
	m_bestDistSolvedThisFrame	= GetDistEeToTarget( target );

	// Save joint info from last frame in case the new solution computed this frame is worse
	// We can disregard the new solution and keep this (last frame) solution instead
	std::vector<JointInfo> prevFrameJointInfoList;
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* curJoint		= m_jointList[i];
		// Populate joint info for debug rendering
		EulerAngles eulerAngles_LS	= curJoint->m_eulerAnglesGoal_LS;
		JointInfo	jointInfo		= JointInfo( eulerAngles_LS );
		prevFrameJointInfoList.push_back( jointInfo );
	}

	// Save prevFrame joint data for debug rendering
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Populate joint info for debug rendering
		//----------------------------------------------------------------------------------------------------------------------
		IK_Joint3D*	 curJoint		= m_jointList[i];
		Mat44 const& lToW_Matrix	= curJoint->GetMatrix_LocalToWorld();
		Vec3  const& jointPos_WS	= lToW_Matrix.GetTranslation3D();
		Vec3  const& fwdDir_WS		= lToW_Matrix.GetIBasis3D();
		Vec3  const& leftDir_WS		= lToW_Matrix.GetJBasis3D();
		Vec3  const& upDir_WS		= lToW_Matrix.GetKBasis3D();
		JointInfo jointInfo			= JointInfo( jointPos_WS, fwdDir_WS, leftDir_WS, upDir_WS );
		m_jointInfosThisIterList.push_back( jointInfo );
	}
	m_solutionsThisFrameList.push_back( m_jointInfosThisIterList );
	m_jointInfosThisIterList.clear();

	//----------------------------------------------------------------------------------------------------------------------
	// Run CCD
	//----------------------------------------------------------------------------------------------------------------------
	// Init common variables
	float distRootToTarget			= GetDistRootToTarget( target );		// Currently, this distance is the prevFrame's distToTarget
	float maxChainLength			= GetMaxChainLength();
	bool  triedDeadlockThisFrame	= false;
	int   resetCount				= 0;
	float bestDistToTarget			= m_bestDistSolvedThisFrame;
	for ( int i = 0; i < m_numIterations; i++ )
	{
		// Early out check if target is too far
		if ( distRootToTarget > maxChainLength )
		{
			CCD_Forward( target );
			m_isTargetTooFar	= true;
			m_iterCount			= i + 1;
			break;
		}
		else
		{
			// Early out check for convergence
			m_prevIterDistToTarget = GetDistEeToTarget( target );
			if ( m_prevIterDistToTarget <= toleranceDist )
			{
				break;
			}
			m_iterCount = i + 1;

			CCD_Forward( target );

			if ( m_attemptDeadlockSolutions )
			{
				//----------------------------------------------------------------------------------------------------------------------
				// Check if deadlock solutions are required
				//----------------------------------------------------------------------------------------------------------------------
				curIterDistToTarget = GetDistEeToTarget( target );
				if ( curIterDistToTarget < m_prevIterDistToTarget )		// Solution brought us closer to target this iteration
				{
					// We got closer
					m_isDeadlock = false;

					// Check if curNew solution is better than lastFrame's solution
					if ( curIterDistToTarget < bestDistToTarget )
					{
						// Keep track of the new best solution
						prevFrameJointInfoList.clear();
						for ( int j = 0; j < m_jointList.size(); j++ )
						{
							IK_Joint3D* curJoint = m_jointList[ j ];
							JointInfo   jointInfo = JointInfo( curJoint->m_eulerAnglesGoal_LS );		// Prev frame's joint info for the curJoint
							prevFrameJointInfoList.push_back( jointInfo );
						}
						bestDistToTarget = curIterDistToTarget;
					}
				}
				else
				{
					// Deadlock or solution is worse this iteration 
					m_isDeadlock = true;
					// Only reset IF deadlock was already tried this frame AND
					// Its not the last iteration (to avoid rendering with a straight chain) AND 
					// Haven't straightened the chain this frame (Only allowed the straighten the chain TWICE per frame)
					if ( (m_curDeadlockCount > 10) && ( i < (m_numIterations - 1) ) && (resetCount < 2) )
					{
						curIterDistToTarget = GetDistEeToTarget( target );
						if ( curIterDistToTarget > 2.0f )
						{
							// If we've already tried a deadlock solution this frame, AND the solution is still 
							// too far, just reset all angles to 0
							ResetEulerForAllJoints();
							resetCount++;
							//						DebuggerPrintf( Stringf("Times reset Euler:  %i | iterCount this iteration:  %i | deadlockCount this iteration:  %i\n", resetCount, m_iterCount, m_curDeadlockCount).c_str() );
							std::string curDeadlockDebugInfo = Stringf("Euler Reset Count: %i | curIter: %i | deadlockCount this iter:  %i\n", resetCount, m_iterCount, m_curDeadlockCount );
							m_deadlockDebugInfoList.push_back( curDeadlockDebugInfo );
						}
					}
					else
					{
						triedDeadlockThisFrame	= true;	// This is set to true this iteration, but deadlock solutions will only be attempted next iteration
						m_prevDeadlockCount		= m_curDeadlockCount;
						m_curDeadlockCount++;
					}

					//----------------------------------------------------------------------------------------------------------------------
					// Compute 'debtAngle'
					//----------------------------------------------------------------------------------------------------------------------
					// eeParent_fwdDir in MS
					IK_Joint3D* eeParent			= m_finalJoint->m_parent;
					Mat44 lToM_Matrix				= eeParent->GetMatrix_LocalToModel(); 
					Vec3  eeParentFwdDir_MS			= lToM_Matrix.GetIBasis3D();
					Vec3  eeParentJointPos_MS		= lToM_Matrix.GetTranslation3D();
					// targetPos in MS
					Mat44 wToM_Matrix				= m_finalJoint->GetIkChainMatrix_ModelToWorld().GetOrthoNormalInverse();
					Vec3  targetPos_MS				= wToM_Matrix.TransformPosition3D( target.m_currentPos_WS );
					// Compute dispEeParentToTarget_MS
					Vec3  dir_eeParentToTarget_MS	= ( targetPos_MS - eeParentJointPos_MS ).GetNormalized();
					m_debtAngle						= GetAngleDegreesBetweenVectors3D( eeParentFwdDir_MS, dir_eeParentToTarget_MS );
				}
			}


			//----------------------------------------------------------------------------------------------------------------------
			// Populate list of solutionsPerIeration for debugging
			//----------------------------------------------------------------------------------------------------------------------
			m_solutionsThisFrameList.push_back( m_jointInfosThisIterList );
			m_jointInfosThisIterList.clear();
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Disregard new solution if worse than last frame's solution
	//----------------------------------------------------------------------------------------------------------------------
 	if ( m_prevFrameDistToTarget < curIterDistToTarget )
 	{
 		// Overwrite new jointPos_MS and eulerAngles_MS data with last frame's data
 		for ( int i = 0; i < prevFrameJointInfoList.size(); i++ )
 		{
  			JointInfo&  prevFrameJointInfo	= prevFrameJointInfoList[i];
  			IK_Joint3D* curJoint			= m_jointList[i];
  			curJoint->m_eulerAnglesGoal_LS	= prevFrameJointInfo.m_eulerAngles_LS;
 		}
 	}

	//----------------------------------------------------------------------------------------------------------------------
	// ATTEMPT TO SOLVE ROLL as a post process step after running the regular CCD algorithm
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_solveRoll )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Attempt to orient target (roll) as a "post process step" after solving Yaw + Pitch
		//----------------------------------------------------------------------------------------------------------------------
		float debtAngle_roll = 0;
		for ( int j = int( m_jointList.size() - 1); j >= 0; j-- )		// For loop backwards, end (child) to root
		{
			IK_Joint3D* curJoint = m_jointList[j];
			if ( curJoint->m_child == nullptr )
			{
				// Transform targetLeft_WS into endJoint's LS (targetLeft_LS)
				Mat44 wToL_Matrix				= curJoint->m_parent->GetMatrix_LocalToWorld().GetOrthoNormalInverse();
				Vec3  targetFwdDir_LS			= wToL_Matrix.TransformVectorQuantity3D( target.m_fwdDir  );
				Vec3  targetLeftDir_LS			= wToL_Matrix.TransformVectorQuantity3D( target.m_leftDir );
				// Inherit targetLeft_LS
				curJoint->m_eulerAnglesGoal_LS	= curJoint->GetEulerFromFwdAndLeft( targetFwdDir_LS, targetLeftDir_LS );
				// Clamp endJoint roll
				curJoint->ClampYPR();

				// Compute debt angle
				Vec3 endJointFwd_LS, endJointLeft_LS, endJointUp_LS;
				curJoint->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( endJointFwd_LS, endJointLeft_LS, endJointUp_LS );
				debtAngle_roll = GetAngleDegreesBetweenVectors3D( targetLeftDir_LS, endJointLeft_LS ); 
				debtAngle_roll = GetSignedAngleDegreesBetweenVectors( endJointLeft_LS, targetLeftDir_LS, endJointFwd_LS );			// Figuring out which direction to roll
			}
			else   // Distribute debtAngle "up the chain", starting at endJoint's parentJoint
			{
				// Early out check if roll is satisfied
				if ( CompareIfFloatsAreEqual(debtAngle_roll, 0.0f, 0.01f) )
				{
					break;
				}

				// Compute curJoint fwd and left (LS)
				Vec3 curJointFwd_LS, curJointLeft_LS, curJointUp_LS;
				curJoint->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( curJointFwd_LS, curJointLeft_LS, curJointUp_LS );
				Vec3 childJointFwd_LS, childJointLeft_LS, childJointUp_LS;
				curJoint->m_child->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( childJointFwd_LS, childJointLeft_LS, childJointUp_LS );

				// Rotate POSITIVE roll 
				if ( debtAngle_roll > 0.0f )
				{
					// Resolve as much of debtAngle as possible
					float elbowRoom	= curJoint->m_rollConstraints_LS.m_max - curJoint->m_eulerAnglesGoal_LS.m_rollDegrees;
					if ( elbowRoom >= debtAngle_roll )
					{
						// Absolve all debt since theres enough elbow room for all debt
						curJointLeft_LS = RotateVectorAboutArbitraryAxis( curJointLeft_LS, curJointFwd_LS, debtAngle_roll );
						debtAngle_roll -= debtAngle_roll;
					}
					else
					{
						// Absolve as much debt possible, NOT enough elbow room for all debt
						curJointLeft_LS = RotateVectorAboutArbitraryAxis( curJointLeft_LS, curJointFwd_LS, elbowRoom );
						debtAngle_roll -= elbowRoom;
					}
				}
				else   // Rotate NEGATIVE roll
				{
					// Resolve as much of debtAngle as possible
					float elbowRoom	= curJoint->m_rollConstraints_LS.m_min - curJoint->m_eulerAnglesGoal_LS.m_rollDegrees;
					if ( fabsf(elbowRoom) >= fabsf(debtAngle_roll) )
					{
						// Absolve all debt since theres enough elbow room for all debt
						curJointLeft_LS = RotateVectorAboutArbitraryAxis( curJointLeft_LS, curJointFwd_LS, debtAngle_roll );
						debtAngle_roll -= debtAngle_roll;
					}
					else
					{
						// Absolve as much debt possible, NOT enough elbow room for all debt
						curJointLeft_LS = RotateVectorAboutArbitraryAxis( curJointLeft_LS, curJointFwd_LS, elbowRoom );
						debtAngle_roll -= elbowRoom;
					}
				}
				// Update joint data and clamp
				curJoint->m_eulerAnglesGoal_LS = curJoint->GetEulerFromFwdAndLeft( curJointFwd_LS, curJointLeft_LS );
				curJoint->ClampYPR();
			}
		}
	}

	// Update target position to keep data fresh
	m_targetPos_LastFrame = target.m_currentPos_WS;
}


//----------------------------------------------------------------------------------------------------------------------
// Child to root
//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::CCD_Forward( Target target )
{
	float distBeforeSolve_eeToTarget = GetDistEeToTarget( target );
	UNUSED( distBeforeSolve_eeToTarget );
	int	  numLimbs	= ( int( m_jointList.size() ) - 1 );
	for ( int i = numLimbs; i >= 0; i-- )		// Loop forwards  (child to root)
// 	for ( int i = 0; i < numLimbs; i++ )		// Loop backwards (root to child)
	{
		IK_Joint3D* curJoint = m_jointList[ i ];
		if ( curJoint == m_finalJoint )
		{
			// Skip the final joint (end effector)
			continue;
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Main solver logic
		//----------------------------------------------------------------------------------------------------------------------
		if ( curJoint->m_parent == nullptr )	// curJoint is rootJoint
		{
 			//----------------------------------------------------------------------------------------------------------------------
 			// Axis angle rotation approach
 			//----------------------------------------------------------------------------------------------------------------------
 			// 1. Transform target and EE pos to root space
			Mat44 modelToWorldMatrix		= m_eulerAngles_WS.GetAsMatrix_XFwd_YLeft_ZUp();
			modelToWorldMatrix.SetTranslation3D( m_position_WS );
			Mat44 worldToModelMatrix		= modelToWorldMatrix.GetOrthoNormalInverse();
			Vec3  target_MS					= worldToModelMatrix.TransformPosition3D( target.m_currentPos_WS );
			// 1b. EE to model Space
 			Mat44 endEffector_LToM_Matrix	= m_finalJoint->GetMatrix_LocalToModel();
 			Vec3  endEffectorPos_MS			= endEffector_LToM_Matrix.GetTranslation3D();			// 'LToW_Matrix' means 'localToWorldMatrix'
 			// 2. Compute disps
 			Vec3  curJointToEE_MS			= endEffectorPos_MS   - curJoint->m_jointPos_LS;
 			Vec3  curJointToTarget_MS		= target_MS - curJoint->m_jointPos_LS;
 			// 3. Compute angle between disps
 			float angleToRotate				= GetAngleDegreesBetweenVectors3D( curJointToEE_MS, curJointToTarget_MS );
 			// 4. Compute rotation axis
 			Vec3 rotationAxis				= CrossProduct3D( curJointToEE_MS, curJointToTarget_MS );
 			rotationAxis.Normalize();
			if ( rotationAxis == Vec3::ZERO )
			{
				rotationAxis = Vec3::Y_LEFT;
			}
 			// 5. Rotate using Axis-angle 
			Vec3 curJointFwd, curJointLeft, curJointUp;
			curJoint->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( curJointFwd, curJointLeft, curJointUp );
 			curJointFwd						= RotateVectorAboutArbitraryAxis( curJointFwd, rotationAxis, angleToRotate );
 			curJointFwd.Normalize();
 			curJointLeft					= RotateVectorAboutArbitraryAxis( curJointLeft, rotationAxis, angleToRotate );
 			curJointLeft.Normalize();
			// 6. Update currentJoint eulerAngles
//			curJoint->m_eulerAnglesGoal_LS	= curJoint->GetEulerFromFwdAndLeft( curJointFwd, curJointLeft );
			curJoint->m_eulerAnglesGoal_LS	= curJoint->GetEulerFromFwdDir( curJointFwd );
			// 7. Clamp eulerAngles if exceeding constrains
 			curJoint->ClampYPR();
 			int brainCells = 0;
			UNUSED( brainCells );

			//----------------------------------------------------------------------------------------------------------------------
			// Deadlock solution
			//----------------------------------------------------------------------------------------------------------------------

			//----------------------------------------------------------------------------------------------------------------------
			// Bend more if isDeadlock
			//----------------------------------------------------------------------------------------------------------------------
			if ( m_isDeadlock )
			{
				// Deadlock Attempt 4: Distribute debtAngle using pitch AND yaw
				if ( !CompareIfFloatsAreEqual( m_debtAngle, 0.0f, 0.01f ) )
				{
					// Compute directions for MIN & MAX yaw and MIN & MAX pitch
					EulerAngles actualEulerAngles = curJoint->m_eulerAnglesGoal_LS;
					// MAX pitch dir MS
					float		maxPitch		= curJoint->m_pitchConstraints_LS.m_max;
					EulerAngles maxEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, maxPitch, actualEulerAngles.m_rollDegrees ); 
					curJoint->m_eulerAnglesGoal_LS	= maxEulerPitch;
					Vec3		dirMaxPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
					// MIN pitch dir MS
					float		minPitch		= curJoint->m_pitchConstraints_LS.m_min;
					EulerAngles minEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, minPitch, actualEulerAngles.m_rollDegrees ); 
					curJoint->m_eulerAnglesGoal_LS	= minEulerPitch;
					Vec3		dirMinPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
					// MAX yaw dir MS
					float		maxYaw			= curJoint->m_yawConstraints_LS.m_max;
					EulerAngles maxEulerYaw		= EulerAngles( maxYaw, actualEulerAngles.m_pitchDegrees, actualEulerAngles.m_rollDegrees ); 
					curJoint->m_eulerAnglesGoal_LS	= maxEulerPitch;
					Vec3		dirMaxYaw_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
					// MIN yaw dir MS
					float		minYaw			= curJoint->m_yawConstraints_LS.m_min;
					EulerAngles minEulerYaw		= EulerAngles( minYaw, actualEulerAngles.m_pitchDegrees, actualEulerAngles.m_rollDegrees ); 
					curJoint->m_eulerAnglesGoal_LS	= minEulerYaw;
					Vec3		dirMinYaw_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
					// Reset joint euler to actual values
					curJoint->m_eulerAnglesGoal_LS	= actualEulerAngles;

					// Compute dir_JointToTarget_MS: Transform joint and target into MS
					Vec3  jointPos_MS			= curJoint->GetMatrix_LocalToModel().GetTranslation3D();
					Mat44 wToM_Matrix			= curJoint->GetIkChainMatrix_ModelToWorld().GetOrthoNormalInverse();
					Vec3  targetPos_MS			= wToM_Matrix.TransformPosition3D( target.m_currentPos_WS );
					Vec3 dir_JointToTarget_MS	= ( targetPos_MS - jointPos_MS ).GetNormalized();
					// Compute dot results 
					float maxDotPositivity_Pitch	= DotProduct3D( dirMaxPitch_MS, dir_JointToTarget_MS );
					float minDotPositivity_Pitch	= DotProduct3D( dirMinPitch_MS, dir_JointToTarget_MS );
					float maxDotPositivity_Yaw		= DotProduct3D( dirMaxYaw_MS,	dir_JointToTarget_MS );
					float minDotPositivity_Yaw		= DotProduct3D( dirMinYaw_MS,	dir_JointToTarget_MS );
					// Compute 'elbowRoom' based on dotResult comparisons, choose the negative or lesser result
					float newPitch	 = 0.0f;
					float newYaw	 = 0.0f;
					float elbowRoom  = 0.0f;
					float curPitch	 = curJoint->m_eulerAnglesGoal_LS.m_pitchDegrees;
					float curYaw	 = curJoint->m_eulerAnglesGoal_LS.m_yawDegrees;

					// Toggle deadlock bend dir preference 
					if ( m_prevDeadlockCount != m_curDeadlockCount )
					{
// 						curJoint->ToggleDeadlockDir_Pitch();
// 						curJoint->ToggleDeadlockDir_Yaw();
					}
					//----------------------------------------------------------------------------------------------------------------------
					// BEND MORE using PITCH
					//----------------------------------------------------------------------------------------------------------------------
					// Pitch TOWARD target
					if ( curJoint->m_deadlockDir_Pitch == DEADLOCK_BEND_TOWARD )
					{
						// Choose more AGREE-ING dotResult_pitch
						if ( minDotPositivity_Pitch > maxDotPositivity_Pitch )
						{
							// Joint should pitch NEGATIVE to bend toward target
							elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_min );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newPitch = m_debtAngle * -1.0f;			// Debt angle is always positive, so we need to negative the angle to get a negative pitch
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newPitch = elbowRoom;
							}
						}
						else	// dotMaxPitch AGREES more than dotMinPitch
						{
							// Joint should pitch POSITIVE to bend toward target
							elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_max );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newPitch = m_debtAngle;
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newPitch = elbowRoom;
							}
						}
					}
					// Pitch AWAY from target
					else if ( curJoint->m_deadlockDir_Pitch == DEADLOCK_BEND_AWAY )
					{
						// Choose more DISAGREE-ING dotResult_pitch
						if ( minDotPositivity_Pitch < maxDotPositivity_Pitch )
						{
							// Joint should pitch NEGATIVE to bend away from target
							elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_min );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newPitch = m_debtAngle * -1.0f;			// Debt angle is always positive, so we need to negative the angle to get a negative pitch
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newPitch = elbowRoom;
							}
						}
						else	// dotMaxPitch DISAGREES more than dotMinPitch
						{
							// Joint should pitch POSITIVE to bend away from target
							elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_max );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newPitch = m_debtAngle;
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newPitch = elbowRoom;
							}
						}
					}
					curJoint->m_bendMore_pitch					= newPitch;
					curJoint->m_eulerAnglesGoal_LS.m_pitchDegrees  += newPitch;
					m_debtAngle								   -= fabsf( newPitch );

					//----------------------------------------------------------------------------------------------------------------------
					// BEND MORE using YAW
					//----------------------------------------------------------------------------------------------------------------------
					// Yaw TOWARD target
					if ( curJoint->m_deadlockDir_Yaw == DEADLOCK_BEND_TOWARD )
					{
						// Choose more AGREE-ING dotResult_yaw
						if ( minDotPositivity_Yaw > maxDotPositivity_Yaw )
						{
							// Joint should pitch NEGATIVE to bend toward target
							elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_min );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newYaw = m_debtAngle * -1.0f;		// Debt angle is always positive, so we need to negate the angle to get a negative yaw			
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newYaw = elbowRoom;
							}
						}
						else    // dotMaxYaw AGREES more than dotMinYaw
						{
							// Joint should pitch POSITIVE to bend toward target
							elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_max );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newYaw = m_debtAngle;
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newYaw = elbowRoom;
							}
						}
					}
					else if ( curJoint->m_deadlockDir_Yaw == DEADLOCK_BEND_AWAY )
					{
						// Choose DISAGREE-ING dotResult_yaw
						if ( minDotPositivity_Yaw < maxDotPositivity_Yaw )
						{
							// Joint should yaw NEGATIVE to bend away from target
							elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_min );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newYaw = m_debtAngle * -1.0f;		// Debt angle is always positive, so we need to negate the angle to get a negative yaw			
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newYaw = elbowRoom;
							}
						}
						else    // dotMaxYaw DISAGREES than dotMinYaw
						{
							// Joint should yaw POSITIVE to bend away from target
							elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_max );
							if ( fabsf( elbowRoom ) >= fabsf( m_debtAngle ) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newYaw = m_debtAngle;
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newYaw = elbowRoom;
							}
						}
					}
					curJoint->m_bendMore_yaw = newYaw;
					curJoint->m_eulerAnglesGoal_LS.m_yawDegrees += newYaw;
					m_debtAngle -= fabsf( newYaw );
				}
				else
				{
					curJoint->m_bendMore_yaw = 0.0f;
				}
			}
			curJoint->ClampYPR();
		}
		else	// Update children joints
		{
			//----------------------------------------------------------------------------------------------------------------------
			// Solution for end's parent to be fwd aligned with target
			//----------------------------------------------------------------------------------------------------------------------
			// Attempt to orient IK solver to target left
//				float distRootToTarget	= GetDistRootToTarget( target );		// Currently, this distance is the prevFrame's distToTarget
//				float maxChainLength	= GetMaxChainLength();
//				bool targetIsReachable  = (maxChainLength >= distRootToTarget) ? true : false;
//  			if ( (curJoint->m_child->m_child == nullptr) && targetIsReachable )
//  			{
//  				// Solve eeParent by inheriting target fwd and left
//  				//----------------------------------------------------------------------------------------------------------------------
//  				// Axis angle rotation approach
//  				//----------------------------------------------------------------------------------------------------------------------			
//  				// 1.  Transform target and EE pos to local space
//  				// 1a. Note: Target is transformed in 2 steps (World-To-Model, then Model-To-Local)
//  				//	   ALSO NOTE: "local space" means relative to parent (current joint's world is defined by parent IJKT)
//  				Mat44 worldToModelMatrix		= curJoint->GetIkChainMatrix_ModelToWorld().GetOrthoNormalInverse();
//  				Vec3  targetFwd_MS, targetLeft_MS;
//  				Vec3  targetFwd_LS, targetLeft_LS;
//  				// Transform targetFwd_MS to LS
//  				targetFwd_MS					= worldToModelMatrix.TransformVectorQuantity3D( target.m_fwdDir );
//  				Mat44 modelToLocalMatrix		= curJoint->m_parent->GetMatrix_LocalToModel().GetOrthoNormalInverse();
//  				targetFwd_LS					= modelToLocalMatrix.TransformVectorQuantity3D( targetFwd_MS );
//  				// Transform targetLeft_MS to LS
//  				targetLeft_MS					= worldToModelMatrix.TransformVectorQuantity3D( target.m_leftDir );
//  				modelToLocalMatrix				= curJoint->m_parent->GetMatrix_LocalToModel().GetOrthoNormalInverse();
//  				targetLeft_LS					= modelToLocalMatrix.TransformVectorQuantity3D( targetLeft_MS );
//  				// 2. Inherit target basis
//  				Vec3 curJointFwd, curJointLeft, curJointUp;
//  				curJointFwd						= targetFwd_LS.GetNormalized();
//  				curJointLeft					= targetLeft_LS.GetNormalized();
//  				
//  				// 3. Update curJoint data (matrix to eulerAngles)
//  //				curJoint->m_eulerAnglesGoal_LS	= curJoint->GetEulerFromFwdAndLeft( curJointFwd, curJointLeft );
//  				curJoint->m_eulerAnglesGoal_LS	= curJoint->GetEulerFromFwdDir( curJointFwd );
//  				// 4. Clamp eulerAngles if exceeding constrains
//  				curJoint->ClampYPR();
//  			}
//  			else	// Solve curJoint using normal CCD
//  			{
//  				//----------------------------------------------------------------------------------------------------------------------
//  				// Axis angle rotation approach
//  				//----------------------------------------------------------------------------------------------------------------------			
//  				// 1.  Transform target and EE pos to local space
//  				// 1a. Note: Target is transformed in 2 steps (World-To-Model, then Model-To-Local)
//  				//	   ALSO NOTE: "local space" means relative to parent (current joint's world is defined by parent IJKT)
//  				Mat44 modelToWorldMatrix		= m_eulerAngles_WS.GetAsMatrix_XFwd_YLeft_ZUp();
//  				modelToWorldMatrix.SetTranslation3D( m_position_WS );
//  				Mat44 worldToModelMatrix		= modelToWorldMatrix.GetOrthoNormalInverse();
//  				Vec3  target_MS					= worldToModelMatrix.TransformPosition3D( target.m_currentPos_WS );
//  				Mat44 localToModelMatrix		= curJoint->m_parent->GetMatrix_LocalToModel();
//  				Mat44 modelToLocalMatrix		= localToModelMatrix.GetOrthoNormalInverse();
//  				Vec3  target_LS					= modelToLocalMatrix.TransformPosition3D( target_MS );
//  				// 1b. EE to localSpace
//  				//	   Note: EE is transformed in 2 steps (EeLocal-To-Model, then Model-To-curJointLocal)
//  				//	   ALSO NOTE: "local space" means relative to parent (current joint's world is defined by parent IJKT)			
//  				Mat44 endEffector_LToM_Matrix	= m_finalJoint->GetMatrix_LocalToModel();
//  				Vec3  endEffectorPos_WS			= endEffector_LToM_Matrix.GetTranslation3D();												// 'LToW_Matrix' means 'localToWorldMatrix'
//  				Vec3  endEffectorPos_MS			= modelToLocalMatrix.TransformPosition3D( endEffectorPos_WS	);
//  				// 2. Compute disps
//  				Vec3  curJointToEE_LS			= endEffectorPos_MS - curJoint->m_jointPos_LS;
//  				Vec3  curJointToTarget_LS		= target_LS - curJoint->m_jointPos_LS;
//  				// 3. Compute angle between disps
//  				float angleToRotate				= GetAngleDegreesBetweenVectors3D( curJointToEE_LS, curJointToTarget_LS );
//  				// 4. Compute rotation axis 
//  				Vec3 rotationAxis				= CrossProduct3D( curJointToEE_LS, curJointToTarget_LS );
//  				rotationAxis.Normalize();
//  				// 5. Rotate using Axis-angle 
//  				Vec3 curJointFwd, curJointLeft, curJointUp;
//  				curJoint->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( curJointFwd, curJointLeft, curJointUp );
//  				curJointFwd						= RotateVectorAboutArbitraryAxis( curJointFwd, rotationAxis, angleToRotate );
//  				curJointFwd.Normalize();
//  				curJointLeft					= RotateVectorAboutArbitraryAxis( curJointLeft, rotationAxis, angleToRotate );
//  				curJointLeft.Normalize();
//  				// 6. Update currentJoint eulerAngles
//  //				curJoint->m_eulerAnglesGoal_LS	= curJoint->GetEulerFromFwdAndLeft( curJointFwd, curJointLeft );
//  				curJoint->m_eulerAnglesGoal_LS	= curJoint->GetEulerFromFwdDir( curJointFwd );
//  				//----------------------------------------------------------------------------------------------------------------------
//  				// 7. Clamp eulerAngles if exceeding constrains
//  				curJoint->ClampYPR();
//  				int brainCells = 0;
//  				UNUSED( brainCells );
//   			}


			//----------------------------------------------------------------------------------------------------------------------
			// Axis angle rotation approach (end's parent NOT fwd aligned with target)
			//----------------------------------------------------------------------------------------------------------------------			
  			// 1.  Transform target and EE pos to local space
  			// 1a. Note: Target is transformed in 2 steps (World-To-Model, then Model-To-Local)
  			//	   ALSO NOTE: "local space" means relative to parent (current joint's world is defined by parent IJKT)
  			Mat44 modelToWorldMatrix		= m_eulerAngles_WS.GetAsMatrix_XFwd_YLeft_ZUp();
  			modelToWorldMatrix.SetTranslation3D( m_position_WS );
  			Mat44 worldToModelMatrix		= modelToWorldMatrix.GetOrthoNormalInverse();
  			Vec3  target_MS					= worldToModelMatrix.TransformPosition3D( target.m_currentPos_WS );
  			Mat44 localToModelMatrix		= curJoint->m_parent->GetMatrix_LocalToModel();
  			Mat44 modelToLocalMatrix		= localToModelMatrix.GetOrthoNormalInverse();
  			Vec3  target_LS					= modelToLocalMatrix.TransformPosition3D( target_MS );
  			// 1b. EE to localSpace
  			//	   Note: EE is transformed in 2 steps (EeLocal-To-Model, then Model-To-curJointLocal)
  			//	   ALSO NOTE: "local space" means relative to parent (current joint's world is defined by parent IJKT)			
  			Mat44 endEffector_LToM_Matrix	= m_finalJoint->GetMatrix_LocalToModel();
  			Vec3  endEffectorPos_WS			= endEffector_LToM_Matrix.GetTranslation3D();												// 'LToW_Matrix' means 'localToWorldMatrix'
  			Vec3  endEffectorPos_MS			= modelToLocalMatrix.TransformPosition3D( endEffectorPos_WS	);
  			// 2. Compute disps
  			Vec3  curJointToEE_LS			= endEffectorPos_MS - curJoint->m_jointPos_LS;
  			Vec3  curJointToTarget_LS		= target_LS - curJoint->m_jointPos_LS;
  			// 3. Compute angle between disps
  			float angleToRotate				= GetAngleDegreesBetweenVectors3D( curJointToEE_LS, curJointToTarget_LS );
  			// 4. Compute rotation axis 
  			Vec3 rotationAxis				= CrossProduct3D( curJointToEE_LS, curJointToTarget_LS );
  			rotationAxis.Normalize();
  			// 5. Rotate using Axis-angle 
  			Vec3 curJointFwd, curJointLeft, curJointUp;
  			curJoint->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( curJointFwd, curJointLeft, curJointUp );
  			curJointFwd						= RotateVectorAboutArbitraryAxis( curJointFwd, rotationAxis, angleToRotate );
  			curJointFwd.Normalize();
  			curJointLeft					= RotateVectorAboutArbitraryAxis( curJointLeft, rotationAxis, angleToRotate );
  			curJointLeft.Normalize();
  			// 6. Update currentJoint eulerAngles
  			//				curJoint->m_eulerAnglesGoal_LS	= curJoint->GetEulerFromFwdAndLeft( curJointFwd, curJointLeft );
  			curJoint->m_eulerAnglesGoal_LS	= curJoint->GetEulerFromFwdDir( curJointFwd );
  			//----------------------------------------------------------------------------------------------------------------------
  			// 7. Clamp eulerAngles if exceeding constrains
  			curJoint->ClampYPR();
  			int brainCells = 0;
  			UNUSED( brainCells );	

			//----------------------------------------------------------------------------------------------------------------------
			// Bend more if isDeadlock
			//----------------------------------------------------------------------------------------------------------------------
			if ( m_isDeadlock )
			{
				// Deadlock Attempt 4: Distribute debtAngle using pitch AND yaw
				if ( !CompareIfFloatsAreEqual( m_debtAngle, 0.0f, 0.01f ) )
				{
					// Compute directions for MIN & MAX yaw and MIN & MAX pitch
					EulerAngles actualEulerAngles = curJoint->m_eulerAnglesGoal_LS;
					// MAX pitch dir MS
					float		maxPitch		= curJoint->m_pitchConstraints_LS.m_max;
					EulerAngles maxEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, maxPitch, actualEulerAngles.m_rollDegrees ); 
					curJoint->m_eulerAnglesGoal_LS	= maxEulerPitch;
					Vec3		dirMaxPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
					// MIN pitch dir MS
					float		minPitch		= curJoint->m_pitchConstraints_LS.m_min;
					EulerAngles minEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, minPitch, actualEulerAngles.m_rollDegrees ); 
					curJoint->m_eulerAnglesGoal_LS	= minEulerPitch;
					Vec3		dirMinPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
					// MAX yaw dir MS
					float		maxYaw			= curJoint->m_yawConstraints_LS.m_max;
					EulerAngles maxEulerYaw		= EulerAngles( maxYaw, actualEulerAngles.m_pitchDegrees, actualEulerAngles.m_rollDegrees ); 
					curJoint->m_eulerAnglesGoal_LS	= maxEulerPitch;
					Vec3		dirMaxYaw_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
					// MIN yaw dir MS
					float		minYaw			= curJoint->m_yawConstraints_LS.m_min;
					EulerAngles minEulerYaw		= EulerAngles( minYaw, actualEulerAngles.m_pitchDegrees, actualEulerAngles.m_rollDegrees ); 
					curJoint->m_eulerAnglesGoal_LS	= minEulerYaw;
					Vec3		dirMinYaw_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
					// Reset joint euler to actual values
					curJoint->m_eulerAnglesGoal_LS	= actualEulerAngles;

					// Compute dir_JointToTarget_MS: Transform joint and target into MS
					Vec3  jointPos_MS			= curJoint->GetMatrix_LocalToModel().GetTranslation3D();
					Mat44 wToM_Matrix			= curJoint->GetIkChainMatrix_ModelToWorld().GetOrthoNormalInverse();
					Vec3  targetPos_MS			= wToM_Matrix.TransformPosition3D( target.m_currentPos_WS );
					Vec3  dir_JointToTarget_MS	= ( targetPos_MS - jointPos_MS ).GetNormalized();
					// Compute dot results 
					float maxDotPositivity_Pitch	= DotProduct3D( dirMaxPitch_MS, dir_JointToTarget_MS );
					float minDotPositivity_Pitch	= DotProduct3D( dirMinPitch_MS, dir_JointToTarget_MS );
					float maxDotPositivity_Yaw		= DotProduct3D( dirMaxYaw_MS,	dir_JointToTarget_MS );
					float minDotPositivity_Yaw		= DotProduct3D( dirMinYaw_MS,	dir_JointToTarget_MS );
					// Compute 'elbowRoom' based on dotResult comparisons, choose the negative or lesser result
					float newPitch	 = 0.0f;
					float newYaw	 = 0.0f;
					float elbowRoom  = 0.0f;
					float curPitch	 = curJoint->m_eulerAnglesGoal_LS.m_pitchDegrees;
					float curYaw	 = curJoint->m_eulerAnglesGoal_LS.m_yawDegrees;


					// Toggle deadlock bend dir preference 
					if ( m_prevDeadlockCount != m_curDeadlockCount )
					{
// 						curJoint->ToggleDeadlockDir_Pitch();
// 						curJoint->ToggleDeadlockDir_Yaw();
					}
					//----------------------------------------------------------------------------------------------------------------------
					// BEND MORE using PITCH
					//----------------------------------------------------------------------------------------------------------------------
					// Pitch TOWARD target
					if ( curJoint->m_deadlockDir_Pitch == DEADLOCK_BEND_TOWARD )
					{
						// Choose more AGREE-ING dotResult_pitch
						if ( minDotPositivity_Pitch > maxDotPositivity_Pitch )
						{
							// Joint should pitch NEGATIVE to bend toward target
							elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_min );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newPitch = m_debtAngle * -1.0f;			// Debt angle is always positive, so we need to negative the angle to get a negative pitch
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newPitch = elbowRoom;
							}
						}
						else	// dotMaxPitch AGREES more than dotMinPitch
						{
							// Joint should pitch POSITIVE to bend toward target
							elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_max );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newPitch = m_debtAngle;
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newPitch = elbowRoom;
							}
						}
					}
					// Pitch AWAY from target
					else if ( curJoint->m_deadlockDir_Pitch == DEADLOCK_BEND_AWAY )
					{
						// Choose more DISAGREE-ING dotResult_pitch
						if ( minDotPositivity_Pitch < maxDotPositivity_Pitch )
						{
							// Joint should pitch NEGATIVE to bend away from target
							elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_min );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newPitch = m_debtAngle * -1.0f;			// Debt angle is always positive, so we need to negative the angle to get a negative pitch
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newPitch = elbowRoom;
							}
						}
						else	// dotMaxPitch DISAGREES more than dotMinPitch
						{
							// Joint should pitch POSITIVE to bend away from target
							elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_max );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newPitch = m_debtAngle;
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newPitch = elbowRoom;
							}
						}
					}
					curJoint->m_bendMore_pitch					= newPitch;
					curJoint->m_eulerAnglesGoal_LS.m_pitchDegrees  += newPitch;
					m_debtAngle								   -= fabsf( newPitch );

					//----------------------------------------------------------------------------------------------------------------------
					// BEND MORE using YAW
					//----------------------------------------------------------------------------------------------------------------------
					// Yaw TOWARD target
					if ( curJoint->m_deadlockDir_Yaw == DEADLOCK_BEND_TOWARD )
					{
						// Choose more AGREE-ING dotResult_yaw
						if ( minDotPositivity_Yaw > maxDotPositivity_Yaw )
						{
							// Joint should pitch NEGATIVE to bend toward target
							elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_min );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newYaw = m_debtAngle * -1.0f;		// Debt angle is always positive, so we need to negate the angle to get a negative yaw			
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newYaw = elbowRoom;
							}
						}
						else    // dotMaxYaw AGREES more than dotMinYaw
						{
							// Joint should pitch POSITIVE to bend toward target
							elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_max );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newYaw = m_debtAngle;
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newYaw = elbowRoom;
							}
						}
					}
					else if ( curJoint->m_deadlockDir_Yaw == DEADLOCK_BEND_AWAY )
					{
						// Choose DISAGREE-ING dotResult_yaw
						if ( minDotPositivity_Yaw < maxDotPositivity_Yaw )
						{
							// Joint should yaw NEGATIVE to bend away from target
							elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_min );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newYaw = m_debtAngle * -1.0f;		// Debt angle is always positive, so we need to negate the angle to get a negative yaw			
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newYaw = elbowRoom;
							}
						}
						else    // dotMaxYaw DISAGREES than dotMinYaw
						{
							// Joint should yaw POSITIVE to bend away from target
							elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_max );
							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
							{
								// If there's enough space for all the debt, let's pay all the debt
								newYaw = m_debtAngle;
							}
							else
							{
								// If there's NOT enough space for all the debt, lets pay the max amount we can
								newYaw = elbowRoom;
							}
						}
					}
					curJoint->m_bendMore_yaw					 = newYaw;
					curJoint->m_eulerAnglesGoal_LS.m_yawDegrees += newYaw;
					m_debtAngle									-= fabsf( newYaw );
				}
				else
				{
					curJoint->m_bendMore_yaw = 0.0f;
				}
			}
			curJoint->ClampYPR();
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Populate joint info for debug rendering
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D*	 curJoint		= m_jointList[i];
		Mat44 const& lToW_Matrix	= curJoint->GetMatrix_LocalToWorld();
		Vec3  const& jointPos_WS	= lToW_Matrix.GetTranslation3D();
		Vec3  const& fwdDir_WS		= lToW_Matrix.GetIBasis3D();
		Vec3  const& leftDir_WS		= lToW_Matrix.GetJBasis3D();
		Vec3  const& upDir_WS		= lToW_Matrix.GetKBasis3D();
		JointInfo jointInfo			= JointInfo( jointPos_WS, fwdDir_WS, leftDir_WS, upDir_WS );
		m_jointInfosThisIterList.push_back( jointInfo );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Keep track of best solution achieved this frame
	//----------------------------------------------------------------------------------------------------------------------
	float distAfterSolve_eeToTarget = GetDistEeToTarget( target );
	if ( distAfterSolve_eeToTarget < m_bestDistSolvedThisFrame )
	{
		// Update best solved dist if found
		m_bestDistSolvedThisFrame = distAfterSolve_eeToTarget;
	}
}


//----------------------------------------------------------------------------------------------------------------------
// Child to Root (endEffector to parent)
//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::FABRIK_Forward( Target target )
{
	// Update chainRootPosition before moving the chain
	if ( m_firstJoint )
	{
		m_chainRootPosBeforeFwdFABRIK_MS = m_firstJoint->m_jointPos_MS;
	}

	// Target becomes childStartPos every frame
	Target curTarget = target;
	for ( int i = m_finalJoint->m_jointIndex; i >= 0; i-- )
	{
		IK_Joint3D* curJoint	= m_jointList[i];
		IK_Joint3D* childJoint	= curJoint->m_child;
		if ( curJoint->m_jointConstraintType == JOINT_CONSTRAINT_TYPE_EULER )
		{
			// Update positions and IJK
			if ( curJoint->m_child != nullptr )
			{
				// Since curTarget.pos is the child joint's position in model space, no need to transform
				Vec3 fwdDir_MS				= ( childJoint->m_jointPos_MS - curJoint->m_jointPos_MS ).GetNormalized();
				curJoint->m_jointPos_MS		= childJoint->m_jointPos_MS - ( fwdDir_MS * curJoint->m_distToChild );
			}
			// Logic for the endEffector
			else
			{ 
				// Transform target WS to MS
 				Mat44 modelToWorldMatrix	= curJoint->GetIkChainMatrix_ModelToWorld();
 				Mat44 worldToModelMatrix	= modelToWorldMatrix.GetOrthoNormalInverse();
				// Set jointPos_MS (end effector) at targetPos_MS
				curJoint->m_jointPos_MS		= worldToModelMatrix.TransformPosition3D( curTarget.m_currentPos_WS );		// Relative to parent
			}

			// Update 
			// Transform chain root to match chain position/orientation
// 			if ( curJoint->m_child != nullptr )
// 			{
// 				// Create rootJoint matrix relative to target and child joint
// 				Vec3 rootFwd			= ( childJoint->m_jointPos_MS - curJoint->m_jointPos_MS ).GetNormalized();
// 				EulerAngles rootEuler	= curJoint->GetEulerFromFwdDir( rootFwd );
// 				Mat44 root_mToW_Matrix	= rootEuler.GetAsMatrix_XFwd_YLeft_ZUp();
// 				root_mToW_Matrix.SetTranslation3D( curJoint->m_jointPos_MS );
// 				// Update joint data
// 				curJoint->m_eulerAnglesGoal_LS = rootEuler;
// 				if ( curJoint->m_parent == nullptr )
// 				{
// 					// Append rootJointMatrix to target_mToW_Matrix;
// 					Mat44 target_mToW_Matrix( target.m_fwdDir, target.m_leftDir, target.m_upDir, target.m_currentPos_WS );
// 					target_mToW_Matrix.Append( root_mToW_Matrix );
// 					m_position_WS = target_mToW_Matrix.GetTranslation3D();
// 				}
// 			}
// 			else
// 			{
// 				Mat44 endJoint_mToW_Matrix( target.m_fwdDir, target.m_leftDir, target.m_upDir, target.m_currentPos_WS );
// 				curJoint->m_eulerAnglesGoal_LS = curJoint->GetEulerFromFwdDir( endJoint_mToW_Matrix.GetIBasis3D() );
// 			}
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Solve_FABRIK( Target target )
{
	// Save joint info from last frame in case the new solution computed this frame is worse
	// We can disregard the new solution and keep this (last frame) solution instead
	std::vector<JointInfo> prevFrameJointInfoList;
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* curJoint	= m_jointList[i];
		JointInfo   jointInfo	= JointInfo( curJoint->m_jointPos_MS, curJoint->m_eulerAnglesGoal_LS );		// Prev frame's joint info for the curJoint
		prevFrameJointInfoList.push_back( jointInfo );
	}

	// Reset variables that need to be recomputed per frame
	m_isTargetTooFar	= false;
	m_isDeadlock		= false;
	m_debtAngle			= 0.0f;
	m_curDeadlockCount  = 0;
//	ResetBendMoreAngles();
	m_solutionsThisFrameList.clear();
	m_jointInfosThisIterList.clear();
	m_deadlockDebugInfoList.clear();

	// Init common variables
	SaveDebugPose_curIter();
	float toleranceDist				= 0.0001f;
	float curIterDistToTarget		= 0.0f;
	m_prevFrameDistToTarget			= GetDistEeToTarget( target );
	m_breakFABRIK					= false;
	float chainMaxLength			= GetMaxChainLength();
	bool  triedDeadlockThisFrame	= false;
	int   resetCount				= 0;
	float bestDistToTarget			= m_prevFrameDistToTarget;
	for ( int i = 0; i < m_numIterations; i++ )
	{
		// Early out check: Is target is too far?
		float distRootToTarget = GetDistRootToTarget( target );
		if ( distRootToTarget > chainMaxLength )
		{
			// Solve one full pass 
			FABRIK_Forward ( target );		// Child to parent			
			FABRIK_Backward( target );		// Parent to child
			m_isTargetTooFar = true;
			m_iterCount		 = i + 1;
			break;
		}
		else  // Actually run FABRIK with deadlock solutions
		{
			// Early out check for convergence
			m_prevIterDistToTarget = GetDistEeToTarget( target );
			if ( m_prevIterDistToTarget <= toleranceDist )
			{
				break;
			}
			m_iterCount = i + 1;

			// Solve one full pass 
			FABRIK_Forward ( target );		// Child to parent			
			FABRIK_Backward( target );		// Parent to child


			if ( m_attemptDeadlockSolutions )
			{
				//----------------------------------------------------------------------------------------------------------------------
				// Check if deadlock solutions are required
				//----------------------------------------------------------------------------------------------------------------------
				curIterDistToTarget = GetDistEeToTarget( target );
				if ( curIterDistToTarget < m_prevIterDistToTarget )
				{
					// We got closer
					m_isDeadlock = false;

					// Check if curNew solution is better than lastFrame's solution
					if ( curIterDistToTarget < bestDistToTarget )
					{
						// Keep track of the new best solution
						prevFrameJointInfoList.clear();
						for ( int j = 0; j < m_jointList.size(); j++ )
						{
							IK_Joint3D* curJoint = m_jointList[ j ];
							JointInfo   jointInfo = JointInfo( curJoint->m_jointPos_MS, curJoint->m_eulerAnglesGoal_LS );		// Prev frame's joint info for the curJoint
							prevFrameJointInfoList.push_back( jointInfo );
						}
						bestDistToTarget = curIterDistToTarget;
					}
				}
				else
				{
					// Deadlock or solution this iteration is worse
					m_isDeadlock = true;
					// Only reset IF deadlock was already tried this frame AND
					// Its not the last iteration (to avoid rendering with a straight chain) AND 
					// Haven't straightened the chain this frame (Only allowed the straighten the chain TWICE per frame)
					if ( (m_curDeadlockCount > 10) && ( i < (m_numIterations - 1) ) && (resetCount < 2) )
					{
						curIterDistToTarget = GetDistEeToTarget( target );
						if ( curIterDistToTarget > 2.0f )
						{
							// If we've already tried a deadlock solution this frame, AND the solution is still 
							// too far, just reset all angles to 0
// 							if ( m_deadlockSolverType == DEADLOCK_SOLVER_RESET )
// 							{
// 								m_debtAngle = 0.0f;
// 								DeadlockSolver_Reset();
// 							}
// 							else if ( m_deadlockSolverType == DEADLOCK_SOLVER_RESET_AND_POSE_ROOT_TO_Z )
// 							{
// 								m_debtAngle = 0.0f;
// 								DeadlockSolver_ResetAndPoseRootZUp();
// 							}
 							ResetEulerForAllJoints();
							resetCount++;
	//						DebuggerPrintf( Stringf("Times reset Euler:  %i | iterCount this iteration:  %i | deadlockCount this iteration:  %i\n", resetCount, m_iterCount, m_curDeadlockCount).c_str() );
							std::string curDeadlockDebugInfo = Stringf("Euler Reset Count: %i | curIter: %i | deadlockCount this iter:  %i\n", resetCount, m_iterCount, m_curDeadlockCount );
							m_deadlockDebugInfoList.push_back( curDeadlockDebugInfo );
						}
					}
					else
					{
						triedDeadlockThisFrame	= true;	// This is set to true this iteration, but deadlock solutions will only be attempted next iteration
						m_prevDeadlockCount		= m_curDeadlockCount;
						m_curDeadlockCount++;
					}

					
					//----------------------------------------------------------------------------------------------------------------------
					// Compute 'debtAngle'
					//----------------------------------------------------------------------------------------------------------------------
					// eeParent_fwdDir in MS
					IK_Joint3D* eeParent			= m_finalJoint->m_parent;
					Mat44 lToM_Matrix				= eeParent->GetMatrix_LocalToModel(); 
					Vec3  eeParentFwdDir_MS			= lToM_Matrix.GetIBasis3D();
					Vec3  eeParentJointPos_MS		= lToM_Matrix.GetTranslation3D();
					// targetPos in MS
					Mat44 wToM_Matrix				= m_finalJoint->GetIkChainMatrix_ModelToWorld().GetOrthoNormalInverse();
					Vec3  targetPos_MS				= wToM_Matrix.TransformPosition3D( target.m_currentPos_WS );
					// Compute dispEeParentToTarget_MS
					Vec3  dir_eeParentToTarget_MS	= ( targetPos_MS - eeParentJointPos_MS ).GetNormalized();
					m_debtAngle						= GetAngleDegreesBetweenVectors3D( eeParentFwdDir_MS, dir_eeParentToTarget_MS );
				}
			}

			//----------------------------------------------------------------------------------------------------------------------
			// Populate list of solutionsPerIeration for debugging
			//----------------------------------------------------------------------------------------------------------------------
			SaveDebugPose_curIter();
// 			m_solutionsThisFrameList.push_back( m_jointInfosThisIterList );
// 			m_jointInfosThisIterList.clear();
		}
	}
	std::string curDeadlockDebugInfo = Stringf( "Total deadlockCount solutions this frame:  %i\n", resetCount, m_iterCount, m_curDeadlockCount );
	m_deadlockDebugInfoList.push_back( curDeadlockDebugInfo );
// 	DebuggerPrintf( Stringf( "Total deadlockCount this frame:  %i\n", resetCount, m_iterCount, m_curDeadlockCount ).c_str() );
// 	DebuggerPrintf( "----\n");


	//----------------------------------------------------------------------------------------------------------------------
	// Disregard new solution if worse than last frame's solution
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_prevFrameDistToTarget < curIterDistToTarget )
	{
		// Overwrite new jointPos_MS and eulerAngles_MS data with last frame's data
		for ( int i = 0; i < prevFrameJointInfoList.size(); i++ )
		{
			JointInfo&  prevFrameJointInfo		= prevFrameJointInfoList[i];
			IK_Joint3D* curJoint				= m_jointList[i];
			curJoint->m_jointPos_MS				= prevFrameJointInfo.m_jointPos_MS;
			curJoint->m_eulerAnglesGoal_LS		= prevFrameJointInfo.m_eulerAngles_MS;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
// Root to child
//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::FABRIK_Backward( Target target )
{
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* curJoint	= m_jointList[i];
		IK_Joint3D* parentJoint = curJoint->m_parent;
		IK_Joint3D* childJoint	= curJoint->m_child;

		if ( curJoint->m_jointConstraintType == JOINT_CONSTRAINT_TYPE_EULER )
		{
			if ( parentJoint )		// Update children joints
			{
				// Set curJointPos_MS relative to parent
				Vec3 parentFwdDir_MS		= ( curJoint->m_jointPos_MS - parentJoint->m_jointPos_MS ).GetNormalized();
				curJoint->m_jointPos_MS		= parentJoint->m_jointPos_MS + ( parentFwdDir_MS * parentJoint->m_distToChild );
				// Compute fwdDir_MS
				Vec3 fwdDir_MS;
				if ( childJoint )
				{
					fwdDir_MS = childJoint->m_jointPos_MS - curJoint->m_jointPos_MS;
					fwdDir_MS.Normalize();
				}
				else
				{
					fwdDir_MS = parentFwdDir_MS;
				}
 				// Transform fwdDir_MS to fwdDir_LS
 				Mat44 localToModelMatrix	= curJoint->m_parent->GetMatrix_LocalToModel();
 				Mat44 modelToLocalMatrix	= localToModelMatrix.GetOrthoNormalInverse();
				Vec3  fwdDir_LS				= modelToLocalMatrix.TransformVectorQuantity3D( fwdDir_MS );
				// Update and clamp eulerAngles_LS 
				curJoint->m_eulerAnglesGoal_LS	= curJoint->GetEulerFromFwdDir( fwdDir_LS );
				curJoint->ClampYPR();


				//----------------------------------------------------------------------------------------------------------------------
				// Bend more if isDeadlock
				//----------------------------------------------------------------------------------------------------------------------
				if ( m_isDeadlock )
				{
					// Bend more attempt2: distribute debtAngle per joint across chain
//					if ( !CompareIfFloatsAreEqual( m_debtAngle, 0.0f, 0.01f ) )
//					{
//						float elbowRoom	= curJoint->m_pitchConstraints_LS.m_max - curJoint->m_eulerAngles_LS.m_pitchDegrees;
//						float newPitch	= 0.0f;
//						if ( elbowRoom >= m_debtAngle )
//						{
//							// If there's enough space for all the debt, let's pay all the debt
//							newPitch = m_debtAngle;
//						}
//						else
//						{
//							// If there's NOT enough space for all the debt, lets pay the max amount we can
//							newPitch = elbowRoom;
//						}
////						newPitch								   *= -1.0f;
//						curJoint->m_bendMore_pitch					= newPitch;
//						curJoint->m_eulerAngles_LS.m_pitchDegrees  += newPitch;
//						m_debtAngle								   -= newPitch;
//					}
//					else
//					{
//						curJoint->m_bendMore_pitch = 0.0f;
//					}

					// Bend more attempt3: distribute debtAngle per joint across chain in the NEGATIVE dot direction
// 					if ( !CompareIfFloatsAreEqual( m_debtAngle, 0.0f, 0.01f ) )
// 					{
// 						EulerAngles actualEulerAngles = curJoint->m_eulerAngles_LS;
// 						// Compute MAX pitch dir MS
// 						float		maxPitch		= curJoint->m_pitchConstraints_LS.m_max;
// 						EulerAngles maxEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, maxPitch, actualEulerAngles.m_rollDegrees ); 
// 						curJoint->m_eulerAngles_LS	= maxEulerPitch;
// 						Vec3		dirMaxPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
// 						// Compute MIN pitch dir MS
// 						float		minPitch		= curJoint->m_pitchConstraints_LS.m_min;
// 						EulerAngles minEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, minPitch, actualEulerAngles.m_rollDegrees ); 
// 						curJoint->m_eulerAngles_LS	= minEulerPitch;
// 						Vec3		dirMinPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
// 						// Reset joint euler to actual values
// 						curJoint->m_eulerAngles_LS	= actualEulerAngles;
// 						// Compute dir_JointToTarget_MS: Transform joint and target into MS
// 						Vec3  jointPos_MS			= curJoint->GetMatrix_LocalToModel().GetTranslation3D();
// 						Mat44 wToM_Matrix			= curJoint->GetIkChainMatrix_ModelToWorld().GetOrthoNormalInverse();
// 						Vec3  targetPos_MS			= wToM_Matrix.TransformPosition3D( target.m_currentPos_WS );
// 						Vec3 dir_JointToTarget_MS	= ( targetPos_MS - jointPos_MS ).GetNormalized();
// 						// Compute dot results 
// 						float maxDotResult			= DotProduct3D( dirMaxPitch_MS, dir_JointToTarget_MS );
// 						float minDotResult			= DotProduct3D( dirMinPitch_MS, dir_JointToTarget_MS );
// 						// Compute 'elbowRoom'based after comparing dotResults, choose the negative or lesser result
// 						float newPitch	 = 0.0f;
// 						float elbowRoom  = 0.0f;
// 						float curPitch	 = curJoint->m_eulerAngles_LS.m_pitchDegrees;
// 						if ( minDotResult < maxDotResult )
// 						{
// 							// Joint should pitch NEGATIVE to bend away from target
// 							elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_min );
// 							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 							{
// 								// If there's enough space for all the debt, let's pay all the debt
// 								newPitch = m_debtAngle * -1.0f;			// Debt angle is always positive, so we need to negative the angle to get a negative pitch
// 							}
// 							else
// 							{
// 								// If there's NOT enough space for all the debt, lets pay the max amount we can
// 								newPitch = elbowRoom;
// 							}
// 						}
// 						else
// 						{
// 							// Joint should pitch POSITIVE to bend away from target
// 							elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_max );
// 							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 							{
// 								// If there's enough space for all the debt, let's pay all the debt
// 								newPitch = m_debtAngle;
// 							}
// 							else
// 							{
// 								// If there's NOT enough space for all the debt, lets pay the max amount we can
// 								newPitch = elbowRoom;
// 							}
// 						}
// 						curJoint->m_bendMore_pitch					= newPitch;
// 						curJoint->m_eulerAngles_LS.m_pitchDegrees  += newPitch;
// 						m_debtAngle								   -= fabsf( newPitch );
// 					}
// 					else
// 					{
// 						curJoint->m_bendMore_pitch = 0.0f;
// 					}

					// Deadlock Attempt 4: Distribute debtAngle using pitch AND yaw
// 					if ( !CompareIfFloatsAreEqual( m_debtAngle, 0.0f, 0.01f ) )
// 					{
// 						// Compute directions for MIN & MAX yaw and MIN & MAX pitch
// 						EulerAngles actualEulerAngles = curJoint->m_eulerAnglesGoal_LS;
// 						// MAX pitch dir MS
// 						float		maxPitch		= curJoint->m_pitchConstraints_LS.m_max;
// 						EulerAngles maxEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, maxPitch, actualEulerAngles.m_rollDegrees ); 
// 						curJoint->m_eulerAnglesGoal_LS	= maxEulerPitch;
// 						Vec3		dirMaxPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
// 						// MIN pitch dir MS
// 						float		minPitch		= curJoint->m_pitchConstraints_LS.m_min;
// 						EulerAngles minEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, minPitch, actualEulerAngles.m_rollDegrees ); 
// 						curJoint->m_eulerAnglesGoal_LS	= minEulerPitch;
// 						Vec3		dirMinPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
// 						// MAX yaw dir MS
// 						float		maxYaw			= curJoint->m_yawConstraints_LS.m_max;
// 						EulerAngles maxEulerYaw		= EulerAngles( maxYaw, actualEulerAngles.m_pitchDegrees, actualEulerAngles.m_rollDegrees ); 
// 						curJoint->m_eulerAnglesGoal_LS	= maxEulerPitch;
// 						Vec3		dirMaxYaw_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
// 						// MIN yaw dir MS
// 						float		minYaw			= curJoint->m_yawConstraints_LS.m_min;
// 						EulerAngles minEulerYaw		= EulerAngles( minYaw, actualEulerAngles.m_pitchDegrees, actualEulerAngles.m_rollDegrees ); 
// 						curJoint->m_eulerAnglesGoal_LS	= minEulerYaw;
// 						Vec3		dirMinYaw_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
// 						// Reset joint euler to actual values
// 						curJoint->m_eulerAnglesGoal_LS	= actualEulerAngles;
// 
// 						// Compute dir_JointToTarget_MS: Transform joint and target into MS
// 						Vec3  jointPos_MS			= curJoint->GetMatrix_LocalToModel().GetTranslation3D();
// 						Mat44 wToM_Matrix			= curJoint->GetIkChainMatrix_ModelToWorld().GetOrthoNormalInverse();
// 						Vec3  targetPos_MS			= wToM_Matrix.TransformPosition3D( target.m_currentPos_WS );
// 						Vec3  dir_JointToTarget_MS	= ( targetPos_MS - jointPos_MS ).GetNormalized();
// 						// Compute dot results 
// 						float maxDotPositivity_Pitch	= DotProduct3D( dirMaxPitch_MS, dir_JointToTarget_MS );
// 						float minDotPositivity_Pitch	= DotProduct3D( dirMinPitch_MS, dir_JointToTarget_MS );
// 						float maxDotPositivity_Yaw		= DotProduct3D( dirMaxYaw_MS,	dir_JointToTarget_MS );
// 						float minDotPositivity_Yaw		= DotProduct3D( dirMinYaw_MS,	dir_JointToTarget_MS );
// 						// Compute 'elbowRoom' based on dotResult comparisons, choose the negative or lesser result
// 						float newPitch	 = 0.0f;
// 						float newYaw	 = 0.0f;
// 						float elbowRoom  = 0.0f;
// 						float curPitch	 = curJoint->m_eulerAnglesGoal_LS.m_pitchDegrees;
// 						float curYaw	 = curJoint->m_eulerAnglesGoal_LS.m_yawDegrees;
// 
// 
// 						// Toggle deadlock bend dir preference 
// 						if ( m_prevDeadlockCount != m_curDeadlockCount )
// 						{
// 							curJoint->ToggleDeadlockDir_Pitch();
// 							curJoint->ToggleDeadlockDir_Yaw();
// 						}
// 						//----------------------------------------------------------------------------------------------------------------------
// 						// BEND MORE using PITCH
// 						//----------------------------------------------------------------------------------------------------------------------
// 						// Pitch TOWARD target
// 						if ( curJoint->m_deadlockDir_Pitch == DEADLOCK_BEND_TOWARD )
// 						{
// 							// Choose more AGREE-ING dotResult_pitch
// 							if ( minDotPositivity_Pitch > maxDotPositivity_Pitch )
// 							{
// 								// Joint should pitch NEGATIVE to bend toward target
// 								elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_min );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newPitch = m_debtAngle * -1.0f;			// Debt angle is always positive, so we need to negative the angle to get a negative pitch
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newPitch = elbowRoom;
// 								}
// 							}
// 							else	// dotMaxPitch AGREES more than dotMinPitch
// 							{
// 								// Joint should pitch POSITIVE to bend toward target
// 								elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_max );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newPitch = m_debtAngle;
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newPitch = elbowRoom;
// 								}
// 							}
// 						}
// 						// Pitch AWAY from target
// 						else if ( curJoint->m_deadlockDir_Pitch == DEADLOCK_BEND_AWAY )
// 						{
// 							// Choose more DISAGREE-ING dotResult_pitch
// 							if ( minDotPositivity_Pitch < maxDotPositivity_Pitch )
// 							{
// 								// Joint should pitch NEGATIVE to bend away from target
// 								elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_min );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newPitch = m_debtAngle * -1.0f;			// Debt angle is always positive, so we need to negative the angle to get a negative pitch
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newPitch = elbowRoom;
// 								}
// 							}
// 							else	// dotMaxPitch DISAGREES more than dotMinPitch
// 							{
// 								// Joint should pitch POSITIVE to bend away from target
// 								elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_max );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newPitch = m_debtAngle;
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newPitch = elbowRoom;
// 								}
// 							}
// 						}
// 						curJoint->m_bendMore_pitch					= newPitch;
// 						curJoint->m_eulerAnglesGoal_LS.m_pitchDegrees  += newPitch;
// 						m_debtAngle								   -= fabsf( newPitch );
// 
// 						//----------------------------------------------------------------------------------------------------------------------
// 						// BEND MORE using YAW
// 						//----------------------------------------------------------------------------------------------------------------------
// 						// Yaw TOWARD target
// 						if ( curJoint->m_deadlockDir_Yaw == DEADLOCK_BEND_TOWARD )
// 						{
// 							// Choose more AGREE-ING dotResult_yaw
// 							if ( minDotPositivity_Yaw > maxDotPositivity_Yaw )
// 							{
// 								// Joint should pitch NEGATIVE to bend toward target
// 								elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_min );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newYaw = m_debtAngle * -1.0f;		// Debt angle is always positive, so we need to negate the angle to get a negative yaw			
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newYaw = elbowRoom;
// 								}
// 							}
// 							else    // dotMaxYaw AGREES more than dotMinYaw
// 							{
// 								// Joint should pitch POSITIVE to bend toward target
// 								elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_max );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newYaw = m_debtAngle;
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newYaw = elbowRoom;
// 								}
// 							}
// 						}
// 						else if ( curJoint->m_deadlockDir_Yaw == DEADLOCK_BEND_AWAY )
// 						{
// 							// Choose DISAGREE-ING dotResult_yaw
// 							if ( minDotPositivity_Yaw < maxDotPositivity_Yaw )
// 							{
// 								// Joint should yaw NEGATIVE to bend away from target
// 								elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_min );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newYaw = m_debtAngle * -1.0f;		// Debt angle is always positive, so we need to negate the angle to get a negative yaw			
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newYaw = elbowRoom;
// 								}
// 							}
// 							else    // dotMaxYaw DISAGREES than dotMinYaw
// 							{
// 								// Joint should yaw POSITIVE to bend away from target
// 								elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_max );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newYaw = m_debtAngle;
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newYaw = elbowRoom;
// 								}
// 							}
// 						}
// 						curJoint->m_bendMore_yaw					 = newYaw;
// 						curJoint->m_eulerAnglesGoal_LS.m_yawDegrees += newYaw;
// 						m_debtAngle									-= fabsf( newYaw );
// 					}
// 					else
// 					{
// 						curJoint->m_bendMore_yaw = 0.0f;
// 					}

					// Filtering Deadlock solvers
					if ( m_deadlockSolverType == DEADLOCK_SOLVER_DEBT_ANGLE )
					{
						// Deadlock Attempt 4: Distribute debtAngle using pitch AND yaw
						DeadlockSolver_DebtAngle( curJoint, target );
					}
// 					else if ( m_deadlockSolverType == DEADLOCK_SOLVER_RESET )
// 					{
// 						m_debtAngle = 0.0f;
// 						DeadlockSolver_Reset( curJoint, target );
// 					}
// 					else if ( m_deadlockSolverType == DEADLOCK_SOLVER_RESET_AND_POSE_ROOT_TO_Z )
// 					{
// 						m_debtAngle = 0.0f;
// 						DeadlockSolver_ResetAndPoseRootZUp( curJoint, target );
// 					}
				}
				curJoint->ClampYPR();

				// Update MS positions
				localToModelMatrix		= curJoint->GetMatrix_LocalToModel();
				curJoint->m_jointPos_MS = localToModelMatrix.GetTranslation3D();
			}
			else // curJoint is rootJoint
			{
				// Set rootPos_MS to "origin"
				curJoint->m_jointPos_MS	= m_chainRootPosBeforeFwdFABRIK_MS;
				// Compute fwdDir_MS
				Vec3 fwdDir_MS;
				if ( childJoint )
				{
					// Point fwdDir towards childJoint if it exists
					fwdDir_MS = childJoint->m_jointPos_MS - curJoint->m_jointPos_MS;
				}
				else
				{
					// Point fwdDir towards target if childJoint does NOT exists
					Mat44 modelToWorldMatrix	= curJoint->GetMatrix_LocalToWorld();
					Mat44 worldToModelMatrix	= modelToWorldMatrix.GetOrthoNormalInverse();
					Vec3 targetPos_MS			= worldToModelMatrix.TransformPosition3D( target.m_currentPos_WS );
					fwdDir_MS					= targetPos_MS - curJoint->m_jointPos_MS;
				}
				fwdDir_MS.Normalize();
				// Update and clamp eulerAngles_LS 
 				// No need to transform fwdDir MS to LS since its the same space for the root joint
				curJoint->m_eulerAnglesGoal_LS = curJoint->GetEulerFromFwdDir( fwdDir_MS );
				curJoint->ClampYPR();


				//----------------------------------------------------------------------------------------------------------------------
				// Bend more if isDeadlock
				//----------------------------------------------------------------------------------------------------------------------
				if ( m_isDeadlock )
				{
					// Bend more attempt1: max pitch root joint
//					curJoint->m_eulerAngles_LS.m_pitchDegrees = curJoint->m_pitchConstraints_LS.m_max;

 					// Bend more attempt2: distribute debtAngle per joint across chain
//  					if ( !CompareIfFloatsAreEqual( m_debtAngle, 0.0f, 0.01f ) )
//  					{
//  						float elbowRoom = curJoint->m_pitchConstraints_LS.m_max - curJoint->m_eulerAngles_LS.m_pitchDegrees;
//  //						float elbowRoom = 0.0f;
//  // 						if ( m_debtAngle > 0.0f )
//  // 						{
//  // 							// If debtAngle is POSITIVE, this joint needs to apply NEGATIVE pitch
//  // 							elbowRoom = curJoint->m_pitchConstraints_LS.m_min - curJoint->m_eulerAngles_LS.m_pitchDegrees;
//  // 						}
//  // 						else
//  // 						{
//  // 							// If debtAngle is NEGATIVE, this joint needs to apply POSITIVE pitch
//  // 							elbowRoom = curJoint->m_pitchConstraints_LS.m_max - curJoint->m_eulerAngles_LS.m_pitchDegrees;
//  // 						}
//  						float newPitch	= 0.0f;
//  						if ( elbowRoom >= m_debtAngle )
//  						{
//  							// If there's enough space for all the debt, let's pay all the debt
//  							newPitch = m_debtAngle;
//  						}
//  						else
//  						{
//  							// If there's NOT enough space for all the debt, lets pay the max amount we can
//  							newPitch = elbowRoom;
//  						}
//  //						newPitch								   *= -1.0f;
//  						curJoint->m_bendMore_pitch					= newPitch;
//  						curJoint->m_eulerAngles_LS.m_pitchDegrees  += newPitch;
//  						m_debtAngle								   -= newPitch;
//  					}
//  					else
//  					{
//  						curJoint->m_bendMore_pitch = 0.0f;
//  					}

					// Bend more attempt3: distribute debtAngle per joint across chain in the NEGATIVE dot direction
//					if ( !CompareIfFloatsAreEqual( m_debtAngle, 0.0f, 0.01f ) )
//					{
//						EulerAngles actualEulerAngles = curJoint->m_eulerAngles_LS;
//						// Compute MAX pitch dir MS
//						float		maxPitch		= curJoint->m_pitchConstraints_LS.m_max;
//						EulerAngles maxEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, maxPitch, actualEulerAngles.m_rollDegrees ); 
//						curJoint->m_eulerAngles_LS	= maxEulerPitch;
//						Vec3		dirMaxPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
//						// Compute MIN pitch dir MS
//						float		minPitch		= curJoint->m_pitchConstraints_LS.m_min;
//						EulerAngles minEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, minPitch, actualEulerAngles.m_rollDegrees ); 
//						curJoint->m_eulerAngles_LS	= minEulerPitch;
//						Vec3		dirMinPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
//						// Reset joint euler to actual values
//						curJoint->m_eulerAngles_LS	= actualEulerAngles;
//						// Compute dir_JointToTarget_MS: Transform joint and target into MS
//						Vec3  jointPos_MS			= curJoint->GetMatrix_LocalToModel().GetTranslation3D();
//						Mat44 wToM_Matrix			= curJoint->GetIkChainMatrix_ModelToWorld().GetOrthoNormalInverse();
//						Vec3  targetPos_MS			= wToM_Matrix.TransformPosition3D( target.m_currentPos_WS );
//						Vec3 dir_JointToTarget_MS	= ( targetPos_MS - jointPos_MS ).GetNormalized();
//						// Compute dot results 
//						float maxDotResult			= DotProduct3D( dirMaxPitch_MS, dir_JointToTarget_MS );
//						float minDotResult			= DotProduct3D( dirMinPitch_MS, dir_JointToTarget_MS );
//						// Compute 'elbowRoom'based after comparing dotResults, choose the negative or lesser result
//						float newPitch	 = 0.0f;
//						float elbowRoom  = 0.0f;
//						float curPitch	 = curJoint->m_eulerAngles_LS.m_pitchDegrees;
//						if ( minDotResult < maxDotResult )
//						{
//							// Joint should pitch NEGATIVE to bend away from target
//							elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_min );
//							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
//							{
//								// If there's enough space for all the debt, let's pay all the debt
//								newPitch = m_debtAngle * -1.0f;
//							}
//							else
//							{
//								// If there's NOT enough space for all the debt, lets pay the max amount we can
//								newPitch = elbowRoom;
//							}
//						}
//						else
//						{
//							// Joint should pitch POSITIVE to bend away from target
//							elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_max );
//							if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
//							{
//								// If there's enough space for all the debt, let's pay all the debt
//								newPitch = m_debtAngle;
//							}
//							else
//							{
//								// If there's NOT enough space for all the debt, lets pay the max amount we can
//								newPitch = elbowRoom;
//							}
//						}
//						curJoint->m_bendMore_pitch					= newPitch;
//						curJoint->m_eulerAngles_LS.m_pitchDegrees  += newPitch;
//						m_debtAngle								   -= newPitch;
//					}
//					else
//					{
//						curJoint->m_bendMore_pitch = 0.0f;
// 					}

// 					// Deadlock Attempt 4: Distribute debtAngle using pitch AND yaw
// 					if ( !CompareIfFloatsAreEqual( m_debtAngle, 0.0f, 0.01f ) )
// 					{
// 						// Compute directions for MIN & MAX yaw and MIN & MAX pitch
// 						EulerAngles actualEulerAngles = curJoint->m_eulerAnglesGoal_LS;
// 						// MAX pitch dir MS
// 						float		maxPitch		= curJoint->m_pitchConstraints_LS.m_max;
// 						EulerAngles maxEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, maxPitch, actualEulerAngles.m_rollDegrees ); 
// 						curJoint->m_eulerAnglesGoal_LS	= maxEulerPitch;
// 						Vec3		dirMaxPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
// 						// MIN pitch dir MS
// 						float		minPitch		= curJoint->m_pitchConstraints_LS.m_min;
// 						EulerAngles minEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, minPitch, actualEulerAngles.m_rollDegrees ); 
// 						curJoint->m_eulerAnglesGoal_LS	= minEulerPitch;
// 						Vec3		dirMinPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
// 						// MAX yaw dir MS
// 						float		maxYaw			= curJoint->m_yawConstraints_LS.m_max;
// 						EulerAngles maxEulerYaw		= EulerAngles( maxYaw, actualEulerAngles.m_pitchDegrees, actualEulerAngles.m_rollDegrees ); 
// 						curJoint->m_eulerAnglesGoal_LS	= maxEulerPitch;
// 						Vec3		dirMaxYaw_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
// 						// MIN yaw dir MS
// 						float		minYaw			= curJoint->m_yawConstraints_LS.m_min;
// 						EulerAngles minEulerYaw		= EulerAngles( minYaw, actualEulerAngles.m_pitchDegrees, actualEulerAngles.m_rollDegrees ); 
// 						curJoint->m_eulerAnglesGoal_LS	= minEulerYaw;
// 						Vec3		dirMinYaw_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
// 						// Reset joint euler to actual values
// 						curJoint->m_eulerAnglesGoal_LS	= actualEulerAngles;
// 
// 						// Compute dir_JointToTarget_MS: Transform joint and target into MS
// 						Vec3  jointPos_MS			= curJoint->GetMatrix_LocalToModel().GetTranslation3D();
// 						Mat44 wToM_Matrix			= curJoint->GetIkChainMatrix_ModelToWorld().GetOrthoNormalInverse();
// 						Vec3  targetPos_MS			= wToM_Matrix.TransformPosition3D( target.m_currentPos_WS );
// 						Vec3 dir_JointToTarget_MS	= ( targetPos_MS - jointPos_MS ).GetNormalized();
// 						// Compute dot results 
// 						float maxDotPositivity_Pitch	= DotProduct3D( dirMaxPitch_MS, dir_JointToTarget_MS );
// 						float minDotPositivity_Pitch	= DotProduct3D( dirMinPitch_MS, dir_JointToTarget_MS );
// 						float maxDotPositivity_Yaw		= DotProduct3D( dirMaxYaw_MS,	dir_JointToTarget_MS );
// 						float minDotPositivity_Yaw		= DotProduct3D( dirMinYaw_MS,	dir_JointToTarget_MS );
// 						// Compute 'elbowRoom' based on dotResult comparisons, choose the negative or lesser result
// 						float newPitch	 = 0.0f;
// 						float newYaw	 = 0.0f;
// 						float elbowRoom  = 0.0f;
// 						float curPitch	 = curJoint->m_eulerAnglesGoal_LS.m_pitchDegrees;
// 						float curYaw	 = curJoint->m_eulerAnglesGoal_LS.m_yawDegrees;
// 						
// 						// Toggle deadlock bend dir preference 
// 						if ( m_prevDeadlockCount != m_curDeadlockCount )
// 						{
// 							curJoint->ToggleDeadlockDir_Pitch();
// 							curJoint->ToggleDeadlockDir_Yaw();
// 						}
// 						//----------------------------------------------------------------------------------------------------------------------
// 						// BEND MORE using PITCH
// 						//----------------------------------------------------------------------------------------------------------------------
// 						// Pitch TOWARD target
// 						if ( curJoint->m_deadlockDir_Pitch == DEADLOCK_BEND_TOWARD )
// 						{
// 							// Choose more AGREE-ING dotResult_pitch
// 							if ( minDotPositivity_Pitch > maxDotPositivity_Pitch )
// 							{
// 								// Joint should pitch NEGATIVE to bend toward target
// 								elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_min );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newPitch = m_debtAngle * -1.0f;			// Debt angle is always positive, so we need to negative the angle to get a negative pitch
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newPitch = elbowRoom;
// 								}
// 							}
// 							else	// dotMaxPitch AGREES more than dotMinPitch
// 							{
// 								// Joint should pitch POSITIVE to bend toward target
// 								elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_max );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newPitch = m_debtAngle;
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newPitch = elbowRoom;
// 								}
// 							}
// 						}
// 						// Pitch AWAY from target
// 						else if ( curJoint->m_deadlockDir_Pitch == DEADLOCK_BEND_AWAY )
// 						{
// 							// Choose more DISAGREE-ING dotResult_pitch
// 							if ( minDotPositivity_Pitch < maxDotPositivity_Pitch )
// 							{
// 								// Joint should pitch NEGATIVE to bend away from target
// 								elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_min );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newPitch = m_debtAngle * -1.0f;			// Debt angle is always positive, so we need to negative the angle to get a negative pitch
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newPitch = elbowRoom;
// 								}
// 							}
// 							else	// dotMaxPitch DISAGREES more than dotMinPitch
// 							{
// 								// Joint should pitch POSITIVE to bend away from target
// 								elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_max );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newPitch = m_debtAngle;
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newPitch = elbowRoom;
// 								}
// 							}
// 						}
// 						curJoint->m_bendMore_pitch					= newPitch;
// 						curJoint->m_eulerAnglesGoal_LS.m_pitchDegrees  += newPitch;
// 						m_debtAngle								   -= fabsf( newPitch );
// 
// 						//----------------------------------------------------------------------------------------------------------------------
// 						// BEND MORE using YAW
// 						//----------------------------------------------------------------------------------------------------------------------
// 						// Yaw TOWARD target
// 						if ( curJoint->m_deadlockDir_Yaw == DEADLOCK_BEND_TOWARD )
// 						{
// 							// Choose more AGREE-ING dotResult_yaw
// 							if ( minDotPositivity_Yaw > maxDotPositivity_Yaw )
// 							{
// 								// Joint should pitch NEGATIVE to bend toward target
// 								elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_min );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newYaw = m_debtAngle * -1.0f;		// Debt angle is always positive, so we need to negate the angle to get a negative yaw			
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newYaw = elbowRoom;
// 								}
// 							}
// 							else    // dotMaxYaw AGREES more than dotMinYaw
// 							{
// 								// Joint should pitch POSITIVE to bend toward target
// 								elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_max );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newYaw = m_debtAngle;
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newYaw = elbowRoom;
// 								}
// 							}
// 						}
// 						else if ( curJoint->m_deadlockDir_Yaw == DEADLOCK_BEND_AWAY )
// 						{
// 							// Choose DISAGREE-ING dotResult_yaw
// 							if ( minDotPositivity_Yaw < maxDotPositivity_Yaw )
// 							{
// 								// Joint should yaw NEGATIVE to bend away from target
// 								elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_min );
// 								if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newYaw = m_debtAngle * -1.0f;		// Debt angle is always positive, so we need to negate the angle to get a negative yaw			
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newYaw = elbowRoom;
// 								}
// 							}
// 							else    // dotMaxYaw DISAGREES than dotMinYaw
// 							{
// 								// Joint should yaw POSITIVE to bend away from target
// 								elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_max );
// 								if ( fabsf( elbowRoom ) >= fabsf( m_debtAngle ) )
// 								{
// 									// If there's enough space for all the debt, let's pay all the debt
// 									newYaw = m_debtAngle;
// 								}
// 								else
// 								{
// 									// If there's NOT enough space for all the debt, lets pay the max amount we can
// 									newYaw = elbowRoom;
// 								}
// 							}
// 						}
// 						curJoint->m_bendMore_yaw = newYaw;
// 						curJoint->m_eulerAnglesGoal_LS.m_yawDegrees += newYaw;
// 						m_debtAngle -= fabsf( newYaw );
// 					}
// 					else
// 					{
// 					curJoint->m_bendMore_yaw = 0.0f;
// 					}

					// Filtering Deadlock solvers
					if ( m_deadlockSolverType == DEADLOCK_SOLVER_DEBT_ANGLE )
					{
						// Deadlock Attempt 4: Distribute debtAngle using pitch AND yaw
						DeadlockSolver_DebtAngle( curJoint, target );
					}
// 					else if ( m_deadlockSolverType == DEADLOCK_SOLVER_RESET )
// 					{
// 						m_debtAngle = 0.0f;
// 						DeadlockSolver_Reset( curJoint, target );
// 					}
// 					else if ( m_deadlockSolverType == DEADLOCK_SOLVER_RESET_AND_POSE_ROOT_TO_Z )
// 					{
// 						m_debtAngle = 0.0f;
// 						DeadlockSolver_ResetAndPoseRootZUp( curJoint, target );
// 					}
				}
				curJoint->ClampYPR();

				// Update MS positions
				Mat44 localToModelMatrix = curJoint->GetMatrix_LocalToModel();
				curJoint->m_jointPos_MS  = localToModelMatrix.GetTranslation3D();
			}


			//----------------------------------------------------------------------------------------------------------------------
			// Populate joint info
			//----------------------------------------------------------------------------------------------------------------------
//			SaveDebugPose_curIter( curJoint );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::Solve_FABRIK_fwdOnly( Target target )
{
	// Update chain euler angle and position relative to the root
// 	Vec3 rootFwd_WS		= m_firstJoint->m_eulerAnglesGoal_LS.GetForwardDir_XFwd_YLeft_ZUp();
// 	m_eulerAngles_WS	= m_firstJoint->GetEulerFromFwdDir( rootFwd_WS );
// 	m_position_WS		= m_firstJoint->m_jointPos_MS;
	// Solve one full pass 
	FABRIK_fwdOnly( target );		// Child to parent			
	
	// Update chain euler and position AGAIN after solving
// 	rootFwd_WS			= m_firstJoint->m_eulerAnglesGoal_LS.GetForwardDir_XFwd_YLeft_ZUp();
// 	m_eulerAngles_WS	= m_firstJoint->GetEulerFromFwdDir( rootFwd_WS );
// 	m_position_WS		= m_firstJoint->m_jointPos_MS;
}


//----------------------------------------------------------------------------------------------------------------------
// Child to Root (endEffector to parent)
//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::FABRIK_fwdOnly( Target target )
{
	for ( int i = m_finalJoint->m_jointIndex; i >= 0; i-- )
	{
		IK_Joint3D* curJoint	= m_jointList[i];
		IK_Joint3D* childJoint	= curJoint->m_child;
		if ( curJoint->m_jointConstraintType == JOINT_CONSTRAINT_TYPE_EULER )
		{
			// Update positions and IJK
			if ( curJoint->m_child != nullptr )
			{
				// Since curTarget.pos is the child joint's position in model space, no need to transform
				Vec3 fwdDir_MS				= ( childJoint->m_jointPos_MS - curJoint->m_jointPos_MS ).GetNormalized();
				curJoint->m_jointPos_MS		= childJoint->m_jointPos_MS - ( fwdDir_MS * curJoint->m_distToChild );
				// Update euler from disps
				// 1. transform fwdDir MS to LS
				Vec3 fwdDir_LS				= curJoint->GetMatrix_LocalToModel().GetOrthoNormalInverse().TransformVectorQuantity3D( fwdDir_MS );
				// 2. Convert matrix to Euler
				curJoint->m_eulerAnglesGoal_LS = curJoint->GetEulerFromFwdDir( fwdDir_LS );
			}
			// Logic for the endEffector
			else
			{
				// Transform target WS to MS
				Mat44 modelToWorldMatrix		= curJoint->GetIkChainMatrix_ModelToWorld();
				Mat44 worldToModelMatrix		= modelToWorldMatrix.GetOrthoNormalInverse();
				// Set jointPos_MS (end effector) at targetPos_MS
				curJoint->m_jointPos_MS			= worldToModelMatrix.TransformPosition3D( target.m_currentPos_WS );		// Relative to parent
				Vec3 targetFwd_LS				= worldToModelMatrix.TransformVectorQuantity3D( target.m_fwdDir  );
				Vec3 targetLeft_LS 				= worldToModelMatrix.TransformVectorQuantity3D( target.m_leftDir );
				curJoint->m_eulerAnglesGoal_LS	= curJoint->GetEulerFromFwdAndLeft( targetFwd_LS, targetLeft_LS );
			}
		}
	}	
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::SolveTwoBoneIK_TriangulationMethod( Target target )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Bend the entire chain more using the "Triangulation Method"
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_creatureOwner != nullptr )
	{
		if ( m_jointList.size() != 3 )
		{
			ERROR_RECOVERABLE( "The 2BoneIK chain DOESN'T have 3 joints!" );
		}
		// Reset common variables per frame
		m_solutionsThisFrameList.clear();

		float distRootToTarget	= GetDistRootToTarget( target );
		float maxChainLength	= GetMaxChainLength();
		if ( distRootToTarget > maxChainLength )
		{
			// ONLY Run 1 iteration of triangulation algorithm if target is too far away
			ComputeBendAngle_Cos_Sine( target );
		}
		else
		{
			// Save best solution from prev frame
			std::vector<JointInfo> jointInfoList_prevFrame;
			for ( int i = 0; i < m_jointList.size(); i++ )
			{
				IK_Joint3D* curJoint	= m_jointList[ i ];
				//			JointInfo  jointInfo = JointInfo( curJoint->m_jointPos_MS, curJoint->m_eulerAngles_LS );
				Mat44	  lToW_Matrix	= curJoint->GetMatrix_LocalToWorld();
				Vec3	  jointPos_WS	= lToW_Matrix.GetTranslation3D();
				Vec3 const& fwdDir_WS	= lToW_Matrix.GetIBasis3D();
				Vec3 const& leftDir_WS	= lToW_Matrix.GetJBasis3D();
				Vec3 const& upDir_WS	= lToW_Matrix.GetKBasis3D();
				JointInfo jointInfo		= JointInfo( jointPos_WS, fwdDir_WS, leftDir_WS, upDir_WS );
				jointInfoList_prevFrame.push_back( jointInfo );
			}
			float distEeToTarget_prevFrame = GetDistEeToTarget( target );
			UNUSED(distEeToTarget_prevFrame);

			// Run 1 iteration of triangulation algorithm
			ComputeBendAngle_Cos_Sine( target );

			// Save best solution BEFORE deadlock
			std::vector<JointInfo> jointInfoList_beforeDeadlock;
			for ( int i = 0; i < m_jointList.size(); i++ )
			{
				IK_Joint3D* curJoint	= m_jointList[ i ];
				//			JointInfo  jointInfo = JointInfo( curJoint->m_jointPos_MS, curJoint->m_eulerAngles_LS );
				Mat44	  lToW_Matrix	= curJoint->GetMatrix_LocalToWorld();
				Vec3	  jointPos_WS	= lToW_Matrix.GetTranslation3D();
				Vec3 const& fwdDir_WS	= lToW_Matrix.GetIBasis3D();
				Vec3 const& leftDir_WS	= lToW_Matrix.GetJBasis3D();
				Vec3 const& upDir_WS	= lToW_Matrix.GetKBasis3D();
				JointInfo jointInfo		= JointInfo( jointPos_WS, fwdDir_WS, leftDir_WS, upDir_WS );
				jointInfoList_beforeDeadlock.push_back( jointInfo );
			}
			float distEeToTarget_beforeDeadlock = GetDistEeToTarget( target );


			//----------------------------------------------------------------------------------------------------------------------
			// Check for deadlock & keep track of debt angle for deadlock solutions
			//----------------------------------------------------------------------------------------------------------------------
			// Check for deadlock
			float tolerance	= 0.1f;
			if ( distEeToTarget_beforeDeadlock > tolerance )
			{
				m_isDeadlock = true;
				// Redo the algorithm if deadlock is occurring
				ComputeBendAngle_Cos_Sine( target );
			}
			else
			{
				m_isDeadlock = false;
				m_debtAngle  = 0.0f;
			}

			// [DEBUG] Save best solution AFTER deadlock 
			std::vector<JointInfo> jointInfoList_afterDeadlock;
			for ( int i = 0; i < m_jointList.size(); i++ )
			{
				IK_Joint3D* curJoint = m_jointList[ i ];
				//			JointInfo  jointInfo = JointInfo( curJoint->m_jointPos_MS, curJoint->m_eulerAngles_LS );
				Mat44	  lToW_Matrix	= curJoint->GetMatrix_LocalToWorld();
				Vec3	  jointPos_WS	= lToW_Matrix.GetTranslation3D();
				Vec3 const& fwdDir_WS	= lToW_Matrix.GetIBasis3D();
				Vec3 const& leftDir_WS	= lToW_Matrix.GetJBasis3D();
				Vec3 const& upDir_WS	= lToW_Matrix.GetKBasis3D();
				JointInfo jointInfo		= JointInfo( jointPos_WS, fwdDir_WS, leftDir_WS, upDir_WS );
				jointInfoList_afterDeadlock.push_back( jointInfo );
			}
			float distEeToTarget_afterDeadlock = GetDistEeToTarget( target );

			//----------------------------------------------------------------------------------------------------------------------
			// Disregard deadlock solution IF worse than solution before deadlock
			if ( distEeToTarget_beforeDeadlock < distEeToTarget_afterDeadlock )
			{
// 				if ( distEeToTarget_beforeDeadlock < distEeToTarget_prevFrame )
// 				{
					// Use solution before deadlock was attempted
					for ( int i = 0; i < jointInfoList_prevFrame.size(); i++ )
					{
// 						IK_Joint3D* curJoint		= m_jointList[i];
// 						JointInfo&  curJointInfo	= jointInfoList_beforeDeadlock[i];
// 						curJoint->m_jointPos_MS		= curJointInfo.m_jointPos_MS;
// 						curJoint->m_eulerAngles_LS	= curJointInfo.m_eulerAngles_MS;
					}
//  			}
//  			else
//  			{
//  				// Disregard solution found this frame if worse than prev frame's solution
//  				for ( int i = 0; i < jointInfoList_prevFrame.size(); i++ )
//  				{
// 	 					IK_Joint3D* curJoint		= m_jointList[i];
// 	 					JointInfo&  curJointInfo	= jointInfoList_prevFrame[i];
// 	 					curJoint->m_jointPos_MS		= curJointInfo.m_jointPos_MS;
// 	 					curJoint->m_eulerAngles_LS	= curJointInfo.m_eulerAngles_MS;
// 					}
// 				}
			}

			//----------------------------------------------------------------------------------------------------------------------
			// Debug save all joint solutions this frame
			//----------------------------------------------------------------------------------------------------------------------
			m_solutionsThisFrameList.push_back( jointInfoList_prevFrame		 );
			m_solutionsThisFrameList.push_back( jointInfoList_beforeDeadlock );
			m_solutionsThisFrameList.push_back( jointInfoList_afterDeadlock  );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
// Law of Cosine formula:				 c^2 = a^2 + b^2 - 2abCos( kneeAngle );
// -- 
// Solution below:
//							 Cos( kneeAngle ) = (c^2 - a^2 - b^2) / -2ab;		
//							 float tempResult = (c^2 - a^2 - b^2) / -2ab
//									kneeAngle = acosf( tempResult );
// --
// Law of Sine formula: distJoint0ToJoint1 / sine( hipAngle ) = distJoint0ToTarget / sine( kneeAngle );
// --
// Solution below:
//								 distJoint0ToJoint1 = sine( hipAngle ) * distJoint0ToTarget / sine( kneeAngle );
//								    float numerator = distJoint0ToJoint1 * sine( kneeAngle ) / distJoint0ToTarget;
//								    sin( hipAngle ) = numerator;
//										   hipAngle = asinf( numerator );
//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::ComputeBendAngle_Cos_Sine( Target target )
{
	// Init common variables
	IK_Joint3D* rootJoint = m_firstJoint;
	IK_Joint3D* midJoint  = rootJoint->m_child;
	Mat44 LToW_Matrix	  = rootJoint->GetIkChainMatrix_ModelToWorld();
	Mat44 WToL_Matrix	  = LToW_Matrix.GetOrthoNormalInverse();
	// Even though we used a WToL_Matrix, the joint0's localSpace and modelSpace are the same
	// So transforming the targetPos_WS to joint0's LS produces targetPos_MS
	Vec3 targetPos_MS	  = WToL_Matrix.TransformPosition3D( target.m_currentPos_WS );

	//----------------------------------------------------------------------------------------------------------------------
	// 1. Compute angle to bend using the Law of Cosine and Sine
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// Law of Cosine
	float a								= midJoint->m_distToChild;
	float b								= rootJoint->m_distToChild;
	float c								= ( targetPos_MS - rootJoint->m_jointPos_MS ).GetLength();
	float tempResult					= ( ( c * c ) - ( a * a ) - ( b * b ) ) / ( -2 * a * b );
	tempResult							= GetClamped( tempResult, -1.0f, 1.0f );	// Clamp tempResult to ensure we only use "valid" values for the acosf function
	float obtuseAngle					= acosf( tempResult );
	obtuseAngle							= ConvertRadiansToDegrees( obtuseAngle );
	//----------------------------------------------------------------------------------------------------------------------
	// Law of Sine
	obtuseAngle							= SinDegrees( obtuseAngle ); 
	float numerator						= ( a * obtuseAngle );
	float denominator					= c;
	if ( (numerator == 0.0f) && (denominator == 0.0f) )
	{
		// Handle edge case to avoid "nans" when ( 0.0f / 0.0f )
		tempResult						= 0.0f; 
	}
	else
	{
		tempResult						= numerator / denominator; 
	}
	tempResult							= GetClamped( tempResult, -1.0f, 1.0f );	// Clamp tempResult to ensure we only use "valid" values for the acosf function
	float halfAperture_rootToTarget		= asinf( tempResult );
	halfAperture_rootToTarget			= ConvertRadiansToDegrees( halfAperture_rootToTarget );
	halfAperture_rootToTarget			*= -1.0f;
	// Deadlock solution
	if ( m_isDeadlock )
	{
		halfAperture_rootToTarget		*= -1.0f;
		m_isDeadlock = false;
	}
//	ROTATE CCD (and 2 bone?) left vector AND fwd vector about the same axis and recompute new euler angles to preserve roll
	//----------------------------------------------------------------------------------------------------------------------
	// #ToDo: Make sure invalid rotation axis are not computed by requiring a pole vector be used
	//		  or figure out a way to compute a valid rotation axis that will allow convergence for constrained FABRIK 
	// Compute rotationAxis 
	Vec3  fwdDir_MS_rootJoint			= rootJoint->m_eulerAnglesGoal_LS.GetForwardDir_XFwd_YLeft_ZUp();
	Mat44 LToM_Matrix					= midJoint->GetMatrix_LocalToModel();
	Vec3  fwdDir_MS_midJoint			= LToM_Matrix.GetIBasis3D();
	Vec3  dir_rootToTarget_MS			= ( targetPos_MS - rootJoint->m_jointPos_MS ).GetNormalized();
//	Vec3  rotationAxis_MS				= CrossProduct3D( fwdDir_MS_rootJoint, fwdDir_MS_midJoint );
	Vec3  rotationAxis_MS				= CrossProduct3D( fwdDir_MS_rootJoint, dir_rootToTarget_MS );
	// Compute rotation axis CrossProduct( rootFwd, rootEndToTarget );
// 	Vec3 rootEnd_MS				= rootJoint->m_jointPos_MS + ( fwdDir_MS_rootJoint * rootJoint->m_distToChild );
// 	Vec3 dir_rootEndToTarget	= ( targetPos_MS - rootEnd_MS ).GetNormalized();
//	rotationAxis_MS				= CrossProduct3D( fwdDir_MS_rootJoint, dir_rootEndToTarget );
//	rotationAxis_MS.Normalize();
	// Compute rotationAxis using poleVector
	Vec3 dir_rootToPoleVector_MS;
	if ( m_poleVector_MS != Vec3::ZERO )
	{
		dir_rootToPoleVector_MS			= ( m_poleVector_MS - rootJoint->m_jointPos_MS ).GetNormalized(); 
		rotationAxis_MS					= CrossProduct3D( dir_rootToPoleVector_MS, dir_rootToTarget_MS );
	}
	float rotationAxisLength = rotationAxis_MS.GetLength();
	if ( CompareIfFloatsAreEqual( rotationAxisLength, 0.0f, 0.01f ) )
	{
		// Handle edge case, most often caused when the chain is straight
//		ERROR_RECOVERABLE( "Edge case, rotationAxis is 0" );
		Vec3 fwdDir, leftDir, upDir;
		rootJoint->m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( fwdDir, leftDir, upDir );
		rotationAxis_MS = leftDir;
	}
	rotationAxis_MS.Normalize();
	m_debugVector_RotationAxis_MS		= rotationAxis_MS;
	m_debugVector_rootToPV_MS			= dir_rootToPoleVector_MS;
	m_debugVector_rootToTarget_MS		= dir_rootToTarget_MS;
	// Rotate joint1 using halfAperture
	fwdDir_MS_rootJoint					= RotateVectorAboutArbitraryAxis( dir_rootToTarget_MS, rotationAxis_MS, halfAperture_rootToTarget );
	
	//----------------------------------------------------------------------------------------------------------------------
	// 2. Update joint data
	//    - JointPos_MS and eulerAngles_LS 
	//    - Clamp eulerAngles_LS
	//    - Re-update jointPos_MS data
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// Root Joint
	rootJoint->m_jointPos_MS			= rootJoint->m_jointPos_LS;
	rootJoint->m_eulerAnglesGoal_LS		= rootJoint->GetEulerFromFwdDir( fwdDir_MS_rootJoint );
	rootJoint->ClampYPR();
	fwdDir_MS_rootJoint					= rootJoint->m_eulerAnglesGoal_LS.GetForwardDir_XFwd_YLeft_ZUp();
	//----------------------------------------------------------------------------------------------------------------------
	// Mid Joint
	midJoint->m_jointPos_MS				= rootJoint->m_jointPos_MS + ( fwdDir_MS_rootJoint * rootJoint->m_distToChild );
	// Transform midJoint's dirToTarget_MS to dirToTarget_LS, then compute newEuler_LS
	Vec3  dirToTarget_MS_midJoint		= ( targetPos_MS - midJoint->m_jointPos_MS ).GetNormalized();
	LToM_Matrix							= midJoint->m_parent->GetMatrix_LocalToModel();
	Mat44 MToL_Matrix					= LToM_Matrix.GetOrthoNormalInverse();
	Vec3  dirToTarget_LS_midJoint		= MToL_Matrix.TransformVectorQuantity3D( dirToTarget_MS_midJoint );
	midJoint->m_eulerAnglesGoal_LS		= midJoint->GetEulerFromFwdDir( dirToTarget_LS_midJoint );
	midJoint->ClampYPR();
	// Transform clampedEuler_Joint2_LS to clampedFwd_Joint2_MS
	dirToTarget_LS_midJoint				= midJoint->m_eulerAnglesGoal_LS.GetForwardDir_XFwd_YLeft_ZUp();
	LToM_Matrix							= midJoint->GetMatrix_LocalToModel();
	dirToTarget_MS_midJoint				= LToM_Matrix.GetIBasis3D();
	//----------------------------------------------------------------------------------------------------------------------
	// Final Joint
	m_finalJoint->m_jointPos_MS			= midJoint->m_jointPos_MS + ( dirToTarget_MS_midJoint * midJoint->m_distToChild );
	m_finalJoint->m_eulerAnglesGoal_LS	= EulerAngles();
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DeadlockSolver_DebtAngle( IK_Joint3D* curJoint, Target target )
{
	// Deadlock Attempt 4: Distribute debtAngle using pitch AND yaw
	if ( !CompareIfFloatsAreEqual( m_debtAngle, 0.0f, 0.01f ) )
	{
		// Compute directions for MIN & MAX yaw and MIN & MAX pitch
		EulerAngles actualEulerAngles = curJoint->m_eulerAnglesGoal_LS;
		// MAX pitch dir MS
		float		maxPitch		= curJoint->m_pitchConstraints_LS.m_max;
		EulerAngles maxEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, maxPitch, actualEulerAngles.m_rollDegrees ); 
		curJoint->m_eulerAnglesGoal_LS	= maxEulerPitch;
		Vec3		dirMaxPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
		// MIN pitch dir MS
		float		minPitch		= curJoint->m_pitchConstraints_LS.m_min;
		EulerAngles minEulerPitch	= EulerAngles( actualEulerAngles.m_yawDegrees, minPitch, actualEulerAngles.m_rollDegrees ); 
		curJoint->m_eulerAnglesGoal_LS	= minEulerPitch;
		Vec3		dirMinPitch_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
		// MAX yaw dir MS
		float		maxYaw			= curJoint->m_yawConstraints_LS.m_max;
		EulerAngles maxEulerYaw		= EulerAngles( maxYaw, actualEulerAngles.m_pitchDegrees, actualEulerAngles.m_rollDegrees ); 
		curJoint->m_eulerAnglesGoal_LS	= maxEulerPitch;
		Vec3		dirMaxYaw_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
		// MIN yaw dir MS
		float		minYaw			= curJoint->m_yawConstraints_LS.m_min;
		EulerAngles minEulerYaw		= EulerAngles( minYaw, actualEulerAngles.m_pitchDegrees, actualEulerAngles.m_rollDegrees ); 
		curJoint->m_eulerAnglesGoal_LS	= minEulerYaw;
		Vec3		dirMinYaw_MS	= curJoint->GetMatrix_LocalToModel().GetIBasis3D();
		// Reset joint euler to actual values
		curJoint->m_eulerAnglesGoal_LS	= actualEulerAngles;

		// Compute dir_JointToTarget_MS: Transform joint and target into MS
		Vec3  jointPos_MS			= curJoint->GetMatrix_LocalToModel().GetTranslation3D();
		Mat44 wToM_Matrix			= curJoint->GetIkChainMatrix_ModelToWorld().GetOrthoNormalInverse();
		Vec3  targetPos_MS			= wToM_Matrix.TransformPosition3D( target.m_currentPos_WS );
		Vec3  dir_JointToTarget_MS	= ( targetPos_MS - jointPos_MS ).GetNormalized();
		// Compute dot results 
		float maxDotPositivity_Pitch	= DotProduct3D( dirMaxPitch_MS, dir_JointToTarget_MS );
		float minDotPositivity_Pitch	= DotProduct3D( dirMinPitch_MS, dir_JointToTarget_MS );
		float maxDotPositivity_Yaw		= DotProduct3D( dirMaxYaw_MS,	dir_JointToTarget_MS );
		float minDotPositivity_Yaw		= DotProduct3D( dirMinYaw_MS,	dir_JointToTarget_MS );
		// Compute 'elbowRoom' based on dotResult comparisons, choose the negative or lesser result
		float newPitch	 = 0.0f;
		float newYaw	 = 0.0f;
		float elbowRoom  = 0.0f;
		float curPitch	 = curJoint->m_eulerAnglesGoal_LS.m_pitchDegrees;
		float curYaw	 = curJoint->m_eulerAnglesGoal_LS.m_yawDegrees;
		
		//----------------------------------------------------------------------------------------------------------------------
		// TEST: point root towards target MS then yaw/pitch at min/max constraints
		//----------------------------------------------------------------------------------------------------------------------
 		if ( curJoint == m_firstJoint )
 		{
 			EulerAngles testEuler				= m_firstJoint->GetEulerFromFwdDir( dir_JointToTarget_MS );
 			m_firstJoint->m_eulerAnglesGoal_LS	= testEuler;
 			curYaw								= testEuler.m_yawDegrees;
 			curPitch							= testEuler.m_pitchDegrees;
 		} 


		// Toggle deadlock bend dir preference 
		if ( m_prevDeadlockCount != m_curDeadlockCount )
		{
			curJoint->ToggleDeadlockDir_Pitch();
			curJoint->ToggleDeadlockDir_Yaw();
		}
		//----------------------------------------------------------------------------------------------------------------------
		// BEND MORE using PITCH
		//----------------------------------------------------------------------------------------------------------------------
		// Pitch TOWARD target
		if ( curJoint->m_deadlockDir_Pitch == DEADLOCK_BEND_TOWARD )
		{
			// Choose more AGREE-ING dotResult_pitch
			if ( minDotPositivity_Pitch > maxDotPositivity_Pitch )
			{
				// Joint should pitch NEGATIVE to bend toward target
				elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_min );
				if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
				{
					// If there's enough space for all the debt, let's pay all the debt
					newPitch = m_debtAngle * -1.0f;			// Debt angle is always positive, so we need to negative the angle to get a negative pitch
				}
				else
				{
					// If there's NOT enough space for all the debt, lets pay the max amount we can
					newPitch = elbowRoom;
				}
			}
			else	// dotMaxPitch AGREES more than dotMinPitch
			{
				// Joint should pitch POSITIVE to bend toward target
				elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_max );
				if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
				{
					// If there's enough space for all the debt, let's pay all the debt
					newPitch = m_debtAngle;
				}
				else
				{
					// If there's NOT enough space for all the debt, lets pay the max amount we can
					newPitch = elbowRoom;
				}
			}
		}
		// Pitch AWAY from target
		else if ( curJoint->m_deadlockDir_Pitch == DEADLOCK_BEND_AWAY )
		{
			// Choose more DISAGREE-ING dotResult_pitch
			if ( minDotPositivity_Pitch < maxDotPositivity_Pitch )
			{
				// Joint should pitch NEGATIVE to bend away from target
				elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_min );
				if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
				{
					// If there's enough space for all the debt, let's pay all the debt
					newPitch = m_debtAngle * -1.0f;			// Debt angle is always positive, so we need to negative the angle to get a negative pitch
				}
				else
				{
					// If there's NOT enough space for all the debt, lets pay the max amount we can
					newPitch = elbowRoom;
				}
			}
			else	// dotMaxPitch DISAGREES more than dotMinPitch
			{
				// Joint should pitch POSITIVE to bend away from target
				elbowRoom = GetShortestAngularDispDegrees( curPitch, curJoint->m_pitchConstraints_LS.m_max );
				if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
				{
					// If there's enough space for all the debt, let's pay all the debt
					newPitch = m_debtAngle;
				}
				else
				{
					// If there's NOT enough space for all the debt, lets pay the max amount we can
					newPitch = elbowRoom;
				}
			}
		}
		curJoint->m_bendMore_pitch						 = newPitch;
		curJoint->m_eulerAnglesGoal_LS.m_pitchDegrees   += newPitch;
		m_debtAngle										-= fabsf( newPitch );

		//----------------------------------------------------------------------------------------------------------------------
		// BEND MORE using YAW
		//----------------------------------------------------------------------------------------------------------------------
		// Yaw TOWARD target
		if ( curJoint->m_deadlockDir_Yaw == DEADLOCK_BEND_TOWARD )
		{
			// Choose more AGREE-ING dotResult_yaw
			if ( minDotPositivity_Yaw > maxDotPositivity_Yaw )
			{
				// Joint should pitch NEGATIVE to bend toward target
				elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_min );
				if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
				{
					// If there's enough space for all the debt, let's pay all the debt
					newYaw = m_debtAngle * -1.0f;		// Debt angle is always positive, so we need to negate the angle to get a negative yaw			
				}
				else
				{
					// If there's NOT enough space for all the debt, lets pay the max amount we can
					newYaw = elbowRoom;
				}
			}
			else    // dotMaxYaw AGREES more than dotMinYaw
			{
				// Joint should pitch POSITIVE to bend toward target
				elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_max );
				if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
				{
					// If there's enough space for all the debt, let's pay all the debt
					newYaw = m_debtAngle;
				}
				else
				{
					// If there's NOT enough space for all the debt, lets pay the max amount we can
					newYaw = elbowRoom;
				}
			}
		}
		else if ( curJoint->m_deadlockDir_Yaw == DEADLOCK_BEND_AWAY )
		{
			// Choose DISAGREE-ING dotResult_yaw
			if ( minDotPositivity_Yaw < maxDotPositivity_Yaw )
			{
				// Joint should yaw NEGATIVE to bend away from target
				elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_min );
				if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
				{
					// If there's enough space for all the debt, let's pay all the debt
					newYaw = m_debtAngle * -1.0f;		// Debt angle is always positive, so we need to negate the angle to get a negative yaw			
				}
				else
				{
					// If there's NOT enough space for all the debt, lets pay the max amount we can
					newYaw = elbowRoom;
				}
			}
			else    // dotMaxYaw DISAGREES than dotMinYaw
			{
				// Joint should yaw POSITIVE to bend away from target
				elbowRoom = GetShortestAngularDispDegrees( curYaw, curJoint->m_yawConstraints_LS.m_max );
				if ( fabsf(elbowRoom) >= fabsf(m_debtAngle) )
				{
					// If there's enough space for all the debt, let's pay all the debt
					newYaw = m_debtAngle;
				}
				else
				{
					// If there's NOT enough space for all the debt, lets pay the max amount we can
					newYaw = elbowRoom;
				}
			}
		}
		curJoint->m_bendMore_yaw					 = newYaw;
		curJoint->m_eulerAnglesGoal_LS.m_yawDegrees += newYaw;
		m_debtAngle									-= fabsf( newYaw );
	}
	else
	{
		curJoint->m_bendMore_yaw = 0.0f;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DeadlockSolver_Reset()
{
	// Reset orientation
	ResetEulerForAllJoints();
	// Reset positions
	if ( m_solverType == CHAIN_SOLVER_FABRIK )
	{
		for ( int i = 0; i < m_jointList.size(); i++ )
		{
			IK_Joint3D* currentJoint = m_jointList[ i ];
			currentJoint->m_jointPos_MS = currentJoint->GetMatrix_LocalToModel().GetTranslation3D();
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::DeadlockSolver_ResetAndPoseRootZUp()
{
	// Reset orientation
	DeadlockSolver_Reset();
	m_firstJoint->m_eulerAnglesGoal_LS		= EulerAngles(0.0f, -90.0f, 0.0f);
//	m_firstJoint->m_eulerAnglesCurrent_LS	= EulerAngles(0.0f, -90.0f, 0.0f);
	// Reset positions
	if ( m_solverType == CHAIN_SOLVER_FABRIK )
	{
		m_firstJoint->m_jointPos_MS = m_firstJoint->m_jointPos_LS;
		for ( int i = 1; i < m_jointList.size(); i++ )
		{
			IK_Joint3D* currentJoint = m_jointList[ i ];
			currentJoint->ClampYPR();
			currentJoint->m_jointPos_MS = currentJoint->GetMatrix_LocalToModel().GetTranslation3D();
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::CanMove()
{
	bool canMove = false;
	// Cannot move, is locked.					// Stay in the same place
	if ( IsAnchor_Locked() )
	{
		canMove = false;
	}
	// Is moving, cannot lock.					// In the process of moving, don't do anything
	else if ( IsAnchor_Moving() )
	{
		canMove = false;
	}
	// Is not moving, can lock, can also move,	// Limb can be locked or moved
	else if ( IsAnchor_Free() )
	{
		canMove = true;
	}
	return canMove;
}


//----------------------------------------------------------------------------------------------------------------------
// otherChain is the chain to compare against
//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::TryFreeAnchor( IK_Chain3D* const otherChain )
{
	if ( IsAnchor_Moving() )
	{
		// DON'T free this chain if currently moving
		return false;
	}
	bool isFree_otherChain	 = otherChain->IsAnchor_Free();
	bool isLocked_otherChain = otherChain->IsAnchor_Locked();
	if ( isFree_otherChain || isLocked_otherChain )
	{
		SetAnchor_Free();
		return true;
	}
	// Return false if otherChain "isMoving"
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::TryFreeAnchor( IK_Chain3D* const otherChainA, IK_Chain3D* const otherChainB )
{
	if ( IsAnchor_Moving() )
	{
		// DON'T free this chain if currently moving
		return false;
	}
	bool isFree_otherChainA		= otherChainA->IsAnchor_Free();
	bool isLocked_otherChainA	= otherChainA->IsAnchor_Locked();
	bool isFree_otherChainB		= otherChainB->IsAnchor_Free();
	bool isLocked_otherChainB	= otherChainB->IsAnchor_Locked();
	if ( isFree_otherChainA || isLocked_otherChainA && isFree_otherChainB || isLocked_otherChainB )
	{
		SetAnchor_Free();
		return true;
	}
	// Return false if otherChain "isMoving"
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::IsAnyJointBentToMaxConstraints()
{
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentJoint = m_jointList[i];
		// Yaw
		if  ( currentJoint->m_eulerAnglesGoal_LS.m_yawDegrees >= currentJoint->m_yawConstraints_LS.m_max ||
			  currentJoint->m_eulerAnglesGoal_LS.m_yawDegrees <= currentJoint->m_yawConstraints_LS.m_min	
			)
		{
			// Don't consider this joint as clamped if it has no degrees of freedom for yaw
			if ( currentJoint->m_yawConstraints_LS.m_min != currentJoint->m_yawConstraints_LS.m_max )
			{
				return true;
			}
		}
		// Pitch
		if  ( currentJoint->m_eulerAnglesGoal_LS.m_pitchDegrees >= currentJoint->m_pitchConstraints_LS.m_max ||
			  currentJoint->m_eulerAnglesGoal_LS.m_pitchDegrees	<= currentJoint->m_pitchConstraints_LS.m_min	
			)
		{
			// Don't consider this joint as clamped if it has no degrees of freedom for pitch 
			if ( currentJoint->m_pitchConstraints_LS.m_min != currentJoint->m_pitchConstraints_LS.m_max )
			{
				return true;
			}
		}			
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
float IK_Chain3D::GetMaxChainLength()
{
	float totalLength = 0.0f;
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D const* currentSegment = m_jointList[ i ];
		totalLength += currentSegment->m_distToChild;
	}
	return totalLength;
}


//----------------------------------------------------------------------------------------------------------------------
// The ref vectors parameters belong to the reference orientation
// Does NOT belong to "us"
//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::UpdateTargetOrientationToRef( Vec3 const& fwd, Vec3 const& left, Vec3 const& up )
{
	m_target.m_fwdDir	= fwd;
	m_target.m_leftDir	= left;
	m_target.m_upDir	= up;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::UpdateJointsPositions_MS()
{
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* curJoint	 = m_jointList[i];
		// Convert joints to model space
		Mat44 localToModelMatrix = curJoint->GetMatrix_LocalToModel();
		curJoint->m_jointPos_MS	 = localToModelMatrix.GetTranslation3D();
	}
}


//----------------------------------------------------------------------------------------------------------------------
// To reduce complexity and ambiguity, this function eliminates roll when computing Euler from ONLY FwdDir
//----------------------------------------------------------------------------------------------------------------------
// EulerAngles IK_Chain3D::GetEulerFromFwdDir( IK_Joint3D* curJoint, Vec3 const& fwdDir )
// {
// 	EulerAngles newEulerAngles = EulerAngles();
// 	float sign = 1.0f;
// 	if ( fwdDir.x < 0.0f )
// 	{
// 		// Preserve the negative sign since we lose it 
// 		// by squaring the length when calculating "projectedFwdOnPlaneXY"
// 		sign = -1.0f;
// 	}
// 	float projectedFwdOnPlaneXY	= sqrtf( ( fwdDir.x * fwdDir.x ) + ( fwdDir.y * fwdDir.y ) );
// 	bool  isFacingWorldZ		= CompareIfFloatsAreEqual( projectedFwdOnPlaneXY, 0.0f, 0.0001f );
// 	if ( isFacingWorldZ )
// 	{
// 		// Handle Gimble lock edge case
// 		// This issue occurs when our "fwdDir" is facing world -Z or Z+
// 		// Since Y is around the "world" Z axis and not the "kBasis", this issue occurs.
// 		// When this happens, we need to compute EulerAngles from our dirVectors differently
// 		float yawDegrees	= 0.0f;
// 		float pitchDegrees	= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
// 		float rollDegrees	= 0.0f;
// 		newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
// 	}
// 	else
// 	{
// 		//----------------------------------------------------------------------------------------------------------------------
// 		// Handle ambiguity edge case
// 		// One vector can be represented with multiple EulerAngles
// 		//----------------------------------------------------------------------------------------------------------------------
// // 		// Prefer "yaw" over pitch
// // 		float yawDegrees_y		= Atan2Degrees(  fwdDir.y, fwdDir.x );
// // 		float pitchDegrees_y	= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
// // 		float rollDegrees_y		= 0.0f;
// // 
// // 		//----------------------------------------------------------------------------------------------------------------------
// // 		// Handle ambiguity edge case
// // 		// One vector can be represented with multiple EulerAngles
// // 		//----------------------------------------------------------------------------------------------------------------------
// // 		if ( sign < 0.0f )
// // 		{
// // 			// Prefer pitch over yaw
// // 			if ( curJoint->m_pitchConstraints_LS.m_max >=  90.0f ||
// // 				 curJoint->m_pitchConstraints_LS.m_min <= -90.0f )
// // 			{
// // 				// Re-compute pitch calculations if pitch is preferred over yaw
// // 				projectedFwdOnPlaneXY *= sign;
// // 				pitchDegrees_y		   = Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
// // 				if ( yawDegrees_y >= 90.0f )
// // 				{
// // 					yawDegrees_y -= 180.0f;
// // 				}
// // 				else if ( yawDegrees_y <= -90.0f )
// // 				{
// // 					yawDegrees_y += 180.0f;
// // 				}
// // 			}
// // 		}
// // 		newEulerAngles = EulerAngles( yawDegrees_y, pitchDegrees_y, rollDegrees_y );
// 
// 		//----------------------------------------------------------------------------------------------------------------------
// 		// 1. Compute both cases of YPR (yawPreferred & pitchPreferred)
// 		// 2. Compute and choose "nearest" euler
// 		// 3. Compare both cases of euler and choose most similar to prevEuler solution
// 		//----------------------------------------------------------------------------------------------------------------------
// 		// 1a. Compute Yaw preferred solution
// 		float yawDegrees_y		= Atan2Degrees(  fwdDir.y, fwdDir.x );
// 		float pitchDegrees_y	= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
// 		// 1b. Compute pitch preferred solution
// 		float yawDegrees_p		= yawDegrees_y;
// 		float pitchDegrees_p	= pitchDegrees_y;
//  		if ( sign < 0.0f )
//  		{
// 			// Re-compute pitch calculations
// 			projectedFwdOnPlaneXY  *= -1.0f;
// 			yawDegrees_p			= yawDegrees_y;
// 			pitchDegrees_p			= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
// 			if ( yawDegrees_y >= 90.0f )
// 			{
// 				yawDegrees_p -= 180.0f;
// 			}
// 			else if ( yawDegrees_y <= -90.0f )
// 			{
// 				yawDegrees_p += 180.0f;
// 			}
// 		}
// //  		// 2. Compute and choose "nearest" euler (Euler comparison)
// // 		//	  "_y" means "yawPreferred" 
// // 		//	  "_p" means "pitchPreferred"
// // 		EulerAngles yawPreferredEuler	= EulerAngles( yawDegrees_y, pitchDegrees_y, 0.0f );
// //  		EulerAngles pitchPreferredEuler = EulerAngles( yawDegrees_p, pitchDegrees_p, 0.0f );
// //  		float absYawDif_y	= fabsf( m_prevEuler_LastFrame.m_yawDegrees	  ) - fabsf( yawPreferredEuler.m_yawDegrees		);	 
// //  		float absPitchDif_y = fabsf( m_prevEuler_LastFrame.m_pitchDegrees ) - fabsf( yawPreferredEuler.m_pitchDegrees	);	 
// //  		float absYawPitch_p	= fabsf( m_prevEuler_LastFrame.m_yawDegrees	  ) - fabsf( pitchPreferredEuler.m_yawDegrees	);   
// //  		float absPitchDif_p = fabsf( m_prevEuler_LastFrame.m_pitchDegrees ) - fabsf( pitchPreferredEuler.m_pitchDegrees ); 
// //  		float totalDif_y	= fabsf( absYawDif_y )	 + fabsf( absPitchDif_y );
// //  		float totalDif_p	= fabsf( absYawPitch_p ) + fabsf( absPitchDif_p );
// //  		// 3. Compare both cases of euler and choose most similar to prevEuler solution
// //   		if ( totalDif_y <= totalDif_p )
// //   		{
// //   			// yaw preferred solution is more similar
// //   			newEulerAngles			= yawPreferredEuler;
// //  			m_prevEuler_LastFrame	= newEulerAngles;
// //   		}
// //   		else if ( totalDif_y > totalDif_p )
// //  		{
// //   			// pitch preferred solution is more similar
// //   			newEulerAngles			= pitchPreferredEuler;
// //  			m_prevEuler_LastFrame	= newEulerAngles;
// //   		}
// //   		else
// //   		{
// //  			// This case should never get triggered?
// //  			int brainCells = 0;
// //   		}
// 
// 
// 		//----------------------------------------------------------------------------------------------------------------------
//  		// 2. Convert YPR to vectors (leftDir AND upDir comparison)
// 		//----------------------------------------------------------------------------------------------------------------------
//  		EulerAngles yawPreferredEuler	= EulerAngles( yawDegrees_y, pitchDegrees_y, 0.0f );
//  		EulerAngles pitchPreferredEuler = EulerAngles( yawDegrees_p, pitchDegrees_p, 0.0f );
//  		Vec3 fwd_y, left_y, up_y;
//  		Vec3 fwd_p, left_p, up_p;
//  		  yawPreferredEuler.GetAsVectors_XFwd_YLeft_ZUp( fwd_y, left_y, up_y );
//  		pitchPreferredEuler.GetAsVectors_XFwd_YLeft_ZUp( fwd_p, left_p, up_p );
//  		// 3. Dot product vectors to prev solution
//  		float leftSimilarity_y	= DotProduct3D( curJoint->m_leftDir_lastFrame, left_y );
//  		float leftSimilarity_p	= DotProduct3D( curJoint->m_leftDir_lastFrame, left_p );
//  		float upSimilarity_y	= DotProduct3D( curJoint->m_upDir_lastFrame,   up_y	);
//  		float upSimilarity_p	= DotProduct3D( curJoint->m_upDir_lastFrame,   up_p	);
//  		// 4. Return euler most "similar" to prev solution
//  		// Yaw is preferred
// 		if ( leftSimilarity_y >= leftSimilarity_p )
// 		{
//  			if ( upSimilarity_y >= upSimilarity_p )
//  			{
//  				// yawPreferred solution is more similar to the newLeft and newUp 
//  				newEulerAngles					= EulerAngles( yawDegrees_y, pitchDegrees_y, 0.0f );
//  				curJoint->m_leftDir_lastFrame	= left_y;
// 				curJoint->m_upDir_lastFrame		= up_y;
// 				curJoint->m_euler_LastFrame		= newEulerAngles;
//  			}
//  			else if ( upSimilarity_y < upSimilarity_p )
//  			{
// 				// pitchPreferred solution is more similar to the newLeft and newUp 
// 				newEulerAngles					= EulerAngles( yawDegrees_p, pitchDegrees_p, 0.0f );
//  				curJoint->m_leftDir_lastFrame	= left_p;
//  				curJoint->m_upDir_lastFrame		= up_p;
//  				curJoint->m_euler_LastFrame		= newEulerAngles;
//  			}
//  		}
// 		// Pitch is preferred
//  		else if ( leftSimilarity_y < leftSimilarity_p  )
//  		{
// 			if ( upSimilarity_y >= upSimilarity_p )
// 			{
// 				// yawPreferred solution is more similar to the newLeft and newUp 
// 				newEulerAngles					= EulerAngles( yawDegrees_y, pitchDegrees_y, 0.0f );
// 				curJoint->m_leftDir_lastFrame	= left_y;
// 				curJoint->m_upDir_lastFrame		= up_y;
// 				curJoint->m_euler_LastFrame		= newEulerAngles;
// 			}
// 			else if ( upSimilarity_y < upSimilarity_p )
// 			{
// 				// pitchPreferred solution is more similar to the newLeft and newUp 
// 				newEulerAngles					= EulerAngles( yawDegrees_p, pitchDegrees_p, 0.0f );
// 				curJoint->m_leftDir_lastFrame	= left_p;
// 				curJoint->m_upDir_lastFrame		= up_p;
// 				curJoint->m_euler_LastFrame		= newEulerAngles;
// 			}
//  		}
// 		else
// 		{
// 			int brainCells = 0;
// 			UNUSED( brainCells );
// 		}
// 	}
// 	return newEulerAngles;
// }


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::ResetEulerForAllJoints()
{
	// Reset all joint euler to (0,0,0)
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentJoint	   = m_jointList[ i ];
		currentJoint->m_eulerAnglesGoal_LS = EulerAngles();
	}
}


//----------------------------------------------------------------------------------------------------------------------
float IK_Chain3D::GetDistEeToTarget( Target target )
{
	Vec3 eePos_WS		= m_finalJoint->GetMatrix_LocalToWorld().GetTranslation3D();
	Vec3 dispEeToTarget	= target.m_currentPos_WS - eePos_WS;
	float newDist		= dispEeToTarget.GetLength();
	return newDist;
}


//----------------------------------------------------------------------------------------------------------------------
float IK_Chain3D::GetDistRootToTarget( Target target )
{
	Vec3  firstJointPos_WS			= m_firstJoint->GetMatrix_LocalToWorld().GetTranslation3D();
	Vec3  disp_firstJointToTarget	= target.m_currentPos_WS - firstJointPos_WS;
	float dist_firstJointToTarget	= disp_firstJointToTarget.GetLength();
	return dist_firstJointToTarget;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::ResetAllJointConstraints()
{
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* currentSegment = m_jointList[ i ];
		currentSegment->SetConstraints_YPR();
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::SetSolverType( ChainSolveType solverType /*= CHAIN_SOLVER_NUM */ )
{
	m_solverType = solverType;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::ResetBendMoreAngles()
{
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* curJoint		= m_jointList[i];
		curJoint->m_bendMore_pitch	= 0.0f;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::SetChainPreference_MatrixToEuler( MatrixToEulerPreference rotationPreference )
{
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* curJoint		= m_jointList[i];
		curJoint->m_matrixToEulerPreference	= rotationPreference;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::SaveDebugPose_curIter()
{
	for ( int i = 0; i < m_jointList.size(); i++ )
	{
		IK_Joint3D* curJoint	= m_jointList[ i ];
		Vec3	  jointPos_WS	= curJoint->GetIkChainMatrix_ModelToWorld().TransformPosition3D( curJoint->m_jointPos_MS );
		Mat44	  lToW_Matrix	= curJoint->GetMatrix_LocalToWorld();
		Vec3 const& fwdDir_WS	= lToW_Matrix.GetIBasis3D();
		Vec3 const& leftDir_WS	= lToW_Matrix.GetJBasis3D();
		Vec3 const& upDir_WS	= lToW_Matrix.GetKBasis3D();
		JointInfo jointInfo		= JointInfo( jointPos_WS, fwdDir_WS, leftDir_WS, upDir_WS );
		m_jointInfosThisIterList.push_back( jointInfo );
	}
	m_solutionsThisFrameList.push_back( m_jointInfosThisIterList );
	m_jointInfosThisIterList.clear();
}

//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::SaveDebugPose_curIter( IK_Joint3D* const& curJoint )
{
	Vec3	  jointPos_WS	= curJoint->GetIkChainMatrix_ModelToWorld().TransformPosition3D( curJoint->m_jointPos_MS );
	Mat44	  lToW_Matrix	= curJoint->GetMatrix_LocalToWorld();
	Vec3 const& fwdDir_WS	= lToW_Matrix.GetIBasis3D();
	Vec3 const& leftDir_WS	= lToW_Matrix.GetJBasis3D();
	Vec3 const& upDir_WS	= lToW_Matrix.GetKBasis3D();
	JointInfo jointInfo		= JointInfo( jointPos_WS, fwdDir_WS, leftDir_WS, upDir_WS );
	m_jointInfosThisIterList.push_back( jointInfo );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::SetAnchor_Locked()
{
	m_anchorState = ANCHOR_STATE_LOCKED;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::SetAnchor_Moving()
{
	m_anchorState = ANCHOR_STATE_MOVING;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Chain3D::SetAnchor_Free()
{
	m_anchorState = ANCHOR_STATE_FREE;
}


//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::IsAnchor_Locked()
{
	if ( m_anchorState == ANCHOR_STATE_LOCKED )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::IsAnchor_Moving()
{
	if ( m_anchorState == ANCHOR_STATE_MOVING )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool IK_Chain3D::IsAnchor_Free()
{
	if ( m_anchorState == ANCHOR_STATE_FREE )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
std::string IK_Chain3D::GetAnchorAsString()
{
	std::string enumAsString = "INVALID";
	if ( IsAnchor_Locked() )
	{
		enumAsString = "Locked";
	}
	else if ( IsAnchor_Free() )
	{
		enumAsString = "Free";
	}
	else if ( IsAnchor_Moving() )
	{
		enumAsString = "Moving";
	}
	return enumAsString;
}