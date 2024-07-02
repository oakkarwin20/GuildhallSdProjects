#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"

//----------------------------------------------------------------------------------------------------------------------
constexpr int NUM_SINGLE_TRI_VERTS = 3;

//----------------------------------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void StartUp();
	void Shutdown();
	void Update( float deltaSeconds );
	void Render() const;

	float randNumX = 50.f;
	float randNumY = 50.f;

private:
	void UpdatePauseQuitAndSlowMo( float& deltaSeconds );
	void UpdateReturnToAttractMode();
	void UpdateEntities(float deltaSeconds);
	void AttractModeInput();
	void RenderEntities() const;
	void UpdateCameras();
	void RenderAttractMode() const;
	void InitializeAttractModeVerts();

	bool m_isPaused			= false;
	bool m_isSlowMo			= false;
	bool m_AttractModeIsOn	= true;

	Camera m_worldCamera;
	Camera m_screenCamera;
	Camera m_attractCamera;

	Vertex_PCU	m_localVerts[NUM_SINGLE_TRI_VERTS];

	Clock		m_clock;
};