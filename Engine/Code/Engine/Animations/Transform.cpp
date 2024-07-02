#include "Engine/Animations/Transform.hpp"


//----------------------------------------------------------------------------------------------------------------------
Transform::Transform()
{
}


//----------------------------------------------------------------------------------------------------------------------
Transform::Transform( Vec3 const& position, Quaternion const& rotation, Vec3 const& scale )
	: m_position( position ), m_rotation( rotation ), m_scale( scale )
{
}


//----------------------------------------------------------------------------------------------------------------------
Transform Transform::ApplyChildToParentTransform( Transform const& child, Transform const& parent )
{
	Transform result;

	// Combine scale
	result.m_scale = parent.m_scale * child.m_scale;
	// Combine rotation
	result.m_rotation = parent.m_rotation * child.m_rotation;
	// Combine position
	result.m_position = parent.m_rotation * ( parent.m_scale * child.m_position );	// Get child position into parent space
	result.m_position = parent.m_position + result.m_position;	

	return result;
}
