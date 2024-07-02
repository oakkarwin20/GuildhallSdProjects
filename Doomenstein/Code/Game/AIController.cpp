#include "Game/AIController.hpp"
#include "Game/Map.hpp"
#include "Game/ActorDefinition.hpp"

#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
AIController::AIController()
{
}

//----------------------------------------------------------------------------------------------------------------------
AIController::~AIController()
{
}

//----------------------------------------------------------------------------------------------------------------------
void AIController::DamagedBy( Actor const* actor )
{
	// Tells the AI, it was damaged and which actor damaged it
 	m_targetUID = actor->m_currentActorUID; 
}

//----------------------------------------------------------------------------------------------------------------------
void AIController::Update( float deltaSeconds )
{
	Actor* possessedActor = GetActor();

	// Check if AI does NOT currently have a target or is NOT possessing an actor
	if ( possessedActor == nullptr )
	{
		return;
	}

	Actor* closestEnemy = m_currentMap->GetClosestVisibleEnemy( possessedActor );
	if ( closestEnemy != nullptr )
	{
		// Assign UID
		m_targetUID = closestEnemy->m_currentActorUID;
	}

	if ( !m_targetUID.IsValid() )
	{
		return;
	}

	// Try to find the closest enemy  // If target is found, move towards target
	if ( m_targetUID.m_saltAndIndexData == ActorUID::INVALID )
	{
		return;
	}
	

	//----------------------------------------------------------------------------------------------------------------------
	// If we do currently have a target
	Actor* targetActor = m_currentMap->GetActorByUID( m_targetUID );
	if ( targetActor == nullptr )
	{
		return;
	}

	if ( possessedActor->m_currentWeapon->m_weaponDef->m_name == "DemonMelee" )		// If the possessedActor is a Demon
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Check if distance to target is < specified distance
		// Calculate distance
		Vec2 possessedActorPosV2 = Vec2( possessedActor->m_position.x, possessedActor->m_position.y );
		Vec2 targetActorPosV2	 = Vec2(	targetActor->m_position.x, targetActor->m_position.y );
		float distanceToTarget	 = GetDistance2D( possessedActorPosV2, targetActorPosV2 );	

		// If target is within attack range, attack
		if ( distanceToTarget <= possessedActor->m_currentWeapon->m_weaponDef->m_meleeRange - 0.1f )
		{
			// Attack()
			possessedActor->Attack();

			// Mark close enough
	//		m_closeEnough = true;
			// Apply impulse
		}
		else
		{
			// Move in actor's forward direction
			possessedActor->m_orientation.GetAsVectors_XFwd_YLeft_ZUp( possessedActor->m_forward, possessedActor->m_left, possessedActor->m_up );
			possessedActor->MoveInDirection( possessedActor->m_forward, possessedActor->m_actorDef->m_runSpeed );
		}
	}
	else if ( possessedActor->m_currentWeapon->m_weaponDef->m_name == "DemonPoop" )	// If the possessedActor is a rangedDemon
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Check if demon should chase or run away from the player
		
		// Calculate distance
		Vec2 possessedActorPosV2 = Vec2( possessedActor->m_position.x, possessedActor->m_position.y );
		Vec2 targetActorPosV2	 = Vec2( targetActor->m_position.x, targetActor->m_position.y );
		float distanceToTarget	 = GetDistance2D( possessedActorPosV2, targetActorPosV2 );

		// If target is too close, run away
		if ( distanceToTarget <= possessedActor->m_currentWeapon->m_weaponDef->m_projectileAttackRange )
		{
			// Move in actor's forward direction
			possessedActor->m_orientation.GetAsVectors_XFwd_YLeft_ZUp( possessedActor->m_forward, possessedActor->m_left, possessedActor->m_up );
			possessedActor->MoveInDirection( -possessedActor->m_forward, possessedActor->m_actorDef->m_runSpeed );
			
			// Attack()		// Leave Demon Poop mines while running away
			possessedActor->Attack();
		}
		// Chase if target too far away
		else if ( distanceToTarget >= possessedActor->m_actorDef->m_chasePlayerRange )
		{
			// Move in actor's forward direction
			possessedActor->m_orientation.GetAsVectors_XFwd_YLeft_ZUp( possessedActor->m_forward, possessedActor->m_left, possessedActor->m_up );
			possessedActor->MoveInDirection( possessedActor->m_forward, possessedActor->m_actorDef->m_runSpeed );
		}
	}

	// Turn towards actor 	
	Vec3 dirFromMyselfToTarget	= (targetActor->m_position - possessedActor->m_position).GetNormalized();		// Calculate direction from actor to target	
	float deltaDegrees			= possessedActor->m_actorDef->m_turnSpeed * deltaSeconds;
	possessedActor->TurnInDirection( dirFromMyselfToTarget, deltaDegrees );
}
