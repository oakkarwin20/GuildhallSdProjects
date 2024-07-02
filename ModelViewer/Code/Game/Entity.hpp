#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"


#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class Game;
class VertexBuffer;
class IndexBuffer;

//----------------------------------------------------------------------------------------------------------------------
class Entity
{
public:
	Entity( Game* game );
	virtual ~Entity();

	virtual void Update( float deltaSeconds ) = 0;
	virtual void Render() const = 0;
	Mat44 GetModelMatrix( Vec3 position, EulerAngles orientation ) const;

public:
	Vec3						m_position;
	Vec3						m_velocity;
	EulerAngles					m_orientationDegrees; 
	EulerAngles					m_angularVelocity;	// spin rate in degrees per second
	std::vector<Vertex_PCU>		m_verts_PCU;
	Rgba8						m_color = Rgba8::WHITE;
	Game*						m_game = nullptr;

	//----------------------------------------------------------------------------------------------------------------------
	// Objects to render
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCUTBN>  m_vertList_PCUTBN;
	std::vector<unsigned int>   m_indexList;
	VertexBuffer*				m_vbo			= nullptr;
	IndexBuffer*				m_ibo			= nullptr;
};