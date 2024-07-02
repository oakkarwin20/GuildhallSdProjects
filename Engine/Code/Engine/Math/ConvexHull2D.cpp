#include "Engine/Math/ConvexHull2D.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//----------------------------------------------------------------------------------------------------------------------
ConvexHull2D::ConvexHull2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
ConvexHull2D::ConvexHull2D( ConvexPoly2D const& convexPoly2D )
{
	std::vector<Vec2> const& orderedPoints = convexPoly2D.GetCcwOrderedPoints();
	int	sizeConvexPolyPoints = int( orderedPoints.size() );
	// 1. Loop through all points in convexPoly
	for ( int i = 0; i < sizeConvexPolyPoints; i++ )
	{
		// 2. Get normal from disps (curPoint to nextPoint) rotated -90 degrees
		Vec2 pointA = orderedPoints[i + 0];
		Vec2 pointB = Vec2::ZERO;
		// 2a. Handle edge case for end of loop
		//	   i + 1 will cause index out of bounds error
		//	   instead, we choose pointB as the 0 index
		if ( i == ( sizeConvexPolyPoints - 1 ) )
		{
			pointB = orderedPoints[0];
		}
		else
		{
			pointB = orderedPoints[i + 1];
		}
		Vec2 planeNormal	= ( pointB - pointA ).GetNormalized();
		planeNormal			= -planeNormal.GetRotated90Degrees();
		// 3. Calculate dist from origin
		float distFromOrigin = DotProduct2D( pointA, planeNormal );
		// 4. Create planes from normals and dists
		Plane2D plane = Plane2D( planeNormal, distFromOrigin );
		// 5. Compute and store debug positions for rendering plane normals
		Vec2 lengthAToB				= pointB - pointA;
		lengthAToB				   *= 0.5f;
		Vec2 debugNormalPos			= pointA + lengthAToB;
		plane.m_debugNormalPos		= debugNormalPos;
		// 6. Add to boundingPlanesList
		m_boundingPlanesList.push_back( plane );
	}
}


//----------------------------------------------------------------------------------------------------------------------
ConvexHull2D::ConvexHull2D( std::vector<Plane2D> boundingPlanesList )
{
	m_boundingPlanesList = boundingPlanesList;
}


//----------------------------------------------------------------------------------------------------------------------
ConvexHull2D::~ConvexHull2D()
{
}


//----------------------------------------------------------------------------------------------------------------------
bool ConvexHull2D::IsPointInside( Vec2 const& point )
{
	UNUSED( point );
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
Vec2 ConvexHull2D::GetNearestPoint( Vec2 const& pointToCheck )
{
	UNUSED( pointToCheck );
	return Vec2();
}


//----------------------------------------------------------------------------------------------------------------------
std::vector<Plane2D> const& ConvexHull2D::GetPlanesList() const
{
	return m_boundingPlanesList;
}