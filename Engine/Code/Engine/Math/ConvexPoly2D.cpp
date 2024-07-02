#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EngineCommon.hpp"

//----------------------------------------------------------------------------------------------------------------------
ConvexPoly2D::ConvexPoly2D()
{
}


//----------------------------------------------------------------------------------------------------------------------
ConvexPoly2D::ConvexPoly2D( std::vector<Vec2> ccwOrderedPoints )
{
	m_ccwOrderedPoints = ccwOrderedPoints;
}


//----------------------------------------------------------------------------------------------------------------------
ConvexPoly2D::~ConvexPoly2D()
{
}


//----------------------------------------------------------------------------------------------------------------------
bool ConvexPoly2D::IsPointInside( Vec2 const& point )
{
	UNUSED( point );
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
Vec2 ConvexPoly2D::GetNearestPoint( Vec2 const& pointToCheck )
{
	UNUSED( pointToCheck );
	return Vec2();
}


//----------------------------------------------------------------------------------------------------------------------
std::vector<Vec2> const& ConvexPoly2D::GetCcwOrderedPoints() const
{
	return m_ccwOrderedPoints;
}