#pragma once

#include "Game/GameModeBase.hpp"

#include "Engine/Renderer/Camera.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Football2D;
class Player2D;

//----------------------------------------------------------------------------------------------------------------------
class GameModeFifaTest2D : public GameModeBase
{
public:
	GameModeFifaTest2D();
	virtual ~GameModeFifaTest2D();
	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Shutdown();

	// Camera and Render Functions
	void UpdateFifaTest2DCamera();
	void RenderWorldObjects()	const;
	void RenderUIObjects()		const;

	// Game input and debug functions
	void UpdatePauseQuitAndSlowMo();

	// Player Functions
	void UpdatePlayerPhysics( float deltaSeconds );

	// Football Functions
	void UpdateFootballPhysics( float deltaSeconds );
	
public:
	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	Camera	m_fifaTestWorldCamera;
	Camera	m_fifaTestUICamera;

	// Football Variables
	Football2D* m_football = nullptr;

	// Player Variables
	Player2D*	m_player = nullptr;
};
