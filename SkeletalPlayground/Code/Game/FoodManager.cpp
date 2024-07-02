#include "Game/FoodManager.hpp"
#include "Game/GameMode3D.hpp"
#include "Game/GameMode_RobotArm_3D.hpp"
#include "Game/Quadruped.hpp"
#include "Game/Map_GameMode3D.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/VertexUtils.hpp"


//----------------------------------------------------------------------------------------------------------------------
FoodManager::FoodManager( int numFoodOrbs, GameMode3D* game )
{
	m_game1 = game;
	for ( int i = 0; i < numFoodOrbs; i++ )
	{
		Vec3	randPos		 = GetRandPos_MaxZ();
		Vec3	randGoalPos	 = GetRandPos_MaxZ();
		FoodOrb newFoodOrb	 = FoodOrb( randPos );
		newFoodOrb.m_goalPos = randGoalPos;
		m_foodList.push_back( newFoodOrb );
	}
}


//----------------------------------------------------------------------------------------------------------------------
FoodManager::FoodManager( int numFoodOrbs, GameMode_RobotArm_3D* game ) 
{
	m_game_Robot = game;
	for ( int i = 0; i < numFoodOrbs; i++ )
	{
		Vec3	randPos						= GetRandPos_MaxZ();
		Vec3	randGoalPos					= GetRandPos_MaxZ();
		FoodOrb newFoodOrb					= FoodOrb( randPos );
		newFoodOrb.m_goalPos				= randGoalPos;
		m_foodList.push_back( newFoodOrb );
	}
}


//----------------------------------------------------------------------------------------------------------------------
FoodManager::~FoodManager()
{
	m_game_Robot = nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
void FoodManager::Update( float deltaSeconds )
{
	MoveFoodOrbs( deltaSeconds );
}

//----------------------------------------------------------------------------------------------------------------------
void FoodManager::Render( std::vector<Vertex_PCU>& verts )
{
	for ( int i = 0; i < m_foodList.size(); i++ )
	{
		FoodOrb& currentFoodOrb = m_foodList[ i ];
		if ( !currentFoodOrb.m_isConsumed )
		{
			if ( currentFoodOrb.m_isGrabbed )
			{
				AddVertsForSphere3D( verts, currentFoodOrb.m_position, 2.0f, 8.0f, 8.0f, Rgba8::YELLOW );
			}
			else
			{
				AddVertsForSphere3D( verts, currentFoodOrb.m_goalPos, 2.0f, 8.0f, 8.0f, Rgba8::DARKER_RED );
			}
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
Vec3 FoodManager::GetRandPos_MaxZ()
{
//	AABB3 abb3   = m_game->m_map->m_floor_SE->m_aabb3;
	float dist   = 30.0f;
	AABB3 abb3   = AABB3( -dist, -dist, -1.0f, dist, dist, -0.5f );
	Vec3 randPos = Vec3::ZERO;
	randPos.x	 = g_theRNG->RollRandomFloatInRange( abb3.m_mins.x, abb3.m_maxs.x );
	randPos.y	 = g_theRNG->RollRandomFloatInRange( abb3.m_mins.y, abb3.m_maxs.y );
	randPos.z	 = abb3.m_maxs.z + 0.5f;
	return randPos;
}


//----------------------------------------------------------------------------------------------------------------------
void FoodManager::ConsumeFood( FoodOrb& foodToConsume )
{
	foodToConsume.m_isConsumed = true;
}


//----------------------------------------------------------------------------------------------------------------------
void FoodManager::MoveFoodOrbs( float deltaSeconds )
{
	for ( int i = 0; i < m_foodList.size(); i++ )
	{
		FoodOrb& currentFoodOrb = m_foodList[ i ];
		if ( !currentFoodOrb.m_isConsumed && !currentFoodOrb.m_isGrabbed )
		{
			// Check dist from goal pos
			float distCurPosToGoal = GetDistance3D( currentFoodOrb.m_position, currentFoodOrb.m_goalPos );
			// if close enough, generate new goal pos
			if ( distCurPosToGoal < 3.0f )
			{
				currentFoodOrb.m_prevPos	= currentFoodOrb.m_goalPos;
//				currentFoodOrb.m_goalPos	= GetRandPos_MaxZ();
				currentFoodOrb.m_isGrabbed	= true;

				// Generate bezier curve to trash can
				// Calculate curve positions
				if ( m_game_Robot != nullptr )
				{
					Vec3 const& start			= currentFoodOrb.m_prevPos;
					Vec3 const& end				= m_game_Robot->m_trashCanTopPos;
					Vec3		dispCurrentGoal	= end - start;
					Vec3		fwdNess1		= ( dispCurrentGoal * 0.33f );
					Vec3		fwdNess2		= ( dispCurrentGoal * 0.66f );
					Vec3		upNess			= 10.0f * Vec3::Z_UP;						// #ToDo: Replace ( maxLength * 0.5f ) with values from raycast approach
					Vec3		guide1			= start + fwdNess1 + upNess;
					Vec3		guide2			= start + fwdNess2 + upNess; 
					// Set curve positions
					currentFoodOrb.m_bezierCurve.m_startPos	 = start;
					currentFoodOrb.m_bezierCurve.m_guidePos1 = guide1;
					currentFoodOrb.m_bezierCurve.m_guidePos2 = guide2;
					currentFoodOrb.m_bezierCurve.m_endPos	 = end;
					currentFoodOrb.m_bezierTimer.Start();
				}
			}
			else
			{
				// if too far, lerp towards goal pos
				float fractionTowardsEnd	= deltaSeconds * 0.75f;
				currentFoodOrb.m_position	= Interpolate( currentFoodOrb.m_position, currentFoodOrb.m_goalPos, fractionTowardsEnd );
			}
		}
		else if ( currentFoodOrb.m_isGrabbed )
		{
			//----------------------------------------------------------------------------------------------------------------------
			// Update Bezier Curve "t"
			//----------------------------------------------------------------------------------------------------------------------
			float elapsedFraction = currentFoodOrb.m_bezierTimer.GetElapsedFraction();
			if ( currentFoodOrb.m_bezierTimer.HasDurationElapsed() )
			{
				// Stop Bezier if finished lerping 
				currentFoodOrb.m_bezierTimer.Stop();
				currentFoodOrb.m_isGrabbed = false;
				GetRandPos_MaxZ();
				currentFoodOrb.m_goalPos	= GetRandPos_MaxZ();
			}
			else if ( elapsedFraction > 0.0f )
			{
				// Update targetCurrentPos if Bezier is still lerping
				float smoothStepFraction	= SmoothStep5( elapsedFraction );
				Vec3  bezierLerp			= currentFoodOrb.m_bezierCurve.GetPointAtFraction( smoothStepFraction );
				currentFoodOrb.m_position	= bezierLerp;
			}
		}
	}
}