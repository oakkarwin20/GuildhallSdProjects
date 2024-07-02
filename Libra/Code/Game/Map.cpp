#include "Game/Bullet.hpp"
#include "Game/Scorpio.hpp"
#include "Game/Aries.hpp"
#include "Game/Leo.hpp"
#include "Game/PlayerTank.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/TileDefinition.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
Map::Map( Game* game, IntVec2 dimensions ) 
	: m_distanceFieldFromStart( dimensions, MAX_MAP_DIST )
{
	m_game = game;
	m_dimensions = dimensions; 

	int numTiles = m_dimensions.x * m_dimensions.y;
	m_tiles.resize(numTiles);
	bool isDoneGenerating = false;
	while ( !isDoneGenerating )
	{
		PopulateTiles();
		PopulateHeatMaps();
		isDoneGenerating = IsValid();
	}

	// PlayerTank* playerTank = new PlayerTank( m_game, Vec2( 2.0f, 2.0f ) );
	// AddEntityToList( playerTank, m_entities );
}

//----------------------------------------------------------------------------------------------------------------------
Map::~Map()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Map::Startup()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	UpdateEntities( deltaSeconds );
	UpdateDevCheatsInput();
	UpdateEntitiesPhysics( deltaSeconds );
	DeleteGarbageEntities();
}

//----------------------------------------------------------------------------------------------------------------------
void Map::Render() const
{
	RenderTiles();
	RenderEntities();
	RenderDebug();
	if ( m_isDebugHeatMapOn )
	{
		RenderHeatMap();
	}
}

void Map::Shutdown() const
{
}
  
IntVec2 Map::GetTileCoordsForWorldPos( Vec2 position )
{
	return IntVec2( RoundDownToInt( position.x ), RoundDownToInt( position.y ) );
}

//----------------------------------------------------------------------------------------------------------------------
int Map::GetTileIndexForTileCoords( int tileX, int tileY ) const
{
	return tileX + ( tileY * m_dimensions.x );
}

AABB2 Map::GetTileBoundsForTileCoords(int tileX, int tileY)
{
	return AABB2( Vec2( static_cast<float>( tileX ), static_cast<float>( tileY ) ), Vec2( static_cast<float>( tileX + 1 ), static_cast<float>( tileY + 1) ) );
}
 
//----------------------------------------------------------------------------------------------------------------------
void Map::SetTileType( int tileX, int tileY, TileType type )
{	
	int index = GetTileIndexForTileCoords( tileX, tileY );
	SetTileType( index, type );
}

//----------------------------------------------------------------------------------------------------------------------
void Map::SetTileType( int tileIndex, TileType type )
{
	m_tiles[tileIndex].m_tileDefIndex = type;
}

//----------------------------------------------------------------------------------------------------------------------
void Map::AddEntityToMap( Entity* entity )
{
	AddEntityToList( entity, m_allEntities );
	AddEntityToList( entity, m_entityListsByType[ entity->m_type ] );
	entity->m_map = this;
}

void Map::RemoveEntityFromMap( Entity* entity )
{
	RemoveEntityFromList( entity, m_allEntities );
	RemoveEntityFromList( entity, m_entityListsByType[ entity->m_type ] );
	entity->m_map = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntityOfType( EntityType type, Vec2 const& position, float orientationDegrees )
{	
	Entity* entityPointer = nullptr;

	if ( type == ENTITY_TYPE_GOOD_PLAYER )
	{
		entityPointer = new PlayerTank( this, position, orientationDegrees, type );
	}
	else if ( type == ENTITY_TYPE_EVIL_SCORPIO )
	{
		entityPointer = new Scorpio( this, position, orientationDegrees, type );
	}
	else if ( type == ENTITY_TYPE_EVIL_LEO )
	{
		entityPointer = new Leo( this, position, orientationDegrees, type );
	}
	else if ( type == ENTITY_TYPE_EVIL_ARIES )
	{
		entityPointer = new Aries( this, position, orientationDegrees, type );
	}
	else if ( type == ENTITY_TYPE_GOOD_BULLET )
	{
		entityPointer = new Bullet( this, position, orientationDegrees, type );
	}
	else if ( type == ENTITY_TYPE_EVIL_BULLET )
	{
		entityPointer = new Bullet( this, position, orientationDegrees, type );
	}

	if ( entityPointer != nullptr )
	{
		entityPointer->m_type = type;
		entityPointer->m_orientationDegrees = orientationDegrees;
	}

	AddEntityToMap(	entityPointer );

	return entityPointer;
}

//----------------------------------------------------------------------------------------------------------------------
EntityList const& Map::GetEntitiesByType( EntityType type ) const
{
	return m_entityListsByType[ type ];
}

//----------------------------------------------------------------------------------------------------------------------
void Map::PopulateTiles()
{
	for (int tileIndex = 0; tileIndex < m_tiles.size(); tileIndex++)
	{
		Tile& tile = m_tiles[tileIndex];
		tile.m_tileCoords.y = tileIndex / m_dimensions.x;
		tile.m_tileCoords.x = tileIndex % m_dimensions.x;

		//----------------------------------------------------------------------------------------------------------------------
		// choose random tile num and // randomly assign stone or grass
		int randNum = g_theRNG->RollRandomIntInRange(0, 15);

		if (randNum > 5)
		{
			tile.m_tileDefIndex = TILE_TYPE_GRASS;
		}
		else if (randNum <= 5)
		{
			tile.m_tileDefIndex = TILE_TYPE_STONE;
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Set world borders to Stone
		if ( tile.m_tileCoords.x == 0 || ( tile.m_tileCoords.x == m_dimensions.x -1 ) || 
			 tile.m_tileCoords.y == 0 || ( tile.m_tileCoords.y == m_dimensions.y -1 ) )
		{
			tile.m_tileDefIndex = TILE_TYPE_STONE;
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Set BOTTOM LEFT bunker 5x5 tiles to grass
		if ( tile.m_tileCoords.x < 6 && tile.m_tileCoords.y < 6 &&
			 tile.m_tileCoords.x > 0 && tile.m_tileCoords.y > 0 )
		{
			tile.m_tileDefIndex = TILE_TYPE_GRASS;
		}

		// set BOTTOM LEFT bunker to stone
		if (tile.m_tileCoords.x == 2 && tile.m_tileCoords.y == 4 ||
			tile.m_tileCoords.x == 3 && tile.m_tileCoords.y == 4 ||
			tile.m_tileCoords.x == 4 && tile.m_tileCoords.y == 4 ||
			tile.m_tileCoords.x == 4 && tile.m_tileCoords.y == 2 ||
			tile.m_tileCoords.x == 4 && tile.m_tileCoords.y == 3 )
		{
			tile.m_tileDefIndex = TILE_TYPE_STONE;
		}

		// #ToDo remove later but fix function logic first, SetTileType() logic written incorrect.
		/* 
		// Render 5x5 safe space
		//tile.m_tilePosition = m_safeSpaceDimensions;
			= tile.m_tileType = TILE_TYPE_TEST;
		SetTileType(1, 1, TILE_TYPE_TEST);
		SetTileType(1, 2, TILE_TYPE_TEST);
		SetTileType(1, 3, TILE_TYPE_TEST);
		SetTileType(1, 4, TILE_TYPE_TEST);
		SetTileType( 1, 5, TILE_TYPE_TEST);

		SetTileType(2, 1, TILE_TYPE_TEST);
		SetTileType(2, 2, TILE_TYPE_TEST);
		SetTileType(2, 3, TILE_TYPE_TEST);
		SetTileType(2, 4, TILE_TYPE_TEST);
		SetTileType(2, 5, TILE_TYPE_TEST);

		SetTileType(3, 1, TILE_TYPE_TEST);
		SetTileType(3, 2, TILE_TYPE_TEST);
		SetTileType(3, 3, TILE_TYPE_TEST);
		SetTileType(3, 4, TILE_TYPE_TEST);
		SetTileType(3, 5, TILE_TYPE_TEST);

		SetTileType(4, 1, TILE_TYPE_TEST);
		SetTileType(4, 2, TILE_TYPE_TEST);
		SetTileType(4, 3, TILE_TYPE_TEST);
		SetTileType(4, 4, TILE_TYPE_TEST);
		SetTileType(4, 5, TILE_TYPE_TEST);

		SetTileType(5, 1, TILE_TYPE_TEST);
		SetTileType(5, 2, TILE_TYPE_TEST);
		SetTileType(5, 3, TILE_TYPE_TEST);
		SetTileType(5, 4, TILE_TYPE_TEST);
		SetTileType(5, 5, TILE_TYPE_TEST);
*/

		//----------------------------------------------------------------------------------------------------------------------
		// Set TOP RIGHT bunker 7x7 to grass
		if ( tile.m_tileCoords.x > m_dimensions.x - 7 && tile.m_tileCoords.y > m_dimensions.y - 7 &&
			 tile.m_tileCoords.x < m_dimensions.x -	1 && tile.m_tileCoords.y < m_dimensions.y - 1 )
		{
			tile.m_tileDefIndex = TILE_TYPE_GRASS; 
		}

		// set TOP RIGHT to bunker stone
		if (tile.m_tileCoords.x == m_dimensions.x - 3  && tile.m_tileCoords.y == m_dimensions.y - 5 ||
			tile.m_tileCoords.x == m_dimensions.x - 4  && tile.m_tileCoords.y == m_dimensions.y - 5 ||
			tile.m_tileCoords.x == m_dimensions.x - 5  && tile.m_tileCoords.y == m_dimensions.y - 5 ||
			tile.m_tileCoords.x == m_dimensions.x - 5  && tile.m_tileCoords.y == m_dimensions.y - 4 ||
			tile.m_tileCoords.x == m_dimensions.x - 5  && tile.m_tileCoords.y == m_dimensions.y - 3 )
		{
			tile.m_tileDefIndex = TILE_TYPE_STONE;
		}
	}
	m_exitTileCoords = IntVec2( m_dimensions.x - 2, m_dimensions.y - 2 );
}


//----------------------------------------------------------------------------------------------------------------------
void Map::PopulateHeatMaps()
{
	PopulateDistanceField( m_distanceFieldFromStart, IntVec2( 2, 2 ), (int)MAX_MAP_DIST );
}

//----------------------------------------------------------------------------------------------------------------------
void Map::AddEntityToList( Entity* entity, std::vector<Entity*>& list )
{
	for ( int i = 0; i < list.size(); i++ )
	{
		if ( list[i] == nullptr )
		{
			list[i] = entity;
			return;
		}
	}

	list.push_back( entity );
}

//----------------------------------------------------------------------------------------------------------------------
void Map::UpdateEntities(float deltaSeconds)
{
	for (int i = 0; i < m_allEntities.size(); i++)
	{
		Entity* entity = m_allEntities[i];
		if (entity)
		{
			entity->Update( deltaSeconds );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::PushEntitiesOutOfWalls(float deltaSeconds)
{
	if ( m_isNoClip )
	{
		return;
	}

	UNUSED( deltaSeconds );

	for ( int i = 0; i < m_allEntities.size(); i++ ) 
	{
		Entity* entity = m_allEntities[i];
		if (entity)
		{
			PushEntityOutOfWalls( *entity );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::PushEntityOutOfWalls( Entity& entity )
{
	IntVec2 tileCoords = GetTileCoordsForWorldPos( entity.m_position );

	// cardinal tile checks
	IntVec2 NorthTile	= IntVec2( tileCoords.x,	 tileCoords.y + 1 );
	IntVec2 SouthTile	= IntVec2( tileCoords.x,	 tileCoords.y - 1 );
	IntVec2 EastTile	= IntVec2( tileCoords.x + 1, tileCoords.y );
	IntVec2 WestTile	= IntVec2( tileCoords.x - 1, tileCoords.y );

	PushEntityOutOfTileIfSolid( entity, NorthTile);
	PushEntityOutOfTileIfSolid( entity, SouthTile);
	PushEntityOutOfTileIfSolid( entity, EastTile);
	PushEntityOutOfTileIfSolid( entity, WestTile);

	// diagonals tile checks
	IntVec2 NorthEastTile = IntVec2( tileCoords.x + 1, tileCoords.y + 1);
	IntVec2 NorthWestTile = IntVec2( tileCoords.x - 1, tileCoords.y + 1);
	IntVec2 SouthEastTile = IntVec2( tileCoords.x + 1, tileCoords.y - 1);
	IntVec2 SouthWestTile = IntVec2( tileCoords.x - 1, tileCoords.y - 1);

	PushEntityOutOfTileIfSolid( entity, NorthEastTile );
	PushEntityOutOfTileIfSolid( entity, NorthWestTile );
	PushEntityOutOfTileIfSolid( entity, SouthEastTile );
	PushEntityOutOfTileIfSolid( entity, SouthWestTile );
}

//----------------------------------------------------------------------------------------------------------------------
void Map::PushEntityOutOfTileIfSolid( Entity& e, IntVec2 const& tileCoords )
{
	if ( IsOutOfBounds( tileCoords ) )
	{
		return;
	}

	int tileIndex = ( m_dimensions.x * tileCoords.y ) + tileCoords.x;
	Tile& currentTile = m_tiles[tileIndex];

	if ( !IsTileSolid( tileCoords ) )
	{
		return;
	}
	PushDiscOutOfFixedAABB2D( e.m_position, e.m_physicsRadius, currentTile.GetBounds() );
}

//----------------------------------------------------------------------------------------------------------------------
void Map::PushEntitiesOutOfEachOther( Entity* entityA, Entity* entityB )
{
	// check if entities are overlapping
	// if not, return 
	// check if can they be pushed by each other	// based on can be pushed member variables
	//			3 cases, 
	//			 1. A pushes B out
	//			 1. B pushes A out
	//			 1. A and B pushes each other 
	// if true, push discs out of each other
	// if only one can be pushed, pushEntity out of fixedDisc

	if ( entityA == nullptr || entityB == nullptr )
	{
		return;
	}
	
	bool discsOverlap = DoDiscsOverlap( entityA->m_position, entityA->m_physicsRadius, entityB->m_position, entityB->m_physicsRadius );

	bool APushesB = entityA->m_doesPushEntities && entityB->m_isPushedByEntities;
	bool BPushesA = entityB->m_doesPushEntities && entityA->m_isPushedByEntities;

	if ( discsOverlap )
	{
		if ( APushesB && BPushesA )
		{
			PushDiscsOutOfEachOther2D( entityA->m_position, entityA->m_physicsRadius, entityB->m_position, entityB->m_physicsRadius );
		}
		else if ( APushesB && !BPushesA )
		{
			PushDiscOutOfFixedDisc2D( entityB->m_position, entityB->m_physicsRadius, entityA->m_position, entityA->m_physicsRadius );
		}
		else if ( BPushesA && !APushesB )
		{
			PushDiscOutOfFixedDisc2D( entityA->m_position, entityA->m_physicsRadius, entityB->m_position, entityB->m_physicsRadius );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool Map::IsTileSolid( IntVec2 const& tileCoords )
{
	if ( IsOutOfBounds( tileCoords ) )
	{
		return false;
	}

	// use tile coords to find tile at that position
	// check if tile type is solid 
	int index = GetTileIndexForTileCoords( tileCoords.x, tileCoords.y );
	Tile const& thisTile = m_tiles[index];

	return TileDefinition::s_definitions[ thisTile.m_tileDefIndex ].m_isSolid;
}

// #ToDo this function isn't finished, please complete
//----------------------------------------------------------------------------------------------------------------------
void Map::CheckBulletCollisionWithEntities()
{
	/* Approach to implementing bullet
	1. Travels at constant velocity( fixed dir n speed )
		a. Might be a hint for position* speed* deltaSeconds
		b. Orientation needs to be entity / player’s forward normal.
	2. Checks every frame for wall overlap.If true, bounce 3 times then die.
		a. Use isTileSolid
	*/

//	for ( int i = 0; i < GetEntitiesByType(ENTITY_TYPE_GOOD_BULLET).size(); i++ )
//	{
//		bool didBulletHit = IsPointInsideDisc2D( m_position, m_entities[i].m_type.ENTITY_TYPE_EVIL_ARIES, m_physicsRadius );
//
//		if ( didBulletHit )
//		{
//
//			
//		}
//	}	
}

//----------------------------------------------------------------------------------------------------------------------
Libra_RaycastResult2D Map::RaycastVsTiles( Vec2 startPos, Vec2 const& fwdDir, float maxDist )
{
	Libra_RaycastResult2D result;
	result.m_impactdist = 0.0f;
	constexpr int NUM_STEPS_PER_TILE = 100;
	constexpr float DIST_PER_STEP = 1.0f / static_cast<float>(NUM_STEPS_PER_TILE);

	Vec2 eachStep	= fwdDir * DIST_PER_STEP;
	Vec2 currentPos = startPos;

	while ( result.m_impactdist < maxDist )
	{
		IntVec2 currentPosIntVec2 = GetTileCoordsForWorldPos( currentPos );
		if ( IsTileSolid( currentPosIntVec2 ) )
		{
			result.m_impactPos	= currentPos;
			result.m_didImpact	= true;
			return result;
		}
		currentPos += eachStep;
		result.m_impactdist += DIST_PER_STEP;
	}

	result.m_impactdist = maxDist;
	result.m_didImpact	= false; 
	return result;
}

//----------------------------------------------------------------------------------------------------------------------
bool Map::HasLineOfSight( Vec2 const& entityPos, Vec2 const& playerPos )
{
	Libra_RaycastResult2D result;
	float lineOfSightDist = GetDistance2D( entityPos, playerPos );
	Vec2 toPlayer = playerPos - entityPos;
	result = RaycastVsTiles( entityPos, toPlayer.GetNormalized(), lineOfSightDist );
	return result.m_didImpact == false;
}

//----------------------------------------------------------------------------------------------------------------------
void Map::PopulateDistanceField( TileHeatMap& out_distanceField, IntVec2 startingTilePos, int maxTileDistance )
{
	// set all tiles to be unreachably far away (e.g. 999)		// #ToDo how big should this be?
	out_distanceField.SetAllValues( static_cast<float>(maxTileDistance ) );
	// put zero in start tile
	out_distanceField.SetHeatValueAtTileCoord( startingTilePos, 0.0f );

	// account for being out of bounds, not affecting lower numbers or solid tiles when hitting max tile distance
	for ( int passNum = 0; passNum < maxTileDistance; ++passNum )
	{
		float currentPassTileheat = static_cast<float>(passNum);
		DoDistanceFieldPass( out_distanceField, currentPassTileheat );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::DoDistanceFieldPass( TileHeatMap& distanceField, float currentPassTileHeat )
{
	for ( int tileY = 0; tileY < distanceField.m_dimensions.y; ++tileY)
	{
		for ( int tileX = 0; tileX < distanceField.m_dimensions.x; ++tileX)
		{
			if ( currentPassTileHeat == distanceField.GetHeatValueAtTileCoord( IntVec2( tileX, tileY ) ))
			{
				SpreadHeatFromTile( distanceField, currentPassTileHeat + 1.0f, IntVec2(     tileX, tileY + 1) );
				SpreadHeatFromTile( distanceField, currentPassTileHeat + 1.0f, IntVec2(     tileX, tileY -1 ) );
				SpreadHeatFromTile( distanceField, currentPassTileHeat + 1.0f, IntVec2( tileX + 1, tileY    ) );
				SpreadHeatFromTile( distanceField, currentPassTileHeat + 1.0f, IntVec2( tileX - 1, tileY    ) );
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::SpreadHeatFromTile( TileHeatMap& distanceFiled, float nextTileHeat, IntVec2 currentTileCoords )
{
	// account for being out of bounds, not affecting lower numbers/solid tiles and when hitting max tile distance
	if ( IsOutOfBounds( currentTileCoords) )
	{
		return;
	}

	if ( IsTileSolid( currentTileCoords ) )
	{
		return;
	}
	if ( distanceFiled.GetHeatValueAtTileCoord( currentTileCoords ) < nextTileHeat )
	{
		return;
	}
	distanceFiled.SetHeatValueAtTileCoord( currentTileCoords, nextTileHeat );
}

//----------------------------------------------------------------------------------------------------------------------
void Map::UpdateDevCheatsInput()
{
	// DebugMode dev cheats
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		m_isDebugDisplayOn = !m_isDebugDisplayOn;
	}

	// NoClip dev cheats
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F3 ) )
	{
		m_isNoClip = !m_isNoClip;
	}

	//  HeatMap test cheat
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F6 ) )
	{
		m_isDebugHeatMapOn = !m_isDebugHeatMapOn;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::UpdateEntitiesPhysics( float deltaSeconds )
{
	for ( int i = 0; i < m_allEntities.size(); i++ )
	{
		for ( int j = 0; j < m_allEntities.size(); j++ )
		{
			// return if entity player = entity player
			if ( m_allEntities[i] == m_allEntities[j] )
			{
				continue;
			}

			PushEntitiesOutOfEachOther( m_allEntities[i], m_allEntities[j] );
		}
	}

	PushEntitiesOutOfWalls( deltaSeconds );
	CheckBulletCollisionWithEntities();
}

//----------------------------------------------------------------------------------------------------------------------
void Map::RemoveEntityFromList( Entity* entity, EntityList& entityList )
{
	for ( int i = 0; i < entityList.size(); i++ )
	{
		if ( entityList[i] == entity )
		{
			entityList[i] = nullptr;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool Map::IsValid()
{
	float distToExit = m_distanceFieldFromStart.GetHeatValueAtTileCoord( m_exitTileCoords );

	if ( distToExit == MAX_MAP_DIST )
	{
		DebuggerPrintf( "rejecting map unreachable exit\n");
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 const Map::GetPlayerPosition() const
{
	if (m_allEntities[0] != nullptr)
	{
		return m_allEntities[0]->m_position;
	}

	return Vec2( -1.0f, -1.0f );
}

//----------------------------------------------------------------------------------------------------------------------
void Map::AddVertsForTile(std::vector<Vertex_PCU>& verts, int tileIndex) const
{
	Tile const& tile = m_tiles[tileIndex];
	AABB2 bounds = tile.GetBounds();
	Rgba8 color = tile.GetColor();

	AddVertsForAABB2D( verts, bounds, color );
}

//----------------------------------------------------------------------------------------------------------------------
void Map::RenderTiles() const
{
	// 3 * 2 is accounting for 6 verts (2 triangles) needed to render a tile (box or AABB2)
	int bestGuessVertexCount = 3 * 2 * m_dimensions.x * m_dimensions.y;
	std::vector<Vertex_PCU> tileVerts;
	tileVerts.reserve( bestGuessVertexCount );

	for ( int tileIndex = 0; tileIndex <static_cast<int>( m_tiles.size() ); tileIndex++ )
	{
		AddVertsForTile( tileVerts, tileIndex );
	}
	

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( tileVerts.size() ), tileVerts.data() );
} 

//----------------------------------------------------------------------------------------------------------------------
void Map::RenderEntities() const
{
	for (int i = 0; i < m_allEntities.size(); i++)
	{
		Entity const* entity = m_allEntities[i];
		if (entity)
		{
			entity->Render();
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::RenderDebug() const
{
	if ( m_isDebugDisplayOn )
	{
		Rgba8 cyan		= Rgba8(   0, 255, 255, 255 );
		Rgba8 magenta	= Rgba8( 255,   0, 255, 255 );
		Rgba8 blue		= Rgba8(   0,   0, 255, 255 );
		Rgba8 red		= Rgba8( 255,   0,   0, 255 );
		Rgba8 green		= Rgba8(   0, 255,   0, 255 );

		Vec2 playerPos = GetPlayerPosition();
		Vec2 tankFowardOrientation			= m_game->m_playerTank->GetFowardNormal();
		Vec2 tankleftOrientation			= tankFowardOrientation.GetRotated90Degrees();
		float& turretOrientation			= m_game->m_playerTank->m_turretOrientationDegrees;
		float& tankGoalOrientationDegrees	= m_game->m_playerTank->m_tankGoalOrientationDegrees;
		float& turretGoalOrientationDegrees = m_game->m_playerTank->m_turretGoalOrientationDegrees;
		float& physicsRadiusLength			= m_game->m_playerTank->m_physicsRadius;
		float& cosmeticRadiusLength			= m_game->m_playerTank->m_cosmeticRadius;

		// turret orientation
		Vec2 normDispTurret					= Vec2::MakeFromPolarDegrees( turretOrientation );
		Vec2 vecToPhysicsRadius				= normDispTurret * cosmeticRadiusLength;
		Vec2 dispTankToPhysicsRadius		= playerPos + vecToPhysicsRadius;

		// tank goal orientation
		Vec2 normDispTankGoal						= Vec2::MakeFromPolarDegrees( tankGoalOrientationDegrees );
		Vec2 vecTankToCosmeticRadius				= normDispTankGoal * cosmeticRadiusLength;
		Vec2 longVecTankToCosmeticRadius			= normDispTankGoal * ( cosmeticRadiusLength * 1.1f );
		Vec2 dispTankToCosmeticRadius				= playerPos + vecTankToCosmeticRadius;
		Vec2 longerDispTankCenterToCosmeticRadius	= playerPos + longVecTankToCosmeticRadius;

		// turret goal orientation
		Vec2 normDispTurretGoal						= Vec2::MakeFromPolarDegrees( turretGoalOrientationDegrees );
		Vec2 vecTurretToCosmeticRadius				= normDispTurretGoal * cosmeticRadiusLength;
		Vec2 longVecTurretToCosmeticRadius			= normDispTurretGoal * ( cosmeticRadiusLength * 1.1f );
		Vec2 dispTurretToCosmeticRadius				= playerPos + vecTurretToCosmeticRadius;
		Vec2 longerDispTurretCenterToCosmeticRadius = playerPos + longVecTurretToCosmeticRadius;

		// tank forward normal
		Vec2 fowardNormal = playerPos + ( tankFowardOrientation * cosmeticRadiusLength );

		// tank left normal
		Vec2 leftNormal = playerPos + ( tankleftOrientation * cosmeticRadiusLength );

		std::vector<Vertex_PCU> verts;
		// tank goal orientation line
		AddVertsForLineSegment2D( verts, dispTankToCosmeticRadius, longerDispTankCenterToCosmeticRadius, 0.1f, red );
		// turret goal orientation line
		AddVertsForLineSegment2D( verts, dispTurretToCosmeticRadius, longerDispTurretCenterToCosmeticRadius, 0.1f, blue );
		// tank forward normal
		AddVertsForLineSegment2D( verts, playerPos, fowardNormal, 0.1f, red );
		// tank left normal
		AddVertsForLineSegment2D( verts, playerPos, leftNormal, 0.1f, green );
		// turret orientation line
		AddVertsForLineSegment2D( verts, playerPos, dispTankToPhysicsRadius, 0.2f, blue );
		// physics ring
		AddVertsForRing2D( verts, playerPos, physicsRadiusLength, 0.1f, cyan );
		// cosmetic ring
		AddVertsForRing2D( verts, playerPos, cosmeticRadiusLength, 0.1f, magenta );

		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->DrawVertexArray( (int)verts.size(), verts.data() );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Map::RenderHeatMap() const
{
	std::vector<Vertex_PCU> heatmapTilesVerts;

	for ( int tilesIndex = 0; tilesIndex < (int)m_tiles.size(); ++tilesIndex )
	{
		Tile const& currentTile		= m_tiles[tilesIndex];
		AABB2 tileBounds			= currentTile.GetBounds();
		float heatValue				= m_distanceFieldFromStart.m_heatValues[tilesIndex];
		float heatPercent			= GetClampedZeroToOne( heatValue / 50.0f );
		unsigned char colorByte		=  static_cast<unsigned char>( heatPercent * 255.0f );
		Rgba8 heatColor				= Rgba8( colorByte, 0, 0, 200 );
		AddVertsForAABB2D( heatmapTilesVerts, tileBounds, heatColor );
	}

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( (int)heatmapTilesVerts.size(), heatmapTilesVerts.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void Map::DeleteGarbageEntities()
{
	// loops through all entities
	// if m_garbage = true, delete and set to nullptr
	for ( int i = 0; i < m_allEntities.size(); i++ )
	{
		if ( m_allEntities[i]->m_isGarbage )
		{
			delete m_allEntities[i];
			m_allEntities[i] = nullptr;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
// #ToDo check if this function is written correctly
bool Map::IsPointInSolid( Vec2 position )
{
	IntVec2 IntVec2Pos = GetTileCoordsForWorldPos( position );
	return IsTileSolid( IntVec2Pos );
}

//----------------------------------------------------------------------------------------------------------------------
bool Map::IsOutOfBounds( IntVec2 tileCoords )
{
	return ( tileCoords.x >= m_dimensions.x || tileCoords.y >= m_dimensions.y || tileCoords.x < 0 || tileCoords.y < 0 );
}

