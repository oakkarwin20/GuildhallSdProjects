#pragma once

#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------------------------------
class Plane3D
{
public:
	Plane3D( Vec3 const& normal, float distFromOrigin );
	~Plane3D();

	float GetSignedAltitude();

public:
	Vec3	m_normal			= Vec3( 0.0f, 0.0f, 0.0 );
	float	m_distFromOrigin	= 0.0f;
};