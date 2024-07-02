#include "Game/GameModeBase.hpp"
#include "Game/GameModeFifaTest2D.hpp"
#include "Game/GameModeFifaTest3D.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
GameModeBase::GameModeBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameModeBase::~GameModeBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBase::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBase::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
GameModeBase* GameModeBase::CreateNewGameOfType( GameMode type )
{
	switch ( type )
	{
		case GAMEMODE_2D_FIFA_TEST:		return new GameModeFifaTest2D();
		case GAMEMODE_3D_FIFA_TEST:		return new GameModeFifaTest3D();

		default:
		{
			ERROR_AND_DIE( Stringf( "ERROR: Unknown GameMode #%i", type ) );
		}
	}
}
