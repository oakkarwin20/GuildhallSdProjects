#pragma once

#include "Game/Entity.hpp"
#include "Engine/Renderer/Camera.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Player : public Entity
{
public:
	Player( Game* game );
	~Player();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;

	void AddVertsForCompass( std::vector<Vertex_PCU>& compassVerts, Vec3 position, float axisLength, float axisThickness ) const;

	void UpdatePlayerInput( float deltaSeconds );

public:
	float	m_defaultSpeed	= 4.0f;
	float	m_currentSpeed	= 4.0f;
	float	m_fasterSpeed	= m_defaultSpeed * 15.0f;
	float	m_slowerSpeed	= m_defaultSpeed * 0.25f;
	float	m_elevateSpeed  = 6.0f;
	float	m_turnRate		= 90.0f;
	Camera	m_worldCamera;
};