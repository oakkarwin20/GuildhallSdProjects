#include "Game/PlayerController.hpp"
#include "Game/AIController.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Map.hpp"
#include "Game/ActorDefinition.hpp"

#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
PlayerController::PlayerController()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Audio
	bool is3D = true;
	m_drunkWobbleSID = g_theAudio->CreateOrGetSound( "Data/Audio/DrunkWobble.wav", is3D );	
}

//----------------------------------------------------------------------------------------------------------------------
PlayerController::~PlayerController()
{
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerController::Update( float deltaSeconds )
{	
	// Get deltaSeconds from SystemClock
	deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();

	//----------------------------------------------------------------------------------------------------------------------
	// Respawn timer logic 
	m_currentActor = GetActor();
	Actor* currentActor = GetActor();
	if ( currentActor->m_isGarbageStopwatch.HasDurationElapsed() )
	{
		m_currentMap->SpawnPlayer();
		m_index++;
	}
	else if ( !currentActor->m_isGarbageStopwatch.HasDurationElapsed() && currentActor->m_isDead )
	{
		if ( !m_isFreeFlyOn )
		{
			// "Death animation"
			float height = RangeMapClamped( currentActor->m_isGarbageStopwatch.GetElapsedTime(), 0.0f, currentActor->m_isGarbageStopwatch.m_duration, m_position.z, 0.08f );
			m_position.z = height;
			m_worldCamera.SetTransform( m_position, m_orientation );
			return;
		}
	}

	// Reset speed
	if ( m_currentSpeed == -1.0f )
	{
		m_currentSpeed = currentActor->m_actorDef->m_walkSpeed;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Update Input
	if ( m_isFreeFlyOn )
	{
		UpdatePlayerInput( deltaSeconds );
	}
	else 
	{
		UpdatePossessedActorInput( deltaSeconds );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Unpossess actor
	if ( g_theInput->WasKeyJustPressed( 'F' ) )
	{
		// Toggle freeFly bool
		m_isFreeFlyOn = !m_isFreeFlyOn; 

		// Set currentActor->isPossessed = false
//		Actor* currentActor = GetActor();
		currentActor->m_isPossessed = false;

		// Set playerController position and orientation to playerActor's 
//		m_position	  = Vec3( currentActor->m_position.x, currentActor->m_position.y, currentActor->m_actorDef->m_eyeHeight );
		m_orientation = currentActor->m_orientation;
	}

	PossessNextActor();
	//----------------------------------------------------------------------------------------------------------------------
	// Update Cameras;
	UpdateCamera();

	g_theAudio->UpdateListener( m_playerIndex, m_position, m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D(), m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetKBasis3D() );
//	g_theAudio->UpdateListener( m_playerIndex, m_position, m_actorForward, m_actorUp );
//	DebuggerPrintf( Stringf("PlayerPos = %f, %f, %f\n", m_position.x, m_position.y, m_position.z).c_str() );
}
 
//----------------------------------------------------------------------------------------------------------------------
void PlayerController::Render() const
{
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerController::RenderUI() const
{
	if ( m_currentActor )
	{	
		if ( m_currentActor->m_actorDef->m_name == "Marine" )
		{
			m_currentActor->m_currentWeapon->RenderUI();

			//----------------------------------------------------------------------------------------------------------------------
			// Render tint color if on drugs
			if ( m_drugEffectIsActive )
			{
				std::vector<Vertex_PCU> drugsVertsPCU;
				AABB2 drugsBounds = AABB2( Vec2(0.0f, 0.0f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y) );
				
				float totalSeconds	= g_theApp->m_gameClock.GetTotalSeconds();
				float sine			= SinDegrees( totalSeconds * 100.0f ); 
				float baseRed		= 255;
				float morphingRed	= baseRed * (sine * 0.5f);
				float baseBlue		= 55;
				float morphingBlue	= (baseBlue * (sine * 0.5f) );
				float baseAlpha		= 25;
				float morphingAlpha = (baseAlpha + (sine * 15.0f) );
				Rgba8 drugsColor  = Rgba8( static_cast<unsigned char>(morphingRed), 0, static_cast<unsigned char>(morphingBlue), static_cast<unsigned char>(morphingAlpha) );
				
				AddVertsForAABB2D( drugsVertsPCU, drugsBounds, drugsColor );
//				DebuggerPrintf( Stringf( "R = %f, B = %f, A = %f, sine = %f\n", morphingRed, morphingBlue, morphingAlpha, sine ).c_str() );

				// Draw call Solid 
				g_theRenderer->SetBlendMode( BlendMode::ALPHA );
				g_theRenderer->BindTexture( nullptr );
				g_theRenderer->BindShader( nullptr );
				g_theRenderer->SetModelConstants();
				g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
				g_theRenderer->DrawVertexArray( static_cast<int>( drugsVertsPCU.size() ), drugsVertsPCU.data() );
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerController::Possess( ActorUID const& actorToPossessUID )
{
	Controller::Possess(actorToPossessUID);
	Actor* possessedActor = m_currentMap->GetActorByUID(actorToPossessUID);
	if ( possessedActor == nullptr )
	{
		ERROR_AND_DIE("Player Controller should always possess an actor!");
	}

	if ( !m_isFreeFlyOn )
	{
		m_orientation = possessedActor->m_orientation;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerController::UpdatePlayerInput( float deltaSeconds )
{
	// Handle Mouse camera controls
	Vec2 cursorClientDelta	= g_theInput->GetCursorClientDelta();
	float mouseSpeed		= 0.05f;
	float yaw				= cursorClientDelta.x * mouseSpeed;
	float pitch				= cursorClientDelta.y * mouseSpeed;

	m_orientation.m_yawDegrees -= yaw;
	m_orientation.m_pitchDegrees += pitch;

	//----------------------------------------------------------------------------------------------------------------------
	XboxController const& controller = g_theInput->GetController(0);
	EulerAngles orientationPerFrame = EulerAngles( m_turnRate, m_turnRate, m_turnRate );

	m_orientation.GetAsVectors_XFwd_YLeft_ZUp( m_actorForward, m_actorLeft, m_actorUp );

	Actor* currentActor = GetActor();

	// Speed up speed variable
		if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
		{
//			m_currentSpeed = m_defaultSpeed;
			m_currentSpeed = currentActor->m_actorDef->m_walkSpeed;
		}
		if ( controller.WasButtonJustReleased( BUTTON_A ) )
		{
//			m_currentSpeed = m_defaultSpeed;
			m_currentSpeed = currentActor->m_actorDef->m_walkSpeed;
		}
		// Keyboard
		if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
		{
			m_currentSpeed = m_doublespeed;
			m_currentSpeed = currentActor->m_actorDef->m_runSpeed;
		}
		// Xbox controls
		if ( controller.IsButtonDown( BUTTON_A ) )
		{
//			m_currentSpeed = m_doublespeed;
			m_currentSpeed = currentActor->m_actorDef->m_runSpeed;
		}

	// Slow down speed variable
		// Keyboard
		if ( g_theInput->IsKeyDown( KEYCODE_CONTROL ) )
		{
			m_currentSpeed = m_halfspeed;
		}
		// Slow down speed variable
		if ( g_theInput->WasKeyJustReleased( KEYCODE_CONTROL ) )
		{
			m_currentSpeed = m_defaultSpeed;
		}
	
	if ( m_playerIsControllable )
	{
		// Forward
			if ( g_theInput->IsKeyDown('W') )
			{
				m_position += ( m_currentSpeed * m_actorForward * deltaSeconds );
			}
			
		// Back
			if ( g_theInput->IsKeyDown( 'S' ) )
			{
				m_position -= ( m_currentSpeed * m_actorForward * deltaSeconds );
			}

		// Left
			if ( g_theInput->IsKeyDown( 'A' ) )
			{
				m_position += ( m_currentSpeed * m_actorLeft * deltaSeconds);
			}

		// Right
			if ( g_theInput->IsKeyDown( 'D' ) )
			{
				m_position -= ( m_currentSpeed * m_actorLeft * deltaSeconds );
			}

			// Xbox movement and camera controls
			float leftStickMagnitude	= controller.GetLeftJoyStick().GetMagnitude();
			float rightStickMagnitude	= controller.GetRightJoyStick().GetMagnitude();
			Mat44 modelMatrix			= m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
			if ( leftStickMagnitude > 0.0f )
			{
				Vec3 x		= controller.GetLeftJoyStick().GetPosition().x * -modelMatrix.GetJBasis3D();
				Vec3 y		= controller.GetLeftJoyStick().GetPosition().y * modelMatrix.GetIBasis3D();
				m_velocity  = x + y;
				m_position += ( m_currentSpeed * m_velocity * deltaSeconds );
			}
			if ( rightStickMagnitude > 0.0f )
			{
				float orientationDegrees		  = controller.GetRightJoyStick().GetOrientationDegrees();
				m_orientation.m_yawDegrees	 -= rightStickMagnitude * CosDegrees( orientationDegrees ) * 3.0f;
				m_orientation.m_pitchDegrees -= rightStickMagnitude * SinDegrees( orientationDegrees ) * 3.0f;
			}
	}

	// Pitch up
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) )
	{
		m_orientation.m_pitchDegrees -= orientationPerFrame.m_pitchDegrees * m_rollSpeed * deltaSeconds;
	}

	// Pitch down
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) )
	{
		m_orientation.m_pitchDegrees += orientationPerFrame.m_pitchDegrees * m_rollSpeed * deltaSeconds;
	}

	// Yaw left
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) )
	{
		m_orientation.m_yawDegrees += orientationPerFrame.m_yawDegrees * m_rollSpeed * deltaSeconds;
	}

	// Yaw right 
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
	{
		m_orientation.m_yawDegrees -= orientationPerFrame.m_yawDegrees * m_rollSpeed * deltaSeconds;
	}

	if ( m_playerIsControllable )
	{
		// Elevate
		if ( g_theInput->IsKeyDown( 'Z' ) || controller.GetButton(LEFT_SHOULDER).m_isPressed  )
		{
			m_position.z += ( m_currentSpeed * deltaSeconds );
		}

		// De-Elevate
		if ( g_theInput->IsKeyDown( 'C' ) || controller.GetButton(RIGHT_SHOULDER).m_isPressed )
		{
			m_position.z -= ( m_currentSpeed * deltaSeconds );
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Check Mouse Button Input

//	// Render long Raycast Line
//	if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFT_MOUSE ) )
//	{
//		m_rayLength = 10.0f;
//		m_rayEnd	  = m_position + ( m_actorForward * m_rayLength );
//		m_radius	  = 0.01f;
//		m_duration  = 10.0f;
//
//		// Draw raycast 
//		DebugAddWorldArrow( m_position, m_rayEnd, m_radius, m_duration, m_color, m_color, DebugRenderMode::X_RAY );
//
//		RaycastResult3D raycastAll = m_currentMap->RaycastAll( m_position, m_actorForward, m_rayLength, m_actorIHit );
//		if ( raycastAll.m_didImpact )
//		{
//			// If raycast hit, render ImpactPoint and ImpactNormal
//			Vec3 tinyImpactNormal = ( raycastAll.m_impactPos + raycastAll.m_impactNormal * 0.2f );
//			DebugAddWorldPoint( raycastAll.m_impactPos, 0.05f, m_duration );
//			DebugAddWorldArrow( raycastAll.m_impactPos, tinyImpactNormal, 0.02f, m_duration, Rgba8::BLUE, Rgba8::BLUE );
//		}
//	}

//	//----------------------------------------------------------------------------------------------------------------------
//	// Render short Raycast Line
//	if ( g_theInput->WasKeyJustPressed( KEYCODE_RIGHT_MOUSE ) )
//	{
//		m_rayLength	= 0.25f;
//		m_rayEnd		= m_position + ( m_actorForward * m_rayLength );
//		m_radius		= 0.01f; 
//		m_duration	= 10.0f;
//		DebugAddWorldLine( m_position, m_rayEnd, m_radius, m_duration, m_color, m_color, DebugRenderMode::X_RAY );
//	}

	//----------------------------------------------------------------------------------------------------------------------
	// Clamp pitch and roll
	//m_orientation = m_orientation;
	m_orientation.m_pitchDegrees = GetClamped( m_orientation.m_pitchDegrees, -85.0f, 85.0f );
	m_orientation.m_rollDegrees  = GetClamped(  m_orientation.m_rollDegrees, -45.0f, 45.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Transform Camera
	m_worldCamera.SetTransform( m_position, m_orientation );
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerController::UpdatePossessedActorInput( float deltaSeconds )
{
	// Set up nicknames for cleaner code
	Actor* currentActor   = GetActor();	
	Vec3& currentActorPos = currentActor->m_position;

	// If actor does NOT exist, return from this 
	if ( currentActor == nullptr )
	{
		return;
	}

	// Set IJK Basis (Forward, Left, Up)
	currentActor->m_orientation.GetAsVectors_XFwd_YLeft_ZUp( m_actorForward, m_actorLeft, m_actorUp );
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp( m_playerControllerforward, m_playerControllerleft, m_playerControllerup );

	//----------------------------------------------------------------------------------------------------------------------
	// Handle Mouse camera controls
	Vec2 cursorClientDelta = g_theInput->GetCursorClientDelta();
	float mouseSpeed	= 0.05f;
	float yaw			= cursorClientDelta.x * mouseSpeed;
	float pitch			= cursorClientDelta.y * mouseSpeed;

	currentActor->m_orientation.m_yawDegrees	 -= yaw;
	m_orientation.m_yawDegrees = currentActor->m_orientation.m_yawDegrees;
	m_orientation.m_pitchDegrees += pitch;

	// Invert controls if player isOnDrugs
	if ( currentActor->m_isOnDrugs )
	{
		// Forward
		if ( g_theInput->IsKeyDown( 'S' ) )
		{
			currentActorPos += ( m_currentSpeed * m_actorForward * deltaSeconds );
		}

		// Back
		if ( g_theInput->IsKeyDown( 'W' ) )
		{
			currentActorPos -= ( m_currentSpeed * m_actorForward * deltaSeconds );
		}

		// Left
		if ( g_theInput->IsKeyDown( 'D' ) )
		{
			currentActorPos += ( m_currentSpeed * m_actorLeft * deltaSeconds );
		}

		// Right
		if ( g_theInput->IsKeyDown( 'A' ) )
		{
			currentActorPos -= ( m_currentSpeed * m_actorLeft * deltaSeconds );
		}
	}
	else
	{
		// Forward
		if ( g_theInput->IsKeyDown('W') )
		{
			currentActorPos += ( m_currentSpeed * m_actorForward * deltaSeconds );
		}
		
		// Back
		if ( g_theInput->IsKeyDown( 'S' ) )
		{
			currentActorPos -= ( m_currentSpeed * m_actorForward * deltaSeconds );
		}

		// Left
		if ( g_theInput->IsKeyDown( 'A' ) )
		{
			currentActorPos += ( m_currentSpeed * m_actorLeft * deltaSeconds );
		}

		// Right
		if ( g_theInput->IsKeyDown( 'D' ) )
		{
			currentActorPos -= ( m_currentSpeed * m_actorLeft * deltaSeconds );
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Speed up speed variable
		if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
		{
//			m_currentSpeed = m_defaultSpeed;
			m_currentSpeed = currentActor->m_actorDef->m_walkSpeed;
		}
		// Keyboard
		if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
		{
			m_currentSpeed = m_doublespeed;
			m_currentSpeed = currentActor->m_actorDef->m_runSpeed;
		}

	// Select Pistol
	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		for ( int i = 0; i < currentActor->m_weaponList.size(); i++ )
		{
			if ( currentActor->m_weaponList[i]->m_weaponDef->m_name == "Pistol" )
			{
				currentActor->m_currentWeaponIndex = i;
				currentActor->m_currentWeapon = currentActor->m_weaponList[currentActor->m_currentWeaponIndex];
				return;
			}
		}
	}

	// Select Plasma Rifle
	if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		for ( int i = 0; i < currentActor->m_weaponList.size(); i++ )
		{
			if ( currentActor->m_weaponList[i]->m_weaponDef->m_name == "PlasmaRifle" )
			{
				currentActor->m_currentWeaponIndex = i;
				currentActor->m_currentWeapon = currentActor->m_weaponList[currentActor->m_currentWeaponIndex];
				return;
			}
		}
	}

	// Select Mouse Missile
	if ( g_theInput->WasKeyJustPressed( '3' ) )
	{
		for ( int i = 0; i < currentActor->m_weaponList.size(); i++ )
		{
			if ( currentActor->m_weaponList[i]->m_weaponDef->m_name == "MissileMice" )
			{
				currentActor->m_currentWeaponIndex = i;
				currentActor->m_currentWeapon = currentActor->m_weaponList[currentActor->m_currentWeaponIndex];
				return;
			}
		}
	}

	// Next weapon
	if ( g_theInput->WasKeyJustPressed( KEYCODE_RIGHTARROW ) )
	{
		currentActor->NextWeapon();
	}

	// Previous weapon
	if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFTARROW ) )
	{
		currentActor->PreviousWeapon();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Check Mouse Button Input
	if ( g_theInput->IsKeyDown( KEYCODE_LEFT_MOUSE ) )
	{
		currentActor->Attack();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Clamp pitch and roll
	m_orientation.m_pitchDegrees = GetClamped( m_orientation.m_pitchDegrees, -85.0f, 85.0f );
	m_orientation.m_rollDegrees  = GetClamped(  m_orientation.m_rollDegrees, -45.0f, 45.0f );

	//----------------------------------------------------------------------------------------------------------------------
	// Transform Camera
	m_position = Vec3(currentActorPos.x, currentActorPos.y, currentActor->m_actorDef->m_eyeHeight);

	// Move camera position (sway) if drugEffectIsActive
	if ( m_drugEffectIsActive )
	{
		float totalSeconds  = g_theApp->m_gameClock.GetTotalSeconds();
		float sine			= SinDegrees( totalSeconds * 25.0f );				
		float cos			= SinDegrees( totalSeconds * 50.0f );
		float sineScaled	= sine * 0.05f;	// 0.75
		float cosScaled		= cos  * 0.5f;	// 0.25
		float yOffset		= m_position.y - sineScaled;
		float xOffset		= m_position.x + cosScaled;
		Vec3 drunkPos		= Vec3( xOffset, yOffset, m_position.z );
		m_worldCamera.SetTransform( drunkPos, m_orientation );
//		DebuggerPrintf( Stringf( "Drunk X = %f, Y = %f, Z = %f, sine = %f\n", drunkPos.x, drunkPos.y, drunkPos.z, sine ).c_str() );

		if ( !g_theAudio->IsPlaying( m_drunkWobbleSPBID ) )
		{
			bool is3D = true;
			m_drunkWobbleSID = g_theAudio->CreateOrGetSound( "Data/Audio/DrunkWobble.wav", is3D );
			m_drunkWobbleSPBID = g_theAudio->StartSoundAt( m_drunkWobbleSID, drunkPos );	
		}
	}
	else
	{
		m_worldCamera.SetTransform( m_position, m_orientation );
//		DebuggerPrintf( Stringf( "Sober X = %f, Y = %f, Z = %f\n", m_position.x, m_position.y, m_position.z ).c_str() );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerController::UpdateCamera()
{
	// Update Camera settings // Set eye height	// Set FOV
	Actor* possessedActor = GetActor();
	
	// Check if current possessed actor is exists
	if ( possessedActor != nullptr )
	{
		// Change playerCam FOV if drugEffectIsActive
		if ( m_drugEffectIsActive )
		{
			m_previousFOV = possessedActor->m_actorDef->m_cameraFOVDegrees;

			float maxDrunkTime = 60.0f;
			if ( m_startDrugsStopwatch )
			{
				m_drugsStopwatch = Stopwatch( maxDrunkTime );
				m_drugsStopwatch.Start();
				m_startDrugsStopwatch  = false;
			}

			float totalSeconds	= m_drugsStopwatch.GetElapsedTime();
			float timeScaled	= SinDegrees( totalSeconds * 75.0f );
			float halfPrevFOV	= m_previousFOV * 0.2f;
			m_drugsCameraFOV	= RangeMapClamped( timeScaled, 0.0f, maxDrunkTime, m_previousFOV, halfPrevFOV );
			float aspectSkewed	= ( timeScaled * -1.5f ) + 2.5f;
			m_worldCamera.SetPerspectiveView( aspectSkewed, m_drugsCameraFOV, 0.1f, 100.0f );		// #ToDo // Set aspect ration based on width / height
//			m_worldCamera.SetPerspectiveView( 2.0f, m_drugsCameraFOV, 0.1f, 100.0f );				// #ToDo // Set aspect ration based on width / height

//			DebuggerPrintf( Stringf( "aspectSkewed %f, timeScaled %f, drugsCameraFOV %f, prevFOV %f \n", aspectSkewed, timeScaled, m_drugsCameraFOV, m_previousFOV ).c_str() );

			// Reseting variables
			m_maxCamRollOnDrugs = 0.0f;
			m_drugsStopwatch.DecrementDurationIfElapsed();
		}
		else	// If drugEffect is NOT active 
		{
			m_previousFOV = possessedActor->m_actorDef->m_cameraFOVDegrees;
			m_worldCamera.SetPerspectiveView( 2.0f, m_previousFOV, 0.1f, 100.0f );			// #ToDo // Set aspect ration based on width / height
			m_maxCamRollOnDrugs = 0.0f;
			m_startDrugsStopwatch = true;
		}

		if ( possessedActor->m_actorDef->m_name == "Marine" )
		{
			m_marineFOV = possessedActor->m_actorDef->m_cameraFOVDegrees;
		}
	}
	
	m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f ), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) );
//	m_worldCamera.SetPerspectiveView( 2.0f, m_previousFOV, 0.1f, 100.0f );			// #ToDo // Set aspect ration based on width / height	

	if ( m_isFreeFlyOn )
	{
		m_worldCamera.SetPerspectiveView( 2.0f, m_marineFOV, 0.1f, 100.0f );			// #ToDo // Set aspect ration based on width / height	
	}
}

//----------------------------------------------------------------------------------------------------------------------
void PlayerController::PossessNextActor()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Possess next actor
	if ( g_theInput->WasKeyJustPressed( 'N' ) )
	{
		// Get current actor's index in actorList
		m_index = m_currentlyPossessedActorUID.GetIndex();

		// Tell the AI to re-possess the current actor 
		if ( ( m_currentMap->m_actorList[m_index] != nullptr ) && m_currentMap->m_actorList[m_index]->m_actorDef->m_aiEnabled )
		{
			m_currentMap->m_actorList[m_index]->AIPossessCurrentActor( m_currentlyPossessedActorUID );
			//			m_currentMap->m_actorList[index]->m_AIController->m_currentMap = m_currentMap;
			//			m_currentMap->m_actorList[index]->m_AIController->Possess( m_currentMap->m_actorList[index]->m_currentActorUID );
		}

		// Find next actor to possess
		while ( true )
		{
			// If we are at the end of the actorList, restart from the beginning of the list
			if ( m_index == (unsigned int)m_currentMap->m_actorList.size() - 1 )
			{
				// Toggle bool so this actor will render
				m_currentMap->m_actorList[m_index]->m_isPossessed = false;

				// Restart from beginning of the list
				m_index = 0;
			}
			else
			{
				// Toggle bool so this actor will render
				m_currentMap->m_actorList[m_index]->m_isPossessed = false;

				// Increment index to next actor in list
				m_index++;

				// Check if actor does NOT exist in current index AND index is not end of actorList
				while ( m_currentMap->m_actorList[m_index] == nullptr )
				{
					if ( m_index == m_currentMap->m_actorList.size() - 1 )
					{
						m_index = 0;
					}
					else
					{
						m_index++;
					}
				}									
			}

			// Possess the actor in the current slot if an actor exists and it is a "Demon"
			if ( (m_currentMap->m_actorList[m_index] != nullptr) && (m_currentMap->m_actorList[m_index]->m_actorDef->m_faction == "Demon") ||  
				(m_currentMap->m_actorList[m_index]->m_actorDef->m_name == "Marine") )
			{
				// If AI already exists on current actor, tell the AI to unpossess, then player can possess the current actor
				if ( m_currentMap->m_actorList[m_index]->m_AIController )
				{
					m_currentMap->m_actorList[m_index]->m_AIController->UnPossess( m_currentMap->m_actorList[m_index]->m_currentActorUID );
				}

				// Player possess current actor
				Possess( m_currentMap->m_actorList[m_index]->m_currentActorUID );
				m_currentMap->m_actorList[m_index]->m_isPossessed = true;
				return;
			}
		}
	}
}
