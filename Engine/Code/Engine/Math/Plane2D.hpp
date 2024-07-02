#pragma once

#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------------------------------
class Plane2D
{
public:
	Plane2D( Vec2 const& normal, float distFromOrigin );
	~Plane2D();

	float		GetSignedAltitude( Vec2 const& pointToCheck );
	Vec2  const GetNearestPointToOrigin() const;
public:
	Vec2	m_normal			= Vec2::X_FWD;
	float	m_distFromOrigin	= 0.0f;
	Vec2	m_debugNormalPos	= Vec2::ZERO;
};