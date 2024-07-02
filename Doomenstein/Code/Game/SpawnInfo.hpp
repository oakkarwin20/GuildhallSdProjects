#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

#include <string>

//----------------------------------------------------------------------------------------------------------------------
class SpawnInfo
{
public:
	SpawnInfo();
	~SpawnInfo();

public:
	std::string		m_actorName			= std::string("INVALID ACTOR NAME");
	Vec3			m_actorPosition		= Vec3( -1.0f, -1.0f, -1.0f );
	EulerAngles		m_actorOrientation	= EulerAngles( 0.0f, 0.0f, 0.0f );
};