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

	void UpdatePlayerInput( float deltaSeconds );

	float	m_speed			= 2.0f;
	float	m_elevateSpeed  = 6.0f;
	float	m_turnRate		= 90.0f;
	Camera	m_worldCamera;
};