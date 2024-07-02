#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/App.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include <Engine/Core/EngineCommon.hpp>
#include "Engine/Core/Vertex_PCU.hpp"


PlayerShip::PlayerShip( Game* game, Vec2 spawnPosition ) : Entity( game, spawnPosition )
{
	m_physicsRadius = 1.75f;
	m_cosmeticRadius = 2.25f;

	initializelocalVerts();
	initializeThrusterVerts();
}
 
void PlayerShip::Startup()
{
}

void PlayerShip::Shutdown()
{
}
 
void PlayerShip::Update(float deltaSeconds)
{
	//Playership moves forward
	if (g_theInput->IsKeyDown('E') || g_theInput->GetController(0).GetRightTrigger())
	{
		//----------------------------------------------------------------------------------------------------------------------
		// randomize playerShip thruster flickering effect
		float randNum = g_theRNG->RollRandomFloatInRange( -2.0f, -5.0f );
		if ( deltaSeconds != 0 )
		{
			m_localThrusterVerts[2].m_position.x = randNum;
		}

		Vec2 fowardDirection = Vec2( 1.0f, 0.0f );
		fowardDirection.SetOrientationDegrees( m_orientationDegrees );
		m_velocity += (fowardDirection * PLAYER_SHIP_ACCERLERATION * deltaSeconds);
	}
		m_position += (m_velocity * deltaSeconds);

	// Controller rotates ship
	float controllerMagnitude = g_theInput->GetController(0).GetLeftJoyStick().GetMagnitude();
	if ( controllerMagnitude > 0.0f )
	{
		m_orientationDegrees = g_theInput->GetController(0).GetLeftJoyStick().GetOrientationDegrees();
	}

	//Playership rotates left
	if (g_theInput->IsKeyDown('S'))
	{
		m_orientationDegrees += ( PLAYER_SHIP_TURN_SPEED * deltaSeconds );
	}

	//Playership rotates right
	if (g_theInput->IsKeyDown( 'F' ))
	{
		m_orientationDegrees -= ( PLAYER_SHIP_TURN_SPEED * deltaSeconds );
	}

	//Player respawns at screen center
	if (g_theInput->WasKeyJustPressed( 'N' ) || g_theInput->GetController(0).IsButtonDown(BUTTON_START))
	{
		SoundID respawnSound = g_theAudio->CreateOrGetSound( "Data/Audio/Click.mp3" );
		g_theAudio->StartSound( respawnSound );


		m_position = m_game->m_shipSpawnPosition;
		m_orientationDegrees = 0;
		m_velocity *= 0;
	}

	//Playership shoots 1 bullet when 'SPACEBAR' is pressed
	if (g_theInput->WasKeyJustPressed( KEYCODE_SPACE_BAR ) || g_theInput->GetController(0).IsButtonDown(BUTTON_A))
	{
		SoundID shootSound = g_theAudio->CreateOrGetSound( "Data/Audio/shootSound.wav" );
		g_theAudio->StartSound( shootSound );

		//add code for shooting bullet
//		Bullet* bullet = new Bullet( m_game,  m_position + (this->GetFowardNormal() * 0.5f ), m_orientationDegrees );

		// Render Bullets
		Bullet* bullet = new Bullet( m_game, this->GetFowardNormal() );
		bullet->Update( deltaSeconds ); 
	}

	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		m_game->isDebugDisplayOn = !m_game->isDebugDisplayOn;
	}
}

void PlayerShip::Render() const
{
	Vertex_PCU tempWorldVerts[NUM_PLAYERSHIP_VERTS];

	for (int index = 0; index < NUM_PLAYERSHIP_VERTS; index++)
	{
		tempWorldVerts[index] = m_localVerts[index];
	}

	TransformVertexArrayXY3D( NUM_PLAYERSHIP_VERTS, tempWorldVerts, 1.0f, m_orientationDegrees, m_position );
	g_theRenderer->DrawVertexArray( NUM_PLAYERSHIP_VERTS, tempWorldVerts);

	if ( g_theInput->IsKeyDown( 'E' ) || g_theInput->GetController( 0 ).GetRightTrigger() )
	{
		DrawPlayerThruster();
	//	g_theRenderer->DrawVertexArray( NUM_PLAYERSHIP_THRUSTER_VERTS, tempWorldVerts);
	}

	DrawPlayerHealthIcon1();
	DrawPlayerHealthIcon2();
	DrawPlayerHealthIcon3();
}

void PlayerShip::UpdateFromController(float deltaSeconds)
{
	UNUSED( deltaSeconds );

	XboxController const& controller = g_theInput->GetController(0);

	// Respawn
	if (m_isDead)
	{
		if (controller.WasButtonJustPressed(XboxButtonID::BUTTON_START) && m_extraLives > 0 )
		{
			//respawn();		//TODO: need to create respawn function
			return;
		}
		return;
	}

	// Drive
	float leftStickMagnitude = controller.GetLeftJoyStick().GetMagnitude();
	if (leftStickMagnitude > 0.0f)
	{
		m_thrustFraction = leftStickMagnitude;
		m_orientationDegrees = controller.GetLeftJoyStick().GetOrientationDegrees();
	}

	// Shoot
	if ( controller.WasButtonJustPressed( XboxButtonID::BUTTON_A ) )
	{
		Vec2 forwardNormal = GetFowardNormal();
		Vec2 nosePosition = m_position + ( forwardNormal * 1.0f );
		m_game->SpawnBullet( nosePosition, m_orientationDegrees );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerShip::initializelocalVerts()
{
	// Positions
	m_localVerts[0].m_position =  Vec3( -2, 1, 0);		// Triangle A, point A
	m_localVerts[1].m_position =  Vec3(  2, 1, 0);		// Triangle A, point B
	m_localVerts[2].m_position =  Vec3(  0, 2, 0);		// Triangle A, point C

	m_localVerts[3].m_position =  Vec3( -2, 1, 0);		// Triangle B, point A
	m_localVerts[4].m_position =  Vec3( -2,-1, 0);		// Triangle B, point B
	m_localVerts[5].m_position =  Vec3(  0, 1, 0);		// Triangle B, point C

	m_localVerts[6].m_position =  Vec3( -2,-1, 0);		// Triangle C, point A
	m_localVerts[7].m_position =  Vec3(  0,-1, 0);		// Triangle C, point B
	m_localVerts[8].m_position =  Vec3(  0, 1, 0);		// Triangle C, point C

	m_localVerts[9].m_position =  Vec3(  0, 1, 0);		// Triangle D, point A
	m_localVerts[10].m_position =  Vec3(  0,-1, 0);		// Triangle D, point B
	m_localVerts[11].m_position =  Vec3(  1, 0, 0);		// Triangle D, point C

	m_localVerts[12].m_position =  Vec3( -2,-1, 0);		// Triangle E, point A
	m_localVerts[13].m_position =  Vec3(  0,-2, 0);		// Triangle E, point B
	m_localVerts[14].m_position =  Vec3(  2,-1, 0);		// Triangle E, point C

	//Color
	m_localVerts[0].m_color =  Rgba8( 255, 0, 0,  255);		// Triangle A, point A
	m_localVerts[1].m_color =  Rgba8( 255, 0, 0,  255);		// Triangle A, point B
	m_localVerts[2].m_color =  Rgba8( 255, 0, 0,  255);		// Triangle A, point C
					
	m_localVerts[3].m_color =  Rgba8( 255, 0, 0,  255);		// Triangle B, point A
	m_localVerts[4].m_color =  Rgba8( 255, 0, 0,  255);		// Triangle B, point B
	m_localVerts[5].m_color =  Rgba8( 255, 0, 0,  255);		// Triangle B, point C
					  				  
	m_localVerts[6].m_color =  Rgba8( 255, 0, 0,  255);		// Triangle C, point A
	m_localVerts[7].m_color =  Rgba8( 255, 0, 0,  255);		// Triangle C, point B
	m_localVerts[8].m_color =  Rgba8( 255, 0, 0,  255);		// Triangle C, point C
									 
	m_localVerts[9].m_color  = Rgba8( 255, 0, 0,  255);		// Triangle D, point A
	m_localVerts[10].m_color = Rgba8( 255, 0, 0,  255);		// Triangle D, point B
	m_localVerts[11].m_color = Rgba8( 255, 0, 0,  255);		// Triangle D, point C
								  				 
	m_localVerts[12].m_color = Rgba8( 255, 0, 0,  255);		// Triangle E, point A
	m_localVerts[13].m_color = Rgba8( 255, 0, 0,  255);		// Triangle E, point B
	m_localVerts[14].m_color = Rgba8( 255, 0, 0,  255);		// Triangle E, point C
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerShip::initializeThrusterVerts()
{
	m_localThrusterVerts[0].m_position =  Vec3( -2, -1, 0);		// Triangle A, point A
	m_localThrusterVerts[1].m_position =  Vec3( -2,  1, 0);		// Triangle A, point B
	m_localThrusterVerts[2].m_position =  Vec3( -2,  0, 0);		// Triangle A, point C		// should be -5

	m_localThrusterVerts[0].m_color = Rgba8::ORANGE;
	m_localThrusterVerts[1].m_color = Rgba8::ORANGE;
	m_localThrusterVerts[2].m_color = Rgba8::ORANGE;
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerShip::DrawPlayerThruster() const
{
	Vertex_PCU tempWorldVerts[NUM_PLAYERSHIP_THRUSTER_VERTS];
	for ( int index = 0; index < NUM_PLAYERSHIP_THRUSTER_VERTS; index++ )
	{
		tempWorldVerts[index] = m_localThrusterVerts[index];
	}

	TransformVertexArrayXY3D( NUM_PLAYERSHIP_THRUSTER_VERTS, tempWorldVerts, 1.0f, ( this->m_orientationDegrees ), this->m_position );
	g_theRenderer->DrawVertexArray( NUM_PLAYERSHIP_THRUSTER_VERTS, tempWorldVerts );
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerShip::DrawPlayerHealthIcon1() const
{
	Vertex_PCU tempWorldVerts[NUM_PLAYERSHIP_VERTS];

	for ( int index = 0; index < NUM_PLAYERSHIP_VERTS; index++ )
	{
		tempWorldVerts[index] = m_localVerts[index];
	}

	TransformVertexArrayXY3D( NUM_PLAYERSHIP_VERTS, tempWorldVerts, 1.0f, 0.0, Vec2( 10.0f, 95.0f ) );
	g_theRenderer->DrawVertexArray( NUM_PLAYERSHIP_VERTS, tempWorldVerts );
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerShip::DrawPlayerHealthIcon2() const
{
	Vertex_PCU tempWorldVerts[NUM_PLAYERSHIP_VERTS];

	for ( int index = 0; index < NUM_PLAYERSHIP_VERTS; index++ )
	{
		tempWorldVerts[index] = m_localVerts[index];
	}

	TransformVertexArrayXY3D( NUM_PLAYERSHIP_VERTS, tempWorldVerts, 1.0f, 0.0, Vec2( 15.0f, 95.0f ) );
	g_theRenderer->DrawVertexArray( NUM_PLAYERSHIP_VERTS, tempWorldVerts );
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerShip::DrawPlayerHealthIcon3() const
{
	Vertex_PCU tempWorldVerts[NUM_PLAYERSHIP_VERTS];

	for ( int index = 0; index < NUM_PLAYERSHIP_VERTS; index++ )
	{
		tempWorldVerts[index] = m_localVerts[index];
	}

	TransformVertexArrayXY3D( NUM_PLAYERSHIP_VERTS, tempWorldVerts, 1.0f, 0.0, Vec2( 20.0f, 95.0f ) );
	g_theRenderer->DrawVertexArray( NUM_PLAYERSHIP_VERTS, tempWorldVerts );
}
