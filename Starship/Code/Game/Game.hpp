#pragma once
#include "Game/PlayerShip.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"

//---------------------------------------------------------------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void StartUp();
	void Shutdown();
	void Update();
	void Render() const;

	Vec2 SpawnBullet( Vec2 tipOfShip, float orientationDegrees );
	void SpawnAsteroids();
	void RenderDebug() const;

	Vec2 m_shipSpawnPosition = Vec2( WORLD_CENTER_X, WORLD_CENTER_Y );
	Vec2 m_AstSpawnPosition = Vec2( randNumX, randNumY );
	Vec2 m_bulletSpawnPosition = Vec2(m_shipSpawnPosition.x + 1, m_shipSpawnPosition.y );

	bool isDebugDisplayOn = false;
	float randNumX = 50.f;
	float randNumY = 50.f;

	Clock m_gameClock;

private:
	void UpdateEntities();
	void AttractModeInput();
	void UpdatePauseAndSlowMo();
	void RenderEntities() const;
	void UpdateCameras();
	void RenderAttractMode() const;
	void InitializeAttractModeVerts();

	PlayerShip* m_playerShip = nullptr;					
	Asteroid*	m_asteroids [Num_Max_Asteriods] = {};	
	Bullet*		m_bullets [Max_Bullets] = {};			

	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_AttractModeIsOn = true;
	float m_attractModeShipOrientation = 0.0f;

	Camera m_worldCamera;
	Camera m_screenCamera;
	
	Vertex_PCU m_localVerts[NUM_PLAYERSHIP_VERTS];
};