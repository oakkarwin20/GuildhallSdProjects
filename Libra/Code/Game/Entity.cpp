#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"

//----------------------------------------------------------------------------------------------------------------------
// #ToDo add EntityType type, EntityFaction faction, to the constructor
Entity::Entity( Map* currentMap, Vec2 position, float orientation, EntityType type )
	: m_map( currentMap )
	, m_position( position )
	, m_orientationDegrees( orientation )
	, m_type( type )
{
}

Entity::~Entity()
{
}

//----------------------------------------------------------------------------------------------------------------------
bool Entity::IsOffscreen() const
{
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool Entity::IsAlive() const
{
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 Entity::GetFowardNormal() const
{
	Vec2 forwardNormal = Vec2( CosDegrees( m_orientationDegrees ), SinDegrees( m_orientationDegrees ) );
	return forwardNormal;
}

void Entity::Die()
{
	m_isDead	= true;
	m_isGarbage	= true;
}

//----------------------------------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	m_position += ( m_velocity * deltaSeconds );
	m_orientationDegrees += ( m_angularVelocity * deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void Entity::Render() const
{
}