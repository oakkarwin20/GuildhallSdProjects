#include "Engine/SkeletalSystem/CreatureBase.hpp"
#include "Engine/SkeletalSystem/IK_Joint3D.hpp"
#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"


//----------------------------------------------------------------------------------------------------------------------
CreatureBase::CreatureBase( Vec3 const& rootStartPos, float length )
{
	m_modelRoot = new IK_Joint3D( 0, rootStartPos, length );
}


//----------------------------------------------------------------------------------------------------------------------
CreatureBase::~CreatureBase()
{
}


//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::Update( float deltaSeconds )
{
	for ( int i = 0; i < m_skeletalList.size(); i++ )
	{
		m_skeletalList[i]->Update( deltaSeconds );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::RenderMesh( Renderer* g_theRenderer, Texture* texture, Shader* const shader, Rgba8 const& color ) const
{
	for ( int i = 0; i < m_skeletalList.size(); i++ )
	{
		if ( m_skeletalList[ i ]->m_shouldRender )
		{
			m_skeletalList[ i ]->RenderMesh( g_theRenderer, texture, shader, color );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::Render( std::vector<Vertex_PCU>& verts, Rgba8 const& limbColor, Rgba8 const& jointColor, bool renderDebugJointBasis, bool const& renderDebugCurrentPos_EE ) const
{
	for ( int i = 0; i < m_skeletalList.size(); i++ )
	{
		if ( m_skeletalList[ i ]->m_shouldRender )
		{
			m_skeletalList[i]->Render( verts, limbColor, jointColor, renderDebugJointBasis, renderDebugCurrentPos_EE  );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
// localOffsetToRoot is the position of the new skeletal system being created, relative to the root
//----------------------------------------------------------------------------------------------------------------------
IK_Chain3D* CreatureBase::CreateChildChain( std::string const& name, Vec3 const& position_WS,  IK_Joint3D* ownerSkeletonFirstJoint, bool shouldReachInsteadOfDrag )
{
	if ( ownerSkeletonFirstJoint == nullptr )
	{
		ownerSkeletonFirstJoint = m_modelRoot;
	}
	IK_Chain3D* newChain = new IK_Chain3D( name, position_WS, ownerSkeletonFirstJoint, this, shouldReachInsteadOfDrag );
	m_skeletalList.emplace_back( newChain );
	return newChain;
}


//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::InitJointMesh( IK_Chain3D* const ikChain, float radius, float radiusDecrement, MeshType meshType )
{
	for ( int i = 0; i < ikChain->m_jointList.size(); i++ )
	{
		IK_Joint3D* curJoint = ikChain->m_jointList[ i ];
		if ( curJoint->m_child == nullptr )
		{
			continue;
		}
		if ( meshType == MESH_TYPE_CYLINDER )
		{
			curJoint->InitMesh_Cylinder( Vec3::ZERO, curJoint->m_child->m_jointPos_LS, radius );
			radius -= radiusDecrement;
		} 
		else if ( meshType == MESH_TYPE_ARROW )
		{
// 			curJoint->InitMesh_Arrow( Vec3::ZERO, curJoint->m_child->m_jointPos_LS, radius );
// 			radius -= radiusDecrement;
		}
		else if ( meshType == MESH_TYPE_CYLINDER_TAPERED )
		{
			curJoint->InitMesh_CylinderTapered( Vec3::ZERO, curJoint->m_child->m_jointPos_LS, radius + radiusDecrement, radius );
			radius -= radiusDecrement;
		}
		else if ( meshType == MESH_TYPE_SPHERE )
		{
			curJoint->InitMesh_Sphere( Vec3::ZERO, radius );
			radius -= radiusDecrement;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::InitJointMeshes_Ellipsoid( IK_Chain3D* const ikChain, Vec3 const& start, Vec3 const& end, float startRadius, float endRadius, Vec2 const& startRadiusScalarXY, Vec2 const& endRadiusScalarXY, Rgba8 const& color )
{
	for ( int i = 0; i < (ikChain->m_jointList.size() - 1); i++ )
	{
		IK_Joint3D* curJoint = ikChain->m_jointList[ i ];
		curJoint->InitMesh_CapsuleTaperedEllipse( start, end, startRadius, endRadius, startRadiusScalarXY, endRadiusScalarXY, color );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::InitMesh_2PerSegment( IK_Chain3D* const ikChain, float radius, float lengthPerSegment, float gapDistPerSegment )
{
	float numJoints		= float( ikChain->m_jointList.size() );
	float halfNumJoints = numJoints / 2.0f;
	float maxRadius		= radius;
	float minRadius		= maxRadius - ( gapDistPerSegment * 2.0f );
	for ( int i = 0; i < numJoints; i++ )
	{
		IK_Joint3D* curJoint = ikChain->m_jointList[ i ];
		if ( curJoint->m_child == nullptr )
		{
			continue;
		}

		if ( i < halfNumJoints )
		{
			radius = float( RangeMapClamped( float(i), 0.0f, halfNumJoints, minRadius, maxRadius ) );
		}
		else
		{
			radius = float( RangeMapClamped( float(i), halfNumJoints, numJoints, maxRadius, minRadius ) );
		}

		Vec3 fwdDir		= curJoint->m_child->m_jointPos_LS.GetNormalized();
		Vec3 endPos		= Vec3::ZERO + ( fwdDir * lengthPerSegment );	// Compute 1st segment
		curJoint->InitMesh_Cylinder( Vec3::ZERO, endPos, radius );
		Vec3 gapPos		= endPos + ( fwdDir * gapDistPerSegment );		// Create gap between 1st & 2nd segments
		Vec3 newEndPos	= gapPos + ( fwdDir * lengthPerSegment	);		// Compute 2nd segment
		curJoint->InitMesh_Cylinder( endPos, newEndPos, minRadius );
	}
}


//----------------------------------------------------------------------------------------------------------------------
IK_Chain3D* CreatureBase::GetSkeletonByName( std::string const& nameOfSkeletalSystem )
{
	// Return a pointer to the skeletal system with a matching name
	for ( int i = 0; i < m_skeletalList.size(); i++ )
	{
		IK_Chain3D* currentSkeleton = m_skeletalList[ i ];
		if ( currentSkeleton->m_name == nameOfSkeletalSystem )
		{
			return currentSkeleton;
		}
	}
	// If we got here, there was no valid skeletal system matching the "name"
	ERROR_AND_DIE( "INVALID SKELETON NAME" );
}


//----------------------------------------------------------------------------------------------------------------------
// "maxDist" is default initalized to -1. Unless a valid value is specified
// this function will compare dist with the maxLength of the entire skeletal system 
//----------------------------------------------------------------------------------------------------------------------
bool CreatureBase::IsLimbTooFarFromRoot( std::string const& nameOfSkeletalSystem, float maxDist )
{
	IK_Chain3D* currentLimb = GetSkeletonByName( nameOfSkeletalSystem );	
	if ( maxDist == -1.0f )
	{
		// If "maxDist" was NOT specified, use the default value ( "maxLength" of this Skeletal System )
		maxDist = currentLimb->GetMaxChainLength();
	}
	// Check if limb is placed too far from Root
	float distLimbEndToRootSq = GetDistance3D( currentLimb->m_target.m_currentPos_WS, m_modelRoot->m_jointPos_LS );
	float maxDistSq = maxDist;
	if ( distLimbEndToRootSq > maxDistSq )
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool CreatureBase::IsLimbTooFarFromPos( IK_Chain3D* const currentLimb, Vec3 const& refPosition, float maxDist )
{
	if ( maxDist == -1.0f )
	{
		// If "maxDist" was NOT specified, use the default value ( "maxLength" of this Skeletal System )
		maxDist = currentLimb->GetMaxChainLength() * 0.9f;
	}
	// Check if limb is placed too far from Root
	float distLimbEndToRoot	= GetDistance3D( currentLimb->m_target.m_currentPos_WS, refPosition );
	if ( distLimbEndToRoot > maxDist )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::ToggleIsControlled()
{
	m_isControlled = !m_isControlled;
}


//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::SetAllChainsCurTargetPosToTargetGoal()
{
	for ( int i = 0; i < m_skeletalList.size(); i++ )
	{
		IK_Chain3D* curChain = m_skeletalList[i];
		curChain->m_target.m_currentPos_WS = curChain->m_idealStepPos_WS;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void CreatureBase::DebugDrawAnchorStates( std::vector<Vertex_PCU>& verts, bool renderAtGoalOrientation, float radius ) const
{
	for ( int i = 0; i < m_skeletalList.size(); i++ )
	{
		IK_Chain3D* curChain = m_skeletalList[i];
		curChain->DebugDrawAnchorStates( verts, renderAtGoalOrientation, radius );
	}
}
