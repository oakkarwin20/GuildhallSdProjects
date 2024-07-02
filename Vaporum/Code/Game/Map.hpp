#pragma once

#include "Game/Tile.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Player.hpp"

#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/Mat44.hpp"

#include <vector>


//----------------------------------------------------------------------------------------------------------------------
struct RaycastResult3D;

//----------------------------------------------------------------------------------------------------------------------
class Map
{
public:
 	Map( MapDefinition const* mapDef, Game* game );
	~Map();

	void Update( float deltaSeconds );
	void Render() const;
	void EndFrame();
	
	void CheckIfHexNearestToCursorShouldBeHighlighted();
	void RenderMapBounds() const;

	//----------------------------------------------------------------------------------------------------------------------
	// Hex Functions
	Vec2		GetNearestHexPosToCursor( int& nearestTileIndex ) const; 
	bool		IsHexCenterWithinMapBounds( Vec2 const& hexCenterPos ) const;
	void		CheckIfCursorHoversEnemyUnit();
	void		CheckIfSelectedTankBlueHexBorderShouldBeRendered();
	IntVec2		GetTileHoveredByCursor();

	//----------------------------------------------------------------------------------------------------------------------
	// Tile and Pos Functions
	bool		IsPositionInBounds( Vec3 position, float const tolerance = 0.0f ) const;
	bool		IsOutOfBoundsXY( IntVec2 tileCoords ) const;
	Tile* const GetTile( int x, int y ) const;
	Tile&		GetTileFromTileCoord( IntVec2 tileCoord );
	IntVec2		GetTileCoordsForTileIndex( int tileIndex );
	int			GetTileIndexForTileCoords( int tileX, int tileY ) const;
	IntVec2		GetHexCoordsFromCursorPos( Vec2 position );
	IntVec2		GetHexCoordsFromCursorPos();
	Vec2		GetWorldPosFromTileCoords( IntVec2 tileCoord ) const;
	bool		IsTileBlocked( IntVec2 const& tileCoords ) const;
	bool		IsTileValid( IntVec2 tileCoords );

public:
	std::vector<Vertex_PCUTBN>	m_vertexList;
	std::vector<Vertex_PCUTBN>	m_hexVertsList;
	std::vector<unsigned int>	m_indexList;
	std::vector<unsigned int>	m_hexIndexList;
	std::vector<Tile>			m_tileList;
	VertexBuffer*				m_vertexBuffer						= nullptr;
	IndexBuffer*				m_indexBuffer						= nullptr;
	VertexBuffer*				m_vertexBuffer_greenHexBorder		= nullptr;
	IndexBuffer*				m_indexBuffer_greenHexBorder		= nullptr;
	MapDefinition const*		m_mapDef;
	IntVec2						m_gridSize							= IntVec2( 0, 0 );
	Mat44						m_posMat							= Mat44();
	Vec3						m_highlightedHexCenterPos			= Vec3::ZERO;
	Vec3						m_highlightedHexCoord				= Vec3::ZERO;
	Game*						m_game								= nullptr;

	// Borders
	bool						m_shouldRenderHighlightedHex_Green	= false;
	bool						m_shouldRenderHighlightedHex_Red	= false;
	bool						m_shouldRenderHighlightedHex_Blue	= false;
	Vec2						m_borderHexPos_red					= Vec2::ZERO;
	Vec2						m_borderHexPos_blue					= Vec2::ZERO;
};