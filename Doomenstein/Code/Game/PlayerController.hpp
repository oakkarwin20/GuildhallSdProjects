#pragma once

#include "Game/Controller.hpp"

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class Map;

//----------------------------------------------------------------------------------------------------------------------
class PlayerController : public Controller
{
public:
	PlayerController();
	virtual ~PlayerController();
	
	void Update( float deltaSeconds );
	void Render() const;

	void RenderUI() const;

	virtual void Possess( ActorUID const& actorToPossessUID ) override;

	// Player Functions
	void UpdatePlayerInput( float deltaSeconds );
	void UpdatePossessedActorInput( float deltaSeconds );
	void UpdateCamera();
	void PossessNextActor();

public:
	// Player variables
	float						m_turnRate			=  0.075f;
	float						m_defaultSpeed		=  1.0f;
	float						m_currentSpeed		=  -1.0f;
	float						m_doublespeed		= m_currentSpeed * 15.0f;
	float						m_halfspeed			= m_currentSpeed * 0.5f;
	float						m_elevateSpeed		=  6.0f;
	float						m_rollSpeed			= 600.0f;
	Camera						m_worldCamera;
	float						m_previousFOV		= 0.0f;
	Actor*						m_currentActor		= nullptr;


	//----------------------------------------------------------------------------------------------------------------------
	Vec3						m_position;
	Vec3						m_velocity;
	EulerAngles					m_orientation;
	std::vector<Vertex_PCU>		m_vertexes;					
	Rgba8						m_color			= Rgba8::WHITE;

	//----------------------------------------------------------------------------------------------------------------------
	// Raycast variables
	float m_rayLength		= 10.0f;
	Vec3  m_actorForward	= Vec3( -1.0f, -1.0f, -1.0f );
	Vec3  m_actorLeft		= Vec3( -1.0f, -1.0f, -1.0f );
	Vec3  m_actorUp			= Vec3( -1.0f, -1.0f, -1.0f );

	Vec3  m_playerControllerforward	= Vec3( -1.0f, -1.0f, -1.0f );
	Vec3  m_playerControllerleft	= Vec3( -1.0f, -1.0f, -1.0f );
	Vec3  m_playerControllerup		= Vec3( -1.0f, -1.0f, -1.0f );

	Vec3  m_rayEnd		= m_position + (m_actorForward * m_rayLength);
	float m_radius		= 0.01f; 
	float m_duration	= 10.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// variable for controlMode: Player
	bool m_playerIsControllable = true; 
	bool m_isFreeFlyOn			= false;

	Actor* m_actorIHit			= nullptr;
	unsigned int m_index		= 0;

	float m_marineFOV			= 0.0f;

	int m_deathCounter			= 0;

	// Drugs variable
	bool m_drugEffectIsActive	= false;
	float m_drugsCameraFOV		= -1.0f;
	float m_maxCamRollOnDrugs	= 30.0f;
	bool m_startDrugsStopwatch  = true;
	Stopwatch m_drugsStopwatch	= Stopwatch();

	// Audio and Player Index
	int					m_playerIndex		= 0;
	SoundPlaybackID		m_drunkWobbleSPBID	= SoundID( -1 );
	SoundID				m_drunkWobbleSID	= SoundID( -1 );
};