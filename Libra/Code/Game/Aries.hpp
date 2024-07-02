#pragma once

#include "Game/Entity.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Texture;
struct Vec2;

//----------------------------------------------------------------------------------------------------------------------
class Aries : public Entity
{
public:
	Aries( Map* currentMap, Vec2 position, float orientation, EntityType type );

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

	int m_health = 3;

private:
	Texture* m_ariesTankTexture = nullptr;

	float m_scorpioOrientationDegrees = 0.0f;
};

