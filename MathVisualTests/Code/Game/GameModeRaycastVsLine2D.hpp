#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Rgba8.hpp"

//----------------------------------------------------------------------------------------------------------------------
class BitmapFont;

//----------------------------------------------------------------------------------------------------------------------
struct LineSegments
{
	// Line segment variables
	Vec2	m_lineSegmentStart		= Vec2( 105.0f, 35.0f );
	Vec2	m_lineSegmentEnd		= Vec2( 120.0f, 65.0f );
	Rgba8	m_lineSegmentColor		= Rgba8::MAGENTA;
	float   m_lineSegmentThickness	= 0.5f;
};

//----------------------------------------------------------------------------------------------------------------------
class GameModeRaycastVsLine2D : public GameModeBase
{
public:
	GameModeRaycastVsLine2D();
	virtual ~GameModeRaycastVsLine2D();

	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Reshuffle();
	virtual void Shutdown();

	void UpdateGameCamerasRaycastVsLine2D();
	void RenderWorldObjects() const;
	void RenderUIStuff() const;

	void UpdateRaycastResult2D();
	void UpdateInput( float deltaSeconds );
	void UpdatePauseQuitAndSlowMo();

public:
	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	Camera				m_RaycastVsLine2DWorldCamera;
	Camera				m_RaycastVsLine2DUICamera;

	BitmapFont*	m_textFont = nullptr;

	//----------------------------------------------------------------------------------------------------------------------
	// Raycast variables
	RaycastResult2D		m_raycastVsLineResult2D;
	Vec2				m_rayStartPos			= Vec2( WORLD_CENTER_X, WORLD_CENTER_Y );
	Vec2				m_rayEndPos				= m_rayStartPos + Vec2( 20.0f, 0.0f );
	float				m_arrowSize				= 1.5f;
	float				m_arrowThickness		= 0.5f;
	bool				m_didLineImpact 		= false;
	Vec2				m_updatedImpactPos		= Vec2( -1.0f, -1.0f );
	Vec2				m_updatedImpactNormal	= Vec2( -1.0f, -1.0f );

	Rgba8 m_rayDefaultColor		 = Rgba8::GREEN;
	Rgba8 m_rayImpactDistColor	 = Rgba8::RED;
	Rgba8 m_rayAfterImpactColor  = Rgba8::GRAY;
	Rgba8 m_rayImpactDiscColor	 = Rgba8::WHITE;
	Rgba8 m_rayImpactNormalColor = Rgba8::YELLOW;

	std::vector<LineSegments*> m_lineList;
	int m_NUMLines = 8;

	float m_randMinXInclusive =   .0f;
	float m_randMaxXInclusive = 199.0f;
	float m_randMinYInclusive =	  2.0f;
	float m_randMaxYInclusive =  90.0f;

	int m_currentLine = -1; 
};
