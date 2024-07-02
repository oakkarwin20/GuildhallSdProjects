#pragma once

#include "Game/Entity.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/SkeletalSystem/IK_Segment3D.hpp"
#include "Engine/SkeletalSystem/SkeletalSystem3D.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Player : public Entity
{
public:
	Player( Game* game );
	~Player();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;

	void UpdateGameMode3DCamera();

	void AddVertsForCompass( std::vector<Vertex_PCU>& compassVerts, Vec3 position, float axisLength, float axisThickness ) const;

	void UpdatePlayerInput( float deltaSeconds );

	// Creature Functions
	void UpdateCreature( float deltaSeconds );
	void UpdateCreatureRootPosInput( float deltaSeconds );
	void UpdateCreatureHeight( float deltaSeconds );
	void DetermineBestStepPos();
	bool IsLimbIsTooFarFromRoot( SkeletalSystem3D* currentLimb, Vec3 footTargetPos );
	bool DoesTargetPosOverlapSolidObject( Vec3& footTargetPos );
	void SpecifyTargetPos( Vec3& targetPos, float fwdStepAmount, float leftStepAmount );
	void TurnCreatureTowardsCameraDir();

public:
	float	m_defaultSpeed	= 4.0f;
	float	m_currentSpeed	= 4.0f;
	float	m_fasterSpeed	= m_defaultSpeed * 15.0f;
	float	m_slowerSpeed	= m_defaultSpeed * 0.25f;
	float	m_elevateSpeed  = 6.0f;
	float	m_turnRate		= 90.0f;
	Camera	m_worldCamera;
	Camera	m_uiCamera;

	// SkeletalSystem3D variables
	IK_Segment3D*	  m_root				= nullptr;
	SkeletalSystem3D* m_rightArm			= nullptr;
	SkeletalSystem3D* m_leftArm				= nullptr;
	float			  m_limbLength			=  1.0f;
	float			  m_numLimbs			=  5.0f;
	float			  m_rootDefaultHeightZ	= 20.0f;

	// Creature Limb placement variables
	// Current arm pos
	Vec3 m_rightArmEndEffectorPos	= Vec3::ZERO;
	Vec3 m_leftArmEndEffectorPos	= Vec3::ZERO;
	// Goal, potentially "future" position
	Vec3 m_rightArmGoalPos			= Vec3::ZERO;
	Vec3 m_leftArmGoalPos			= Vec3::ZERO;
};