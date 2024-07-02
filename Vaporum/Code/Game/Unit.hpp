#pragma once

#include "Game/UnitDefinition.hpp"


//----------------------------------------------------------------------------------------------------------------------
enum UnitState
{
	DEFAULT,
	READY, 							// Unit is ready to be chosen, but is currently NOT.
	SELECTED,						// Unit is selected by the player as the unit to move
	MOVED,							// Unit has been moved to a specific location.
	WITHIN_ENEMY_ATTACK_RANGE,		// Is within enemy range but not chosen as the attack target.					// Render a hex border dark red
	TARGETED_BY_ENEMY,				// Is chosen as the attack target by the enemy.									// Render a hex border bright red
	FINISHED_MOVING_THIS_TURN,		// Has finished moving in this turn, but the player turn might not be over		// Render this unit with a dark gray tint
	IS_DEAD,
};


//----------------------------------------------------------------------------------------------------------------------
class Unit
{
public:
	Unit();
	~Unit();

	void Update();

	bool IsWithinAttackRangeOfRefUnit( Unit const* referenceUnit );
	void TakeDamage( int damageAmount );
	bool IsDead();
	bool FinishedMovingThisTurn();

public:
	UnitDefinition const*	m_unitDef					= nullptr;
	IntVec2					m_currentTileCoord			= IntVec2::ZERO;
	IntVec2					m_previousTileCoord			= IntVec2::ZERO;
	UnitState				m_currentUnitState			= UnitState::READY;
	int						m_currentHealth				= 0;
};