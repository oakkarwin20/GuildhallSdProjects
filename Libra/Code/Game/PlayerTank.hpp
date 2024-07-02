#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Texture;

//----------------------------------------------------------------------------------------------------------------------
constexpr int NUM_PLAYER_TANK_TRI			= 3;
constexpr int NUM_PLAYER_TANK_VERTS			= ( NUM_PLAYER_TANK_TRI * 3 );

constexpr int NUM_PLAYER_TANK_TURRET_TRIS	= 2;
constexpr int NUM_PLAYER_TANK_TURRET_VERTS	= ( NUM_PLAYER_TANK_TURRET_TRIS * 3 );
 
//----------------------------------------------------------------------------------------------------------------------
class PlayerTank : public Entity
{
public:
	PlayerTank( Map* map, Vec2 position, float orientationDegrees, EntityType type );

	void			Startup();
	void			Shutdown();
	virtual void	Update(float deltaSeconds) override;
	virtual void	Render() const override;

	void			RenderTank() const;
	void			RenderTurret() const;

public:
	Vec2 m_tankMoveIntention;
	Vec2 m_turrentTurnIntention;

	float&	m_tankOrientationDegrees = m_orientationDegrees;
	float	m_turretOrientationDegrees = 0.0f;
	float	m_tankGoalOrientationDegrees = 0.0f;
	float	m_turretGoalOrientationDegrees = 0.0f;

	Vertex_PCU m_tankLocalVerts[NUM_PLAYER_TANK_VERTS];
	Vertex_PCU m_turretLocalVerts[NUM_PLAYER_TANK_TURRET_VERTS];
	
	EntityFaction m_faction = FACTION_GOOD;
	
	int m_health = 3;

private:
	Texture* m_tankTexture	 = nullptr;
	Texture* m_turretTexture = nullptr;
};