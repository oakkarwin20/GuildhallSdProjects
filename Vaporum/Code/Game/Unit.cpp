#include "Game/Unit.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Math/FloatRange.hpp"

//----------------------------------------------------------------------------------------------------------------------
Unit::Unit()
{
}


//----------------------------------------------------------------------------------------------------------------------
Unit::~Unit()
{
}


//----------------------------------------------------------------------------------------------------------------------
void Unit::Update()
{
    // Check if isDead
    if ( m_currentHealth <= 0 )
    {
        m_currentUnitState = UnitState::IS_DEAD;
    }
}


//----------------------------------------------------------------------------------------------------------------------
bool Unit::IsWithinAttackRangeOfRefUnit( Unit const* refUnit )
{
    //----------------------------------------------------------------------------------------------------------------------
    // 1. Get taxicab distance from currentSelectedUnit to "this" unit
    // 2. Check if taxiCabDist is within currentSelectUnit's attack range
    //      True:  return true
    //      False: return false
    //----------------------------------------------------------------------------------------------------------------------
    if ( m_currentUnitState == UnitState::IS_DEAD )
    {
        return false;
    }

    // Get taxiCabDistance
    float taxiCabDist   = 0.0f;
    float a             = fabsf( float( refUnit->m_currentTileCoord.x - m_currentTileCoord.x ) );
    float b             = fabsf( float( refUnit->m_currentTileCoord.x + refUnit->m_currentTileCoord.y - m_currentTileCoord.x - m_currentTileCoord.y ) );
    float c             = fabsf( float( refUnit->m_currentTileCoord.y - m_currentTileCoord.y ) );
    taxiCabDist         = ( a + b + c ) / 2.0f;
    // Check if taxiCabDist from "this" unit to ref unit is within refUnit's attackRange
    FloatRange attackRange = FloatRange( float( refUnit->m_unitDef->m_groundAttackRangeMin ), float( refUnit->m_unitDef->m_groundAttackRangeMax ) );
    if ( attackRange.IsOnRange( taxiCabDist ) )
    {
        return true;
    }
    return false;
}


//----------------------------------------------------------------------------------------------------------------------
void Unit::TakeDamage( int damageAmount )
{
    int damageDealt  = ( 2 * damageAmount ) / m_unitDef->m_defense;
    m_currentHealth -= damageDealt;  
    if ( m_currentHealth <= 0 )
    {
        m_currentUnitState = UnitState::IS_DEAD;
    }
}


//----------------------------------------------------------------------------------------------------------------------
bool Unit::IsDead()
{
    if ( m_currentHealth <= 0 )
    {
        return true;
    }
    return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool Unit::FinishedMovingThisTurn()
{
    if ( m_currentUnitState == UnitState::FINISHED_MOVING_THIS_TURN )
    {
        return true;
    }
    return false;
}
