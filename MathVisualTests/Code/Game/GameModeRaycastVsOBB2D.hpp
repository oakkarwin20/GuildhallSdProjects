#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"

//----------------------------------------------------------------------------------------------------------------------
class BitmapFont;

//----------------------------------------------------------------------------------------------------------------------
class GameModeRaycastVsOBB2D : public GameModeBase
{
public:
	GameModeRaycastVsOBB2D();
	virtual ~GameModeRaycastVsOBB2D();

	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Reshuffle();
	virtual void Shutdown();

	void UpdateGameCameraRaycastVsAABB2();			
	void RenderWorldObjects() const;
	void RenderUIObjects() const;
	
	void UpdateRaycastResult2D();
	void UpdateInput( float deltaSeconds );
	void UpdatePauseQuitAndSlowMo();

public:
	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	Camera				m_raycastVsOBB2DWorldCamera;
	Camera				m_raycastVsOBB2DUICamera;

	BitmapFont* m_textFont = nullptr;

	//----------------------------------------------------------------------------------------------------------------------
	// Raycast variables
	RaycastResult2D		m_raycastVsOBB2DResult;
	Vec2				m_rayStartPos			= Vec2( WORLD_CENTER_X + 5.0f, WORLD_CENTER_Y - 20.0f );
	Vec2				m_rayEndPos				= m_rayStartPos + Vec2( 40.0f, 30.0f );
	float				m_arrowSize				= 1.5f;
	float				m_arrowThickness		= 0.5f;
	bool				m_didOBB2Impact 		= false;
	Vec2				m_updatedImpactPos		= Vec2( -1.0f, -1.0f );
	Vec2				m_updatedImpactNormal	= Vec2( -1.0f, -1.0f );
	Vec2				m_closestLineStartPos	= Vec2::NEGATIVE_ONE;
	Vec2				m_closestLineEndPos		= Vec2::NEGATIVE_ONE;

	Rgba8 m_rayDefaultColor		 = Rgba8::GREEN;
	Rgba8 m_rayImpactDistColor	 = Rgba8::RED;
	Rgba8 m_rayAfterImpactColor  = Rgba8::GRAY;
	Rgba8 m_rayImpactDiscColor	 = Rgba8::WHITE;
	Rgba8 m_rayImpactNormalColor = Rgba8::YELLOW;

	//----------------------------------------------------------------------------------------------------------------------
	// AABB2 variables
	Vec2 m_AABB2mins	= Vec2::NEGATIVE_ONE;
	Vec2 m_AABB2Maxs	= Vec2::NEGATIVE_ONE;
	Rgba8 m_obb2Color	= Rgba8::MAGENTA;
	
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<OBB2D*> m_obb2List;
	int	m_numOBB2s = 1;	

	float m_randMinXInclusive = 5.0f;
	float m_randMaxXInclusive = 190.0f;
	float m_randMinYInclusive = 10.0f;
	float m_randMaxYInclusive = 90.0f;

	int m_currentLine = -1;
};