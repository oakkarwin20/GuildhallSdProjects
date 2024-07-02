#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Texture;
class IK_Chain3D;
class IK_Joint3D;
class CreatureBase;
class FoodManager;


//----------------------------------------------------------------------------------------------------------------------
class GameMode_RobotArm_3D : public GameModeBase
{
public:
	GameMode_RobotArm_3D();
	virtual ~GameMode_RobotArm_3D();
	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Shutdown();

	// Camera and Render Functions
	void UpdateDebugKeys();
	void UpdateCameraInput( float deltaSeconds );
	void UpdateGameMode3DCamera();
	void RenderWorldObjects()	const;
	void RenderUIObjects()		const;

	// Debug Functions
	void AddVertsForCompass( std::vector<Vertex_PCU>&compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const;
	void UpdateTargetInput( float deltaSeconds);

	//----------------------------------------------------------------------------------------------------------------------
	// CCD 
	//----------------------------------------------------------------------------------------------------------------------
	void UpdateCreature( float deltaSeconds );
	void InitializeIK_ChainCCD();
	void SetIK_ChainConstraints();
	void RobotCustomUpdate( float deltaSeconds );

public:
	//----------------------------------------------------------------------------------------------------------------------
	// Configurable data
	//----------------------------------------------------------------------------------------------------------------------
	float m_boneLength = 10.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Core Variables
	//----------------------------------------------------------------------------------------------------------------------
	float m_sine						= 0.0f;
	float m_currentTime					= 0.0f;
	float m_walkLerpSpeed				= 4.0f;
	float m_sprintLerpSpeed				= m_walkLerpSpeed * 4.0f;
	float m_goalWalkLerpSpeed			= m_walkLerpSpeed;
	float m_currentWalkLerpSpeed		= m_goalWalkLerpSpeed;

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
	float m_distCamAwayFromPlayer	= 180.0f;
	Vec3  m_debugTargetPos			= Vec3::ZERO;
	bool  m_debugStepOneFrame		= false;
	float m_debugSine				= 0.0f;
	float m_debugCos				= 0.0f;
	float m_debugPerlinNoise		= 0.0f;
	Vec3  m_debugNoisePos			= Vec3::ZERO;
	int   m_chainInfoIndex			= 0;	

	//----------------------------------------------------------------------------------------------------------------------
	// CCD creature
	//----------------------------------------------------------------------------------------------------------------------
	CreatureBase*	m_creatureCCD		= nullptr;
	IK_Chain3D*		m_ikChain_CCD		= nullptr;
	float			m_limbLength		= 10.0f;
	float			m_halfLimbLength	= m_limbLength * 0.5f;


	//----------------------------------------------------------------------------------------------------------------------
	// Food orbs
	//----------------------------------------------------------------------------------------------------------------------
	FoodManager* m_foodManager = nullptr;


	// Trash can variables
	Vec3 m_trashCanBottomPos = Vec3( 30.0f, 0.0f, 0.0f );
	Vec3 m_trashCanTopPos	 = Vec3( 30.0f, 0.0f, 5.0f );
};
