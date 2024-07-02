#pragma once

#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

#include "Engine/Core/TileHeatMap.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/TileHeatMap.hpp"
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class Game;

//----------------------------------------------------------------------------------------------------------------------
constexpr float MAX_MAP_DIST = 999.0f;

//----------------------------------------------------------------------------------------------------------------------
class Libra_RaycastResult2D
{
public:
	bool	m_didImpact		= false;
	float	m_impactdist	= 0.0f;
	Vec2	m_impactPos		= Vec2( -1.0f, -1.0f );
};

//----------------------------------------------------------------------------------------------------------------------
struct MapDefinition
{
	IntVec2		m_dimensionsForMapDef							= IntVec2( -1, -1 );
	TileType	m_fillTileType							= TILE_TYPE_GRASS;
	TileType	m_edgeTileType							= TILE_TYPE_STONE;
	TileType	m_sprinkle1TileType						= TILE_TYPE_STONE;
	float		m_sprinkle1Chance						= 0.01f;
	int			m_entitySpawnCounts[NUM_ENTITY_TYPES]	= {0};

};

//----------------------------------------------------------------------------------------------------------------------
class Map
{
public:
	Map( Game* game, IntVec2 dimensions );
	~Map(); 

	void Startup();
	void Update( float deltaSeconds );
	void Render()	const;
	void Shutdown() const;

	IntVec2				GetTileCoordsForWorldPos( Vec2 position );
	int					GetTileIndexForTileCoords( int tileX, int tileY ) const;
	AABB2				GetTileBoundsForTileCoords( int tileX, int tileY );
	void				SetTileType( int tileX, int tileY, TileType type );
	void				SetTileType( int tileIndex, TileType type );
	void				AddEntityToMap( Entity* entity );
	void				RemoveEntityFromMap( Entity* entity );
	Vec2 const			GetPlayerPosition() const;
	Entity*				SpawnNewEntityOfType( EntityType type, Vec2 const& position, float orientationDegrees );
	EntityList const&	GetEntitiesByType( EntityType type ) const;
	bool				IsPointInSolid( Vec2 position );
	bool				HasLineOfSight( Vec2 const& entityPos, Vec2 const& playerPos );
	void				PopulateDistanceField( TileHeatMap& out_distanceField, IntVec2 referenceCoords, int maxCost );
	void				DoDistanceFieldPass( TileHeatMap& distanceField, float currentPassNum );
	void				SpreadHeatFromTile( TileHeatMap& distanceFiled, float nextTileHeat, IntVec2 currentTilePos );
	bool				IsOutOfBounds( IntVec2 tileCoords );

public:
	bool				m_isNoClip			= false;
	bool				m_isDebugDisplayOn	= false;
	bool				m_isDebugHeatMapOn	= false;
	IntVec2				m_dimensions		= IntVec2( 0, 0 );
	EntityList			m_entityListsByType[ NUM_ENTITY_TYPES ];
	EntityList			m_bulletsListsByType[ NUM_FACTIONS ];
	EntityList			m_actorListsByType[ NUM_FACTIONS ];

protected:
	void				PopulateTiles();
	void				PopulateHeatMaps();			// Populate Heatmaps  
	void				AddEntityToList( Entity* entity, std::vector<Entity*>& list );
	void				UpdateEntities( float deltaSeconds );
	void				UpdateDevCheatsInput();
	void				UpdateEntitiesPhysics( float deltaSeconds );
	void				PushEntitiesOutOfWalls( float deltaSeconds );
	void				PushEntityOutOfWalls( Entity& entity );
	void				PushEntityOutOfTileIfSolid( Entity& e, IntVec2 const& tileCoords );
	void				PushEntitiesOutOfEachOther( Entity* entityA, Entity* entityB );
	void				AddVertsForTile( std::vector<Vertex_PCU>& verts, int tileIndex ) const;
	void				RenderTiles() const;
	void				RenderEntities() const;
	void				RenderDebug() const;
	void				RenderHeatMap() const;			// Heatmap Render
	void				DeleteGarbageEntities();
	bool				IsTileSolid( IntVec2 const& tileCoords );
	void				CheckBulletCollisionWithEntities();
	Libra_RaycastResult2D		RaycastVsTiles( Vec2 startPos, Vec2 const& fwdDir, float maxDist );
	void				RemoveEntityFromList( Entity* entity, EntityList& entityList );
	bool				IsValid();

private:
	Game*							m_game = nullptr;
	IntVec2							m_safeSpaceDimensions = IntVec2( 5, 5 );
	std::vector<Tile>				m_tiles;
	std::vector<Entity*>			m_allEntities;
	
	TileHeatMap						m_distanceFieldFromStart;
	IntVec2							m_exitTileCoords;
};