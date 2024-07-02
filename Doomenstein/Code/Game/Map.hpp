#pragma once

#include "Game/Tile.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Actor.hpp"
#include "Game/PlayerController.hpp"
#include "Game/SpawnInfo.hpp"

#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Texture.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
struct RaycastResult3D;

//----------------------------------------------------------------------------------------------------------------------
class Map
{
public:
 	Map( MapDefinition const* mapDef, PlayerController* player );
	~Map();

	void Update( float deltaSeconds );
	void Render() const;
	void EndFrame();
	
	//----------------------------------------------------------------------------------------------------------------------
	// Tile and Pos Functions
	bool		IsPositionInBounds( Vec3 position, float const tolerance = 0.0f ) const;
	bool		IsOutOfBoundsXY( IntVec2 tileCoords ) const;
	Tile* const GetTile( int x, int y ) const;
	IntVec2		GetTileCoordsForTileIndex( int tileIndex );
	int			GetTileIndexForTileCoords( int tileX, int tileY ) const;
	IntVec2		GetTileCoordsForWorldPos( Vec2 position );

	//----------------------------------------------------------------------------------------------------------------------
	// Collision Functions
	void CollideActors();
	void CollideActors( Actor* actorA, Actor* actorB );
	void CollideActorsWithMap();
	void CollideActorsWithMap( Actor* actor );
	bool IsTileSolid( IntVec2 const& tileCoords ) const;
	void PushActorOutOfTileIfSolid( Actor* actor, IntVec2 tileCoords );

	// Raycast Functions
	RaycastResult3D RaycastAll			( Vec3 const& start, Vec3 const& direction, float distance, Actor*& outActor, Actor* actorToIgnore ) const;
	RaycastResult3D RaycastWorldXY		( Vec3 const& start, Vec3 const& direction, float distance ) const;
	RaycastResult3D RaycastWorldZ		( Vec3 const& start, Vec3 const& direction, float distance ) const;
	RaycastResult3D RaycastWorldActors	( Vec3 const& start, Vec3 const& direction, float distance, Actor*& outActor, Actor* actorToIgnore ) const;

	//----------------------------------------------------------------------------------------------------------------------
	// Player Functions
	void	SpawnPlayer();
	Actor*	SpawnActor( SpawnInfo spawnInfo );
	Actor*	GetActorByUID( ActorUID const& actorUID );
	void	DeleteDestroyedActors();
	Actor*	GetClosestVisibleEnemy( Actor* actorOnLookout );
	void	DebugPossessNext();

	// UI functions
	void RenderUI() const;

public:
//	std::vector<Vertex_PCU>		m_vertexList;
	std::vector<Vertex_PCUTBN>	m_vertexList;
	std::vector<unsigned int>	m_indexList;
	std::vector<Actor*>			m_actorList;
	std::vector<Tile>			m_tiles;
	VertexBuffer*				m_vertexBuffer		= nullptr;
	IndexBuffer*				m_indexBuffer		= nullptr;
	MapDefinition const*		m_mapDef;
	IntVec2						m_dimensions		= IntVec2( 0, 0 );
	PlayerController*			m_player			= nullptr;
	unsigned int				m_actorSalt			= static_cast<unsigned int>(-1);

	bool m_isRenderBillboard = false;

	// Lighting Variables
	Vec3 m_sunDirection		 = Vec3(2.0f, 1.0f, -1.0f);
	float m_sunIntensity	 = 0.85f;
	float m_ambientIntensity = 0.35f;

	// Audio Variables
	SoundPlaybackID m_missileMiceBounceSPBID	= SoundPlaybackID( -1 );
	SoundPlaybackID m_demonSightSPBID			= SoundPlaybackID( -1 );
	SoundID			m_missileMiceBounceSID		= SoundID( -1 );
	SoundID			m_demonSightSID				= SoundID( -1 );

	bool m_playEnemySightSound = true;
};