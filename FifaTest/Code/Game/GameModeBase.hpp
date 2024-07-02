#pragma once

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Clock.hpp"

//----------------------------------------------------------------------------------------------------------------------
enum GameMode
{
	GAMEMODE_INVALID = -1,
	GAMEMODE_2D_FIFA_TEST,
	GAMEMODE_3D_FIFA_TEST,
	NUM_GAMEMODES,
};

//----------------------------------------------------------------------------------------------------------------------
class GameModeBase
{
public:
	GameModeBase();
	virtual ~GameModeBase();
	virtual void Startup();
	virtual void Update( float deltaSeconds )	= 0;
	virtual void Render() const					= 0;
	virtual void Shutdown();

	static GameModeBase* CreateNewGameOfType( GameMode type );

public:
	Clock				m_gameClock;
	bool				m_isSlowMo;
};