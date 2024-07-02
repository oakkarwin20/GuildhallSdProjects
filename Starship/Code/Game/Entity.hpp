#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec2.hpp"
//#include "Game/Game.hpp"

class Game;

class Entity
{
public:
	Entity( Game* game, Vec2 position );

//accessors
	bool	isOffscreen() const; //if radius off screen then return true
	bool	isAlive() const;
	Vec2	GetFowardNormal() const;

//variables
	Vec2	m_position; 
	Vec2	m_velocity;
	float	m_orientationDegrees; 
	float	m_angularVelocity = 0.f;	//spin rate in degrees per second
	float	m_physicsRadius = 0.f;		//inner radius
	float	m_cosmeticRadius = 0.f;		//outer radius
	int		m_health = 1;
	bool	m_isDead = false;
	bool	m_isGarbage = false;

	Game* m_game = nullptr;

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
};