#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Texture;

//----------------------------------------------------------------------------------------------------------------------
class GameMode3D : public GameModeBase
{
public:
	GameMode3D();
	virtual ~GameMode3D();
	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Shutdown();

	// Camera and Render Functions
	void UpdateCameraInput();
	void UpdateGameMode3DCamera();
	void RenderWorldObjects()	const;
	void RenderUIObjects()		const;

	// Debug keys
	void AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const;

public:
	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	//----------------------------------------------------------------------------------------------------------------------
	Camera	m_gameMode3DWorldCamera;
	Camera	m_gameMode3DUICamera;

	// Camera movement Variables
	float	m_defaultSpeed	= 50.0f;
	float	m_currentSpeed	= 2.0f;
	float	m_fasterSpeed	= m_defaultSpeed * 10.0f;
	float	m_slowerSpeed	= m_defaultSpeed * 0.25f;
	float	m_elevateSpeed  = 6.0f;
	float	m_turnRate		= 90.0f;
};
