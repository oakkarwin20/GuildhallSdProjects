#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game.hpp"
#include "Game/Map.hpp"

#include "Engine/Networking/NetSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//----------------------------------------------------------------------------------------------------------------------
Player::Player()
{
}

//----------------------------------------------------------------------------------------------------------------------
Player::Player(Game* game)
	: Entity( game )
{
}

//----------------------------------------------------------------------------------------------------------------------
Player::~Player()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Player::Update( float deltaSeconds )
{	
	m_defaultSpeed = g_theApp->m_cameraPanSpeed;
	m_elevateSpeed = g_theApp->m_cameraElevateSpeed;

	//----------------------------------------------------------------------------------------------------------------------
	UpdatePlayerInput( deltaSeconds );

	//----------------------------------------------------------------------------------------------------------------------
	// Cursor GroundPos
	//----------------------------------------------------------------------------------------------------------------------
	Vec2 cursorPos_screenSpace	= g_theInput->GetCursorClientPosition();
	CalculateWorldMouseRayPos( cursorPos_screenSpace );


	//----------------------------------------------------------------------------------------------------------------------
	// Calculate offset from screenCenterPos to cameraPos with flattened Z
	Vec2  screenBounds				= Vec2( g_theWindow->GetClientDimensions() );
	Vec3  dirWorldCenterPosOnScreen	= m_worldCamera.GetDirCamToScreenCenter_ScreenToWorld( screenBounds );
	Vec3  worldCamFwd				= m_worldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp();
	float rayLengthToScreenCenter	= fabsf( m_worldCamera.m_position.z / worldCamFwd.z );
	m_worldCenterPosOnScreen		= m_worldCamera.m_position + ( dirWorldCenterPosOnScreen * rayLengthToScreenCenter );
	Vec3 offset						= m_worldCenterPosOnScreen - Vec3( m_worldCamera.m_position.x, m_worldCamera.m_position.y, 0.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Clamp camera pos with offsets
	float minX					= m_game->m_currentMap->m_mapDef->m_worldBoundsMin.x - offset.x;
	float minY					= m_game->m_currentMap->m_mapDef->m_worldBoundsMin.y - offset.y;
	float minZ					= g_theApp->m_cameraMinHeight;
	float maxX					= m_game->m_currentMap->m_mapDef->m_worldBoundsMax.x - offset.x;
	float maxY					= m_game->m_currentMap->m_mapDef->m_worldBoundsMax.y - offset.y;
	float maxZ					= m_game->m_currentMap->m_mapDef->m_worldBoundsMax.z;
	m_worldCamera.m_position.x  = GetClamped( m_worldCamera.m_position.x, minX, maxX );
	m_worldCamera.m_position.y  = GetClamped( m_worldCamera.m_position.y, minY, maxY );
	m_worldCamera.m_position.z  = GetClamped( m_worldCamera.m_position.z, minZ, maxZ );
}

//----------------------------------------------------------------------------------------------------------------------
void Player::Render() const
{
	// Render screenCenter
	std::vector<Vertex_PCU> verts;
	AddVertsForSphere3D( verts, m_worldCenterPosOnScreen, 0.1f, 16.0f, 16.0f, Rgba8::CYAN );
	g_theRenderer->DrawVertexArray(	int( verts.size() ), verts.data() );
}


//----------------------------------------------------------------------------------------------------------------------
void Player::CalculateWorldMouseRayPos( Vec2 const& cursorScreenSpace )
{
	Vec2 screenBounds	= Vec2( g_theWindow->GetClientDimensions() );
	Vec3 worldMouseRay	= m_worldCamera.GetDirCamToMouse_ScreenToWorld( cursorScreenSpace, screenBounds );
	float rayLength		= fabsf( m_worldCamera.m_position.z / worldMouseRay.z );
	m_worldMouseRayPos	= m_worldCamera.m_position + ( worldMouseRay * rayLength );
}


//----------------------------------------------------------------------------------------------------------------------
void Player::UpdatePlayerInput( float deltaSeconds )
{
	deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();

	Vec3 iBasis, jBasis, kBasis;
	m_worldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
	iBasis.z = 0.0f;
	jBasis.z = 0.0f;
	iBasis = iBasis.GetNormalized();
	jBasis = jBasis.GetNormalized();
	kBasis = kBasis.GetNormalized();

	// Speed up speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_currentSpeed = m_sprintSpeed;
	}
	// Speed up speed variable
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_currentSpeed = m_defaultSpeed;
	}

	// Slow down speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_CONTROL ) )
	{
		m_currentSpeed = 0.5f;
	}

	// Slow down speed variable
	if ( g_theInput->WasKeyJustReleased( KEYCODE_CONTROL ) )
	{
		m_currentSpeed = m_defaultSpeed;
	}
	
	// Forward
	if ( g_theInput->IsKeyDown('W') )
	{
		m_worldCamera.m_position += ( m_currentSpeed * iBasis * deltaSeconds );
	}

	// Back
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_worldCamera.m_position -= ( m_currentSpeed * iBasis * deltaSeconds );
	}

	// Left
	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		m_worldCamera.m_position += ( m_currentSpeed * jBasis * deltaSeconds);
	}

	// Right
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_worldCamera.m_position -= ( m_currentSpeed * jBasis * deltaSeconds );
	}

	// Elevate
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
		m_worldCamera.m_position.z += ( m_currentSpeed * deltaSeconds );
	}

	// De-Elevate
	if ( g_theInput->IsKeyDown( 'Q' ) )
	{
		m_worldCamera.m_position.z -= ( m_currentSpeed * deltaSeconds );
	}
//	m_worldCamera.m_orientation.m_yawDegrees   = GetClamped(  m_worldCamera.m_orientation.m_yawDegrees, 90.0f, 90.0f );
//	m_worldCamera.m_orientation.m_pitchDegrees = GetClamped( m_worldCamera.m_orientation.m_pitchDegrees, 60, 60.0f );

	// Reset to world center
	if ( g_theInput->WasKeyJustPressed( 'O' ) )
	{
		// Reset Position
		m_position	= Vec3( 0.0f, 0.0f, 5.0f );

		iBasis		= Vec3( 1.0f, 0.0f, 0.0f );
		jBasis		= Vec3( 0.0f, 1.0f, 0.0f );
		kBasis		= Vec3( 0.0f, 0.0f, 1.0f );

		// Reset Orientation
		m_worldCamera.m_orientation.m_yawDegrees	= 90.0f;
		m_worldCamera.m_orientation.m_pitchDegrees	= 60.0f;
		m_worldCamera.m_orientation.m_rollDegrees	=  0.0f;
	}
}


//----------------------------------------------------------------------------------------------------------------------
std::string Player::GetPlayerStateAsString()
{
	std::string currentPlayerState = "Default_Initialized_Value";
	if ( m_currentPlayerState == PlayerState::SELECTING )
	{
		currentPlayerState = "SELECTING";
	}
	else if ( m_currentPlayerState == PlayerState::SELECTED )
	{
		currentPlayerState = "SELECTED";
	}
	if ( m_currentPlayerState == PlayerState::MOVING )
	{
		currentPlayerState = "MOVING";
	}
	if ( m_currentPlayerState == PlayerState::CONFIRM_MOVE )
	{
		currentPlayerState = "CONFIRM_MOVE";
	}
	if ( m_currentPlayerState == PlayerState::TRY_ATTACK )
	{
		currentPlayerState = "TRY_ATTACK";
	}
	if ( m_currentPlayerState == PlayerState::CONFIRM_ATTACK )
	{
		currentPlayerState = "CONFIRM_ATTACK";
	}
	if ( m_currentPlayerState == PlayerState::DEFAULT )
	{
		currentPlayerState = "PlayerState::DEFAULT";
	}
	return currentPlayerState;
}


//----------------------------------------------------------------------------------------------------------------------
std::string Player::GetPlayerTurnStateAsString()
{
	std::string currentPlayerTurnState = "Default Initialized Player Turn State";
	if ( m_currentPlayerTurnState == PlayerTurnState::POP_UP )
	{
		currentPlayerTurnState = "POP_UP";
	}
	else if ( m_currentPlayerTurnState == PlayerTurnState::END_TURN )
	{
		currentPlayerTurnState = "END_TURN";
	}
	else if ( m_currentPlayerTurnState == PlayerTurnState::CONFIRM_END_TURN )
	{
		currentPlayerTurnState = "CONFIRM_END_TURN";
	}
	else if ( m_currentPlayerTurnState == PlayerTurnState::WAITING_FOR_TURN )
	{
		currentPlayerTurnState = "WAITING_FOR_TURN";
	}
	return currentPlayerTurnState;
}


//----------------------------------------------------------------------------------------------------------------------
void Player::ChangePlayerStates()
{
	if ( m_requestedPlayerState != m_currentPlayerState )
	{
		m_currentPlayerState = m_requestedPlayerState;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Player::TogglePlayerTurnStates()
{
	if ( m_currentPlayerTurnState == PlayerTurnState::END_TURN )
	{
		m_requestedPlayerTurnState = PlayerTurnState::CONFIRM_END_TURN;
	}
	else if ( m_currentPlayerTurnState == PlayerTurnState::CONFIRM_END_TURN )
	{
		m_requestedPlayerTurnState = PlayerTurnState::END_TURN;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Player::ChangePlayerTurnStates()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Check if currentState needs to be changed
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_requestedPlayerTurnState != m_currentPlayerTurnState )
	{
		ExitPlayerTurnState(  m_currentPlayerTurnState );
		m_currentPlayerTurnState = m_requestedPlayerTurnState;
		EnterPlayerTurnState( m_currentPlayerTurnState );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Player::ExitPlayerTurnState( PlayerTurnState state )
{
	if ( state == PlayerTurnState::POP_UP )
	{
		// Loop through all units and reset unitStates
		for ( int i = 0; i < m_game->m_player1->m_unitList.size(); i++ )
		{
			Unit& currentUnit = m_game->m_player1->m_unitList[i];
			if ( currentUnit.m_currentUnitState == UnitState::IS_DEAD )
			{
				continue;
			}
			else
			{
				currentUnit.m_currentUnitState = UnitState::READY;
			}
		}
		for ( int i = 0; i < m_game->m_player2->m_unitList.size(); i++ )
		{
			Unit& currentUnit = m_game->m_player2->m_unitList[i];
			if ( currentUnit.m_currentUnitState == UnitState::IS_DEAD )
			{
				continue;
			}
			else
			{
				currentUnit.m_currentUnitState = UnitState::READY;
			}
		}
	}
	if ( state == PlayerTurnState::END_TURN )
	{
	}
	else if ( state == PlayerTurnState::CONFIRM_END_TURN )
	{
		m_game->m_currentSelectedUnit = nullptr;
		m_game->m_currentPlayer->m_requestedPlayerState = PlayerState::SELECTING;
		m_game->RemoveDistFieldAndPathRendering();

		// Loop through all units and reset unitStates
		for ( int i = 0; i < m_game->m_player1->m_unitList.size(); i++ )
		{
			Unit& currentUnit = m_game->m_player1->m_unitList[i];
			if ( currentUnit.m_currentUnitState == UnitState::IS_DEAD )
			{
				continue;
			}
			else
			{
				if ( currentUnit.m_currentUnitState != FINISHED_MOVING_THIS_TURN && currentUnit.m_currentUnitState != IS_DEAD )
				{
					currentUnit.m_currentUnitState = UnitState::READY;
				}
			}
		}
		for ( int i = 0; i < m_game->m_player2->m_unitList.size(); i++ )
		{
			Unit& currentUnit = m_game->m_player2->m_unitList[i];
			if ( currentUnit.m_currentUnitState == UnitState::IS_DEAD )
			{
				continue;
			}
			else
			{
				if ( currentUnit.m_currentUnitState != FINISHED_MOVING_THIS_TURN && currentUnit.m_currentUnitState != IS_DEAD )
				{
					currentUnit.m_currentUnitState = UnitState::READY;
				}
			}
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Player::EnterPlayerTurnState( PlayerTurnState state )
{
	if ( state == PlayerTurnState::POP_UP )
	{
		// Loop through all units and reset unitStates
		for ( int i = 0; i < m_game->m_player1->m_unitList.size(); i++ )
		{
			Unit& currentUnit = m_game->m_player1->m_unitList[ i ];
			if ( currentUnit.m_currentUnitState == UnitState::IS_DEAD )
			{
				continue;
			}
			else
			{
				currentUnit.m_currentUnitState = UnitState::READY;
			}
		}
		for ( int i = 0; i < m_game->m_player2->m_unitList.size(); i++ )
		{
			Unit& currentUnit = m_game->m_player2->m_unitList[ i ];
			if ( currentUnit.m_currentUnitState == UnitState::IS_DEAD )
			{
				continue;
			}
			else
			{
				currentUnit.m_currentUnitState = UnitState::READY;
			}
		}
	}
	if ( state == PlayerTurnState::END_TURN )
	{
	}
	else if ( state == PlayerTurnState::CONFIRM_END_TURN )
	{
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Player::SetNetState()
{
	// Set current player net mode
	if ( g_theNetSystem->m_mode == NetSystem::Mode::CLIENT )
	{
		m_netState = NetState::CLIENT;
	}
	else if ( g_theNetSystem->m_mode == NetSystem::Mode::SERVER )
	{
		m_netState = NetState::SERVER;
	}
	else
	{
		// This means we're playing "offline" or "locally"
		m_netState = NetState::NONE;
	}
}


//----------------------------------------------------------------------------------------------------------------------
std::string Player::GetNetStateEnumAsString()
{
	std::string currentNetState = "INVALID";
	if ( m_netState == NetState::CLIENT )
	{
		currentNetState = "Client";
	}
	else if ( m_netState == NetState::SERVER )
	{
		currentNetState = "Server";
	}
	else
	{
		currentNetState = "None";
	}
	return currentNetState;
}
