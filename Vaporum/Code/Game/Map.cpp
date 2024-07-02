#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Model.hpp"

#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/EngineCommon.hpp"   


//----------------------------------------------------------------------------------------------------------------------
Map::Map( MapDefinition const* mapDef, Game* game ) :
	m_mapDef( mapDef ),
	m_gridSize( mapDef->m_gridSize ),
	m_game( game )
{
	m_game->m_currentMap = this;
	int dimensionSize = m_gridSize.x * m_gridSize.y;
	m_tileList.resize( dimensionSize );
	
	//----------------------------------------------------------------------------------------------------------------------
	// Parse cData to get tile information (num and type to create)
	//----------------------------------------------------------------------------------------------------------------------
	std::string cDataString			= m_mapDef->m_tileCharacterData;
	Strings stringsSplitOnNewLine	= SplitStringOnDelimiter( cDataString, '\n' );
	std::string parsedCDataString;
	for ( int i = int( stringsSplitOnNewLine.size() - 1 ); i >= 0; i-- )
	{
		Strings stringsSplitOnSpace = SplitStringOnDelimiter( stringsSplitOnNewLine[i], ' ' );
		for ( int j = 0; j < stringsSplitOnSpace.size(); j++ )
		{
			parsedCDataString += stringsSplitOnSpace[j];
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Set all tile defs in this map's m_tileList
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < parsedCDataString.size(); i++ )
	{
		// Create a hexagon (tile) of specified type and pos
		char currentString						= parsedCDataString[i];
		TileDefinition const* currentTileDef	= TileDefinition::GetTileDefBySymbol( currentString );
		m_tileList[i].m_tileDef.m_isBlocked		= currentTileDef->m_isBlocked;
		m_tileList[i].m_tileDef.m_name			= currentTileDef->m_name;
		m_tileList[i].m_tileDef.m_symbol		= currentTileDef->m_symbol;
	}

	for ( int tileY = 0; tileY < m_gridSize.y; tileY++ )
	{
		for ( int tileX = 0; tileX < m_gridSize.x; tileX++ )
		{
			IntVec2 currentTileCoord	= IntVec2( tileX, tileY );
			Vec2 worldPos				= GetWorldPosFromTileCoords( currentTileCoord );
			int tileIndex				= GetTileIndexForTileCoords( tileX, tileY );
			if ( m_tileList[tileIndex].m_tileDef.m_isBlocked )
			{
				// Do nothing
				if ( IsHexCenterWithinMapBounds( worldPos ) )
				{
					AddVertsForHexagon2D( m_vertexList, m_indexList, worldPos, CIRCUMRADIUS * 0.9f, Rgba8::BLACK );
				}
			}
			else
			{
				// Check if hex is within map bounds
				if ( IsHexCenterWithinMapBounds( worldPos ) )
				{
					AddVertsForHexagonBorders2D( m_vertexList, m_indexList, worldPos, CIRCUMRADIUS, Rgba8::GRAY );
				}
				m_tileList[tileIndex].m_hexCoord = IntVec2( tileX, tileY );
			}

/*
			AABB3 quad			= AABB3( Vec3( 0.0f, 0.0f, 0.0f ), Vec3( 1.0f, 1.0f, 1.0f ) );
			quad.SetCenterXY( Vec3( worldPos.x, worldPos.y, 0.0f ) );
			Vec3 bottomLeft		= Vec3( quad.m_mins.x, quad.m_mins.y, 0.0f );
			Vec3 bottomRight	= Vec3( quad.m_maxs.x, quad.m_mins.y, 0.0f );
			Vec3 topRight		= Vec3( quad.m_maxs.x, quad.m_maxs.y, 0.0f );
			Vec3 topLeft		= Vec3( quad.m_mins.x, quad.m_maxs.y, 0.0f );
			AddVertsForQuad3D( m_vertexList, m_indexList, bottomLeft, bottomRight, topRight, topLeft, Rgba8::MAGENTA );
*/
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Parse player1CData to get tile information (num and type to create)
	//----------------------------------------------------------------------------------------------------------------------
	std::string player1CDataString		= m_mapDef->m_player1Data;
	Strings playerStringsSplitOnNewLine	= SplitStringOnDelimiter( player1CDataString, '\n' );
	std::string parsedPlayer1CDataString;
	for ( int i = int( playerStringsSplitOnNewLine.size() - 1 ); i >= 0; i-- )
	{
		Strings playerStringsSplitOnSpace = SplitStringOnDelimiter( playerStringsSplitOnNewLine[i], ' ' );
		for ( int j = 0; j < playerStringsSplitOnSpace.size(); j++ )
		{
			parsedPlayer1CDataString += playerStringsSplitOnSpace[j];
		}
	}
	//----------------------------------------------------------------------------------------------------------------------
	//  Set all unitDefs in this map's player1UnitList
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < parsedPlayer1CDataString.size(); i++ )
	{
		// Create a hexagon (tile) of specified type and pos
		char currentString										= parsedPlayer1CDataString[i];
		UnitDefinition const* currentUnitDef					= UnitDefinition::GetUnitDefBySymbol( currentString );
		if ( currentUnitDef != nullptr )
		{
			Unit player1CurrentUnit								= Unit();
			IntVec2 currentTileCoord							= GetTileCoordsForTileIndex( i );
			player1CurrentUnit.m_currentTileCoord				= currentTileCoord;
			player1CurrentUnit.m_previousTileCoord				= currentTileCoord;
			player1CurrentUnit.m_unitDef						= currentUnitDef;
			player1CurrentUnit.m_currentHealth					= player1CurrentUnit.m_unitDef->m_health;
			m_game->m_player1->m_unitList.push_back( player1CurrentUnit );
		}
	}


	//----------------------------------------------------------------------------------------------------------------------
	// Parse player2CData to get tile information (num and type to create)
	//----------------------------------------------------------------------------------------------------------------------
	std::string player2CDataString			= m_mapDef->m_player2Data;
	Strings player2StringsSplitOnNewLine	= SplitStringOnDelimiter( player2CDataString, '\n' );
	std::string parsedPlayer2CDataString;
	for ( int i = int( player2StringsSplitOnNewLine.size() - 1 ); i >= 0; i-- )
	{
		Strings player2StringsSplitOnSpace = SplitStringOnDelimiter( player2StringsSplitOnNewLine[i], ' ' );
		for ( int j = 0; j < player2StringsSplitOnSpace.size(); j++ )
		{
			parsedPlayer2CDataString += player2StringsSplitOnSpace[j];
		}
	}
	//----------------------------------------------------------------------------------------------------------------------
	//  Set all unitDefs in this map's player2UnitList
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < parsedPlayer2CDataString.size(); i++ )
	{
		// Create a hexagon (tile) of specified type and pos
		char currentString											= parsedPlayer2CDataString[i];
		UnitDefinition const* currentUnitDef						= UnitDefinition::GetUnitDefBySymbol( currentString );
		if ( currentUnitDef != nullptr )
		{
			Unit player2CurrentUnit									= Unit();
			IntVec2 currentTileCoord								= GetTileCoordsForTileIndex( i );
			player2CurrentUnit.m_currentTileCoord					= currentTileCoord;
			player2CurrentUnit.m_previousTileCoord					= currentTileCoord;
			player2CurrentUnit.m_unitDef							= currentUnitDef;
			player2CurrentUnit.m_currentHealth						= player2CurrentUnit.m_unitDef->m_health;
			m_game->m_player2->m_unitList.push_back( player2CurrentUnit );
		}
	}


	//----------------------------------------------------------------------------------------------------------------------
	// Create vertex and index buffers
	//----------------------------------------------------------------------------------------------------------------------
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer( m_vertexList.size(), sizeof(Vertex_PCUTBN) );
	m_indexBuffer  = g_theRenderer->CreateIndexBuffer( m_indexList.size() );

	g_theRenderer->Copy_CPU_To_GPU( m_vertexList.data(),  sizeof(Vertex_PCUTBN) * m_vertexList.size(), m_vertexBuffer, sizeof( Vertex_PCUTBN ) );
	g_theRenderer->Copy_CPU_To_GPU(  m_indexList.data(), sizeof(unsigned int) *  m_indexList.size(), m_indexBuffer  );

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize vertex and index buffers for rendering highlighted hex border GREEN
	//----------------------------------------------------------------------------------------------------------------------
	CheckIfHexNearestToCursorShouldBeHighlighted();
	AddVertsForHexagonBorders2D( m_hexVertsList, m_hexIndexList, Vec2( 0.0f, 0.0f ), CIRCUMRADIUS * 0.75, Rgba8::GREEN );
	m_vertexBuffer_greenHexBorder	= g_theRenderer->CreateVertexBuffer( m_hexVertsList.size(), sizeof(Vertex_PCUTBN) );
	m_indexBuffer_greenHexBorder	= g_theRenderer->CreateIndexBuffer( m_hexIndexList.size() );
	g_theRenderer->Copy_CPU_To_GPU( m_hexVertsList.data(),  sizeof(Vertex_PCUTBN) * m_hexVertsList.size(), m_vertexBuffer_greenHexBorder, sizeof(Vertex_PCUTBN) );
	g_theRenderer->Copy_CPU_To_GPU( m_hexIndexList.data(), sizeof(unsigned int) * m_hexIndexList.size(), m_indexBuffer_greenHexBorder );
}	


//----------------------------------------------------------------------------------------------------------------------
Map::~Map()
{
	delete m_vertexBuffer;	
	delete m_indexBuffer;	
	delete m_vertexBuffer_greenHexBorder;	
	delete m_indexBuffer_greenHexBorder;	

	m_vertexBuffer					= nullptr;	
	m_indexBuffer					= nullptr;	
	m_vertexBuffer_greenHexBorder	= nullptr;	
	m_indexBuffer_greenHexBorder	= nullptr;	
}


//----------------------------------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	CheckIfHexNearestToCursorShouldBeHighlighted();
	CheckIfCursorHoversEnemyUnit();
	CheckIfSelectedTankBlueHexBorderShouldBeRendered();
}


//----------------------------------------------------------------------------------------------------------------------
void Map::Render() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Draw call for moon surface
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> moondSurfaceVerts;
	AddVertsForQuad2D( moondSurfaceVerts, Vec2( -40.0f, -20.0f ), Vec2( 40.f, -20.0f ), Vec2( 40.f, 40.f ), Vec2( -40.f, 40.f ), Rgba8::WHITE );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( &m_game->m_mapTexture->GetTexture() );
	g_theRenderer->SetModelConstants( Mat44(), Rgba8::CYAN );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->DrawVertexArray( int( moondSurfaceVerts.size() ), moondSurfaceVerts.data() );
	// Reset bindings 
	g_theRenderer->BindTexture( nullptr );


	//----------------------------------------------------------------------------------------------------------------------
	// Draw call for map
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( nullptr );
	Mat44 mapMat = Mat44();
	mapMat.SetTranslation3D( Vec3(0.0f, 0.0f, -0.0001f) );
	g_theRenderer->SetModelConstants( mapMat );
	g_theRenderer->BindShader( m_mapDef->m_overlayShader );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->DrawVertexAndIndexBuffer( m_vertexBuffer, m_indexBuffer, (int)m_indexList.size() );
	// Reset bindings 
	g_theRenderer->BindShader( nullptr );

	//----------------------------------------------------------------------------------------------------------------------
	// Render DistanceField
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> verts;
	for ( int i = 0; i < m_tileList.size(); i++ )
	{
		Tile currentTile = m_tileList[i];
		if ( currentTile.m_hexDistToSelectedUnit > -1 )
		{
			Vec2 centerPos = GetWorldPosFromTileCoords( currentTile.m_hexCoord );
			AddVertsForHexagon2D( verts, centerPos, CIRCUMRADIUS * 0.75f, Rgba8::CYAN );
		}
	}
	//----------------------------------------------------------------------------------------------------------------------
	// Render DistancePath
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < m_game->m_distPathTileList.size(); i++ )
	{
		if ( m_game->m_distPathTileList.size() <= 1 )
		{
			break;
		}
		Tile currentTile = m_game->m_distPathTileList[i];
		Vec2 centerPos	 = GetWorldPosFromTileCoords( currentTile.m_hexCoord );
		AddVertsForHexagon2D( verts, centerPos, CIRCUMRADIUS * 0.75f, Rgba8::DARK_YELLOW );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Hack Render hoveredHex
	Vec2 hex_WorldSpace = GetWorldPosFromTileCoords( m_game->m_hoveredHex );
	AddVertsForHexagon2D( verts, hex_WorldSpace, CIRCUMRADIUS, Rgba8::MAGENTA );
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// Draw call for distance field
	//----------------------------------------------------------------------------------------------------------------------
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetDepthMode( DepthMode::DISABLED );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->DrawVertexArray( int( verts.size() ), verts.data() );


	if ( m_shouldRenderHighlightedHex_Green )
	{
		// Draw call for highlighted hexes
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetModelConstants( m_posMat );
		g_theRenderer->BindShader( m_mapDef->m_overlayShader );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->DrawVertexAndIndexBuffer( m_vertexBuffer_greenHexBorder, m_indexBuffer_greenHexBorder, int( m_hexIndexList.size() ) );
		// Reset bindings and constants
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetModelConstants();
	}
	if ( m_shouldRenderHighlightedHex_Red )
	{
		std::vector<Vertex_PCU> verts_red;
		AddVertsForHexagonBorders2D( verts_red, m_borderHexPos_red, CIRCUMRADIUS * 0.75f, Rgba8::RED );
		// Draw call for highlighted hexes
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindTexture( nullptr );
		mapMat.SetTranslation3D( Vec3( 0.0f, 0.0f, 0.0001f ) );
		g_theRenderer->SetModelConstants( mapMat );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->DrawVertexArray( int( verts_red.size() ), verts_red.data() );
	}
	if ( m_shouldRenderHighlightedHex_Blue )
	{
		std::vector<Vertex_PCU> verts_blue;
		AddVertsForHexagonBorders2D( verts_blue, m_borderHexPos_blue, CIRCUMRADIUS * 0.75f, Rgba8::BLUE );
		// Draw call for highlighted hexes
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindTexture( nullptr );
		mapMat.SetTranslation3D( Vec3( 0.0f, 0.0f, 0.0001f ) );
		g_theRenderer->SetModelConstants( mapMat );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->DrawVertexArray( int( verts_blue.size() ), verts_blue.data() );
	}

//	RenderMapBounds();

	//----------------------------------------------------------------------------------------------------------------------
	// Render player Unit Models
	//----------------------------------------------------------------------------------------------------------------------
	// Player 1
	for ( int i = 0; i < m_game->m_player1->m_unitList.size(); i++ )
	{
		Unit& currentUnit = m_game->m_player1->m_unitList[i];
		if ( currentUnit.m_currentUnitState != UnitState::IS_DEAD )
		{
			Mat44 transform			= Mat44(); 
			Vec2 worldPos			= GetWorldPosFromTileCoords( m_game->m_player1->m_unitList[i].m_currentTileCoord );
			transform.SetTranslation2D( worldPos );
			if ( currentUnit.m_currentUnitState == UnitState::FINISHED_MOVING_THIS_TURN )
			{
				currentUnit.m_unitDef->m_model->Render( transform, Rgba8::DARKER_GRAY );
			}
			else
			{
				currentUnit.m_unitDef->m_model->Render( transform, Rgba8::DARK_BLUE );
			}
		}
	}
	// Player 2
	for ( int i = 0; i < m_game->m_player2->m_unitList.size(); i++ )
	{
		Unit& currentUnit = m_game->m_player2->m_unitList[ i ];
		if ( currentUnit.m_currentUnitState != UnitState::IS_DEAD )
		{
			Mat44 transform			= Mat44(); 
			Vec2 worldPos			= GetWorldPosFromTileCoords( m_game->m_player2->m_unitList[i].m_currentTileCoord );
			transform.SetTranslation2D( worldPos );
			transform.AppendZRotation( 180.0f );
			if ( currentUnit.m_currentUnitState == UnitState::FINISHED_MOVING_THIS_TURN )
			{
				currentUnit.m_unitDef->m_model->Render( transform, Rgba8::DARKER_GRAY );
			}
			else
			{
				currentUnit.m_unitDef->m_model->Render( transform, Rgba8::SUNSET_ORANGE );
			}
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Map::EndFrame()
{
}


//----------------------------------------------------------------------------------------------------------------------
void Map::CheckIfHexNearestToCursorShouldBeHighlighted()
{
	// Convert hexCoord from hex space to worldSpace before checking
	// if within map bounds and setting translation
	m_shouldRenderHighlightedHex_Green	= false;
	m_highlightedHexCoord				= Vec3( -1.0f, -1.0f, -1.0f );
	m_hexVertsList.clear();
	
	// Check if hex is within map bounds
	Vec2 const& hoveredHex_WorldSpace	= GetWorldPosFromTileCoords( m_game->m_hoveredHex );
	if ( IsHexCenterWithinMapBounds( hoveredHex_WorldSpace ) )
	{
		m_shouldRenderHighlightedHex_Green	= true;
		m_highlightedHexCoord				= Vec3( float( hoveredHex_WorldSpace.x ), float( hoveredHex_WorldSpace.y ), 0.0f );
		// Update posMat to new nearestHex
		m_posMat = Mat44();
		m_posMat.SetTranslation3D( Vec3( hoveredHex_WorldSpace.x, hoveredHex_WorldSpace.y, 0.00005f ) );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Map::RenderMapBounds() const
{
	// Calculate map bounds
	float minX = m_mapDef->m_worldBoundsMin.x;
	float minY = m_mapDef->m_worldBoundsMin.y;
	float maxX = m_mapDef->m_worldBoundsMax.x;
	float maxY = m_mapDef->m_worldBoundsMax.y;

	// Render map bounds
	std::vector<Vertex_PCU> lineVerts;
	Vec2 BL = Vec2( minX, minY );
	Vec2 BR = Vec2( maxX, minY );
	Vec2 TL = Vec2( minX, maxY );
	Vec2 TR	= Vec2( maxX, maxY );
	AddVertsForLineSegment2D( lineVerts, BL, BR, 0.1f, Rgba8::MAGENTA );
	AddVertsForLineSegment2D( lineVerts, BR, TR, 0.1f, Rgba8::MAGENTA );
	AddVertsForLineSegment2D( lineVerts, TR, TL, 0.1f, Rgba8::MAGENTA );
	AddVertsForLineSegment2D( lineVerts, TL, BL, 0.1f, Rgba8::MAGENTA );
	g_theRenderer->DrawVertexArray( int( lineVerts.size() ), lineVerts.data() );
}


//----------------------------------------------------------------------------------------------------------------------
Vec2 Map::GetNearestHexPosToCursor( int& nearestTileIndex ) const 
{
	// Get nearestHex to cursor
	Vec2 mousePos2D						= Vec2( m_game->m_currentPlayer->m_worldMouseRayPos.x, m_game->m_currentPlayer->m_worldMouseRayPos.y );
	Vec2 nearestHex						= Vec2::ZERO;
	float distMouseToNearestHexCenter	= 1.0f;
	nearestTileIndex					= 0;
	for ( int tileY = 0; tileY < m_gridSize.y; tileY++ )
	{
		for ( int tileX = 0; tileX < m_gridSize.x; tileX++ )
		{
			IntVec2 currentTileCoord			= IntVec2( tileX, tileY );
			Vec2 currentHexCenter				= GetWorldPosFromTileCoords( currentTileCoord );
			float distMouseToCurrentHexCenter	= GetDistance2D( currentHexCenter, mousePos2D );
			if ( distMouseToCurrentHexCenter < distMouseToNearestHexCenter )
			{
				distMouseToNearestHexCenter		= distMouseToCurrentHexCenter;
				nearestHex						= currentHexCenter;
				nearestTileIndex				= GetTileIndexForTileCoords( tileX, tileY );
			}
		}
	}

	return nearestHex;
}


//----------------------------------------------------------------------------------------------------------------------
bool Map::IsHexCenterWithinMapBounds( Vec2 const& hexCenterPos ) const
{
	float minX = m_mapDef->m_worldBoundsMin.x;
	float minY = m_mapDef->m_worldBoundsMin.y;
	float maxX = m_mapDef->m_worldBoundsMax.x;
	float maxY = m_mapDef->m_worldBoundsMax.y;

	if ( hexCenterPos.x > minX &&
		 hexCenterPos.y > minY &&
		 hexCenterPos.x < maxX &&
		 hexCenterPos.y < maxY
		)
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
void Map::CheckIfCursorHoversEnemyUnit()
{
	// Get a pointer to the "other player"
	Player* enemyPlayer = m_game->GetPointerToEnemyPlayer();
	
	int deadUnitsCount = 0;
	// Loop through the "other player's" unitList and check if the unitCoords matches with the cursorCoords
	IntVec2 highlightedHex	= m_game->m_hoveredHex;
	for ( int i = 0; i < enemyPlayer->m_unitList.size(); i++)
	{
		Unit& currentEnemyUnit = enemyPlayer->m_unitList[i];
		if ( currentEnemyUnit.m_currentUnitState != UnitState::IS_DEAD )
		{
			if ( currentEnemyUnit.m_currentTileCoord == highlightedHex )
			{
				m_shouldRenderHighlightedHex_Red	= true;
				m_borderHexPos_red					= GetWorldPosFromTileCoords( IntVec2( highlightedHex.x, highlightedHex.y ) );
				break;
			}
			else
			{
				m_shouldRenderHighlightedHex_Red = false;
			}
		}
		else if ( currentEnemyUnit.m_currentUnitState == UnitState::IS_DEAD )
		{
			deadUnitsCount++;
		}
	}
	int numEnemyUnits = int( enemyPlayer->m_unitList.size() );
	// If all enemies are dead, toggle this bool to false
	if ( deadUnitsCount == numEnemyUnits )
	{
		m_shouldRenderHighlightedHex_Red = false;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Map::CheckIfSelectedTankBlueHexBorderShouldBeRendered()
{
	// Render blue hex border when a tank is selected, moving, move confirmed, or trying to attack
	if ( m_game->m_currentSelectedUnit != nullptr )
	{
		if ( m_game->m_currentSelectedUnit->m_currentUnitState == UnitState::SELECTED ||
			 m_game->m_currentSelectedUnit->m_currentUnitState == UnitState::MOVED )
		{
			if ( m_game->m_currentSelectedUnit->m_currentUnitState != UnitState::IS_DEAD )
			{
				m_shouldRenderHighlightedHex_Blue = true;
				// Update m_posMat_blueHexBorder to new selectedUnit pos
				m_borderHexPos_blue	= GetWorldPosFromTileCoords( m_game->m_currentSelectedUnit->m_currentTileCoord );		
			}
		}
		else
		{
			m_shouldRenderHighlightedHex_Blue = false;
		}
	}
	else
	{
		m_shouldRenderHighlightedHex_Blue = false;
	}
}


//----------------------------------------------------------------------------------------------------------------------
IntVec2 Map::GetTileHoveredByCursor()
{
	IntVec2 tileHoveredByCursor = GetHexCoordsFromCursorPos( m_game->m_currentCursorPos );
	return tileHoveredByCursor;
}


//----------------------------------------------------------------------------------------------------------------------
IntVec2 Map::GetTileCoordsForTileIndex( int tileIndex )
{
	IntVec2 tile;
	tile.x = tileIndex % m_gridSize.x;
	tile.y = tileIndex / m_gridSize.x;
	return tile;
}


//----------------------------------------------------------------------------------------------------------------------
int Map::GetTileIndexForTileCoords( int tileX, int tileY ) const
{
	return ( tileY * m_gridSize.x ) + tileX;
}


//----------------------------------------------------------------------------------------------------------------------
IntVec2 Map::GetHexCoordsFromCursorPos( Vec2 position )
{
	// Get nearestHex to cursor
//	m_game->m_currentPlayer->CalculateWorldMouseRayPos( position );
	Vec2 mousePos2D						= Vec2( m_game->m_currentPlayer->m_worldMouseRayPos.x, m_game->m_currentPlayer->m_worldMouseRayPos.y );
	IntVec2 nearestHexTileCoords		= IntVec2( -1, -1 );
	for ( int tileY = 0; tileY < m_gridSize.y; tileY++ )
	{
		for ( int tileX = 0; tileX < m_gridSize.x; tileX++ )
		{
			IntVec2 currentTileCoord  = IntVec2( tileX, tileY );
			Vec2 currentHexCenter	  = GetWorldPosFromTileCoords( currentTileCoord );
			if ( IsPointInsideHexagon2D( mousePos2D, currentHexCenter, CIRCUMRADIUS ) )
			{
				nearestHexTileCoords  = IntVec2( tileX, tileY );
			}
		}
	}
	return nearestHexTileCoords;
}


//----------------------------------------------------------------------------------------------------------------------
IntVec2 Map::GetHexCoordsFromCursorPos()
{
	// Get nearestHex to cursor
	Vec2 mousePos2D = Vec2( m_game->m_currentPlayer->m_worldMouseRayPos.x, m_game->m_currentPlayer->m_worldMouseRayPos.y );
	IntVec2 nearestHexTileCoords = IntVec2( -1, -1 );
	for ( int tileY = 0; tileY < m_gridSize.y; tileY++ )
	{
		for ( int tileX = 0; tileX < m_gridSize.x; tileX++ )
		{
			IntVec2 currentTileCoord = IntVec2( tileX, tileY );
			Vec2 currentHexCenter = GetWorldPosFromTileCoords( currentTileCoord );
			if ( IsPointInsideHexagon2D( mousePos2D, currentHexCenter, CIRCUMRADIUS ) )
			{
				nearestHexTileCoords = IntVec2( tileX, tileY );
			}
		}
	}
	return nearestHexTileCoords;
}


//----------------------------------------------------------------------------------------------------------------------
Vec2 Map::GetWorldPosFromTileCoords( IntVec2 tileCoord ) const
{
	float worldX	= tileCoord.x * 0.866f;
	float worldY	= 0.5f * tileCoord.x + tileCoord.y;
	Vec2 worldPos	= Vec2( worldX, worldY );
	return worldPos;
}


//----------------------------------------------------------------------------------------------------------------------
bool Map::IsPositionInBounds( Vec3 position, float const tolerance ) const
{
	UNUSED( tolerance );

	bool isXInBounds = position.x >= 0.0f && position.x <= m_gridSize.x;
	bool isYInBounds = position.y >= 0.0f && position.y <= m_gridSize.y;
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
	return ( tileCoords.x >= m_gridSize.x || tileCoords.y >= m_gridSize.y || tileCoords.x < 0 || tileCoords.y < 0 );
}


//----------------------------------------------------------------------------------------------------------------------
Tile* const Map::GetTile( int x, int y ) const
{
	UNUSED( x );
	UNUSED( y );
	return nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
Tile& Map::GetTileFromTileCoord( IntVec2 tileCoord )
{
	int tileIndex		= GetTileIndexForTileCoords( tileCoord.x, tileCoord.y );
	Tile& hoveredTile	= m_tileList[tileIndex];
	return hoveredTile;
}


//----------------------------------------------------------------------------------------------------------------------
bool Map::IsTileBlocked( IntVec2 const& tileCoords ) const
{
	int	tileIndex = GetTileIndexForTileCoords( tileCoords.x, tileCoords.y );
	return m_tileList[tileIndex].m_tileDef.m_isBlocked;
}


//----------------------------------------------------------------------------------------------------------------------
bool Map::IsTileValid( IntVec2 tileCoords )
{
	Vec2 worldPos_hexSpace	= GetWorldPosFromTileCoords( tileCoords );
	bool withinMapBounds	= IsHexCenterWithinMapBounds( worldPos_hexSpace );
	if ( withinMapBounds )
	{
		bool isTileBlocked = IsTileBlocked( tileCoords );
		if ( !isTileBlocked )
		{
			return true;
		}
	}
	return false;
}
