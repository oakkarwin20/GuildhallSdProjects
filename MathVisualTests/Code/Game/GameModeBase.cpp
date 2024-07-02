#include "Game/GameModeBase.hpp"
#include "Game/GameModeNearestPoint2D.hpp"
#include "Game/GameModeRaycastVsDisc2D.hpp"
#include "Game/GameModeBilliards2D.hpp"
#include "Game/GameModeTestShapes3D.hpp"
#include "Game/GameModeRaycastVsLine2D.hpp"
#include "Game/GameModeRaycastVsAABB2D.hpp"
#include "Game/GameModeRaycastVsAABB3D.hpp"
#include "Game/GameModeRaycastVsOBB2D.hpp"
#include "Game/GameModeRaycastVsOBB3D.hpp"
#include "Game/GameModePachinkoMachine.hpp"
#include "Game/GameModeBezierCurves.hpp"
#include "Game/GameModeSpinAndFriction.hpp"
#include "Game/GameModeConvexScene2D.hpp"

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
		case GAMEMODE_2D_NEAREST_POINT:		return new GameModeNearestPoint2D();
		case GAMEMODE_2D_RAYCAST_VS_DISC:	return new GameModeRaycastVsDisc2D();
		case GAMEMODE_2D_BILLIARDS:			return new GameModeBilliards2D();
		case GAMEMODE_3D_TEST_SHAPES:		return new GameModeTestShapes3D();
		case GAMEMODE_2D_RAYCAST_VS_LINE:	return new GameModeRaycastVsLine2D();
		case GAMEMODE_2D_RAYCAST_VS_AABB2:	return new GameModeRaycastVsAABB2D();
		case GAMEMODE_2D_PACHINKO_MACHINE:	return new GameModePachinkoMachine();
		case GAMEMODE_2D_BEZIER_CURVES:		return new GameModeBezierCurves();
		case GAMEMODE_2D_SPIN_AND_FRICTION:	return new GameModeSpinAndFriction();
		case GAMEMODE_3D_RAYCAST_VS_AABB3D:	return new GameModeRaycastVsAABB3D();
		case GAMEMODE_2D_RAYCAST_VS_OBB2D:	return new GameModeRaycastVsOBB2D();
		case GAMEMODE_3D_RAYCAST_VS_OBB3D:	return new GameModeRaycastVsOBB3D();
		case GAMEMODE_2D_CONVEX_SCENE:		return new GameModeConvexScene2D();

		default:
		{			
			ERROR_AND_DIE( Stringf( "ERROR: Unknown GameMode #%i", type ) );
		}
	}
}
