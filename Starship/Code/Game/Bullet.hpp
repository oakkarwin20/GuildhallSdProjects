#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

//-------------------------------------------------------------------------------------------------------------------------------
constexpr int Num_Bullet_Triangles = 2;
constexpr int Num_Bullet_Verts = (Num_Bullet_Triangles * 3);

//-------------------------------------------------------------------------------------------------------------------------------
class Bullet : public Entity
{
public:
	Bullet( Game* game, Vec2 spawnPosition );
	~Bullet();
	
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

	float m_BULLET_COSMETIC_RADIUS = 0.f;
	float m_BULLET_PHYSICS_RADIUS  = 0.f;

private:
	void InitializeLocalVerts();

private:
	Vertex_PCU m_localVerts[Num_Bullet_Verts];
};