#pragma once

#include <vector>


//----------------------------------------------------------------------------------------------------------------------
struct Vec3;
class Plane3D;


//-----------------------------------------------------------------------------------------------------------------------
class ConvexHull3D
{
public:
	ConvexHull3D( std::vector<Plane3D> m_boundingPlanesList );
	~ConvexHull3D();

	bool  IsPointInside	 ( Vec3 const& point		);
	Vec3  GetNearestPoint( Vec3 const& pointToCheck );

private:
	std::vector<Plane3D> m_boundingPlanesList;
};