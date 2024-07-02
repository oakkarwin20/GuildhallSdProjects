#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/CubicBezierCurve2D.hpp"

#include "Engine/Math/Spline.hpp"
#include "Engine/Renderer/Camera.hpp"

//----------------------------------------------------------------------------------------------------------------------
class BitmapFont;
struct Vec2;

//----------------------------------------------------------------------------------------------------------------------
class GameModeBezierCurves : public GameModeBase
{
public:
	GameModeBezierCurves();
	virtual ~GameModeBezierCurves();

	virtual void Startup();
	virtual void Update( float deltaSeconds );	
	virtual void Render() const;
	virtual void Reshuffle(); 
	virtual void Shutdown();

	void UpdateGameCameraBezier();
	void RenderWorldObjects()	const;
	void RenderUIObjects()		const;

	// Game input and debug functions
	void UpdatePauseQuitAndSlowMo();

	//----------------------------------------------------------------------------------------------------------------------
	// Easing function
	void DetermineEasingResult();
	float DetermineEasingFunction( float t ) const;

	//----------------------------------------------------------------------------------------------------------------------
	Vec2 GetSplinePointAtTime( float parametricZeroToOne ) const;

private:
	Clock	m_gameClock;
	bool	m_isSlowMo;

public:
	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	Camera		m_bezierWorldCamera;
	Camera		m_bezierUICamera;

	BitmapFont* m_textFont = nullptr;

	bool m_debugBackground = false;

	int		m_numSubdivisions	= 64;
	float	m_thickness			= 0.5f;
	float	paddingX			= 2.0f;
	float	paddingY			= 3.0f;
	float	m_boxSize			= 35.0f;
	float   m_offsetX			= 30.0f;
	float   m_offsetY			= 5.0f;
	float	m_beizerOffset		= 30.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Easing 
	Vec2  m_easingPanel_Mins	= Vec2(			   0.0f + paddingX, SCREEN_CENTER_Y + paddingY		);
	Vec2  m_easingPanel_Maxs	= Vec2( SCREEN_CENTER_X - paddingX, SCREEN_SIZE_Y - paddingY * 2.0f );
	AABB2 m_easingPanelBounds	= AABB2( m_easingPanel_Mins, m_easingPanel_Maxs );

	Vec2 m_easingMins			= Vec2(	m_easingPanel_Mins.x + m_offsetX, m_easingPanel_Mins.y + m_offsetY );
	Vec2 m_easingMaxs			= Vec2(		  m_easingMins.x + m_boxSize, m_easingMins.y + m_boxSize  );
	AABB2 m_easingGraphBounds	= AABB2( m_easingMins, m_easingMaxs );

	//----------------------------------------------------------------------------------------------------------------------
	// Bezier
	Vec2 m_bezierPanel_Mins		= Vec2( SCREEN_CENTER_X + paddingX, SCREEN_CENTER_Y + paddingY		);
	Vec2 m_bezierPanel_Maxs		= Vec2(	  SCREEN_SIZE_X - paddingX, SCREEN_SIZE_Y - paddingY * 2.0f );
	AABB2 m_bezierPanelBounds	= AABB2( m_bezierPanel_Mins, m_bezierPanel_Maxs );

	Vec2 m_bezierMins			= Vec2(	m_bezierPanel_Mins.x + m_offsetX, m_bezierPanel_Mins.y + m_offsetY );
	Vec2 m_bezierMaxs			= Vec2(		  m_bezierMins.x + m_boxSize, m_bezierMins.y + m_boxSize  );
	AABB2 m_bezierGraphBounds	= AABB2( m_bezierMins, m_bezierMaxs);

	//----------------------------------------------------------------------------------------------------------------------
	// Spline
	Vec2 m_splinePanel_Mins		= Vec2(			 0.0f + paddingX, 0.0f + paddingY );
	Vec2 m_splinePanel_Maxs		= Vec2( SCREEN_SIZE_X - paddingX, SCREEN_CENTER_Y - paddingY );
	AABB2 m_splinePanelBounds	= AABB2( m_splinePanel_Mins, m_splinePanel_Maxs );

	Vec2 m_splinePoint1 = Vec2::ZERO;
	Vec2 m_splinePoint2 = Vec2::ZERO;		
	Vec2 m_splinePoint3 = Vec2::ZERO;
	Vec2 m_splinePoint4 = Vec2::ZERO;

	CubicBezierCurve2D m_cubicBezier; 

	int   m_easingModeIndex		= -1;
	int   m_maxGameModeIndex	= 16;
	float m_easingResult		= -1.0f;
	float m_parametricT			= -1.0f;
	std::string m_easingName	= std::string( "SmoothStart 2" );

	Spline m_spline;
};

