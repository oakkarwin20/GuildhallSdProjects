#pragma once

#include "Game/GameCommon.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Map.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Texture.hpp"

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
	void UpdatePauseQuitAndSlowMo();
	void UpdateCameras();
	void RenderUI() const;

	void UpdateToggleControlsToActor( float deltaSeconds );

	void UpdateLightingDebugInput();

	// DebugRenderSystem aka DRS
	void Render_DRS_UI_Text() const;

	// AttractMode
	void UpdateReturnToAttractMode();
	void RenderAttractMode() const;
	void UpdateAttractModeInput();

	// Lobby Functions
	void UpdateLobbyInput();
	void RenderLobby() const;

	// Game state Functions
	void EnterState( GameState state );
	void ExitState( GameState state );

private:
	bool					m_isPaused			= false;
	bool					m_isSlowMo			= false;
	Camera					m_attractCamera;
	Camera					m_screenCamera;
	PlayerController*		m_player			= nullptr;
	Texture*				m_testTexture		= nullptr;

public:
	Clock					m_clock;
	std::string				m_playerPosText; 
	std::string				m_timeText; 
	GameState				m_requestedState	= GameState::ATTRACT;
	GameState				m_currentState		= GameState::ATTRACT;
	Map*					m_currentMap		= nullptr;		
	bool					m_showLightingDebug	= false;

	//----------------------------------------------------------------------------------------------------------------------
	// Audio variables
	SoundPlaybackID			m_soundPlaybackID		= 1;
	Vec3					m_soundPosition			= Vec3::NEGATIVE_ONE;
	SoundID					m_attractMusic			= SoundID(-1);
	SoundID					m_gameMusic				= SoundID(-1);
	SoundPlaybackID			m_attractPlaybackID		= SoundPlaybackID( -1 );
	SoundPlaybackID			m_gamePlaybackID		= SoundPlaybackID( -1 );
	bool					m_restartAttractMusic	= false;
};