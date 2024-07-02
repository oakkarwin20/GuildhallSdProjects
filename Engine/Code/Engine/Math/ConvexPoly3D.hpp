#pragma once

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
struct Vec3;


//-----------------------------------------------------------------------------------------------------------------------
class ConvexPoly3D
{
public:
	ConvexPoly3D( std::vector<Vec3> ccwOrderedPoints );
	~ConvexPoly3D();

	bool  IsPointInside( Vec3 const& point );
	Vec3  GetNearestPoint( Vec3 const& pointToCheck );

private:
	std::vector<Vec3> m_ccwOrderedPoints;
};