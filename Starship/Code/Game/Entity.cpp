#include "Game/Entity.hpp"
#include "Game/Game.hpp"

#include "Engine/Math/MathUtils.hpp"

Entity::Entity(Game* game, Vec2 position)
{
	m_game = game;
	m_position = position;
} 

bool Entity::isOffscreen() const
{
	return false;
}

bool Entity::isAlive() const
{
	return false;
}

Vec2 Entity::GetFowardNormal() const
{
	Vec2 forwardNormal = Vec2( CosDegrees( m_orientationDegrees ), SinDegrees( m_orientationDegrees ) );
	return forwardNormal;
}

void Entity::Update( float deltaSeconds )
{
	(void)deltaSeconds;
}

void Entity::Render() const
{
}