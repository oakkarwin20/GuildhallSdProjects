#pragma once

#include "Game/Controller.hpp"

//----------------------------------------------------------------------------------------------------------------------
class AIController : public Controller
{
public: 
	AIController();
	virtual ~AIController();

	void DamagedBy( Actor const* actor );
	void Update( float deltaSeconds );
	
public:
	ActorUID	m_targetUID;
	bool		m_closeEnough = false;
};