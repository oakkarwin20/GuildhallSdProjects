#pragma once

#include "Game/GameModeBase.hpp"

#include "Engine/Renderer/Camera.hpp"

//----------------------------------------------------------------------------------------------------------------------
class GameMode2D : public GameModeBase
{
public:
	GameMode2D();
	virtual ~GameMode2D();
	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Shutdown();

	// Camera and Render Functions
	void UpdateGameMode2DCamera();
	void RenderWorldObjects()	const;
	void RenderUIObjects()		const;
	
public:
	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	Camera	m_gameModeWorldCamera;
	Camera	m_gameModeUICamera;
};
