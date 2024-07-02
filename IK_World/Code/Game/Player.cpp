#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
Player::Player(Game* game)
	: Entity( game )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Creature
	//----------------------------------------------------------------------------------------------------------------------
	// Create creature root and arm
	m_root		= new IK_Segment3D( Vec3( 0.0f, 0.0f, 67.0f ), 1.0f );
	m_rightArm	= new SkeletalSystem3D( m_root->m_startPos );
	m_leftArm	= new SkeletalSystem3D( m_root->m_startPos );

	// Create specified num segments for rightArm & leftArm
	for ( int i = 0; i < m_numLimbs; i++ )
	{
		m_rightArm->CreateNewLimb( m_limbLength, EulerAngles() );
		 m_leftArm->CreateNewLimb( m_limbLength, EulerAngles() );

		 m_rightArm->m_limbList[i]->m_thickness = 0.1f;
		  m_leftArm->m_limbList[i]->m_thickness = 0.1f;
	}
}

//----------------------------------------------------------------------------------------------------------------------
Player::~Player()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Player::Update( float deltaSeconds )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Camera follows mouse
	//----------------------------------------------------------------------------------------------------------------------
	// Implement displacement.x to yaw and displacement.y to pitch
	//----------------------------------------------------------------------------------------------------------------------

	// Update Creature
	UpdateCreatureRootPosInput( deltaSeconds );
	UpdateCreatureHeight( deltaSeconds );
	UpdateCreature( deltaSeconds );
	DetermineBestStepPos();

	m_position = m_root->m_startPos;

	// Update Camera
	UpdateGameMode3DCamera();
	TurnCreatureTowardsCameraDir();
}

//----------------------------------------------------------------------------------------------------------------------
void Player::Render() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render player compass
	std::vector<Vertex_PCU> compassVerts;
	int estimatedNumVerts		= 120; // Estimated numVerts of Compass
	compassVerts.reserve( estimatedNumVerts );
	Mat44 playerMatrix			= m_worldCamera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 playerForward			= playerMatrix.GetIBasis3D();
	float distFromCam			= 0.2f;

	Vec3 playerCenter			= m_worldCamera.m_position + ( playerForward * distFromCam );
	float playerCompassLength	= 0.01f;
	float playerAxisThickness	= 0.001f;
	AddVertsForCompass( compassVerts, playerCenter, playerCompassLength, playerAxisThickness );

	//----------------------------------------------------------------------------------------------------------------------
	// RenderCreature
	std::vector<Vertex_PCU> verts;
	// Root
	AddVertsForSphere3D( verts, m_root->m_startPos, 0.1f, 16.0f, 16.0f, Rgba8::MAGENTA );
	// Arms 
	m_rightArm->Render( verts, Rgba8::CYAN, Rgba8::DARK_RED );
	 m_leftArm->Render( verts, Rgba8::CYAN, Rgba8::DARK_RED );
	AddVertsForSphere3D( verts, m_rightArmGoalPos, 0.2f, 8.0f, 16.0f, Rgba8::DARK_RED );
	AddVertsForSphere3D( verts,  m_leftArmGoalPos, 0.2f, 8.0f, 16.0f, Rgba8::PURPLE   );
	// Orientation
	Vec3 rootFwdDir = m_root->m_orientation.GetForwardDir_XFwd_YLeft_ZUp();
	AddVertsForArrow3D( verts, m_root->m_startPos, m_root->m_startPos + ( rootFwdDir * 2.0f ), 0.1f, Rgba8::DARK_BLUE );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for Compass
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( compassVerts.size() ), compassVerts.data() );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw Call for Player
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void Player::UpdateGameMode3DCamera()
{
	Vec2 cursorClientDelta	= g_theInput->GetCursorClientDelta();
	float mouseSpeed		= 0.05f;
	float yaw				= cursorClientDelta.x * mouseSpeed;
	float pitch				= cursorClientDelta.y * mouseSpeed;

	// Set cameraPos to stay attached to playerPos
	Vec3 camPos				= m_root->m_startPos - ( m_worldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp() * 10.0f );
	m_worldCamera.SetTransform( camPos, m_worldCamera.m_orientation );
	// Set cameraOrientation
	m_worldCamera.m_orientation.m_yawDegrees	-= yaw;
	m_worldCamera.m_orientation.m_pitchDegrees	+= pitch;
	m_worldCamera.m_orientation.m_pitchDegrees	= GetClamped( m_worldCamera.m_orientation.m_pitchDegrees, -85.0f, 85.0f );
}

//----------------------------------------------------------------------------------------------------------------------
void Player::AddVertsForCompass( std::vector<Vertex_PCU>& compassVerts, Vec3 startPosition, float axisLength, float axisThickness ) const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render stationary world compass
	// X
	Vec3 endPosX = startPosition + Vec3( axisLength, 0.0f, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosX, axisThickness, Rgba8::RED );
	// Y
	Vec3 endPosY = startPosition + Vec3( 0.0f, axisLength, 0.0f );
	AddVertsForArrow3D( compassVerts, startPosition, endPosY, axisThickness, Rgba8::GREEN );
	// Z
	Vec3 endPosZ = startPosition + Vec3( 0.0f, 0.0f, axisLength );
	AddVertsForArrow3D( compassVerts, startPosition, endPosZ, axisThickness, Rgba8::BLUE );
}

//----------------------------------------------------------------------------------------------------------------------
void Player::UpdatePlayerInput( float deltaSeconds )
{
	deltaSeconds = Clock::GetSystemClock().GetDeltaSeconds();

	XboxController const& controller = g_theInput->GetController(0);
	EulerAngles orientationPerFrame = EulerAngles( m_turnRate, m_turnRate, m_turnRate );

	Vec3 forward;
	Vec3 left;
	Vec3 up;

	m_orientationDegrees.GetAsVectors_XFwd_YLeft_ZUp( forward, left, up );
	forward.z	= 0.0f;
	left.z		= 0.0f;
	forward		= forward.GetNormalized();
	left		= left.GetNormalized();

	// Remove block directly under player
//	if ( g_theInput->IsKeyDown( KEYCODE_LEFT_MOUSE ) )
//	{
//		g_theGame->m_currentWorld->ChangeBlockTypeAtOrBelowPlayerToAir();
//	}
//	// Add block directly under player + IntVec3( 0, 0, 1 )
//	if ( g_theInput->IsKeyDown( KEYCODE_RIGHT_MOUSE ) )
//	{
//
//	}

	// Increase speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) || g_theInput->IsKeyDown( ' ' ) || controller.GetButton(BUTTON_A).m_isPressed )
	{
		m_currentSpeed = m_fasterSpeed;
	}
	// Reset speed variable to default
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) ||  g_theInput->WasKeyJustReleased( ' ' ) || ( controller.GetButton(BUTTON_A).m_wasPressedLastFrame ) )
	{
		m_currentSpeed = m_defaultSpeed;
	}

	// Decrease speed variable
	if ( g_theInput->IsKeyDown( KEYCODE_CONTROL ) )
	{
		m_currentSpeed = m_slowerSpeed;
	}

	// Reset speed variable to default
	if ( g_theInput->WasKeyJustReleased( KEYCODE_CONTROL ) )
	{
		m_currentSpeed = m_defaultSpeed;
	}
	
	// Forward
	if ( g_theInput->IsKeyDown('W') || controller.GetLeftJoyStick().GetPosition().y > 0.0f )
	{
		m_position += ( m_currentSpeed * forward * deltaSeconds );
	}

	// Back
	if ( g_theInput->IsKeyDown( 'S' ) ||  controller.GetLeftJoyStick().GetPosition().y < 0.0f )
	{
		m_position -= ( m_currentSpeed * forward * deltaSeconds );
	}

	// Left
	if ( g_theInput->IsKeyDown( 'A' ) ||  controller.GetLeftJoyStick().GetPosition().x < 0.0f )
	{
		m_position += ( m_currentSpeed * left * deltaSeconds);
	}

	// Right
	if ( g_theInput->IsKeyDown( 'D' ) ||  controller.GetLeftJoyStick().GetPosition().x > 0.0f )
	{
		m_position -= ( m_currentSpeed * left * deltaSeconds );
	}

	// Pitch up
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) || controller.GetRightJoyStick().GetPosition().y > 0.0f )
	{
		m_angularVelocity.m_pitchDegrees -= orientationPerFrame.m_pitchDegrees * deltaSeconds;
	}

	// Pitch down
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) || controller.GetRightJoyStick().GetPosition().y < 0.0f )
	{
		m_angularVelocity.m_pitchDegrees += orientationPerFrame.m_pitchDegrees * deltaSeconds;

	}
	// Yaw left
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) || controller.GetRightJoyStick().GetPosition().x < 0.0f )
	{
		m_angularVelocity.m_yawDegrees += orientationPerFrame.m_yawDegrees * deltaSeconds;
	}

	// Yaw right 
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW || controller.GetRightJoyStick().GetPosition().x > 0.0f ) )
	{
		m_angularVelocity.m_yawDegrees -= orientationPerFrame.m_yawDegrees * deltaSeconds;
	}

	// Elevate
	if ( g_theInput->IsKeyDown( 'Z' )	|| 
		 g_theInput->IsKeyDown( 'E' )	|| 
		controller.GetButton(LEFT_SHOULDER).m_isPressed  )
	{
		m_position.z += ( m_currentSpeed * deltaSeconds );
	}

	// De-Elevate
	if ( g_theInput->IsKeyDown( 'C' ) || 
		 g_theInput->IsKeyDown( 'Q' ) || 
		 controller.GetButton(RIGHT_SHOULDER).m_isPressed )
	{
		m_position.z -= ( m_currentSpeed * deltaSeconds );
	}
	m_orientationDegrees = m_angularVelocity;
	m_orientationDegrees.m_pitchDegrees = GetClamped( m_orientationDegrees.m_pitchDegrees, -85.0f, 85.0f );
	m_orientationDegrees.m_rollDegrees  = GetClamped(  m_orientationDegrees.m_rollDegrees, -45.0f, 45.0f );

	// Reset to world center
	if ( g_theInput->WasKeyJustPressed( 'H' ) || controller.GetButton(BUTTON_START).m_isPressed )
	{
		// Reset Position
		m_position	= Vec3( 0.0f, 0.0f, 0.0f );

		forward		= Vec3( 1.0f, 0.0f, 0.0f );
		left		= Vec3( 0.0f, 1.0f, 0.0f );
		up			= Vec3( 0.0f, 0.0f, 1.0f );

		// Reset Orientation
		m_orientationDegrees.m_yawDegrees	= 0.0f;
		m_orientationDegrees.m_pitchDegrees = 0.0f;
		m_orientationDegrees.m_rollDegrees	= 0.0f;
	}
		m_angularVelocity = m_orientationDegrees;
}

//----------------------------------------------------------------------------------------------------------------------
void Player::UpdateCreature( float deltaSeconds )
{
	// Ensure arm positions stay "attached" to the creature's root pos
	m_rightArm->m_position = m_root->m_startPos;
	m_leftArm->m_position = m_root->m_startPos;

	// Lerp creature end effector positions towards goal positions
	float fractionTowardsEnd	= 0.1f;
	fractionTowardsEnd		   += ( deltaSeconds );
	m_rightArmEndEffectorPos   = Interpolate(  m_rightArmEndEffectorPos,  m_rightArmGoalPos, fractionTowardsEnd );
	m_leftArmEndEffectorPos    = Interpolate(   m_leftArmEndEffectorPos,   m_leftArmGoalPos, fractionTowardsEnd );

	// Have arms reach out to respective targPos
	m_rightArm->ReachTargetPos_FABRIK( m_rightArmEndEffectorPos  );
	 m_leftArm->ReachTargetPos_FABRIK( m_leftArmEndEffectorPos   );
}

//----------------------------------------------------------------------------------------------------------------------
void Player::UpdateCreatureRootPosInput( float deltaSeconds )
{
	Vec3 iBasis, jBasis, kBasis;
	m_worldCamera.m_orientation.GetAsVectors_XFwd_YLeft_ZUp( iBasis, jBasis, kBasis );
	iBasis.z = 0.0f;
	jBasis.z = 0.0f;
	iBasis	 = iBasis.GetNormalized(); 
	jBasis	 = jBasis.GetNormalized(); 
//	kBasis   = kBasis.GetNormalized();

	//----------------------------------------------------------------------------------------------------------------------
	// All directions are local
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_SHIFT ) )
	{
		m_currentSpeed = m_fasterSpeed;
	}
	if ( g_theInput->WasKeyJustReleased( KEYCODE_SHIFT ) )
	{
		m_currentSpeed = m_defaultSpeed;
	}
	// Left
	if ( g_theInput->IsKeyDown( 'A' ) )
	{
		m_root->m_startPos += ( ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds );
	}
	// Right
	if ( g_theInput->IsKeyDown( 'D' ) )
	{
		m_root->m_startPos -= ( ( Vec3( jBasis.x, jBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds );
	}
	// Forward
	if ( g_theInput->IsKeyDown( 'W' ) )
	{
		m_root->m_startPos += ( ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds );
	}
	// Backwards
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_root->m_startPos -= ( ( Vec3( iBasis.x, iBasis.y, 0.0f ) * m_currentSpeed ) * deltaSeconds );
	}
	// Sky (+Z)
	if ( g_theInput->IsKeyDown( 'E' ) )
	{
		m_root->m_startPos += ( ( kBasis * m_currentSpeed ) * deltaSeconds );
//		m_gameMode3DWorldCamera.m_orientation.m_rollDegrees++;
	}
	// Ground (-Z)
	if ( g_theInput->IsKeyDown( 'Q' ) )
	{
		m_root->m_startPos -= ( ( kBasis * m_currentSpeed ) * deltaSeconds );
//		m_gameMode3DWorldCamera.m_orientation.m_rollDegrees--;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Player::UpdateCreatureHeight( float deltaSeconds )
{
	float avgHeight		 = ( m_rightArmEndEffectorPos.z + m_leftArmEndEffectorPos.z ) * 0.5f;
	float goalHeightZ	 = avgHeight + m_rootDefaultHeightZ;

	// Lerp from currentRootPos to goalPos
//	float fractionTowardsEnd  = 0.01f;
	float fractionTowardsEnd  = 1.0f;
	fractionTowardsEnd		 += deltaSeconds;
	m_root->m_startPos.z	  = Interpolate( m_root->m_startPos.z, goalHeightZ, fractionTowardsEnd );
}

//----------------------------------------------------------------------------------------------------------------------
void Player::DetermineBestStepPos()
{
	// Right Arm
	if ( IsLimbIsTooFarFromRoot( m_rightArm, m_rightArmEndEffectorPos ) )
	{
/*
		float randFwd  = g_theRNG->RollRandomFloatInRange(  30.0f,  35.0f );
		float randLeft = g_theRNG->RollRandomFloatInRange( -15.0f, -20.0f );
		SpecifyTargetPos( m_rightArmGoalPos, randFwd, randLeft );
*/
//		SpecifyTargetPos( m_rightArmGoalPos, 30.0f, -15.0f );
		SpecifyTargetPos( m_rightArmGoalPos,  2.0f,  -2.0f );
	}
	// Left Arm
	if ( IsLimbIsTooFarFromRoot( m_leftArm, m_leftArmEndEffectorPos ) )
	{
/*
		float randFwd  = g_theRNG->RollRandomFloatInRange(  30.0f,  35.0f );
		float randLeft = g_theRNG->RollRandomFloatInRange(  15.0f,  20.0f );
		SpecifyTargetPos( m_leftArmGoalPos, randFwd , ran1dLeft);
*/
//		SpecifyTargetPos( m_leftArmGoalPos, 30.0f, 15.0f );
		SpecifyTargetPos( m_leftArmGoalPos,  2.0f,  2.0f );
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool Player::IsLimbIsTooFarFromRoot( SkeletalSystem3D* currentLimb, Vec3 footTargetPos )
{
	// Check if limb is placed too far from Root
	float maxLimbLength		= ( currentLimb->m_limbList.size() * m_limbLength ) * 0.75f;
	float distFootPosToRoot = GetDistance3D( footTargetPos, m_root->m_startPos );
	if ( distFootPosToRoot > maxLimbLength )
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool Player::DoesTargetPosOverlapSolidObject( Vec3& footTargetPos )
{
	// Check if targetPos is on a walkable object
	// Option One
		// Raycast down from rootHeight to targetPos to check if it is a walkable object
		// True: step
		// False: do nothing
	// Option Two
		// Check if targetPos is inside walkable object ( 2D, XY plane )
		// True: Get height of object, set the Z to footTargetPos, then step
		// False: Do nothing

	// Get highest non air block.max.z position
	// Set footTargetPos.z to highestBlockZ

	IntVec3 localBlockCoords	= m_game->m_currentWorld->GetLocalBlockCoordsFromWorldPos( Vec3( footTargetPos.x, footTargetPos.y, CHUNK_MAX_INDEX_Z ) );
	int currentBlockIndex		= m_game->m_currentWorld->GetBlockIndexFromLocalBlockCoords( localBlockCoords );
	IntVec2 currentChunkCoords	= m_game->m_currentWorld->GetChunkCoordsFromWorldPos( Vec2( m_root->m_startPos.x, m_root->m_startPos.y ) );
	Chunk* currentChunk			= m_game->m_currentWorld->GetChunkAtCoords( currentChunkCoords ); 
	
	if (						currentChunk == nullptr 
			|| currentChunk->m_northNeighbor == nullptr 
			|| currentChunk->m_southNeighbor == nullptr 
			|| currentChunk->m_eastNeighbor  == nullptr 
			|| currentChunk->m_westNeighbor  == nullptr 
		)
	{
		return false;
	}

	BlockIterator blockIter		= BlockIterator( currentChunk, currentBlockIndex );
	// Check if currentBlock is Solid
	while ( true )
	{
		// Break out of infinite loop if currentBlockIndex is negative
		if ( currentBlockIndex < 0 )
		{
			break;
		}

		// Start looping from the top of the chunk, downwards on Z
		Block& currentBlock				= currentChunk->m_blockList[currentBlockIndex];
		if ( currentBlock.GetBlockDef().m_isSolid )
		{
			BlockIterator currentBlockIter = BlockIterator( currentChunk, currentBlockIndex );
			IntVec3 worldBlockCoords	= m_game->m_currentWorld->GetWorldBlockCoordsFromBlockIter( currentBlockIter );
			footTargetPos.z				= float( worldBlockCoords.z ) + 1.0f;
			return true;
		}

		// Go downwards on Z axis
		currentBlockIndex -= CHUNK_BLOCKS_PER_LAYER;
	}

//	AABB3 nearestBox3D		= m_blockList[0].m_aabb3;
//	bool isRootOnThisFloor	= false;
//	for ( int i = 0; i < m_blockList.size(); i++ )
//	{
//		// Get nearest floor
//		Vec2  footTargetPos2D	= Vec2( footTargetPos.x, footTargetPos.y );
//		Vec2  mins				= Vec2( m_blockList[i].m_aabb3.m_mins.x, m_blockList[i].m_aabb3.m_mins.y );
//		Vec2  maxs				= Vec2( m_blockList[i].m_aabb3.m_maxs.x, m_blockList[i].m_aabb3.m_maxs.y );
//		AABB2 currentFloorAABB2 = AABB2( mins, maxs );
//		isRootOnThisFloor		= IsPointInsideAABB2D( footTargetPos2D, currentFloorAABB2 );	
//		if ( isRootOnThisFloor )
//		{
//			// Set new footPos Z height 
//			footTargetPos.z = m_blockList[i].m_aabb3.m_maxs.z;
//			return true;
//		}
//	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------
void Player::SpecifyTargetPos( Vec3& targetPos, float fwdStepAmount, float leftStepAmount )
{
	Vec3 rootFwdDir		= m_root->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
	Vec3 rootLeftDir	= m_root->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetJBasis3D();
	Vec3 newPos			= Vec3( m_root->m_startPos.x, m_root->m_startPos.y, 0.0f ) + ( rootFwdDir * fwdStepAmount ) + ( rootLeftDir * leftStepAmount );

	bool targetPosOverlapWalkableObject = DoesTargetPosOverlapSolidObject( newPos );
	if ( targetPosOverlapWalkableObject )
	{
		// Set to normal "next step" foot placement position
		targetPos = newPos;
	}
	else
	{
		// Since normal "next step" position is invalid, find better footPlacement position
		while ( targetPosOverlapWalkableObject == false )
		{
			if ( fwdStepAmount <= 0.0f )
			{
				break;
			}

			fwdStepAmount					-= 0.1f;
//			leftStepAmount					-= 0.01f;
			Vec3 fwdStepVector				 = rootFwdDir  * fwdStepAmount; 
//			Vec3 leftStepVector				 = rootLeftDir * leftStepAmount;
//			newPos							-= fwdStepVector + leftStepVector;
			newPos							-= fwdStepVector;
			targetPosOverlapWalkableObject	 = DoesTargetPosOverlapSolidObject( newPos );
		}

		targetPos = newPos;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Player::TurnCreatureTowardsCameraDir()
{
	Vec3 camFwdDir						= m_worldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp();
	float goalDegrees					= Atan2Degrees( camFwdDir.y, camFwdDir.x );
	float rootYaw						= m_root->m_orientation.m_yawDegrees;
	float yawAngle						= GetTurnedTowardDegrees( rootYaw, goalDegrees, 2.0f );
	m_root->m_orientation.m_yawDegrees	= yawAngle;
}
