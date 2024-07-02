#pragma once

#include "Game/Unit.hpp"

#include "Game/Entity.hpp"
#include "Engine/Renderer/Camera.hpp"


//----------------------------------------------------------------------------------------------------------------------
enum class PlayerState
{
	DEFAULT,
	SELECTING,				// No unit selected, nothing has been clicked
	SELECTED,				// A unit been clicked, but the highlights can be switched to other units 
	MOVING,					// Moving the selected unit. A new location for selected unit has been chosen but not confirmed, this will preview the current unit at that location 
	CONFIRM_MOVE,			// Confirming the move for the current unit								 
	TRY_ATTACK,				// If no enemy is within attack range, the player turn ends here
	CONFIRM_ATTACK,			// If an enemy is within attack range, confirm the attack move
};


//----------------------------------------------------------------------------------------------------------------------
enum class PlayerTurnState
{
	DEFAULT,
	POP_UP,					// The state when this player's turn first starts and the pop up appears
	END_TURN,				// Pressing enter the 1st time to transition into "end turn"
	CONFIRM_END_TURN,		// Pressing enter the 2nd time to finish this player's turn
	WAITING_FOR_TURN,		// Not doing anything, waiting for the other playing to finish their turn
};


//----------------------------------------------------------------------------------------------------------------------
enum class NetState
{
	NONE,
	CLIENT,
	SERVER,
};


//----------------------------------------------------------------------------------------------------------------------
class Player : public Entity
{
public:
	Player();
	Player( Game* game );
	~Player();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;

	void		CalculateWorldMouseRayPos( Vec2 const& cursorScreenSpace );
	void		UpdatePlayerInput( float deltaSeconds );
	std::string	GetPlayerStateAsString();
	std::string	GetPlayerTurnStateAsString();

	// Player State Functions
	void ChangePlayerStates();

	// Player Turn State Functions
	void TogglePlayerTurnStates();
	void ChangePlayerTurnStates();
	void ExitPlayerTurnState(  PlayerTurnState state );
	void EnterPlayerTurnState( PlayerTurnState state );

	// Set NetState
	void SetNetState();
	std::string GetNetStateEnumAsString();

public:
	int		m_playerID		= 0;
	float	m_defaultSpeed	= 10.0f;
	float	m_currentSpeed	= 10.0f;
	float	m_elevateSpeed  = 10.0f;
	float	m_sprintSpeed	= m_defaultSpeed * 2.0f;
	float	m_turnRate		= 90.0f;
	Camera	m_worldCamera;

	Vec3 m_worldMouseRayPos			= Vec3::ZERO;
	Vec3 m_worldCenterPosOnScreen	= Vec3::ZERO;

	// Units
	std::vector<Unit> m_unitList;

	//----------------------------------------------------------------------------------------------------------------------
	// Player States
	//----------------------------------------------------------------------------------------------------------------------
	PlayerState		m_currentPlayerState		= PlayerState::SELECTING;
	PlayerState		m_requestedPlayerState		= PlayerState::SELECTING;
	PlayerTurnState m_currentPlayerTurnState	= PlayerTurnState::POP_UP;
	PlayerTurnState m_requestedPlayerTurnState	= PlayerTurnState::POP_UP;

	//----------------------------------------------------------------------------------------------------------------------
	// Network state variables
	//----------------------------------------------------------------------------------------------------------------------
	NetState m_netState = NetState::NONE;

//	bool m_isPlayerReady = false;
};