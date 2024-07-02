#pragma once

#include "Game/ActorUID.hpp"
#include "Game/Actor.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Map;

//----------------------------------------------------------------------------------------------------------------------
class Controller
{
public:
	Controller();
	virtual ~Controller();

	virtual void Possess( ActorUID const& actorToPossessUID );
	virtual void UnPossess( ActorUID const& actorToUnpossessUID );
	Actor* GetActor();

public:
	// Actor UID for current actor
	ActorUID	m_currentlyPossessedActorUID;
	Map*		m_currentMap = nullptr;
};