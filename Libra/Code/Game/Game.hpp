#pragma once

#include "Game/GameCommon.hpp"
#include "Game/PlayerTank.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------
constexpr int NUM_ATTRACTMODE_TRI = 4;
constexpr int NUM_ATTRACTMODE_TRI_VERTS = NUM_ATTRACTMODE_TRI * 3;
constexpr int NUM_PAUSE_SCREEN_TRI = 2;
constexpr int NUM_PAUSE_SCREEN_TRI_VERTS = NUM_PAUSE_SCREEN_TRI * 3;

//----------------------------------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void Startup();
	void Shutdown();
	void Update( float deltaSeconds );
	void Render() const;

public:	
	float randNumX = 50.f;
	float randNumY = 50.f;

	Vec2 m_shipSpawnPosition = Vec2( 2.5f, 2.5f );
	PlayerTank* m_playerTank = nullptr;
	Map* m_currentMap = nullptr;

private:
	void UpdatePauseQuitAndSlowMo( float& deltaSeconds );
	void UpdateReturnToAttractMode();
	void UpdateAttractModeInput();
	void UpdateCameras();

	void RenderAttractMode() const;
	void RenderPauseScreen() const;
	void UpdateAttractMode();
	void InitializePauseScreenVerts();
	void InitializeTileDefs();
	void DebugDrawFont() const;
	void DebugDrawAnimations() const;

	MapDefinition m_map1Def;
	MapDefinition m_map2Def;
	MapDefinition m_map3Def;

private:
	bool m_isPaused						= false;
	bool m_isSlowMo						= false;
	bool m_isFastMo						= false;
	bool m_AttractModeIsOn				= true;
	bool m_shouldGameMusicRepeat		= true;
	bool m_shouldAttractMusicRepeat		= true;
	bool m_isDebugCamOn					= false;

	float m_tilesVisibleOnScreenVertically = 8.0f;

	Camera m_worldCamera;
	Camera m_screenCamera;
	Camera m_attractCamera;

	Vertex_PCU m_pauseScreenLocalVerts[NUM_PAUSE_SCREEN_TRI_VERTS];
	
	Texture* m_attractModeTestTexture	= nullptr;
	Texture* m_attractModeTestSprite	= nullptr;
	Texture* m_attractScreenTexture		= nullptr;
	Texture* m_victoryScreenTexture		= nullptr;
	Texture* m_youDiedScreenTexture		= nullptr;
	Texture* m_enemyAriesTexture		= nullptr;
	Texture* m_animTexture				= nullptr;

	SoundPlaybackID m_attractMusic	= (SoundPlaybackID)-1;
	SoundPlaybackID m_gameMusic		= (SoundPlaybackID)-1;
	SoundPlaybackID m_pauseSound	= (SoundPlaybackID)-1;

	SoundID attractMusic;
	SoundID gameplayMusic;
	SoundID pauseSound;
};