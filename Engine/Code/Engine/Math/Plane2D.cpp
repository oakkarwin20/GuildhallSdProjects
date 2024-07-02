#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/MathUtils.hpp"


//----------------------------------------------------------------------------------------------------------------------
Plane2D::Plane2D( Vec2 const& normal, float distFromOrigin )
{
	m_normal		 = normal;
	m_distFromOrigin = distFromOrigin;
}


//----------------------------------------------------------------------------------------------------------------------
Plane2D::~Plane2D()
{
}


//----------------------------------------------------------------------------------------------------------------------
float Plane2D::GetSignedAltitude( Vec2 const& pointToCheck )
{
	// Note: pointToCheck is technically a position, but its disp
	//		 from origin is (pointToCheck - 0) so we don't bother computing it
	float projectedLengthOnNormal = DotProduct2D( pointToCheck, m_normal );
	float signedAltitude		  = projectedLengthOnNormal - m_distFromOrigin;
	return signedAltitude;
}


//----------------------------------------------------------------------------------------------------------------------
Vec2 const Plane2D::GetNearestPointToOrigin() const
{
	Vec2 nearestPointToOrigin = Vec2::ZERO + ( m_normal * m_distFromOrigin );
	return nearestPointToOrigin;
}