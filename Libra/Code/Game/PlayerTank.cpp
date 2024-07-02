#include "Game/PlayerTank.hpp"
#include "Game/Game.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedStrings.hpp"

//----------------------------------------------------------------------------------------------------------------------
PlayerTank::PlayerTank( Map* currentMap, Vec2 position, float orientationDegrees, EntityType type )
	: Entity( currentMap, position, orientationDegrees, type)
{
	m_isPushedByWalls		= true;
	m_isPushedByEntities	= true;
	m_doesPushEntities		= true;
	m_hitByBullet			= true;

	m_physicsRadius = g_gameConfigBlackboard.GetValue( "playerPhysicsRadius", 0.0f );
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue( "playerCosmeticRadius", 0.0f );

	m_tankTexture	= g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankBase.png");
	m_turretTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankTop.png");
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerTank::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerTank::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerTank::Update(float deltaSeconds)		
{
	XboxController controller = g_theInput->GetController(0);

	m_velocity = Vec2(0.0f, 0.0f);
	m_tankMoveIntention = Vec2( 0.0f, 0.0f);

	// Tank input
	if (g_theInput->IsKeyDown('E'))
	{
		m_tankMoveIntention += Vec2( 0.0f, 1.0f );
	}
	if (g_theInput->IsKeyDown('S'))
	{
		m_tankMoveIntention += Vec2( -1.0f, 0.0f);
	}
	if (g_theInput->IsKeyDown('D'))
	{
		m_tankMoveIntention += Vec2( 0.0f, -1.0f);
	}
	if (g_theInput->IsKeyDown('F'))
	{
		m_tankMoveIntention += Vec2( 1.0f, 0.0f);
	}
	
	m_tankMoveIntention += controller.GetLeftJoyStick().GetPosition();
	m_tankMoveIntention.ClampLength( 1.0f );

	// Tank movement
	if (m_tankMoveIntention.GetLength() > 0.0f)
	{
		m_tankGoalOrientationDegrees = m_tankMoveIntention.GetOrientationDegrees();
		float newTankOrientationAfterRotation = GetTurnedTowardDegrees( m_tankOrientationDegrees, m_tankGoalOrientationDegrees, (PLAYER_TANK_TURN_SPEED * deltaSeconds) );
		
//		float offset1 = GetShortestAngularDispDegrees(m_tankOrientationDegrees, m_tankGoalOrientationDegrees);
		float offset = newTankOrientationAfterRotation - m_tankOrientationDegrees;

		m_tankOrientationDegrees = newTankOrientationAfterRotation;
		m_velocity = GetFowardNormal() * ( PLAYER_TANK_MAX_SPEED * m_tankMoveIntention.GetLength() );

		m_turretOrientationDegrees += offset;
 		m_turretGoalOrientationDegrees  += offset;
	}

	// Turret input
	m_turrentTurnIntention = Vec2(0.0f, 0.0f);

	if (g_theInput->IsKeyDown('I'))
	{
		m_turrentTurnIntention += Vec2(0.0f, 1.0f);
	}
	if (g_theInput->IsKeyDown('J'))
	{
		m_turrentTurnIntention += Vec2(-1.0f, 0.0f);
	}
	if (g_theInput->IsKeyDown('K'))
	{
		m_turrentTurnIntention += Vec2(0.0f, -1.0f);
	}
	if (g_theInput->IsKeyDown('L'))
	{
		m_turrentTurnIntention += Vec2(1.0f, 0.0f);
	}

	m_turrentTurnIntention += controller.GetRightJoyStick().GetPosition();
	m_turrentTurnIntention.ClampLength( 1.0f );

	// Turret movement
	if ( m_turrentTurnIntention.GetLength() > 0.0f )
	{	
		m_turretGoalOrientationDegrees = m_turrentTurnIntention.GetOrientationDegrees();
		m_turretOrientationDegrees = GetTurnedTowardDegrees(m_turretOrientationDegrees, m_turretGoalOrientationDegrees, (PLAYER_TANK_TURN_SPEED * deltaSeconds));
	}

	m_turretOrientationDegrees += (m_angularVelocity * deltaSeconds);

	//----------------------------------------------------------------------------------------------------------------------
	// Bullet input
	if ( g_theInput->IsKeyDown( KEYCODE_SPACE_BAR ) || controller.GetRightTrigger() )
	{
		SoundID shootSound = g_theAudio->CreateOrGetSound( "Data/Audio/shootSound.wav" );
		g_theAudio->StartSound( shootSound );

		m_map->SpawnNewEntityOfType( ENTITY_TYPE_GOOD_BULLET, m_position + ( this->GetFowardNormal() * 0.5f ), m_orientationDegrees );
	}
	 
	Entity::Update(deltaSeconds);
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerTank::Render() const
{
	RenderTank();
	RenderTurret();
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerTank::RenderTank() const 
{
	AABB2 playerTankBounds = AABB2( Vec2( -0.5f, -0.5f ), Vec2(  0.5f, 0.5f ) );
	Rgba8 color	= Rgba8( 255, 255, 255, 255 );
	
	std::vector<Vertex_PCU> tempTankBaseVerts;
	AddVertsForAABB2D( tempTankBaseVerts, playerTankBounds, color );
	TransformVertexArrayXY3D( (int)tempTankBaseVerts.size(), tempTankBaseVerts.data(), 1.0f, m_orientationDegrees, m_position );


	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( m_tankTexture );
	g_theRenderer->DrawVertexArray( (int)tempTankBaseVerts.size(), tempTankBaseVerts.data() );
} 

//----------------------------------------------------------------------------------------------------------------------
void PlayerTank::RenderTurret() const
{
	AABB2 playerTurretBounds = AABB2( Vec2( -0.5f, -0.5f ), Vec2( 0.5f, 0.5f ) );
	Rgba8 color = Rgba8( 255, 255, 255, 255 );
	
	std::vector<Vertex_PCU> tempTurretBaseVerts;
	AddVertsForAABB2D( tempTurretBaseVerts, playerTurretBounds, color );
	TransformVertexArrayXY3D( (int)tempTurretBaseVerts.size(), tempTurretBaseVerts.data(), 1.0f, m_turretOrientationDegrees, m_position );

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( m_turretTexture );
	g_theRenderer->DrawVertexArray( (int)tempTurretBaseVerts.size(), tempTurretBaseVerts.data() );
}