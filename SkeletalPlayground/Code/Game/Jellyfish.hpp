#pragma once

#include "Engine/SkeletalSystem/CreatureBase.hpp"


//----------------------------------------------------------------------------------------------------------------------
class GameMode3D;
class GameMode_Quadruped_3D;


//----------------------------------------------------------------------------------------------------------------------
class Jellyfish : public CreatureBase
{
public:
	Jellyfish( GameMode3D* game, Vec3 const& rootStartPos, float limbLength = 1.0f );
	~Jellyfish();

	void InitJoints();
	void InitMesh();
	void UpdateJoints( float deltaSeconds );
	
	void ComputeAndRenderRootIJK_WS( std::vector<Vertex_PCU>& verts ) const;
	void RenderJellyfish( std::vector<Vertex_PCU>& verts ) const;


public:
	GameMode3D*		m_game					= nullptr;
	Vec3			m_prevPosLastFrame_WS	= Vec3::ZERO;
	float			m_numTentacles			=  2.0f;
	float			m_limbLength			= 10.0f;

	// Skeleton System pointers
	IK_Chain3D* m_body		 = nullptr;
	IK_Chain3D* m_tentacle1	 = nullptr;
	IK_Chain3D* m_tentacle2  = nullptr;
	IK_Chain3D* m_tentacle3  = nullptr;
	IK_Chain3D* m_tentacle4  = nullptr;
	IK_Chain3D* m_tentacle5  = nullptr;
	IK_Chain3D* m_tentacle6  = nullptr;

	// Configurable data 
	float m_tentacleLength			= 10.0f;
	float m_yOffset_tentacleStart	=  5.0f;
	float m_headRadiusStart			=  8.0f; 
	float m_headRadiusEnd			=  4.0f; 

	//----------------------------------------------------------------------------------------------------------------------
	// Hack to add FABRIK fwd only
	//----------------------------------------------------------------------------------------------------------------------
	CreatureBase* m_creature_FABRIK_FWD_ONLY	= nullptr;
	IK_Chain3D*   m_ikChain_FABRIK_FWD_ONLY_1	= nullptr;
	IK_Chain3D*   m_ikChain_FABRIK_FWD_ONLY_2	= nullptr;
	IK_Chain3D*   m_ikChain_FABRIK_FWD_ONLY_3	= nullptr;
	IK_Chain3D*   m_ikChain_FABRIK_FWD_ONLY_4	= nullptr;

};
