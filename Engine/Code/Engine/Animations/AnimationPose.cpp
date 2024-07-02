#include "Engine/Animations/AnimationPose.hpp"


//----------------------------------------------------------------------------------------------------------------------
Transform AnimationPose::LocalToModel_Transform( int jointID ) const
{
	AnimationJoint const&	joint				= m_jointList[jointID];
	int						parentIndex			= joint.m_parentJointIndex;
	Transform				modelSpaceTransform = joint.m_localToParentTransform;
	while ( parentIndex >= 0 )
	{
		Transform const& localToParentTransformOfThisJoint	= modelSpaceTransform;
		Transform const& parentJointTransform				= m_jointList[parentIndex].m_localToParentTransform;
		modelSpaceTransform									= Transform::ApplyChildToParentTransform(localToParentTransformOfThisJoint, parentJointTransform);
		parentIndex											= m_jointList[parentIndex].m_parentJointIndex;
	}
	return modelSpaceTransform;
}