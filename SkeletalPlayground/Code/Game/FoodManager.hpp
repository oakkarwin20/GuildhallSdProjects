#pragma once

#include "Game/App.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/CubicBezierCurve3D.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include <vector>


//----------------------------------------------------------------------------------------------------------------------
class GameMode3D;
class GameMode_RobotArm_3D;
class Quadruped;
class IK_Chain3D;


//----------------------------------------------------------------------------------------------------------------------
struct FoodOrb
{
public: 
	FoodOrb( Vec3 const& position )
	{
		m_position = position;
	}

	Vec3				m_position		= Vec3::ZERO;
	Vec3				m_goalPos		= Vec3::ZERO;
	Vec3				m_prevPos		= Vec3::ZERO;
	bool				m_isConsumed	= false;
	bool				m_isGrabbed		= false;
	float				m_age			= 0.0f;
	CubicBezierCurve3D  m_bezierCurve	= CubicBezierCurve3D( Vec3(-40.0f, 0.0f, 0.0f), Vec3(-20.0f, 0.0f, 25.0f), Vec3(20.0f, 0.0f, 25.0f), Vec3(40.0f, 0.0f, 0.0f) );
	Stopwatch			m_bezierTimer	= Stopwatch( &g_theApp->m_gameClock, 0.85f );
};


//----------------------------------------------------------------------------------------------------------------------
class FoodManager
{
public:
	FoodManager( int numFoodOrbs, GameMode3D* game );
	FoodManager( int numFoodOrbs, GameMode_RobotArm_3D* game );
	~FoodManager();

	void Update( float deltaSeconds );
	void Render( std::vector<Vertex_PCU>& verts );
	Vec3 GetRandPos_MaxZ();
	void SpawnFoodAtRandPos();
	void ConsumeFood( FoodOrb& foodToConsume );
	void MoveFoodOrbs( float deltaSeconds );


public:
	std::vector<FoodOrb>	m_foodList;
	GameMode_RobotArm_3D*	m_game_Robot = nullptr;
	GameMode3D*				m_game1		 = nullptr;
};