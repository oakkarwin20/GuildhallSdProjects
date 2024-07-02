#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"

//----------------------------------------------------------------------------------------------------------------------
class GameModeRaycastVsDisc2D : public GameModeBase
{
public:
	GameModeRaycastVsDisc2D();
	~GameModeRaycastVsDisc2D();
	virtual void Startup()						override;
	virtual void Update( float deltaSeconds ) 	override;
	virtual void Render() const					override;
	virtual void Reshuffle()					override;
	virtual void Shutdown()						override;

	void UpdatePauseQuitAndSlowMo();

public:
	//----------------------------------------------------------------------------------------------------------------------
	// Disc variables
	int	m_numDiscs = 10;
	
	Vec2	m_discCenterArray[10]	= 
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

	float	m_discRadiusArray[10]	=
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

	//----------------------------------------------------------------------------------------------------------------------
	// Raycast variables
	RaycastResult2D		m_raycastResult2D;
	float				m_arrowSize				= 3.0f;
	float				m_arrowThickness		= 0.5f;
	int					m_closestDisc			= 0;
	bool				m_didDiscImpact			= false;
	Vec2				m_updatedImpactPos		= Vec2( -1.0f, -1.0f );
	Vec2				m_updatedImpactNormal	= Vec2( -1.0f, -1.0f );

private:
	void UpdateInput( float deltaSeconds );
	void UpdateGameCamRaycastVsDisc2D();
	void UpdateRaycastResult2D();
	void RenderRandDiscs() const;

private:
	Camera		m_worldCamera;
	Camera		m_screenCamera;
	Vec2		m_worldSize			= Vec2( 200.0f, 100.0f );
	Vec2		m_worldCenter		= Vec2( ( m_worldSize.x / 2.0f ), ( m_worldSize.y / 2.0f ) );
	
	Vec2		m_rayStartPos		= m_worldCenter;
	Vec2		m_rayEndPos			= m_worldCenter + Vec2( 20.0f, 0.0f );

	bool		m_isPaused			= false;
	bool		m_isSlowMo			= false;
	bool		m_isFastMo			= false;

public:
	Clock		m_clock;
};