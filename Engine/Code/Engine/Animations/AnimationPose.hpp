 #pragma once

#include "Engine/Math/Mat44.hpp"
#include "Engine/Animations/Transform.hpp"

#include <string>
#include <vector>


//----------------------------------------------------------------------------------------------------------------------
struct AnimationJoint
{
	Transform m_localToParentTransform;
	int m_parentJointIndex;
	std::string m_jointName;
};


//----------------------------------------------------------------------------------------------------------------------
class AnimationPose
{
public:
	Transform LocalToModel_Transform( int jointID ) const;

public:
// 	std::vector<Mat44>		 m_localToParentTransforms;
// 	std::vector<int>		 m_parentJointIndexes;
// 	std::vector<std::string> m_jointNames;

	std::vector<AnimationJoint> m_jointList;
};