#pragma once

#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/EventSystem.hpp"


//----------------------------------------------------------------------------------------------------------------------
constexpr int NUM_SINGLE_TRI_VERTS = 3;


//----------------------------------------------------------------------------------------------------------------------
class Material;


//----------------------------------------------------------------------------------------------------------------------
enum ObjectToRender
{
	NONE,
	CUBE_BRICKS,
	CUBE_GRASS,
	SPHERE_BRICKS,
	SPHERE_GRASS,
	MODEL,
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
	void RenderNormalLines( std::vector<Vertex_PCUTBN> const& verts ) const;


	float randNumX = 50.f;
	float randNumY = 50.f;

	static bool Command_LoadModel(EventArgs& args);

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

	// Model Viewer
	void LoadModel();

	bool m_isPaused			= false;
	bool m_isSlowMo			= false;

	Camera						m_attractCamera;
	Camera						m_screenCamera;
	Vertex_PCU					m_localVerts[NUM_SINGLE_TRI_VERTS];
	std::vector<Entity*>		m_entityList;

public:
	Player*					m_player;
	Texture*				m_testTexture		= nullptr;
	Texture*				m_grassTexture		= nullptr;
	Texture*				m_brickTexture		= nullptr;
	bool					m_AttractModeIsOn	= false;
	Clock					m_clock;
	std::string				m_playerPosText; 
	std::string				m_timeText; 

	// Materials
	Material*					m_grassMaterial			= nullptr;
	Material*					m_brickMaterial			= nullptr;
	Material*					m_tutorialBoxMaterial	= nullptr;

	bool						m_renderTBN			= false;
	ObjectToRender				m_objectToRender	= NONE;
	Prop*						m_prop_Sphere		= nullptr;
	Prop*						m_prop_Cube			= nullptr;
	// Arrow
	std::vector<Vertex_PCU>		m_arrowVerts;
	VertexBuffer*				m_vboArrow			= nullptr;
	EulerAngles					m_arrowYPR			= EulerAngles();
	float						m_sunIntensity		= 1.0f;
	float						m_specularIntensity	= 1.0f;
	float						m_ambientIntensity	= 0.0f;
	float						m_specularPower		= 32.0f;
	bool						m_isNormalsTexture	= true;
	bool						m_isSpecTexture		= true;
};