#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/CubicBezierCurve3D.hpp"
#include "Engine/Core/Stopwatch.hpp"


//----------------------------------------------------------------------------------------------------------------------
class  Texture;
class  IK_Chain3D;
class  IK_Joint3D;
class  CreatureBase;
class  Quadruped;
struct Block;

//----------------------------------------------------------------------------------------------------------------------
class GameMode_Quadruped_3D : public GameModeBase
{
public:
	GameMode_Quadruped_3D();
	virtual ~GameMode_Quadruped_3D();
	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Shutdown();

	// Camera and Render Functions
	void UpdateDebugKeys();
	void UpdateGameMode3DCamera();
	void RenderWorldObjects()	const;
	void RenderUIObjects()		const;

	// Debug Functions
	void AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const;
	void UpdateTargetInput( float deltaSeconds );

	// Creature Functions
	void RenderCreature(	std::vector<Vertex_PCU>& verts, 
							std::vector<Vertex_PCU>& verts_textured, 
							std::vector<Vertex_PCU>& textVerts, 
							std::vector<Vertex_PCU>& verts_BackFace, 
							std::vector<Vertex_PCU>& verts_chainBody,
							Texture* creatureTexture 
						) const;

	void SetChainConstraints( IK_Chain3D* const currentChain );
	void UpdateCameraAndCreatureInput( float deltaSeconds );
	std::vector<Block*> GetBlockListFromGameMode();

	// Map setup
	void InitFloorAABB3();
	void InitDynamicTerrain();
	void UpdateDynamicTerrain( float deltaSeconds );


public:
	//----------------------------------------------------------------------------------------------------------------------
	// Core Variables
	//----------------------------------------------------------------------------------------------------------------------
	float m_sine				 = 0.0f;
	float m_currentTime			 = 0.0f;
	float m_walkLerpSpeed		 = 4.0f;
	float m_sprintLerpSpeed		 = m_walkLerpSpeed * 4.0f;
	float m_goalWalkLerpSpeed	 = m_walkLerpSpeed;
	float m_currentWalkLerpSpeed = m_goalWalkLerpSpeed;

	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	//----------------------------------------------------------------------------------------------------------------------
	Camera	m_gameMode3DWorldCamera;
	Camera	m_gameMode3DUICamera;
	// Camera movement Variables
	float	m_defaultSpeed	= 30.0f;
	float	m_currentSpeed	= 30.0f;
	float	m_fasterSpeed	= m_defaultSpeed * 4.0f;
	float	m_elevateSpeed  = 6.0f;
	float	m_turnRate		= 90.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Debug variables
	//----------------------------------------------------------------------------------------------------------------------
	float		m_distCamAwayFromPlayer			= 180.0f;
	Vec3		m_debugTargetPos				= Vec3::ZERO;
	float		m_debugTargetAngle_PolarCoords	= 0.0f;
	float		m_debugTargetLength_PolarCoords	= 0.0f;
	bool		m_debugStepOneFrame				= false;
	Vec3		m_debugNoisePos					= Vec3::ZERO;
	IK_Chain3D* m_targetManipulatedChain		= nullptr;
	int			m_indexTargetChain				= 0;

	// Quadruped
	Quadruped* m_quadruped		= nullptr;
	float m_rootDefaultHeightZ	= 6.0f;

	// Map variables
	std::vector<Block*> m_blockList;

	// Lighting Variables
	Vec3	m_sunDirection		= Vec3::ZERO;
	float   m_sunIntensity		= 0.85f;
	float   m_ambientIntensity	= 0.35f;
	float	m_specularIntensity	= 0.75f;
	float	m_specularPower		= 16.0f;


	float m_raylength_Long = 100.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Dynamic Terrain variables
	//----------------------------------------------------------------------------------------------------------------------
	VertexBuffer*				m_vbo			= nullptr;
	IndexBuffer*				m_ibo			= nullptr;
	std::vector<Vertex_PCU>		m_planeVerts;
	std::vector<unsigned int>	m_indexList;
	float						m_noiseTime		= 0.0f;
};
