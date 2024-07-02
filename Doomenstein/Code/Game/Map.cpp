#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/AIController.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/EngineCommon.hpp"   

//----------------------------------------------------------------------------------------------------------------------
Map::Map( MapDefinition const* mapDef, PlayerController* player ) :
	m_mapDef( mapDef ),
	m_dimensions( mapDef->m_image->GetDimensions() ),
	m_player( player )
{
	m_player->m_currentMap = this;

	int dimensionSize = m_dimensions.x * m_dimensions.y;
	m_tiles.resize( dimensionSize );
	
	SpriteSheet currentSpriteSheet = SpriteSheet( *m_mapDef->m_spriteSheetTexture, m_mapDef->m_spriteSheetCellCount );

	//----------------------------------------------------------------------------------------------------------------------
	// Create map
	for ( int tileIndex = 0; tileIndex < dimensionSize; tileIndex++ )
	{
		// Get color for tile 
		IntVec2 tileCoords					= GetTileCoordsForTileIndex( tileIndex );
		Rgba8 texelColor					= m_mapDef->m_image->GetTexelColor( tileCoords );
		TileDefinition const* localTileDef	= TileDefinition::GetTileDefByColor( texelColor );
		m_tiles[tileIndex].m_tileDef		= *localTileDef;
		
		// Calculate tile bounds
		Tile curentTile;
		curentTile.m_bounds.m_mins = Vec3(		  static_cast<float>(tileCoords.x),		   static_cast<float>(tileCoords.y), 0.0f );
		curentTile.m_bounds.m_maxs = Vec3( static_cast<float>(tileCoords.x + 1.0f), static_cast<float>(tileCoords.y + 1.0f), 1.0f );
		
		// Floor bounds
		Vec3 WSB = curentTile.m_bounds.m_mins;																					//	BL
		Vec3 ESB = Vec3( curentTile.m_bounds.m_maxs.x, curentTile.m_bounds.m_mins.y, curentTile.m_bounds.m_mins.z );			//	BR
		Vec3 ENB = Vec3( curentTile.m_bounds.m_maxs.x, curentTile.m_bounds.m_maxs.y, curentTile.m_bounds.m_mins.z );			//	TR
		Vec3 WNB = Vec3( curentTile.m_bounds.m_mins.x, curentTile.m_bounds.m_maxs.y, curentTile.m_bounds.m_mins.z );			//	TL

		// Ceiling bounds
		Vec3 WST = Vec3( curentTile.m_bounds.m_mins.x, curentTile.m_bounds.m_mins.y, curentTile.m_bounds.m_maxs.z );			//	BL															
		Vec3 EST = Vec3( curentTile.m_bounds.m_maxs.x, curentTile.m_bounds.m_mins.y, curentTile.m_bounds.m_maxs.z );			//	BR	
		Vec3 ENT = Vec3( curentTile.m_bounds.m_maxs.x, curentTile.m_bounds.m_maxs.y, curentTile.m_bounds.m_maxs.z );			//	TR	
		Vec3 WNT = Vec3( curentTile.m_bounds.m_mins.x, curentTile.m_bounds.m_maxs.y, curentTile.m_bounds.m_maxs.z );			//	TL	

		AABB3 tileBounds = AABB3( curentTile.m_bounds.m_mins, curentTile.m_bounds.m_maxs );
		m_tiles[tileIndex].m_bounds = tileBounds;

		// If there is a floor in this tile
		if ( localTileDef->m_floorSpriteCoords != IntVec2( -1, -1 ) )
		{
			// Calculate sprite UVs
			int spriteIndex = ( (localTileDef->m_floorSpriteCoords.y * m_mapDef->m_spriteSheetCellCount.x) + localTileDef->m_floorSpriteCoords.x );
			AABB2 spriteUVs = currentSpriteSheet.GetSpriteUVs( spriteIndex );

			AddVertsForQuad3D( m_vertexList, m_indexList, WSB, ESB, ENB, WNB, Rgba8::WHITE, spriteUVs );
		}

		// If there is a ceiling in this tile
		if ( localTileDef->m_ceilingSpriteCoords != IntVec2( -1, -1 ) )
		{
			// Calculate sprite UVs
			int spritIndex	= ( (localTileDef->m_ceilingSpriteCoords.y * m_mapDef->m_spriteSheetCellCount.x) + localTileDef->m_ceilingSpriteCoords.x );
			AABB2 spriteUVs = currentSpriteSheet.GetSpriteUVs( spritIndex );

			AddVertsForQuad3D( m_vertexList, m_indexList, EST, WST, WNT, ENT, Rgba8::WHITE, spriteUVs );
		}

		// If there is a wall in this tile
		if ( localTileDef->m_wallSpriteCoords != IntVec2( -1, -1 ) )
		{
			// Calculate sprite UVs
			int spritIndex = ( ( localTileDef->m_wallSpriteCoords.y * m_mapDef->m_spriteSheetCellCount.x ) + localTileDef->m_wallSpriteCoords.x );
			AABB2 spriteUVs = currentSpriteSheet.GetSpriteUVs( spritIndex );

			AddVertsForQuad3D( m_vertexList, m_indexList, ESB, ENB, ENT, EST, Rgba8::WHITE, spriteUVs );			// East Face
			AddVertsForQuad3D( m_vertexList, m_indexList, WNB, WSB, WST, WNT, Rgba8::WHITE, spriteUVs );			// West Face
			AddVertsForQuad3D( m_vertexList, m_indexList, ENB, WNB, WNT, ENT, Rgba8::WHITE, spriteUVs );			// North Face
			AddVertsForQuad3D( m_vertexList, m_indexList, WSB, ESB, EST, WST, Rgba8::WHITE, spriteUVs );			// South Face
		}
	}

	m_vertexBuffer = g_theRenderer->CreateVertexBuffer( m_vertexList.size(), sizeof(Vertex_PCUTBN) );
	m_indexBuffer  = g_theRenderer->CreateIndexBuffer( m_indexList.size() );

//	g_theRenderer->Copy_CPU_To_GPU( m_vertexList.data(),   sizeof(Vertex_PCU) * m_vertexList.size(), m_vertexBuffer, sizeof( Vertex_PCU ) );
	g_theRenderer->Copy_CPU_To_GPU( m_vertexList.data(),  sizeof(Vertex_PCUTBN) * m_vertexList.size(), m_vertexBuffer, sizeof(Vertex_PCUTBN) );
	g_theRenderer->Copy_CPU_To_GPU(  m_indexList.data(), sizeof(unsigned int) *  m_indexList.size(), m_indexBuffer  );

	//----------------------------------------------------------------------------------------------------------------------
	// Spawn Actor
	for ( int i = 0; i < m_mapDef->m_spawnInfoList.size(); i++ )
	{
		SpawnInfo currentSpawnInfo;
		currentSpawnInfo.m_actorName		= m_mapDef->m_spawnInfoList[i].m_actorName; 
		currentSpawnInfo.m_actorPosition	= m_mapDef->m_spawnInfoList[i].m_actorPosition; 
		currentSpawnInfo.m_actorOrientation = m_mapDef->m_spawnInfoList[i].m_actorOrientation; 
		SpawnActor( currentSpawnInfo );
	}

	// Spawn Player
	SpawnPlayer();
	m_player->m_currentMap = this;

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Audio
	m_missileMiceBounceSID	= g_theAudio->CreateOrGetSound( "Data/Audio/EnemyShoot.wav", true );
	m_demonSightSID			= g_theAudio->CreateOrGetSound( "Data/Audio/DemonSight.wav", true );
}

//----------------------------------------------------------------------------------------------------------------------
Map::~Map()
{
	delete m_vertexBuffer;	
	delete m_indexBuffer;	

	m_vertexBuffer = nullptr;	
	m_indexBuffer  = nullptr;	

	for ( int i = 0; i < m_actorList.size(); i++ )
	{
		delete m_actorList[i];
		m_actorList[i] = nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	for ( int i = 0; i < m_actorList.size(); i++ )
	{	
		if ( m_actorList[i] == nullptr )
		{
			continue;
		}

		// Player check
		if ( m_actorList[i] != nullptr &&						// If current actor exists
			m_actorList[i]->m_currentActorUID.IsValid() )		// If current actorUID is valid
		{
			m_actorList[i]->Update( deltaSeconds );
//			m_actorList[i]->m_currentController->Update( deltaSeconds );
		}

		// AI check
		if ( m_actorList[i] != nullptr &&						// If current actor exists
			 m_actorList[i]->m_AIController != nullptr &&		// If current actor has an AI
			 m_actorList[i]->m_currentActorUID.IsValid() )		// If current actorUID is valid
		{
			m_actorList[i]->Update( deltaSeconds );
			m_actorList[i]->m_AIController->Update( deltaSeconds );
		}

		// Mouse Missile Check
		if ( m_actorList[i] != nullptr &&
			m_actorList[i]->m_actorDef->m_isProjectileActor )
		{
			m_actorList[i]->UpdateExplosiveProjectileActors();
			// Update audio 3D position
			if ( g_theAudio->IsPlaying( m_actorList[i]->m_demonPoopExplodeSPBID ) )
			{
				g_theAudio->SetSoundPosition( m_actorList[i]->m_demonPoopExplodeSPBID, m_actorList[i]->m_position );
			}
			if ( g_theAudio->IsPlaying( m_actorList[i]->m_missileMiceExplodeSPBID ) )
			{
				g_theAudio->SetSoundPosition( m_actorList[i]->m_missileMiceExplodeSPBID, m_actorList[i]->m_position );
			}

		}
	}

	CollideActors();
	CollideActorsWithMap();

	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		m_isRenderBillboard = !m_isRenderBillboard;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::Render() const
{
	Vec3 sunDir	= m_sunDirection.GetNormalized();
	g_theRenderer->SetLightingConstants( sunDir, m_sunIntensity, m_ambientIntensity );
	
// 	DebugAddWorldArrow( Vec3::ZERO, Vec3( 10.0f, 0.0f, 0.0f ), 1.0f, 0.0f, Rgba8::RED,	 Rgba8::RED	  );
// 	DebugAddWorldArrow( Vec3::ZERO, Vec3( 0.0f, 10.0f, 0.0f ), 1.0f, 0.0f, Rgba8::GREEN, Rgba8::GREEN );
// 	DebugAddWorldArrow( Vec3::ZERO, Vec3( 0.0f, 0.0f, 10.0f ), 1.0f, 0.0f, Rgba8::BLUE,	 Rgba8::BLUE  );

	// Draw call
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( m_mapDef->m_spriteSheetTexture );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader( m_mapDef->m_shader );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->DrawVertexAndIndexBuffer( m_vertexBuffer, m_indexBuffer, (int)m_indexList.size() );
	g_theRenderer->BindShader( nullptr );

	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < m_actorList.size(); i++ )
	{
		if ( m_actorList[i] != nullptr )
		{
			m_actorList[i]->Render();
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::EndFrame()
{
	// Check if m_isDead && m_isGarbage
	// If true, delete and null
	for ( int i = 0; i < m_actorList.size(); i++ )
	{
		if ( m_actorList[i] != nullptr )
		{
			if ( m_actorList[i]->m_isDead && m_actorList[i]->m_isGarbage )
			{
				delete m_actorList[i];
				m_actorList[i] = nullptr;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
IntVec2 Map::GetTileCoordsForTileIndex( int tileIndex )
{
	IntVec2 tile;
	tile.x = tileIndex % m_dimensions.x;
	tile.y = tileIndex / m_dimensions.x;
	return tile;
}

//----------------------------------------------------------------------------------------------------------------------
int Map::GetTileIndexForTileCoords( int tileX, int tileY ) const
{
	return ( tileY * m_dimensions.x ) + tileX;
}

//----------------------------------------------------------------------------------------------------------------------
IntVec2 Map::GetTileCoordsForWorldPos( Vec2 position )
{
	return IntVec2( RoundDownToInt( position.x ), RoundDownToInt( position.y ) );
}

//----------------------------------------------------------------------------------------------------------------------
bool Map::IsPositionInBounds( Vec3 position, float const tolerance ) const
{
	UNUSED( tolerance );

	bool isXInBounds = position.x >= 0.0f && position.x <= m_dimensions.x;
	bool isYInBounds = position.y >= 0.0f && position.y <= m_dimensions.y;
	bool isZInBounds = position.z >= 0.0f && position.z <= 1.0f;

	if ( isXInBounds && isYInBounds && isZInBounds )
	{
		return true;
	}
	else
	{
		return false;
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool Map::IsOutOfBoundsXY( IntVec2 tileCoords ) const
{
	// Return if tileCoord is greater than Max or less than min
	return ( tileCoords.x >= m_dimensions.x || tileCoords.y >= m_dimensions.y || tileCoords.x < 0 || tileCoords.y < 0 );
}

//----------------------------------------------------------------------------------------------------------------------
Tile* const Map::GetTile( int x, int y ) const
{
	UNUSED( x );
	UNUSED( y );
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void Map::CollideActors()
{
	for ( int i = 0; i < (m_actorList.size() - 1); i++ )
	{
		if ( (m_actorList[i] == nullptr) || (!m_actorList[i]->m_actorDef->m_collidesWithActors) || m_actorList[i]->m_isDead )
		{
			continue;
		}

		for ( int j = i + 1; j < m_actorList.size(); j++ )
		{
			// Don't check collision for the same actor against itself
			if ( (m_actorList[j] == nullptr) ||								// If the actor exists
				 (!m_actorList[j]->m_actorDef->m_collidesWithActors) )		// If actor collides with other actors
			{
				continue;
			}
			
			if ( !m_actorList[j]->m_isDead )
			{
				CollideActors( m_actorList[j], m_actorList[i] );
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::CollideActors( Actor* actorA, Actor* actorB )
{
	float actorAMins = actorA->m_position.z;
	float actorAMaxs = actorAMins + actorA->m_physicsHeight;
	float actorBMins = actorB->m_position.z;
	float actorBMaxs = actorBMins + actorB->m_physicsHeight;

	if ( (actorAMins >= actorBMins && actorAMins <= actorBMaxs) || 				// If actorA.z mins and maxs overlaps with actorB.z mins and maxs, push discs 
		 (actorAMaxs >= actorBMins && actorAMaxs <= actorBMaxs) )				
	{
		Vec2 actorAPosVec2 = Vec2( actorA->m_position.x, actorA->m_position.y );
		Vec2 actorBPosVec2 = Vec2( actorB->m_position.x, actorB->m_position.y );
//		PushDiscOutOfFixedDisc2D( actorAPosVec2, actorA->m_physicsRadius, actorBPosVec2, actorB->m_physicsRadius );
		PushDiscsOutOfEachOther2D( actorAPosVec2, actorA->m_physicsRadius, actorBPosVec2, actorB->m_physicsRadius );
		actorA->m_position.x = actorAPosVec2.x;
		actorA->m_position.y = actorAPosVec2.y;
		actorB->m_position.x = actorBPosVec2.x;
		actorB->m_position.y = actorBPosVec2.y;

		//----------------------------------------------------------------------------------------------------------------------
		// Check if distance between demons and projectiles are in range
		// Tell colliding Actor (Demon) to take damage
		if ( ( (actorA->m_actorDef->m_isProjectileActor) && (actorB->m_actorDef->m_aiEnabled) ) ||
			 ( (actorB->m_actorDef->m_isProjectileActor) && (actorA->m_actorDef->m_aiEnabled) ) )  
		{
			if ( actorA->m_isDead || actorB->m_isDead )
			{
				return;
			}

			if ( DoDiscsOverlap(actorAPosVec2, actorA->m_actorDef->m_physicsRadius, actorBPosVec2, actorB->m_actorDef->m_physicsRadius) )
			{
				if ( actorA->m_owner == actorB || actorB->m_owner == actorA )
				{
					return;
				}
				actorA->OnCollide( actorB );
				actorB->OnCollide( actorA );
//				float randDamage = g_theRNG->RollRandomFloatInRange( actorA->m_actorDef->m_damageOnCollide.m_min, actorA->m_actorDef->m_damageOnCollide.m_max );
//				Actor* possessedActor = m_player->GetActor();
//				actorB->TakeDamage( randDamage, possessedActor );
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::CollideActorsWithMap()
{
	for ( int i = 0; i < m_actorList.size(); i++ )
	{
		if ( m_actorList[i] == nullptr || !m_actorList[i]->m_actorDef->m_collidesWithWorld )
		{
			continue;
		}

		CollideActorsWithMap( m_actorList[i] );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::CollideActorsWithMap( Actor* actor )
{
	if ( actor->m_isDead )
	{
		return;
	}

	// If actor is above ground and below ceiling, then it is within Z [min and max] range
	float previousActorZPos = actor->m_position.z;
	float actorPosClamped = GetClamped( actor->m_position.z, 0.0f, (1.0f - actor->m_physicsHeight) );

	//----------------------------------------------------------------------------------------------------------------------
	// Check if actor collided with ceiling/floor
	// Check only actors that are not MissileMice
	if ( (actor->m_position.z != actorPosClamped) && (!actor->m_actorDef->m_slidesAlongWalls) )	// If actorPos was clamped, it indicates thats actor was out of ceiling/floor bounds
	{
		actor->m_position.z = actorPosClamped;
		actor->OnCollide( nullptr );
	}

	// Apply "gravity" to MouseMissile every frame
	if ( actor->m_actorDef->m_slidesAlongWalls )
	{
		actor->m_velocity.z += -0.05f;
	}

	// If Mouse Missile clamped ceiling/floor
	if ( (actor->m_position.z != actorPosClamped) && (actor->m_actorDef->m_slidesAlongWalls) )
	{
		actor->m_position.z = actorPosClamped;
		if ( previousActorZPos > actor->m_position.z )
		{
			// Only bounce on Z axis if clamped against ceiling
			actor->m_velocity.z *= -1.0f;

			// Play MissileMice bounce sound
			m_missileMiceBounceSPBID = g_theAudio->StartSoundAt( m_missileMiceBounceSID, actor->m_position );
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Wall check
	
	// Get tileIndex for actorPos
	Vec2	actorPosVec2 = Vec2( actor->m_position.x, actor->m_position.y );
	IntVec2 tileCoords	 = GetTileCoordsForWorldPos( actorPosVec2 );

	// Check cardinal tiles
	IntVec2 northTile = IntVec2( tileCoords.x,	 tileCoords.y + 1 );
	IntVec2 southTile = IntVec2( tileCoords.x,	 tileCoords.y - 1 );
	IntVec2 eastTile  = IntVec2( tileCoords.x + 1, tileCoords.y	  );
	IntVec2 westTile  = IntVec2( tileCoords.x - 1, tileCoords.y	  );
	
	PushActorOutOfTileIfSolid( actor, northTile	);
	PushActorOutOfTileIfSolid( actor, southTile	);
	PushActorOutOfTileIfSolid( actor, eastTile	);
	PushActorOutOfTileIfSolid( actor, westTile	);

	// Check diagonal tiles
	IntVec2 northEastTile = IntVec2( tileCoords.x + 1, tileCoords.y + 1);
	IntVec2 northWestTile = IntVec2( tileCoords.x - 1, tileCoords.y + 1);
	IntVec2 southEastTile = IntVec2( tileCoords.x + 1, tileCoords.y - 1);
	IntVec2 southWestTile = IntVec2( tileCoords.x - 1, tileCoords.y - 1);

	PushActorOutOfTileIfSolid( actor, northEastTile );
	PushActorOutOfTileIfSolid( actor, northWestTile );
	PushActorOutOfTileIfSolid( actor, southEastTile );
	PushActorOutOfTileIfSolid( actor, southWestTile );
}

//----------------------------------------------------------------------------------------------------------------------
bool Map::IsTileSolid( IntVec2 const& tileCoords ) const
{
	int	tileIndex = GetTileIndexForTileCoords( tileCoords.x, tileCoords.y );
	return m_tiles[tileIndex].m_tileDef.m_isSolid;
}

//----------------------------------------------------------------------------------------------------------------------
void Map::PushActorOutOfTileIfSolid( Actor* actor, IntVec2 tileCoords )
{
	// Check if within map bounds
	if ( IsOutOfBoundsXY(tileCoords)  )
	{
		return;
	}

	// Check if tiles are NOT solid
	if ( !IsTileSolid( tileCoords ) )
	{
		return;
	}

	// Get current Tile index
	int tileIndex	  = GetTileIndexForTileCoords( tileCoords.x, tileCoords.y );
	Tile& currentTile = m_tiles[tileIndex];

	// Get floor bounds
	Vec2  floorMins			= Vec2( currentTile.m_bounds.m_mins.x, currentTile.m_bounds.m_mins.y );
	Vec2  floorMaxs			= Vec2( currentTile.m_bounds.m_maxs.x, currentTile.m_bounds.m_maxs.y );
	AABB2 currentBounds2D	= AABB2( floorMins, floorMaxs );

	// Get actorPos as Vec2
	Vec2 actorPosVec2 = Vec2( actor->m_position.x, actor->m_position.y );
	
	// Push out of walls
	bool isPushed = PushDiscOutOfFixedAABB2D( actorPosVec2, actor->m_physicsRadius, currentBounds2D );
	actor->m_position.x = actorPosVec2.x;
	actor->m_position.y = actorPosVec2.y;

	// Check if actor hit the wall
	if ( actor->m_actorDef->m_dieOnCollide && isPushed )
	{
		actor->OnCollide( nullptr );
	}

	if ( actor->m_actorDef->m_slidesAlongWalls && isPushed )
	{
		IntVec2 newTileCoords = GetTileCoordsForWorldPos( actorPosVec2 );
		IntVec2 surfaceNormal = newTileCoords - tileCoords; 

		Vec2 velocityXY		 = Vec2( actor->m_velocity.x, actor->m_velocity.y );
		Vec2 surfaceNormalXY = Vec2( float(surfaceNormal.x), float(surfaceNormal.y) );
		
		velocityXY.Reflect( surfaceNormalXY );

		actor->m_velocity.y = velocityXY.y;
		actor->m_velocity.x = velocityXY.x;

		// Play MissileMice bounce sound
		m_missileMiceBounceSPBID = g_theAudio->StartSoundAt( m_missileMiceBounceSID, actor->m_position );
	}
}

//----------------------------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastAll( Vec3 const& start, Vec3 const& direction, float distance, Actor*& outActor, Actor* actorToIgnore ) const
{
	RaycastResult3D raycastAllResult;
	RaycastResult3D raycastMissResult;

	Actor* firstActorHit = nullptr;

	raycastAllResult.m_impactDist = FLT_MAX;

	// Calculate all raycasts then return the raycastResult closest to the player
	RaycastResult3D raycastWorldXY		= RaycastWorldXY	( start, direction, distance );
	RaycastResult3D raycastWorldZ		= RaycastWorldZ		( start, direction, distance );
	RaycastResult3D raycastWorldActors  = RaycastWorldActors( start, direction, distance, firstActorHit, actorToIgnore );

	// If walls hit
	if ( (raycastWorldXY.m_impactDist < raycastAllResult.m_impactDist) && raycastWorldXY.m_didImpact )
	{
		raycastAllResult = raycastWorldXY;
	}

	// If ceiling or floor hit
	if ( (raycastWorldZ.m_impactDist < raycastAllResult.m_impactDist) && raycastWorldZ.m_didImpact )
	{
		raycastAllResult = raycastWorldZ;
	}
	
	// If actors were hit
	if ( (raycastWorldActors.m_impactDist < raycastAllResult.m_impactDist) && raycastWorldActors.m_didImpact )
	{
		outActor = firstActorHit;
		raycastAllResult = raycastWorldActors;
	}

	return raycastAllResult;
//	return raycastMissResult;		
}

//----------------------------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastWorldXY( Vec3 const& startPos, Vec3 const& forwardNormal, float maxDistance ) const
{
	RaycastResult3D raycastHitResult;
	RaycastResult3D raycastMissResult;

	// Calculate tiles and check if Solid
	IntVec2 tileXY;
	tileXY.y = int( floor(startPos.y) );
	tileXY.x = int( floor(startPos.x) );

	// If Tile is Solid
	if ( !IsOutOfBoundsXY( tileXY ) && (startPos.z >= 0.0f && startPos.z <= 1.0f) )
	{
		if ( IsTileSolid( tileXY ) )
		{
			raycastHitResult.m_didImpact		= true;
			raycastHitResult.m_impactDist		= 0.0f;
			raycastHitResult.m_impactNormal		= forwardNormal * -1.0f;			// How to calculate impact normal
			raycastHitResult.m_impactPos		= startPos;
			raycastHitResult.m_rayFwdNormal		= forwardNormal;
			raycastHitResult.m_rayMaxLength		= maxDistance;
			raycastHitResult.m_rayStartPosition	= startPos;
			return raycastHitResult;
		}
	}

	// Setup stuff for X
	float fwdDistPerXCrossing	= 1.0f / abs( forwardNormal.x );
	int tileStepDirectionX		= 0;
	if ( forwardNormal.x < 0.0f )
	{
		tileStepDirectionX = -1;
	}
	else
	{
		tileStepDirectionX = 1;
	}
	float xAtFirstCrossing		 = float( tileXY.x + ( tileStepDirectionX + 1.0f ) / 2.0f );
	float xDistToFirstXCrossing	 = xAtFirstCrossing - startPos.x;
	float fwdDistAtNextXCrossing = fabsf( xDistToFirstXCrossing ) * fwdDistPerXCrossing;

	// Setup stuff for Y
	float fwdDistPerYCrossing	= 1.0f / abs( forwardNormal.y );
	int tileStepDirectionY		= 0;
	if ( forwardNormal.y < 0.0f )
	{
		tileStepDirectionY = -1;
	}
	else
	{
		tileStepDirectionY = 1;
	}
	float yAtFirstCrossing		 = float( tileXY.y + ( tileStepDirectionY + 1 ) / 2 );
	float yDistToFirstYCrossing  = yAtFirstCrossing - startPos.y;
	float fwdDistAtNextYCrossing = fabsf( yDistToFirstYCrossing ) * fwdDistPerYCrossing;

	// Loop until raycast hits or out of range
	while ( true )
	{
		// If the X intercept is closer
		if ( fwdDistAtNextXCrossing <= fwdDistAtNextYCrossing )
		{
			// If nextPos is longer than length of ray
			if ( fwdDistAtNextXCrossing > maxDistance )
			{
				// Raycast missed
				return raycastMissResult;
			}
			
			// Step horizontally forward
			tileXY.x += tileStepDirectionX;

			Vec3 impactPos = startPos + fwdDistAtNextXCrossing * forwardNormal;
			if ( IsPositionInBounds(impactPos) )
			{
				// If TileIsSolid, impact == true
				if ( IsTileSolid( tileXY ) )
				{
					raycastHitResult.m_didImpact		= true;
					raycastHitResult.m_impactDist		= fwdDistAtNextXCrossing;
					raycastHitResult.m_impactNormal		= Vec3( float(-tileStepDirectionX), 0.0f, 0.0f );
					raycastHitResult.m_impactPos		= impactPos;
					raycastHitResult.m_rayFwdNormal		= forwardNormal;
					raycastHitResult.m_rayMaxLength		= maxDistance;
					raycastHitResult.m_rayStartPosition = startPos;

					return raycastHitResult;	
				}
			}
			
				// No hit, keep stepping
				fwdDistAtNextXCrossing += fwdDistPerXCrossing;
			
		}
		// If Y intercept is closer
		if ( fwdDistAtNextYCrossing < fwdDistAtNextXCrossing )
		{
			// If nextPos is longer than length of ray
			if ( fwdDistAtNextYCrossing > maxDistance ) 
			{
				// Raycast missed	
				return raycastMissResult;
			}

			// Step vertically forward
			tileXY.y += tileStepDirectionY;
			
			Vec3 impactPos = startPos + fwdDistAtNextYCrossing * forwardNormal;
			if ( IsPositionInBounds( impactPos ) )
			{
				// If TileIsSolid, impact == true
				if ( IsTileSolid( tileXY ) )
				{
					raycastHitResult.m_didImpact		= true;
					raycastHitResult.m_impactDist		= fwdDistAtNextYCrossing;
					raycastHitResult.m_impactNormal		= Vec3( 0.0f, float(-tileStepDirectionY), 0.0f );
					raycastHitResult.m_impactPos		= impactPos;
					raycastHitResult.m_rayFwdNormal		= forwardNormal;
					raycastHitResult.m_rayMaxLength		= maxDistance;
					raycastHitResult.m_rayStartPosition = startPos;
					return raycastHitResult;	
				}
			}

			// No hit, keep stepping
			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
			
		}
	}

	return raycastMissResult;
}

//----------------------------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastWorldZ( Vec3 const& start, Vec3 const& direction, float distance ) const
{
	// Raycast against ceiling
	RaycastResult3D raycastResult3D;
	Vec3 raycastVector = ( direction * distance );
	float scale		   = ( 1.0f - start.z ) / raycastVector.z; 

	// Check if within range
	if ( start.z < 1.0f )
	{
		if ( scale > 0.0f && scale < 1.0f )
		{
			Vec3 impactPos = start + ( scale * raycastVector );
			if ( IsPositionInBounds(impactPos) )
			{
				Vec3 impactDistVector = impactPos - start;
				raycastResult3D.m_didImpact = true;
				raycastResult3D.m_impactDist = impactDistVector.GetLength();
				raycastResult3D.m_impactPos = impactPos;
				raycastResult3D.m_impactNormal = Vec3( 0.0f, 0.0f, -1.0f );
				return raycastResult3D;
			}
		}
	}

	// Raycast against floors
	scale = ( 0.0f - start.z ) / raycastVector.z;

	// Check if within range
	if ( start.z > 0.0f )
	{
		if ( scale > 0.0f && scale < 1.0f )
		{
			Vec3 impactPos = start + ( scale * raycastVector );
			if ( IsPositionInBounds( impactPos ) )
			{
				Vec3 impactDistVector = impactPos - start;
				raycastResult3D.m_didImpact		= true;
				raycastResult3D.m_impactDist	= impactDistVector.GetLength();
				raycastResult3D.m_impactPos		= impactPos;
				raycastResult3D.m_impactNormal  = Vec3( 0.0f, 0.0f , 1.0f );   
				return raycastResult3D;
			}
		}
	}

	// If out of range, raycast did not impact;
	raycastResult3D.m_didImpact = false;
	return raycastResult3D;
}

//----------------------------------------------------------------------------------------------------------------------
RaycastResult3D Map::RaycastWorldActors( Vec3 const& startPos, Vec3 const& fwdNormal, float distance, Actor*& outActor, Actor* actorToIgnore ) const
{
	RaycastResult3D raycastHitResult;
	RaycastResult3D raycastMissResult;
//
//	Vec2 startPosVec2  = Vec2(  startPos.x,  startPos.y );
//	Vec2 fwdNormalVec2 = Vec2( fwdNormal.x, fwdNormal.y );
//
	for ( int i = 0; i < m_actorList.size(); i++ )
	{
		if ( m_actorList[i] == nullptr || (m_actorList[i] == actorToIgnore) )
		{
			continue;
		}

//		// call inside for loop
		Vec2 actorPos	  = Vec2( m_actorList[i]->m_position.x, m_actorList[i]->m_position.y );
		float actorMinZ	  = m_actorList[i]->m_position.z;
		float actorMaxZ	  = actorMinZ + m_actorList[i]->m_physicsHeight;
		float actorRadius = m_actorList[i]->m_physicsRadius;

//		RaycastResult2D raycastAgainstDisc = RaycastVsDisc2D( startPosVec2, fwdNormalVec2, distance, actorPos, actorRadius );

 		raycastHitResult = RaycastVsCylinder3D( startPos, fwdNormal, distance, actorPos, actorMinZ, actorMaxZ, actorRadius );

		if ( raycastHitResult.m_didImpact )
		{
			outActor = m_actorList[i];
			return raycastHitResult;
		}
	}

	// No impact, return miss
	return raycastMissResult;
}

//----------------------------------------------------------------------------------------------------------------------
void Map::SpawnPlayer()
{
	// Create spawnPointList 
	std::vector<Actor*> m_spawnPointList;

	// Filter m_actorList for only "spawnPoint"(s) 
	for ( int i = 0; i < m_actorList.size(); i++ )
	{
		if ( m_actorList[i] != nullptr )
		{
			if ( m_actorList[i]->m_actorDef->m_name == "SpawnPoint" )
			{
				m_spawnPointList.push_back( m_actorList[i] );
			}
		}
	}

	// Get randomNumber
	int randNum = g_theRNG->RollRandomIntInRange( 0, (int)m_spawnPointList.size() - 1 );

	// Fill in spawnInfo (name, position, orientation) from spawnPointList
	SpawnInfo playerSpawnInfo;
	playerSpawnInfo.m_actorName		 = "Marine";
	playerSpawnInfo.m_actorPosition	 = m_spawnPointList[randNum]->m_position;
	playerSpawnInfo.m_actorOrientation = m_spawnPointList[randNum]->m_orientation;

	// Create player actor
	Actor* playerActor = SpawnActor( playerSpawnInfo );

	// Handle player controller
	m_player->Possess( playerActor->m_currentActorUID );
	
	if ( !m_player->m_isFreeFlyOn )
	{
		playerActor->m_isPossessed = true;
	}
}

//----------------------------------------------------------------------------------------------------------------------
Actor* Map::SpawnActor( SpawnInfo spawnInfo )
{
	// Create new actor
	Actor* parentActor = new Actor( this, spawnInfo );
	
	// Add to list
	m_actorList.push_back( parentActor );

	// Increment m_actorSalt value
	m_actorSalt++;

	// Generate UID
	for ( int i = 0; i < m_actorList.size(); i++ )
	{
		// Check if empty slot exists inside list
		if ( m_actorList[i] == nullptr )
		{
			// Assign index to current slot
			ActorUID actorUID = ActorUID( m_actorSalt, i );
			parentActor->m_currentActorUID = actorUID;
			return parentActor;
			// construct and return an actor
		}
	}

	// Create actorUID at end of actorList index
	ActorUID actorUID = ActorUID( m_actorSalt, (int)m_actorList.size() );
	parentActor->m_currentActorUID = actorUID;

	parentActor->AIPossessCurrentActor( actorUID );
	return parentActor;
}

//----------------------------------------------------------------------------------------------------------------------
Actor* Map::GetActorByUID( ActorUID const& actorUID )
{
	// Get this actor's index
	unsigned int index = actorUID.GetIndex();	

	if ( index == ActorUID::INVALID )
	{
		return nullptr;
	}

	// Return from function if actorList[index] is empty
	if ( m_actorList[index] == nullptr || (m_actorList[index]->m_actorDef->m_name == "PlasmaProjectile") )
	{
		return nullptr;
	}
	
	// If an actor exists at m_actorList[index]
	// Check if the index actorUID does NOT input's actorUID 
	if ( m_actorList[index]->m_currentActorUID.m_saltAndIndexData != actorUID.m_saltAndIndexData )
	{
		// Return null if != match
		return nullptr;
	}

	// Else, both ActorUID does match, return actor* at that index 
	return m_actorList[index];
}

//----------------------------------------------------------------------------------------------------------------------
void Map::DeleteDestroyedActors()
{
	for ( int i = 0; i < m_actorList.size(); i++ )
	{
		if ( m_actorList[i]->m_isDead )
		{
			delete m_actorList[i];
			m_actorList[i] = nullptr;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
Actor* Map::GetClosestVisibleEnemy( Actor* actorOnLookout )
{
	// Initialize comparison variables
	Actor* closestEnemy = nullptr;
	float  closestDist	= FLT_MAX;

	// Check if actor calling this function exists
	if ( actorOnLookout == nullptr )
	{
		return closestEnemy;
	}

	// Get actor's foward
	Mat44 actorMat				= actorOnLookout->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 actorOnLookoutForward	= actorMat.GetIBasis3D();
	Vec2 actorOnLookoutForwardV2 = Vec2( actorOnLookoutForward.x ,actorOnLookoutForward.y );

	for ( int i = 0; i < m_actorList.size(); i++ )
	{
		// Check if actor at this index exists
		if ( m_actorList[i] == nullptr || m_actorList[i]->m_currentActorUID.m_saltAndIndexData == actorOnLookout->m_currentActorUID.m_saltAndIndexData || 
			m_actorList[i]->m_actorDef->m_name != "Marine" )
		{
			continue;
		}

		// Get displacement owner to target
		Vec2 targetPosV2			= Vec2( m_actorList[i]->m_position.x, m_actorList[i]->m_position.y );
		Vec2 currentActorV2			= Vec2( actorOnLookout->m_position.x, actorOnLookout->m_position.y );
		Vec2 dispOwnerToTargetV2	= targetPosV2 - currentActorV2;
		Vec3 dispOwnerToTargetV3	= m_actorList[i]->m_position - actorOnLookout->m_position;

		// Get degrees between owner and target
		float degreesBetweenLookoutActorForwardAndTarget = GetAngleDegreesBetweenVectors2D( actorOnLookoutForwardV2, dispOwnerToTargetV2 );

		// Eliminate actors outside my view angle
		float halfSightDegrees = actorOnLookout->m_actorDef->m_cameraFOVDegrees * 0.5f;
		if ( degreesBetweenLookoutActorForwardAndTarget > halfSightDegrees )
		{
			continue;
		}
		
		// Check if distance to target is too far
		float distOwnerToTarget = dispOwnerToTargetV2.GetLength();
		if ( distOwnerToTarget >= closestDist )
		{
			continue;
		}

		// If wall is between actors
		Vec3 fwdNormal = dispOwnerToTargetV3.GetNormalized();
		RaycastResult3D raycastResult = RaycastWorldXY( actorOnLookout->m_position, fwdNormal, actorOnLookout->m_actorDef->m_sightRadius );
		if ( raycastResult.m_didImpact && (distOwnerToTarget > raycastResult.m_impactDist) )
		{
			continue;
		}

		if ( !raycastResult.m_didImpact && actorOnLookout->m_actorDef->m_sightRadius < distOwnerToTarget )
		{
			continue;
		}

		// target is too far away
		if ( distOwnerToTarget > actorOnLookout->m_actorDef->m_sightRadius )
		{
			continue;
		}

		// Find closest actor 
		closestDist	 = distOwnerToTarget;
		closestEnemy = m_actorList[i];

		if ( m_playEnemySightSound )
		{
			if ( !g_theAudio->IsPlaying( m_demonSightSPBID ) )
			{
				g_theAudio->StartSoundAt( m_demonSightSID, actorOnLookout->m_position );
				m_playEnemySightSound = false;
			}
		}
	}

	return closestEnemy;
}

//----------------------------------------------------------------------------------------------------------------------
void Map::RenderUI() const
{
	m_player->RenderUI();
}
