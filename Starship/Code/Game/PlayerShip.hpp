#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"

//-------------------------------------------------------------------------------------------------------------------------------
constexpr int NUM_PLAYERSHIP_TRIS = 5;
constexpr int NUM_PLAYERSHIP_VERTS = (NUM_PLAYERSHIP_TRIS * 3);

constexpr int NUM_PLAYERSHIP_ICON_TRIS = 15;
constexpr int NUM_PLAYERSHIP_ICON_VERTS = (NUM_PLAYERSHIP_ICON_TRIS * 3);

constexpr int NUM_PLAYERSHIP_THRUSTER_TRIS = 1;
constexpr int NUM_PLAYERSHIP_THRUSTER_VERTS = (NUM_PLAYERSHIP_THRUSTER_TRIS * 3);

//-------------------------------------------------------------------------------------------------------------------------------
class PlayerShip : public Entity
{ 
public:
	PlayerShip( Game* game, Vec2 spawnPosition );

	void Startup ();
	void Shutdown();
	void Update( float deltaSeconds );
	void Render() const;

	bool m_isDead = false;
	int m_extraLives = 3;
	float m_thrustFraction = 0.0f;

	// add fire bullet in foward direction from "nose" WHEN 'spacebar' keypressed, keydown (holding) does nth.  

	/*Collision logic
	
	dies when colliding with Asteroid

	respawn at screen center with 0 velocity, facing east IF 'N' keypressed while dead

	bounces off "walls" edges, x or y velocity is reversed

	*/

	void UpdateFromController( float deltaSeconds );

private:
	void initializelocalVerts();
	void initializeThrusterVerts();

	void DrawPlayerThruster() const;
	void DrawPlayerHealthIcon1() const;
	void DrawPlayerHealthIcon2() const;
	void DrawPlayerHealthIcon3() const;
private:
	Vertex_PCU m_localVerts[NUM_PLAYERSHIP_VERTS];
	Vertex_PCU m_localThrusterVerts[NUM_PLAYERSHIP_THRUSTER_VERTS];
	Vertex_PCU m_localIconVerts[NUM_PLAYERSHIP_ICON_TRIS];
};
