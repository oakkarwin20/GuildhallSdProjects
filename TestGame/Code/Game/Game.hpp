#pragma once

#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Player.hpp"

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/JobSystem.hpp"

//----------------------------------------------------------------------------------------------------------------------
constexpr int NUM_SINGLE_TRI_VERTS = 3;

//----------------------------------------------------------------------------------------------------------------------
class TestJob : public Job
{
public:
	TestJob( int x, int y, int sleepMS )
		: m_tileCoords( x, y )
		, m_sleepMS( sleepMS )
	{}

	virtual void Execute() override;

	IntVec2						m_tileCoords	= IntVec2::ZERO;
	int							m_sleepMS		= 0;
};

//----------------------------------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void StartUp();
	void Shutdown();
	void Update();
	void Render() const;


	float randNumX = 50.f;
	float randNumY = 50.f;

private:
	void UpdatePauseQuitAndSlowMo();
	void UpdateEntities(float deltaSeconds);
	void UpdateCameras();
	void RenderEntities() const;
	void RenderUI() const;

	// DebugRenderSystem aka DRS
	void Update_DRS_Input();
	void Render_DRS_WorldBasisText();
	void Render_DRS_WorldBasis();
	void Render_DRS_UI_Text() const;

	// AttractMode
	void UpdateReturnToAttractMode();
	void RenderAttractMode() const;
	void AttractModeInput();
	void InitializeAttractModeVerts();

	//----------------------------------------------------------------------------------------------------------------------
	// Multi-Threading
	void DrawJobStatusBlocks() const;
	void CreateTestJobs();
	void UpdateTestJobs();

	AABB2 GetTestJobTileBounds( IntVec2 tileCoords ) const;
	Rgba8 GetTestJobTileColor( JobStatus jobStatus ) const;

public:
	std::vector<TestJob*>	m_allTestJobs;
//	std::vector<IntVec2>	m_retrievedJobCoords;
//	std::vector<Chunk*>		m_chunkGenerationInProgressList;

	bool m_isPaused			= false;
	bool m_isSlowMo			= false;

	Camera						m_attractCamera;
	Camera						m_screenCamera;
	Player*						m_player;
	Vertex_PCU					m_localVerts[NUM_SINGLE_TRI_VERTS];
	std::vector<Entity*>		m_entityList;
	Texture*					m_testTexture		= nullptr;

public:
	bool m_AttractModeIsOn	= true;
	Clock					m_clock;
	std::string				m_playerPosText; 
	std::string				m_timeText; 
};