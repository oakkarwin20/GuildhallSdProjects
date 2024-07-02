#include "Game/Controller.hpp"
#include "Game/Map.hpp"

//----------------------------------------------------------------------------------------------------------------------
Controller::Controller()
{
}

//----------------------------------------------------------------------------------------------------------------------
Controller::~Controller()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Controller::Possess( ActorUID const& actorToPossessUID )
{
	Actor* actor = m_currentMap->GetActorByUID( actorToPossessUID );
	if ( actor )
	{
		m_currentlyPossessedActorUID = actorToPossessUID;
		m_currentMap				 = actor->m_currentMap;
	}
	else
	{

	}
}

//----------------------------------------------------------------------------------------------------------------------
void Controller::UnPossess( ActorUID const& actorToUnpossessUID )
{
	// When player possesses an actor that already has AI, tell the AI to unpossess the current actor
	// When the player unpossesses the actor and leaves, tell the AI to re-possess the current actor
	Actor* actor = m_currentMap->GetActorByUID( actorToUnpossessUID );
	if ( actor )
	{
		m_currentlyPossessedActorUID.m_saltAndIndexData = ActorUID::INVALID;
		m_currentMap = nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------------
Actor* Controller::GetActor()
{
	if ( m_currentlyPossessedActorUID.m_saltAndIndexData == ActorUID::INVALID )
	{
		return nullptr;
	}
	
	unsigned int index = m_currentlyPossessedActorUID.GetIndex();
	return m_currentMap->m_actorList[index];
}