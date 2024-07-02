#pragma once

#include "Game/Entity.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Texture;
struct Vec2;

//----------------------------------------------------------------------------------------------------------------------
enum BulletType
{
	// #ToDo remove hardcoding values for all so the list can be changed dynamically
	TEST_BULLET		= 0,
	GOOD_BULLET		= 1,
	GOOD_BOLT		= 2,
	EVIL_BULLET		= 3,
	EVIL_BOLT		= 4,
};

class Bullet : public Entity
{
public:
	Bullet( Map* currentMap, Vec2 position, float orientation, EntityType type );

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

	void Bounce( Vec2& normalized );

	int m_health = 3;

private:
	Texture* m_bulletTexture = nullptr;

	float m_bulletOrientationDegrees = 0.0f;
};