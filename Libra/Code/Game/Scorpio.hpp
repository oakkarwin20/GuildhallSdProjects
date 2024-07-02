#pragma once

#include "Game/Game.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Texture;
struct Vec2;

//----------------------------------------------------------------------------------------------------------------------
class Scorpio : public Entity
{
public:
	Scorpio( Map* currentMap, Vec2 position, float orientationDegrees, EntityType type );

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

	EntityFaction m_faction = FACTION_EVIL;

	int m_health = 3;

private:
	Texture* m_scorpioTankTexture	= nullptr;
	Texture* m_scorpioTurretTexture	= nullptr;

	float m_scorpioOrientationDegrees = 0.0f;
};