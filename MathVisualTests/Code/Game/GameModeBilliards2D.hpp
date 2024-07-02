#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Clock.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Billiards
{
public:
	Vec2  m_billiardPos			= Vec2( -1.0f, -1.0f );
	Vec2  m_billiardVelocity	= Vec2(	 0.0f,  0.0f );
	float m_billiardRadius		= 3.0f;
	float m_billiardElasticity	= 0.9f;
};

//----------------------------------------------------------------------------------------------------------------------
class Bumper
{
public:
	Vec2  m_bumperPos			= Vec2( -1.0f, -1.0f );
	Vec2  m_bumperVelocity		= Vec2(  0.0f,  0.0f );
	float m_bumperRadius		= 8.0f; 
	float m_bumperElasticity	= 1.0f;
	Rgba8 m_bumperColor			= Rgba8::RED;
};

//----------------------------------------------------------------------------------------------------------------------
class GameModeBilliards2D : public GameModeBase
{
public:
	GameModeBilliards2D();
	virtual ~GameModeBilliards2D();

	virtual void Startup()						override;
	virtual void Update( float deltaSeconds )	override;
	virtual void Render() const					override;
	virtual void Reshuffle()					override;
	virtual void Shutdown()						override;

	void UpdatePauseQuitAndSlowMo();
	void UpdateInput( float deltaSeconds );
	void UpdateRaycastResult2D();
	void RenderUIStuff() const;

	// Billiard functions
	void UpdateGameCamBilliards2D();
	void UpdateSpawnBilliards();
	void UpdateBilliardCollisionAndClampToWorldBounds( float deltaSeconds );

	//----------------------------------------------------------------------------------------------------------------------
	// Bumper variables
	Vec2	m_bumperPosArray[10]	= 
	{
		Vec2( 180.0f, 90.0f ),
		Vec2(  20.0f, 15.0f ),
		Vec2( 120.0f, 40.0f ),
		Vec2(  80.0f, 20.0f ),
		Vec2( 155.0f, 46.0f ),
		Vec2(  30.0f, 70.0f ),
		Vec2(  60.0f, 60.0f ),
		Vec2( 110.0f, 70.0f ),
		Vec2(  90.0f, 30.0f ),
		Vec2(  50.0f, 40.0f ),
	};

	float	m_bumperRadiusArray[10]	=
	{
		1.0f,
		2.0f,
		10.0f,
		4.0f,
		5.0f,
		6.0f,
		7.0f,
		8.0f,
		9.0f,
		10.0f,
	};

	float	m_bumperElasticityArray[10]	=
	{
		0.1f,
		0.2f,
		0.3f,
		0.4f,
		0.5f,
		0.6f,
		0.7f,
		0.8f,
		0.9f,
		1.0f,
	};

	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	Camera				m_billiardsWorldCamera;
	Camera				m_billiardsUICamera;

	//----------------------------------------------------------------------------------------------------------------------
	// Raycast variables
	RaycastResult2D		m_bumperRaycastResult2D;
	RaycastResult2D		m_billiardRaycastResult2D;
	Vec2				m_rayStartPos			= Vec2( WORLD_CENTER_X, WORLD_CENTER_Y );
	Vec2				m_rayEndPos				= m_rayStartPos + Vec2( 20.0f, 0.0f );
	float				m_arrowSize				= 1.5f;
	float				m_billiardRadius		= 3.0f;
	float				m_arrowThickness		= 0.5f;
	int					m_closestDisc			= 0;
	bool				m_didDiscImpact			= false;
	Vec2				m_updatedImpactPos		= Vec2( -1.0f, -1.0f );
	Vec2				m_updatedImpactNormal	= Vec2( -1.0f, -1.0f );

public:
	// Billiard variables
	std::vector<Billiards*>  m_billiardsList;
	std::vector<Bumper*>	 m_bumperList;
	int						 m_numBumpers			 = 10;
	bool					 m_isOverlappingBumper	 = false;
	bool					 m_isOverlappingBilliard = false;

	// Game variables
	Clock					 m_gameClock;
	bool					 m_isSlowMo				 = false;
};