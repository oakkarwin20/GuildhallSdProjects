#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class GameModeProtogame3D;

//----------------------------------------------------------------------------------------------------------------------
class Entity
{
public:
	Entity( GameModeProtogame3D* game );
	virtual ~Entity();

	virtual void Update( float deltaSeconds ) = 0;
	virtual void Render() const = 0;
	Mat44 GetModelMatrix( Vec3 position, EulerAngles orientation ) const;

public:
	Vec3						m_position;
	Vec3						m_velocity;
	EulerAngles					m_orientationDegrees; 
	EulerAngles					m_angularVelocity;	// spin rate in degrees per second
	std::vector<Vertex_PCU>		m_vertexes;
	Rgba8						m_color = Rgba8::WHITE;
	GameModeProtogame3D*		m_game = nullptr;
};