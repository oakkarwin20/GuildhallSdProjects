#pragma once

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Clock.hpp"

//----------------------------------------------------------------------------------------------------------------------
enum GameMode
{
	GAMEMODE_INVALID				= -1,
	GAMEMODE_2D_NEAREST_POINT			,	
	GAMEMODE_2D_RAYCAST_VS_DISC			,	
	GAMEMODE_2D_BILLIARDS				,	
	GAMEMODE_3D_TEST_SHAPES				,
	GAMEMODE_2D_RAYCAST_VS_LINE			,	
	GAMEMODE_2D_RAYCAST_VS_OBB2D		,	
	GAMEMODE_2D_RAYCAST_VS_AABB2		,	
	GAMEMODE_2D_PACHINKO_MACHINE		,	
	GAMEMODE_2D_BEZIER_CURVES			,	
	GAMEMODE_2D_SPIN_AND_FRICTION		,	
	GAMEMODE_3D_RAYCAST_VS_AABB3D		,	
	GAMEMODE_3D_RAYCAST_VS_OBB3D		,	
	GAMEMODE_2D_CONVEX_SCENE,	
	NUM_GAMEMODES						,
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
	virtual void Reshuffle()					= 0; 
	virtual void Shutdown();

	static GameModeBase* CreateNewGameOfType( GameMode type );

public:
	Clock				m_gameClock;
	bool				m_isSlowMo;
};