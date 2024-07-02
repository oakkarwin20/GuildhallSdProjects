#pragma once

#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Player.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Texture.hpp"

//----------------------------------------------------------------------------------------------------------------------
class World;
class SpriteSheet;

//----------------------------------------------------------------------------------------------------------------------
enum class GameState
{
	NONE,
	ATTRACT,
	PLAYING,
	LOBBY,
	COUNT
};

//----------------------------------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void StartUp();
	void Shutdown();
	void Update();
	void Render() const;

private:
	// Core Input
	void UpdatePauseQuitAndSlowMo();
	
	// Camera Functions
	void UpdateCameras();
	void UpdateWorldCamera();
	void UpdateAttractCamera();
	void RenderUI() const;
	
	// Entity Functions
	void UpdateEntities(float deltaSeconds);
	void RenderEntities() const;
	void DeleteEntities();

	// DebugRenderSystem aka DRS
	void Render_DRS_WorldBasisText() const;
	void Render_DRS_WorldBasis() const;
	void Render_DRS_UI_Text() const;

	void CheckDebugCheats();

	//----------------------------------------------------------------------------------------------------------------------
	// Game State Functions
	//----------------------------------------------------------------------------------------------------------------------
	// AttractMode
	void UpdateReturnToAttractMode();
	void UpdateAttractModeInput();
	void RenderAttractMode() const;

	// Game state Functions
	void EnterState( GameState state );
	void ExitState(  GameState state );

public:
	// Game State Variables
	GameState				m_requestedState = GameState::ATTRACT;
	GameState				m_currentState	 = GameState::ATTRACT;

	// Core Variables
	Player*						m_player;
	Clock						m_clock;
	std::vector<Entity*>		m_entityList;
	std::string					m_playerPosText; 
	std::string					m_timeText; 
	bool						m_isPaused			= false;
	bool						m_isSlowMo			= false;

	// World Variables
	World* m_currentWorld		= nullptr;

	// SpriteSheet & Texture Variables
	SpriteSheet* m_blockSpriteSheet = nullptr;
	Texture*	 m_blockTexture		= nullptr;

private:
	Camera						m_attractCamera;
	Camera						m_screenCamera;
	Texture*					m_testTexture		= nullptr;
};