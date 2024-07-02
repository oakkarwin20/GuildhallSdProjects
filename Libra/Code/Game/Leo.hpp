#pragma once

#include "Game/Entity.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Texture;
struct Vec2;

//----------------------------------------------------------------------------------------------------------------------
class Leo : public Entity
{
public:
	Leo( Map* currentMap, Vec2 position, float orientation, EntityType type );

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

	EntityFaction m_faction = FACTION_EVIL;

	int m_health = 3;

private:
	Texture* m_leoTankTexture = nullptr;

	float m_leoOrientationDegrees = 0.0f;
};

