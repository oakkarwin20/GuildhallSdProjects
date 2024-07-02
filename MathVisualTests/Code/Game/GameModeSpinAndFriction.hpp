#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
constexpr int NUM_PLAYERSHIP_TRIS = 5;
constexpr int NUM_PLAYERSHIP_VERTS = ( NUM_PLAYERSHIP_TRIS * 3 );

//----------------------------------------------------------------------------------------------------------------------
class PachinkoBilliards_SpinAndFriction
{
public:
	Vec2  m_billiardPos			= Vec2( 0.0f, 0.0f );
	Vec2  m_billiardVelocity	= Vec2(	0.0f, 0.0f );
	float m_billiardRadius		= 2.0f;
	float m_billiardElasticity	= 0.9f;
	Rgba8 m_billiardColor		= Rgba8::WHITE;
	float m_angularVelocity		= 0.0f;
	float m_orientationDegrees	= 0.0f;
};

//----------------------------------------------------------------------------------------------------------------------
class DiscBumper_SpinAndFriction
{
public:
	Vec2  m_discBumperPos			= Vec2( -1.0f, -1.0f );
	float m_discBumperRadius		= 8.0f; 
	float m_discBumperElasticity	= 1.0f;
	Rgba8 m_discBumperColor			= Rgba8::MAGENTA;
};

//----------------------------------------------------------------------------------------------------------------------
class Obb2Bumper_SpinAndFriction
{
public:
	OBB2D* m_obb2;
	float  m_obb2BumperElasticity	= 1.0f;
	Rgba8  m_Obb2BumperColor		= Rgba8::MAGENTA;
};

//----------------------------------------------------------------------------------------------------------------------
class CapsuleBumper_SpinAndFriction
{
public:
	Vec2 m_capsuleBoneStart				= Vec2::NEGATIVE_ONE;
	Vec2 m_capsuleBoneEnd				= Vec2::NEGATIVE_ONE;
	Vec2 m_capsuleDir					= Vec2::NEGATIVE_ONE;
	float m_capsuleOrientation			= -1.0f;
	float m_capsuleLength				= -1.0f;
	float m_capsuleRadius				= -1.0f;
	float m_capsuleBumperElasticity		= 1.0f;
	Rgba8 m_capsuleBumperColor			= Rgba8::MAGENTA;
};

//----------------------------------------------------------------------------------------------------------------------
class GameModeSpinAndFriction : public GameModeBase
{
public:
	GameModeSpinAndFriction();
	virtual ~GameModeSpinAndFriction();

	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Reshuffle();
	virtual void Shutdown();

	void UpdateGameCameraPachinkoMachine();
	void RenderWorldObjects() const;
	void RenderUIObjects() const;

	// Game input and debug functions
	void UpdateInput( float deltaSeconds );
	void UpdatePauseQuitAndSlowMo();
	
	// Billiard functions
	void	UpdateSpawnBilliards();
	void	UpdateBilliardCollisionAndClampToWorldBounds( float deltaSeconds );
	void	ApplyGravityToBilliards( float deltaSeconds );
	void	UpdateBallOrientation( float deltaSeconds );
	void	ComputeGarwinCollisions();
	float	ComputeGarwinAngularVelocity( float velocityXorY, float angularVelocity, float radius );
	float	ComputeGarwinVelocityX( float velocityX, float angularVelocity, float radius );
	float	ComputeGarwinVelocityY( float velocityY );

public:
	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	Camera m_raycastVsAABB2DWorldCamera;
	Camera m_raycastVsAABB2DUICamera;

	BitmapFont* m_textFont = nullptr;

	//----------------------------------------------------------------------------------------------------------------------
	// Raycast variables
	RaycastResult2D		m_raycastVsAABB2Result2D;
	Vec2				m_rayStartPos			= Vec2( WORLD_CENTER_X + 5.0f, WORLD_CENTER_Y - 20.0f );
	Vec2				m_rayEndPos				= m_rayStartPos + Vec2( 40.0f, 30.0f );
	float				m_arrowSize				= 2.0f;
	float				m_arrowThickness		= 0.5f;
	bool				m_didAABB2Impact 		= false;
	Vec2				m_updatedImpactPos		= Vec2( -1.0f, -1.0f );
	Vec2				m_updatedImpactNormal	= Vec2( -1.0f, -1.0f );

	Rgba8 m_rayDefaultColor		 = Rgba8::GREEN;
	Rgba8 m_rayImpactDistColor	 = Rgba8::RED;
	Rgba8 m_rayAfterImpactColor  = Rgba8::GRAY;
	Rgba8 m_rayImpactDiscColor	 = Rgba8::WHITE;
	Rgba8 m_rayImpactNormalColor = Rgba8::YELLOW;

	//----------------------------------------------------------------------------------------------------------------------
	// Billiards variables
	std::vector<PachinkoBilliards_SpinAndFriction*>  m_billiardsList;
	bool							 m_isWarpOn					= false;
	float							 m_minYWarpThreshold		=  -5.0f;
	float							 m_gravityWeight			= -60.0f;
	float							 m_billiardMinRadius		=	1.0f;
	float							 m_billiardMaxRadius		=	4.5f;

	//----------------------------------------------------------------------------------------------------------------------
	// Bumper variables
	std::vector<DiscBumper_SpinAndFriction*>	 m_discBumperList;
	std::vector<Obb2Bumper_SpinAndFriction*>	 m_obb2BumperList;
	std::vector<CapsuleBumper_SpinAndFriction*>	 m_capsuleBumperList;
	float	m_nearestPointRadius	= 0.05f;
	int		m_numBumpers			= 10;

	// Time step variables
	float m_physicsUpdateDebt		= 0.0f;
	float m_physicsFixedTimeStep	= 0.005f;
	bool  m_isVariableTimeStep		= false;

	float m_wallElasticity = 0.88f;

	//----------------------------------------------------------------------------------------------------------------------
	// Texture
	Texture* m_testTexture = nullptr;
};