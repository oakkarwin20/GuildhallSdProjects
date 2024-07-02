#include "Engine/Math/ConvexHull3D.hpp"
#include "Engine/Math/Plane3D.hpp"
#include "Engine/Core/EngineCommon.hpp"


//----------------------------------------------------------------------------------------------------------------------
ConvexHull3D::ConvexHull3D( std::vector<Plane3D> boundingPlanesList )
{
	m_boundingPlanesList = boundingPlanesList;
}


//----------------------------------------------------------------------------------------------------------------------
ConvexHull3D::~ConvexHull3D()
{
}


//----------------------------------------------------------------------------------------------------------------------
bool ConvexHull3D::IsPointInside( Vec3 const& point )
{
	UNUSED( point );
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
Vec3 ConvexHull3D::GetNearestPoint( Vec3 const& pointToCheck )
{
	UNUSED( pointToCheck );
	return Vec3();
}
