#pragma once

#include "Engine/Math/Plane2D.hpp"

#include <vector>


//----------------------------------------------------------------------------------------------------------------------
struct Vec2;
class  ConvexPoly2D;

//-----------------------------------------------------------------------------------------------------------------------
class ConvexHull2D
{
public:
	ConvexHull2D();
	ConvexHull2D( ConvexPoly2D const& convexPoly2D );
	ConvexHull2D( std::vector<Plane2D> boundingPlanesList );
	~ConvexHull2D();

	bool						IsPointInside  ( Vec2 const& point );
	Vec2						GetNearestPoint( Vec2 const& pointToCheck );
	std::vector<Plane2D> const& GetPlanesList() const;

private:
	std::vector<Plane2D> m_boundingPlanesList;
};