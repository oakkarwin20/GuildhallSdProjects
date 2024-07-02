#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Football3D.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Player3D;
class Texture;

//----------------------------------------------------------------------------------------------------------------------
class GameModeFifaTest3D : public GameModeBase
{
public:
	GameModeFifaTest3D();
	virtual ~GameModeFifaTest3D();
	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Shutdown();

	// Camera and Render Functions
	void UpdateCameraInput();
	void UpdateFifaTest3DCamera();
	void RenderWorldObjects()	const;
	void RenderUIObjects()		const;

	// Game input and debug functions
	void UpdatePauseQuitAndSlowMo();
	void PreCalculateBallTrajectory();

	// Player Functions
	void UpdatePlayer( float deltaSeconds );
	void PossessNextActor();

	// Collision
	void UpdatePlayerVsPlayerCollision();
	void UpdatePlayerVsBallCollision();

	// Football Functions
	void UpdateFootballPhysics( float deltaSeconds );

	// Debug keys
	void AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const;
	void UpdateDebugKeysInput();
	void PredictBallTrajectory( float deltaSeconds );

	// UI Input for "Poc" (point of contact)
	void UpdateBallPoc();

	// Interception
	void CalculatePlayerInterception( Player3D const& currentPlayer, Football3D const& football );

public:
	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	//----------------------------------------------------------------------------------------------------------------------
	Camera	m_fifaTest3DWorldCamera;
	Camera	m_fifaTest3DUICamera;
	bool	m_isCameraControlled		= false;
	bool	m_isCameraTopDown			= false;
	Vec3	m_cameraPerspectiveViewPos	= Vec3( PITCH_CENTER_X,  12.0f, 15.0f );
	Vec3	m_cameraWideViewPos			= Vec3( PITCH_CENTER_X, -10.0f, 50.0f );
	Vec3	m_cameraTopDownViewPos		= Vec3( PITCH_CENTER_X, PITCH_CENTER_Y, 50.0f );

	// TopDown
	float m_topDownYaw	 = 90.0f;
	float m_topDownPitch = 90.0f;
	float m_topDownRoll	 =  0.0f;

	float	m_cameraHeightZAboveBall	= 15.0f;
	float	m_cameraHeightYSouthOfBall  = 22.0f;

	float	m_perspectiveYawDegrees		= 90.0f;
	float	m_perspectivePitchDegrees	= 35.0f;
	float	m_perspectiveRollDegrees	=  0.0f;

	// Camera movement Variables
	float	m_defaultSpeed	= 2.0f;
	float	m_currentSpeed	= 2.0f;
	float	m_fasterSpeed	= m_defaultSpeed * 10.0f;
	float	m_slowerSpeed	= m_defaultSpeed * 0.25f;
	float	m_elevateSpeed  = 6.0f;
	float	m_turnRate		= 90.0f;

	// Football Variables
	Football3D* m_football		= nullptr;
	Vec3 m_footballDefaultPos	= Vec3::ZERO;

	// Player Variables
	Player3D*				m_player1			= nullptr;
	Player3D*				m_player2			= nullptr;
	std::vector<Player3D*>	m_playerList;
	Vec3					m_player1DefaultPos = Vec3( PITCH_CENTER_X - 20.0f, PITCH_CENTER_Y, 0.0f );
	Vec3					m_player2DefaultPos = Vec3( PITCH_CENTER_X + 30.0f, PITCH_CENTER_Y, 0.0f );

	// Texture Variables
	Texture* m_testTexture		= nullptr;
	Texture* m_footballTexture	= nullptr;
	Texture* m_fieldTexture		= nullptr;

	// Time step variables
	float m_physicsUpdateDebt		= 0.0f;
	float m_physicsFixedTimeStep	= 0.005f;
	bool  m_isVariableTimeStep		= true;

	// UI variables 
	float m_discPocRadius_UI		= 1.0f;
	Vec2  m_discPocCenterPos_UI		= Vec2( 180.0f, 80.0f );

	float m_discBackgroundRadius_UI		= 10.0f;
//	Vec2  m_discBackgroundCenterPos_UI	= Vec2( SCREEN_SIZE_X - ( m_discBackgroundRadius_UI * 2.0f ), SCREEN_SIZE_Y - ( m_discBackgroundRadius_UI * 2.0f ) );
	Vec2  m_discBackgroundCenterPos_UI	= Vec2( 180.0f, 80.0f );
};
