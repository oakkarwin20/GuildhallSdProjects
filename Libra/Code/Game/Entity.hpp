#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/VertexUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Game;
class Map;

//----------------------------------------------------------------------------------------------------------------------
enum EntityFaction
{
	FACTION_INVALID	 = -1,
	FACTION_GOOD		 ,
	FACTION_NEUTRAL		 ,
	FACTION_EVIL		 ,
	NUM_FACTIONS		 ,
};

//----------------------------------------------------------------------------------------------------------------------
enum EntityType
{
	ENTITY_TYPE_INVALID			= -1,
	ENTITY_TYPE_GOOD_PLAYER			, 
	ENTITY_TYPE_EVIL_SCORPIO		, 
	ENTITY_TYPE_EVIL_LEO			, 
	ENTITY_TYPE_BANANA				,
	ENTITY_TYPE_EVIL_ARIES			, 
	ENTITY_TYPE_GOOD_BULLET			, 
	ENTITY_TYPE_EVIL_BULLET			, 
	NUM_ENTITY_TYPES				,
};

//----------------------------------------------------------------------------------------------------------------------
class Entity
{
public:
	Entity( Map* currentMap, Vec2 position, float orientation, EntityType type );
	virtual ~Entity();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;

//accessors
	bool			IsOffscreen() const;		// if radius off screen then return true
	bool			IsAlive() const;
	Vec2			GetFowardNormal() const;
	virtual	void	Die();

//variables
	Vec2	m_position; 
	Vec2	m_velocity;
	float	m_orientationDegrees; 
	float	m_angularVelocity = 0.f;	// spin rate in degrees per second
	float	m_physicsRadius = 0.f;		// inner radius
	float	m_cosmeticRadius = 0.f;		// outer radius
	float	m_health = 1.0f;
	bool	m_isDead = false;
	bool	m_isGarbage = false;

	bool	m_isPushedByEntities	= false;
	bool	m_doesPushEntities		= false;
	bool	m_hitByBullet			= false;
	bool	m_isPushedByWalls		= false;

	// #ToDo create the functions below
	// die()
	// takeDamge, isdead = true
	// reduce health by one, if health < 0, kill

	Map*			m_map	= nullptr;
	EntityType		m_type	= ENTITY_TYPE_INVALID;
	EntityFaction	m_faction;
};

typedef std::vector<Entity*> EntityList;