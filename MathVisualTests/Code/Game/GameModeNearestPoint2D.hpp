#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"

//----------------------------------------------------------------------------------------------------------------------
class GameModeNearestPoint2D : public GameModeBase 
{
public:
	GameModeNearestPoint2D();
	~GameModeNearestPoint2D();
	virtual void Startup()						override;
	virtual void Update( float deltaSeconds )	override;
	virtual void Render() const					override;
	virtual void Reshuffle()					override;
	virtual void Shutdown()						override;

	//----------------------------------------------------------------------------------------------------------------------
	// Pre-compute positions
		AABB2	m_box					= AABB2( 20.0f, 10.0f, 40.0f, 20.0f );
		Vec2	m_discCenterPos			= Vec2( 120.0f, 20.0f );
		Vec2	m_lineSegmentStart		= Vec2( 50.0f, 50.0f );
		Vec2	m_lineSegmentEnd		= Vec2( 70.0f, 50.0f );
		Vec2	m_infiniteLineStart		= Vec2( 65.0f, 25.0f );
		Vec2	m_infiniteLineEnd		= Vec2( 70.0f, 30.0f );
		Vec2	m_capsuleBoneStart		= Vec2( 25.0f, 40.0f );
		Vec2	m_capsuleBoneEnd		= Vec2( 30.0f, 40.0f );
		float	m_capsuleRadius			= 2.0f;
		float	m_discRadius			= 10.0f;
		float	m_lineRadius			= 1.0f;
		OBB2D*	m_obb2D;

private:
	void UpdatePauseQuitAndSlowMo();
	void UpdatePlayerDot( float deltaSeconds );
	void UpdateGameCamNearestPoint2D();
	void RenderEntities() const;
	 
	Camera m_worldCamera; 
	Camera m_screenCamera;
	
	bool m_isPaused			= false; 
	bool m_isSlowMo			= false;
	bool m_isFastMo			= false;

	Vec2 m_worldSize	= Vec2( 200.0f, 100.0f );
	Vec2 m_worldCenter	= Vec2( (m_worldSize.x / 2.0f), (m_worldSize.y / 2.0f) );

	Vec2 m_playerDotPos = Vec2( m_worldCenter.x, m_worldCenter.y );

public:
	Clock		m_clock;
};