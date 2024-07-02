#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

//-------------------------------------------------------------------------------------------------------------------------------
constexpr int Num_Asteroid_Triangles_Slices = 16;
constexpr int Num_Asteroid_Verts = (Num_Asteroid_Triangles_Slices * 3);

//-------------------------------------------------------------------------------------------------------------------------------
class Asteroid : public Entity 
{
public:
	Asteroid( Game* game, Vec2 position );
	~Asteroid();

	void Update( float deltaSeconds );
	void Render() const;

	float m_ASTERIOD_COSMETIC_RADIUS = 0;
	float m_ASTERIOD_PHYSICS_RADIUS = 0;

private:
	void initializelocalVerts();

private:
	Vertex_PCU m_localVerts[Num_Asteroid_Verts];
};