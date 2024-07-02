#pragma once

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

//----------------------------------------------------------------------------------------------------------------------
enum GameMode
{
	GAMEMODE_INVALID = -1,
	GAMEMODE_2D,
	GAMEMODE_3D,
	TWO_BONE_IK_3D,
	CCD_3D,
	FABRIK_3D,
	QUADRUPED_3D,
	SPIDER_3D,
	EULER_FROM_VECTOR_VISUAL_TEST_3D,
	ROBOT_ARM_3D,
	FABRIK_FWD_ONLY_3D,
	ANIMATIONS,
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

	// Game common functionality
	void UpdatePauseQuitAndSlowMo();

public:
};