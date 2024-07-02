#include "Game/GameMode_Animations.hpp"
#include "Game/GameMode_Spider_3D.hpp"
#include "Game/GameMode_Quadruped_3D.hpp"
#include "Game/GameMode_2BoneIK_3D.hpp"
#include "Game/GameMode_FABRIK_3D.hpp"
#include "Game/GameMode_FABRIK_FwdOnly_3D.hpp"
#include "Game/GameMode_EulerTest.hpp"
#include "Game/GameMode_RobotArm_3D.hpp"
#include "Game/GameMode_CCD_3D.hpp"
#include "Game/GameModeBase.hpp"
#include "Game/GameMode2D.hpp"
#include "Game/GameMode3D.hpp"
#include "Game/App.hpp"

#include "Engine/Audio/AudioSystem.hpp"
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
		case GAMEMODE_2D:						return new GameMode2D();
		case GAMEMODE_3D:						return new GameMode3D();
		case TWO_BONE_IK_3D:					return new GameMode_2BoneIK_3D();
		case CCD_3D:							return new GameMode_CCD_3D();
		case FABRIK_3D:							return new GameMode_FABRIK_3D();
		case QUADRUPED_3D:						return new GameMode_Quadruped_3D();
		case SPIDER_3D:							return new GameMode_Spider_3D();
		case EULER_FROM_VECTOR_VISUAL_TEST_3D:	return new GameMode_EulerTest();
		case ROBOT_ARM_3D:						return new GameMode_RobotArm_3D();
		case FABRIK_FWD_ONLY_3D:				return new GameMode_FABRIK_FwdOnly_3D();
		case ANIMATIONS:						return new GameMode_Animations();
		
		default:
		{
			ERROR_AND_DIE( Stringf( "ERROR: Unknown GameMode #%i", type ) );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeBase::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if ( g_theInput->WasKeyJustPressed( 'P' ) || g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::BUTTON_START ) )
	{
		SoundID testSound = g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
		g_theAudio->StartSound( testSound );			// Comment out this line of code to remove pause sound playing

		g_theApp->m_gameClock.TogglePause();
	}

	// Slow-Mo functionality
	if ( g_theInput->IsKeyDown( 'T' ) )
	{
		g_theApp->m_gameClock.SetTimeScale( 0.1f );
	}
	// Fast-Mo functionality
	else if ( g_theInput->IsKeyDown( 'G' ) )
	{
		g_theApp->m_gameClock.SetTimeScale( 2.0f );
	}
	else
	{
		g_theApp->m_gameClock.SetTimeScale( 1.0f );
	}


	// Step one frame
	if ( g_theInput->WasKeyJustPressed( 'O' ) )
	{
//		g_theApp->m_gameClock.StepSingleFrame();
	}	
}
