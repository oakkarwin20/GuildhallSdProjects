#include "Game/World.hpp"
#include "Game/App.hpp"
#include "Game/Chunk.hpp"
#include "Game/Block.hpp"
#include "Game/BlockDef.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/BlockTemplate.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/ThirdParty/Squirrel/Noise/SmoothNoise.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/VertexUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
World::World()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Parse Game Config data
	//----------------------------------------------------------------------------------------------------------------------
	m_isHiddenSurfaceRemoval = g_gameConfigBlackboard.GetValue( "hiddenSurfaceRemoval", false );

	// Pre-calculate activation and deactivation ranges
	m_chunkActivationRange	 = g_gameConfigBlackboard.GetValue( "chunkActivationRange", -1 );
	m_chunkDeactivationRange = m_chunkActivationRange + CHUNK_SIZE_X + CHUNK_SIZE_Y;

	m_debugBlockMinDist = g_gameConfigBlackboard.GetValue( "debugBlockMinDist", -1.0f );
	m_debugBlockMaxDist = g_gameConfigBlackboard.GetValue( "debugBlockMaxDist", -1.0f );
	m_debugBlockLayers  = g_gameConfigBlackboard.GetValue( "debugBlockLayers",  -1.0f );

	// Calculate neighborhood bounds
	m_maxChunksRadiusX	= 1 + int( m_chunkActivationRange / CHUNK_SIZE_X );
	m_maxChunksRadiusY	= 1 + int( m_chunkActivationRange / CHUNK_SIZE_Y );
	m_maxNumChunks		= ( 2 * m_maxChunksRadiusX ) * ( 2 * m_maxChunksRadiusY ); // surrounding area		

	// Reserve m_activeChunkList ahead of time
//	m_activeChunkList.reserve(m_maxNumChunks);

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize debug block iter variables
	//----------------------------------------------------------------------------------------------------------------------
	m_debugBlockIterCurrentIndex = m_debugBlockIterStartingIndex;

	//----------------------------------------------------------------------------------------------------------------------
	// Create tree blockTemplate
	//----------------------------------------------------------------------------------------------------------------------
//	CreateTreeBlockTemplate();
	BlockTemplate::InitializeBlockTemplate();
}

//----------------------------------------------------------------------------------------------------------------------
World::~World()
{
	std::map< IntVec2, Chunk* >::iterator chunkIter;
	for ( chunkIter = m_activeChunkList.begin(); chunkIter != m_activeChunkList.end(); chunkIter++ )
	{
		DeactiveChunkAtCoords( chunkIter->first );
	}

	g_theJobSystem->WaitForWorkerThreadToJoin();
//	g_theJobSystem->ClearAllJobListsAndJoinAllWorkers();

	// Clear std::map
	m_activeChunkList.clear();
}

//----------------------------------------------------------------------------------------------------------------------
void World::Update()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Retrieve jobs from worker threads
	ChunkGenerateJob* retrievedJob = (ChunkGenerateJob*)( g_theJobSystem->RetrieveCompletedJob() );			// Cast job* returned from the function to "ChunkGenerateJob*"
	if ( retrievedJob != nullptr )
	{
		IntVec2 chunkCoords = retrievedJob->m_chunk->m_chunkCoords;
		bool threadIsAlreadyWorkingOnThisChunk = IsThreadAlreadyWorkingOnThisChunk( chunkCoords );
		if ( threadIsAlreadyWorkingOnThisChunk )
		{
			// If this chunk was in "processChunkGenerateList", remove that chunkCoord from the list (m_inGenerationProgressChunkList)
			for ( int i = 0; i < m_inGenerationProgressChunkList.size(); i++ )
			{
				if ( m_inGenerationProgressChunkList[i]->m_chunkCoords == chunkCoords )
				{
					m_inGenerationProgressChunkList.erase( m_inGenerationProgressChunkList.begin() + i );
				}
			}

			ActivateChunk( retrievedJob->m_chunk );
		}
	}
	
	//----------------------------------------------------------------------------------------------------------------------
	// Raycast
	//----------------------------------------------------------------------------------------------------------------------
	DebuggerPrintf( "HELLO\n" );
	RaycastVsBlocks();

	//----------------------------------------------------------------------------------------------------------------------
	// Debug test code
	// Manually activate chunk (0,0)	
	//----------------------------------------------------------------------------------------------------------------------
//	if ( m_activateOnce )
//	{
//		Chunk* chunk = new Chunk( IntVec2(0,0) );
//		ActivateChunk( chunk );
//		m_activateOnce = false;
//	}

	//----------------------------------------------------------------------------------------------------------------------
	// Debug block iter sphere and values
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_debugDrawCurrentBlockIter )
	{
		TestBlockIterGetNeighbor();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Print debug light values 
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_debugDrawLightValues )
	{
		m_debugTextList.clear();
		m_debugTextList.reserve( 6 * 1000 );
		AddVertsForLightDebugBlocks( m_debugTextList );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Chunk activation and de-activation
	//----------------------------------------------------------------------------------------------------------------------	
	m_activationOccuredThisFrame = false;
	ActivateNearestMissingChunk();				// Comment out for testing caves
	if ( !m_activationOccuredThisFrame )
	{
		DeactivateFurthestExistingChunk();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Placing and Digging blocks
	//----------------------------------------------------------------------------------------------------------------------
	EquipBlockType();
	// Remove block at or below playerPos
	if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFT_MOUSE ) )
	{
		ChangeBlockTypeAtOrBelowPlayerToAir();
	}
	// Place block at highest ground level below playerPosZ
	if ( g_theInput->WasKeyJustPressed( KEYCODE_RIGHT_MOUSE ) )
	{
		PlaceBlockOnGroundBelowPlayerZ();
	}
	// Process next dirty light
	if ( g_theInput->IsKeyDown( 'N') )
	{
		ProcessNextDirtyLightBlock();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Update Light values
	//----------------------------------------------------------------------------------------------------------------------
	ProcessAllDirtyLighting();		// #Note: Temporarily commented since lighting is currently incomplete/incorrect (indoor and outdoor). 

	//----------------------------------------------------------------------------------------------------------------------
	// Rebuild only one dirty chunk per frame 
	//----------------------------------------------------------------------------------------------------------------------
	m_didChunkGetRebuilt		= false;
//	IntVec2 nearestChunkCoord	= GetNearestChunkToPlayer();
	for ( auto iter = m_activeChunkList.begin(); iter != m_activeChunkList.end(); ++iter )
	{
		Chunk*& currentChunk = iter->second;
		if ( currentChunk != nullptr )
		{
//			if ( currentChunk->m_chunkCoords == nearestChunkCoord )
//			{
				currentChunk->Update();
				if ( m_didChunkGetRebuilt )
				{
					m_didChunkGetRebuilt = false;
					break;		// #ToDo, rewrite this to rebuild only 1 nearest chunk per frame instead of all dirty chunks in one frame
				}
//			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Current logic (incorrect ?)
	// loop through all the list of active chunks and update them IF the chunk exists
	// IF the chunk was rebuilt, break out of the function
	//----------------------------------------------------------------------------------------------------------------------
	// New logic 
	// loop through the m_activeChunksList
	// Get the nearest chunk
	// Update them IF the chunk exists
//	Chunk* nearestChunk = GetNearestChunkToPlayer();
//	if ( nearestChunk != nullptr )
//	{
//		nearestChunk->Update();
//	}
//
	//----------------------------------------------------------------------------------------------------------------------
	// Debug regenerate ALL chunks 
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F8 ) )
	{
		ForceCreateWorldFolder();
		m_worldSeed++;
		for ( auto iter = m_activeChunkList.begin(); iter != m_activeChunkList.end(); ++iter )
		{
			Chunk*& currentChunk = iter->second;
			if ( currentChunk != nullptr )
			{
				currentChunk->RegenerateBlocksInChunk();
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void World::Render() const
{
	// Bind Texture
	Texture* texture = g_debugUseWhiteTexture ? nullptr : &g_theGame->m_blockSpriteSheet->GetTexture();
	g_theRenderer->BindTexture( texture );

	// Bind shader
	static Shader* worldShader		= g_theRenderer->CreateOrGetShaderByName( "Data/Shaders/World" );
	static Shader* defaultShader	= g_theRenderer->CreateOrGetShaderByName( "Data/Shaders/Default" );
	Shader* shaderToUse				= g_debugUseWorldShader ? worldShader : defaultShader;
	g_theRenderer->BindShader( shaderToUse );

	// Set model constants
	g_theRenderer->SetModelConstants( Mat44(), Rgba8::WHITE );

	//----------------------------------------------------------------------------------------------------------------------
	// Shader hacks
	//---------------------------------------------------------------------------------------------------------------------- 
	// Update globals in our game-specific Constant Buffer Object (CBO) for our shaders to access
	SimpleMinerGPUData simpleMinerGPUData;
	double timeNow					= GetCurrentTimeSeconds(); 
	float glowstoneFlickerNoise		= Compute1dPerlinNoise( float(timeNow), 5.0f, 9, 0.5f, 2.0f, true, 0 );
	float indoorLightBrightness		= RangeMap( glowstoneFlickerNoise, -1.0f, 1.0f, 0.1f, 1.0f );

 	float lightningNoise			= Compute1dPerlinNoise( float(timeNow), 5.0f, 9, 0.5f, 2.0f, true, 3 );
	float lightningBrightness		= RangeMapClamped( lightningNoise, 0.0f, 1.0f, 0.1f, 0.9f );
//	Red								= Interpolate( Red, 1.0f, lightningBrightness );
	indoorLightBrightness			= Interpolate( indoorLightBrightness, 1.0f, lightningBrightness );

	simpleMinerGPUData.m_indoorLightColor	= Vec4( indoorLightBrightness, indoorLightBrightness, indoorLightBrightness, 1.0f );
	
	// Bind light values to shader 
	g_theRenderer->BindConstantBuffer( 8, g_simpleMinerCBO );
	g_theRenderer->Copy_CPU_To_GPU( &simpleMinerGPUData, sizeof( SimpleMinerGPUData ), g_simpleMinerCBO );
	//----------------------------------------------------------------------------------------------------------------------

	// Render ALL Chunks
	for ( auto iter = m_activeChunkList.begin(); iter != m_activeChunkList.end(); ++iter )
	{
		Chunk* const & currentChunk = iter->second;
		if ( currentChunk != nullptr )
		{
			currentChunk->Render();
		}
	}

	ToggleDebugRenderingFunctions();
}

//----------------------------------------------------------------------------------------------------------------------
IntVec2 World::GetChunkCoordsFromWorldPos( Vec2 worldPos )
{
	// Slower version
//	IntVec2 playerChunkCoords;
//	playerChunkCoords.x = static_cast<int>( g_theGame->m_player->m_position.x ) / CHUNK_SIZE_X;
//	playerChunkCoords.y = static_cast<int>( g_theGame->m_player->m_position.y ) / CHUNK_SIZE_Y;
//	return playerChunkCoords;

	// Faster version
	IntVec2 chunkCoords;
	int globalBlockCoordsX	= RoundDownToInt( worldPos.x );
	int globalBlockCoordsY	= RoundDownToInt( worldPos.y );
	int chunkCoordX			= globalBlockCoordsX >> CHUNK_BITS_X;		// Same as dividing by 16
	int chunkCoordY			= globalBlockCoordsY >> CHUNK_BITS_Y;		// Same as dividing by 16
	chunkCoords				= IntVec2( chunkCoordX, chunkCoordY );
	return chunkCoords;
}

//----------------------------------------------------------------------------------------------------------------------
Vec2 World::GetChunkCenterPosXYFromChunkCoords( IntVec2 chunkCoords )
{
	Vec2 chunkCenterPos;
	chunkCenterPos.x = float( ( CHUNK_SIZE_X * 0.5 ) + ( chunkCoords.x * CHUNK_SIZE_X ) );
	chunkCenterPos.y = float( ( CHUNK_SIZE_Y * 0.5 ) + ( chunkCoords.y * CHUNK_SIZE_Y ) );
	return chunkCenterPos;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 World::GetPlayerPos()
{
	float worldPosX = g_theGame->m_player->m_position.x;
	float worldPosY = g_theGame->m_player->m_position.y;
	float worldPosZ = g_theGame->m_player->m_position.z;
	Vec3 worldPos = Vec3( worldPosX, worldPosY, worldPosZ );
	return worldPos;
}

//----------------------------------------------------------------------------------------------------------------------
IntVec3 World::GetLocalBlockCoordsFromWorldPos( Vec3 worldPos )
{
	IntVec2 chunkCoords			= GetChunkCoordsFromWorldPos( Vec2(worldPos.x, worldPos.y) );
	int localBlockCoordsX		= int( worldPos.x - (CHUNK_SIZE_X * chunkCoords.x) );
	int localBlockCoordsY		= int( worldPos.y - (CHUNK_SIZE_Y * chunkCoords.y) );
	int localBlockCoordsZ		= int( worldPos.z );
	IntVec3 localBlockCoords	= IntVec3( localBlockCoordsX, localBlockCoordsY, localBlockCoordsZ );
	return localBlockCoords;
}

//----------------------------------------------------------------------------------------------------------------------
int World::GetBlockIndexFromLocalBlockCoords( IntVec3 localBlockCoords )
{
	int indexX  = localBlockCoords.x;
	int indexY  = localBlockCoords.y * CHUNK_SIZE_X;
	int indexZ  = localBlockCoords.z * CHUNK_BLOCKS_PER_LAYER;
	int index	= indexX + indexY + indexZ;
	return index;
}

//----------------------------------------------------------------------------------------------------------------------
bool World::DoesChunkExist( IntVec2 chunkCoords )
{
	for ( auto iter = m_activeChunkList.begin(); iter != m_activeChunkList.end(); ++iter )
	{
		Chunk*& currentChunk = iter->second;
		if ( currentChunk == nullptr )
		{
			continue;
		}

		if ( currentChunk->m_chunkCoords == chunkCoords )
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------
Chunk* World::GetChunkAtCoords( IntVec2 chunkCoords )
{
	// Check if chunkCoords exists inside m_activeChunkList
	std::map< IntVec2, Chunk* >::iterator found = m_activeChunkList.find( chunkCoords );
	if ( found == m_activeChunkList.end() )
	{
		// Return null if nothing was found
		return nullptr;
	}

	// Return valid Chunk* that was found
	return found->second;
}

//----------------------------------------------------------------------------------------------------------------------
IntVec3 World::GetWorldBlockCoordsFromBlockIter( BlockIterator blockIter )
{
	IntVec2 chunkCoords		 = blockIter.m_currentChunk->m_chunkCoords;
	IntVec3 localBlockCoords = blockIter.m_currentChunk->GetLocalBlockCoordsFromIndex( blockIter.m_blockIndex );
	IntVec3 worldBlockCoords = blockIter.m_currentChunk->GetWorldBlockCoordsFromLocalBlockCoords( localBlockCoords );
	return worldBlockCoords;
}

//----------------------------------------------------------------------------------------------------------------------
int World::GetNumActiveChunks() const
{
	int numActiveChunks = 0;
	std::map<IntVec2, Chunk*>::const_iterator iter;
	for ( iter = m_activeChunkList.begin(); iter != m_activeChunkList.end(); ++iter )
	{
		if ( iter->second != nullptr )
		{
			numActiveChunks++;
		}
	}
	
	return numActiveChunks;
}

//----------------------------------------------------------------------------------------------------------------------
BlockIterator World::LocateBlock( IntVec3 worldBlockCoords )
{
	int chunkX = worldBlockCoords.x >> CHUNK_BITS_X;
	int chunkY = worldBlockCoords.y >> CHUNK_BITS_Y;

	Chunk* currentChunk = GetChunkAtCoords( IntVec2( chunkX, chunkY ) );
	if ( currentChunk == nullptr )
	{
		return BlockIterator( nullptr, -1 );
	}

	AABB3 chunkBounds = currentChunk->m_chunkWorldBounds;
	int localX = worldBlockCoords.x - int( chunkBounds.m_mins.x );
	int localY = worldBlockCoords.y - int( chunkBounds.m_mins.y );
	int localZ = worldBlockCoords.z;

	int blockIndex = localX + (localY << CHUNK_BITS_X) + (localZ << (CHUNK_BITS_X + CHUNK_BITS_Y) );
	return BlockIterator( currentChunk, blockIndex );
}

//----------------------------------------------------------------------------------------------------------------------
void World::EquipBlockType()
{
	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		// CobbleStone
		m_equipedBlockType = 9;
	}
	if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		// GlowStone
		m_equipedBlockType = 10;
	}
	if ( g_theInput->WasKeyJustPressed( '3' ) )
	{
		// Stone
		m_equipedBlockType = 1;
	}
	if ( g_theInput->WasKeyJustPressed( '4' ) )
	{
		// Diamond
		m_equipedBlockType = 7;
	}

//	0	"air",
//	1	"stone",
//	2	"dirt",
//	3	"grass",
//	4	"coal",
//	5	"iron",
//	6	"gold",
//	7	"diamond",
//	8	"water",
//	9	"cobblestone",
//	10	"glowstone",
}

//----------------------------------------------------------------------------------------------------------------------
void World::ChangeBlockTypeAtOrBelowPlayerToAir()
{
	// Get chunkCoords from playerWorldPos
	Vec3	playerPos			= GetPlayerPos();
	IntVec2 playerChunkCoords	= GetChunkCoordsFromWorldPos( Vec2(playerPos.x, playerPos.y) );
	IntVec3 localBlockCoords	= GetLocalBlockCoordsFromWorldPos( playerPos );
	int		blockIndex			= GetBlockIndexFromLocalBlockCoords( localBlockCoords );

	// Loop through activeChunkList	
	int newBlockIndex = blockIndex;
	for ( auto iter = m_activeChunkList.begin(); iter != m_activeChunkList.end(); ++iter )
	{
		Chunk*& currentChunk = iter->second;
		if ( currentChunk != nullptr )
		{
			// If player is in this chunk 
			if ( currentChunk->m_chunkCoords == playerChunkCoords )
			{
				// Check if currentBlock is air 
				while ( currentChunk->m_blockList[newBlockIndex].m_blockTypeIndex == 0 )
				{
					// Go downwards on Z axis
					newBlockIndex = newBlockIndex - CHUNK_BLOCKS_PER_LAYER;
				}
				// Change to Air Type 
				currentChunk->m_blockList[newBlockIndex].m_blockTypeIndex = 0;
			
				// Mark lighting flag dirty to cause chain reaction
				BlockIterator currentBlockIter = BlockIterator( currentChunk, newBlockIndex );
				MarkLightingDirty( currentBlockIter );

				//----------------------------------------------------------------------------------------------------------------------
				// Re-calculate light values for blocks below currentBlock IF block one Z above was SKY
				//----------------------------------------------------------------------------------------------------------------------
				int blockIndexOneZAbove = newBlockIndex + CHUNK_BLOCKS_PER_LAYER;
				if ( currentChunk->m_blockList[blockIndexOneZAbove].CanBlockSeeTheSky() )
				{
					while ( true )
					{
						// Mark currentBlock as sky
						currentChunk->m_blockList[newBlockIndex].SetIsSky();

						// Decrement Z index
						newBlockIndex = newBlockIndex - CHUNK_BLOCKS_PER_LAYER;

						// check if block below is NOT opaque
							// Mark is sky
						if ( currentChunk->m_blockList[newBlockIndex].GetBlockDef().m_isOpaque )
						{
							break;
						}

						// Mark currentBlock (one Z below) as sky
						currentChunk->m_blockList[newBlockIndex].SetIsSky();
						BlockIterator newBlockIter = BlockIterator( currentChunk, newBlockIndex );
						MarkLightingDirty( newBlockIter );
					}
				}

				// Set flags for saving and regeneration
				currentChunk->m_isMeshDirty = true;
				currentChunk->m_needsSaving = true;

				break;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void World::PlaceBlockOnGroundBelowPlayerZ()
{
	Vec3	playerPos			= GetPlayerPos();
	IntVec2 playerChunkCoords	= GetChunkCoordsFromWorldPos( Vec2( playerPos.x, playerPos.y ) );
	IntVec3 localBlockCoords	= GetLocalBlockCoordsFromWorldPos( playerPos );
	int		blockIndex			= GetBlockIndexFromLocalBlockCoords( localBlockCoords );

	// Loop through activeChunkList	
	int newBlockIndex = blockIndex;

	for ( auto iter = m_activeChunkList.begin(); iter != m_activeChunkList.end(); ++iter )
	{
		Chunk*& currentChunk = iter->second;
		if ( currentChunk == nullptr )
		{
			continue;
		}
	
		// If player is in this chunk 
		if ( currentChunk->m_chunkCoords == playerChunkCoords )
		{
			// Check if currentBlock is air 
			while ( currentChunk->m_blockList[newBlockIndex].m_blockTypeIndex == 0 )
			{
				// Go downwards on Z axis
				newBlockIndex = newBlockIndex - CHUNK_BLOCKS_PER_LAYER;
			}

			// Go one Z above solid block 
			newBlockIndex = newBlockIndex + CHUNK_BLOCKS_PER_LAYER;
			
			// Change current airBlock to blockType currently equipped 
			int desiredBlockIndex			= newBlockIndex;
			Block& desiredBlock				= currentChunk->m_blockList[desiredBlockIndex];
			desiredBlock.m_blockTypeIndex	= m_equipedBlockType;

			// Mark currentBlock and Non-opaque blocks below as NON-SKY
			Block& previousVersionOfDesiredBlock = currentChunk->m_blockList[newBlockIndex];
			if ( previousVersionOfDesiredBlock.CanBlockSeeTheSky() && desiredBlock.GetBlockDef().m_isOpaque )
			{
				// Clear sky flag for block that was just placed
				desiredBlock.SetIsNotSky();

				while ( true )
				{
					// Go downwards on Z axis
					newBlockIndex = newBlockIndex - CHUNK_BLOCKS_PER_LAYER;

					Block& currentBlock	= currentChunk->m_blockList[newBlockIndex];
					if ( currentBlock.GetBlockDef().m_isOpaque )
					{
						break;
					}
					currentBlock.SetIsNotSky();
				}
			}

			// If currentBlock was changed to a glowStone, mark as Lighting dirty to update calculations and cause chain reaction
			if ( desiredBlock.m_blockTypeIndex == 10 )
			{
				int indexAboveGround		= newBlockIndex + CHUNK_BLOCKS_PER_LAYER;
				BlockIterator blockIter		= BlockIterator( currentChunk, indexAboveGround );
				IntVec3 worldBlockCoords	= GetWorldBlockCoordsFromBlockIter( blockIter );
				MarkLightingDirty( blockIter );
			}

			currentChunk->m_isMeshDirty = true;
			currentChunk->m_needsSaving = true;
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
Chunk* World::GetNearestChunkToPlayer()
{
	// Calculate playerChunkCoords
	Vec3 playerWorldPos			= GetPlayerPos();
	Vec2 playerWorldPosXY		= Vec2(playerWorldPos.x, playerWorldPos.y);
	IntVec2 playerChunkCoords	= GetChunkCoordsFromWorldPos( playerWorldPosXY );

	// Calculate neighborhoodChunkCoords
	IntVec2 neighborhoodMinChunkCoords = playerChunkCoords - IntVec2( m_maxChunksRadiusX, m_maxChunksRadiusY );
	IntVec2 neighborhoodMaxChunkCoords = playerChunkCoords + IntVec2( m_maxChunksRadiusX, m_maxChunksRadiusY );

	// Loop through chunks within neighborhood and check if chunks is active
	float closestMissingChunkDist	= float(m_chunkActivationRange);
	IntVec2 nearestChunkCoord		= playerChunkCoords;
	for ( int chunkY = neighborhoodMinChunkCoords.y; chunkY <= neighborhoodMaxChunkCoords.y; chunkY++ )
	{
		for ( int chunkX = neighborhoodMinChunkCoords.x; chunkX <= neighborhoodMaxChunkCoords.x; chunkX++ )
		{
			// Calculate chunkCenterWorldPos
			IntVec2 chunkCoords			= IntVec2( chunkX, chunkY );				
			Vec2 chunkCenterWorldPos	= GetChunkCenterPosXYFromChunkCoords( chunkCoords );

			// Dist check if localNeighborhoodChunk is within activation range
			float distFromPlayer = GetDistance2D( chunkCenterWorldPos, playerWorldPosXY );
			if ( distFromPlayer < closestMissingChunkDist )
			{
				// Loop through chunkList, check if chunk in this chunkCoord already exists
				if ( !DoesChunkExist( chunkCoords ) )
				{
					// Save this chunk's Coords to activate later at the end of the loop
					closestMissingChunkDist = distFromPlayer;
					nearestChunkCoord		= chunkCoords;
				}
			}
		}
	}

	// Get pointer to nearest Chunk
	Chunk* nearestChunk = nullptr;
	for ( auto iter = m_activeChunkList.begin(); iter != m_activeChunkList.end(); ++iter )
	{
		Chunk*& currentChunk = iter->second;
		if ( currentChunk == nullptr )
		{
			continue;
		}

		if ( currentChunk->m_chunkCoords == nearestChunkCoord )
		{
			nearestChunk = currentChunk;
		}
	}
	return nearestChunk;
}

//----------------------------------------------------------------------------------------------------------------------
void World::ActivateNearestMissingChunk()
{
//	if ( g_theInput->WasKeyJustPressed( 'O' ) )		// Debug keys for testing chunk activation
//	{
		// Do not activate any more chunks if num activeChunks > maxChunks
		if ( GetNumActiveChunks() >= m_maxNumChunks )
		{
			return;
		}

		// Calculate playerChunkCoords
		Vec3 playerWorldPos			= GetPlayerPos();
		Vec2 playerWorldPosXY		= Vec2(playerWorldPos.x, playerWorldPos.y);
		IntVec2 playerChunkCoords	= GetChunkCoordsFromWorldPos( playerWorldPosXY );

		// Calculate neighborhoodChunkCoords
		IntVec2 neighborhoodMinChunkCoords = playerChunkCoords - IntVec2( m_maxChunksRadiusX, m_maxChunksRadiusY );
		IntVec2 neighborhoodMaxChunkCoords = playerChunkCoords + IntVec2( m_maxChunksRadiusX, m_maxChunksRadiusY );

		// Loop through chunks within neighborhood and check if chunks is active
		float closestMissingChunkDist	= float(m_chunkActivationRange);
		IntVec2 nearestChunkCoord		= playerChunkCoords;
		for ( int chunkY = neighborhoodMinChunkCoords.y; chunkY <= neighborhoodMaxChunkCoords.y; chunkY++ )
		{
			for ( int chunkX = neighborhoodMinChunkCoords.x; chunkX <= neighborhoodMaxChunkCoords.x; chunkX++ )
			{
				// Calculate chunkCenterWorldPos
				IntVec2 chunkCoords			= IntVec2( chunkX, chunkY );				
				Vec2 chunkCenterWorldPos	= GetChunkCenterPosXYFromChunkCoords( chunkCoords );

				// Dist check if localNeighborhoodChunk is within activation range
				float distFromPlayer = GetDistance2D( chunkCenterWorldPos, playerWorldPosXY );
				if ( distFromPlayer < closestMissingChunkDist )
				{
					// Loop through chunkList, check if chunk in this chunkCoord already exists
					if ( !DoesChunkExist( chunkCoords ) )
					{
						// Save this chunk's Coords to activate later at the end of the loop
						closestMissingChunkDist = distFromPlayer;
						nearestChunkCoord		= chunkCoords;
					}
				}
			}
		}


		// Activate nearestChunk at nearestChunkCoord
		if ( closestMissingChunkDist < FLT_MAX )
		{
			if ( !DoesChunkExist( nearestChunkCoord ) )
			{
				//----------------------------------------------------------------------------------------------------------------------
				// Activate new Chunk 
				// Create new chunk closest to player and add to m_activeChunklist
				//----------------------------------------------------------------------------------------------------------------------
				// Create a new chunk
				// If chunk was loaded from file
				// True: Activate a chunk (set up neighbor pointers, initialize lighting)
				// False: 
				// 1. exit this function
				// 2. start of world::update(), retrieve jobs
				// 3. if job was retrieved, activateChunk

				bool threadIsAlreadyWorkingOnThisChunk = IsThreadAlreadyWorkingOnThisChunk( nearestChunkCoord );
				if ( !threadIsAlreadyWorkingOnThisChunk )
				{
					Chunk* chunkToActivate = new Chunk( nearestChunkCoord );
					if ( chunkToActivate->m_chunkWasLoadedFromFile )
					{
						ActivateChunk( chunkToActivate );
					}
					else
					{
						m_inGenerationProgressChunkList.push_back( chunkToActivate );
					}
				}
			}
		}
//	}
}

//----------------------------------------------------------------------------------------------------------------------
void World::DeactivateFurthestExistingChunk()
{
	float previousDistPlayerToFurthestChunkCenter	= 0;
	IntVec2 chunkCoords								= IntVec2::ZERO;
	for ( auto iter = m_activeChunkList.begin(); iter != m_activeChunkList.end(); ++iter )
	{
		Chunk*& currentChunk = iter->second;
//		Chunk*& currentChunk = m_activeChunkList[i];
		if ( currentChunk == nullptr )
		{
			continue;
		}

	 	Vec2  chunkCenterPos								= GetChunkCenterPosXYFromChunkCoords( currentChunk->m_chunkCoords );
		Vec3  playerPos										= GetPlayerPos();
		float currentDistancePlayerToFurthestChunkCenter	= GetDistance2D( Vec2(playerPos.x, playerPos.y), chunkCenterPos );
 		if ( currentDistancePlayerToFurthestChunkCenter > m_chunkDeactivationRange )
 		{
			// Filter for furthest chunk
			if ( currentDistancePlayerToFurthestChunkCenter > previousDistPlayerToFurthestChunkCenter )
			{
				previousDistPlayerToFurthestChunkCenter = currentDistancePlayerToFurthestChunkCenter;

				// Assign chunk to deactivate
				chunkCoords = iter->first;
			}
// 			DeactiveChunk( iter->first );
// 
// 			// Only deactivate one chunk this frame
// //			if ( currentChunk == nullptr )
// //			{
// //				break;
// //			}
 		}
	} 

	if ( previousDistPlayerToFurthestChunkCenter > m_chunkDeactivationRange )
	{
		DeactiveChunkAtCoords( chunkCoords );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void World::ActivateChunk( Chunk* chunkToActivate )
{
	IntVec2& chunkCoord = chunkToActivate->m_chunkCoords;

	// Set pointers to neighbors
	chunkToActivate->m_northNeighbor	= GetChunkAtCoords( IntVec2( chunkCoord.x,	   chunkCoord.y + 1 ) );
	chunkToActivate->m_southNeighbor	= GetChunkAtCoords( IntVec2( chunkCoord.x,	   chunkCoord.y - 1 ) );
	chunkToActivate->m_eastNeighbor		= GetChunkAtCoords( IntVec2( chunkCoord.x + 1, chunkCoord.y		) );
	chunkToActivate->m_westNeighbor		= GetChunkAtCoords( IntVec2( chunkCoord.x - 1, chunkCoord.y		) );
	
	// Set neighbors to point back to this chunk
	if ( chunkToActivate->m_northNeighbor )
	{
		chunkToActivate->m_northNeighbor->m_southNeighbor = chunkToActivate;

		// Set NON-opaque boundary blocks dirty (North of currentChunk)
		int localY = CHUNK_MAX_INDEX_Y;
		for ( int localZ = 0; localZ < CHUNK_SIZE_Z; localZ++ )
		{
			for ( int localX = 0; localX < CHUNK_SIZE_X; localX++ )
			{
				IntVec3 localBlockCoords	= IntVec3( localX, localY, localZ );
				int blockIndexY				= GetBlockIndexFromLocalBlockCoords( localBlockCoords );
				BlockIterator blockIterMaxY = BlockIterator( chunkToActivate, blockIndexY );
				MarkLightingDirtyIfBlockIsNotOpqaue( blockIterMaxY );
			}
		}
	}
	if ( chunkToActivate->m_southNeighbor )
	{
		chunkToActivate->m_southNeighbor->m_northNeighbor = chunkToActivate;

		// Set NON-opaque boundary blocks dirty (NORTH of currentChunk)
		int localY = 0;
		for ( int localZ = 0; localZ < CHUNK_SIZE_Z; localZ++ )
		{
			for ( int localX = 0; localX < CHUNK_SIZE_X; localX++ )
			{
				IntVec3 localBlockCoords	= IntVec3( localX, localY, localZ );
				int blockIndexY				= GetBlockIndexFromLocalBlockCoords( localBlockCoords );
				BlockIterator blockIterMinY = BlockIterator( chunkToActivate, blockIndexY );
				MarkLightingDirtyIfBlockIsNotOpqaue( blockIterMinY );
			}
		}
	}
	if ( chunkToActivate->m_eastNeighbor )
	{
		chunkToActivate->m_eastNeighbor->m_westNeighbor = chunkToActivate;

		// Set NON-opaque boundary blocks dirty (EAST of currentChunk)
		int localX = CHUNK_MAX_INDEX_X;
		for ( int localZ = 0; localZ < CHUNK_SIZE_Z; localZ++ )
		{
			for ( int localY = 0; localY < CHUNK_SIZE_Y; localY++ )
			{
				IntVec3 localBlockCoords	= IntVec3( localX, localY, localZ );
				int blockIndexX				= GetBlockIndexFromLocalBlockCoords( localBlockCoords );
				BlockIterator blockIterMaxX = BlockIterator( chunkToActivate, blockIndexX );
				MarkLightingDirtyIfBlockIsNotOpqaue( blockIterMaxX );
			}
		}
	}
	if ( chunkToActivate->m_westNeighbor )
	{
		chunkToActivate->m_westNeighbor->m_eastNeighbor = chunkToActivate;

		// Set NON-opaque boundary blocks dirty (WEST of currentChunk)
		int localX = 0;
		for ( int localZ = 0; localZ < CHUNK_SIZE_Z; localZ++ )
		{
			for ( int localY = 0; localY < CHUNK_SIZE_Y; localY++ )
			{
				IntVec3 localBlockCoords	= IntVec3( localX, localY, localZ );
				int blockIndexX				= GetBlockIndexFromLocalBlockCoords( localBlockCoords );
				BlockIterator blockIterMinX = BlockIterator( chunkToActivate, blockIndexX );
				MarkLightingDirtyIfBlockIsNotOpqaue( blockIterMinX );
			}
		}
	}

	// Set m_activationOccuredThisFrame flag to TRUE
	m_activationOccuredThisFrame = true;
	// Set m_needsSaving flag to FALSE
	chunkToActivate->m_needsSaving = false;
	// Add this chunk to std::map
	m_activeChunkList[chunkToActivate->m_chunkCoords] = chunkToActivate;

	// Initialize lighting
	InitializeOutdoorLightInfluenceLevels( chunkToActivate );
	InitializeIndoorLightInfluenceLevels(  chunkToActivate );
}

//----------------------------------------------------------------------------------------------------------------------
void World::DeactiveChunkAtCoords( IntVec2 const& chunkCoords )
{
	Chunk* chunkToDeactivate = m_activeChunkList[chunkCoords];
	if ( chunkToDeactivate != nullptr )
	{
		if ( chunkToDeactivate->m_needsSaving )
		{
			SaveChunkToFile( chunkToDeactivate );
		}

		delete chunkToDeactivate;
		m_activeChunkList[chunkCoords] = nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool World::IsThreadAlreadyWorkingOnThisChunk( IntVec2 chunkCoord )
{
	bool threadIsAlreadyWorkingOnThisChunk = false;
	for ( int i = 0; i < m_inGenerationProgressChunkList.size(); i++ )
	{
		if ( chunkCoord == m_inGenerationProgressChunkList[i]->m_chunkCoords )
		{
			threadIsAlreadyWorkingOnThisChunk = true;
			return threadIsAlreadyWorkingOnThisChunk;
		}
	}

	return threadIsAlreadyWorkingOnThisChunk;
}

//----------------------------------------------------------------------------------------------------------------------
void World::EncodeChunkData( std::vector<char>& out_chunkData, Chunk*& chunkToSave )
{
	// Add 4CC code sequence
	out_chunkData.push_back( unsigned char('G') );
	out_chunkData.push_back( unsigned char('C') );
	out_chunkData.push_back( unsigned char('H') );
	out_chunkData.push_back( unsigned char('K') );
	out_chunkData.push_back( unsigned char(VERSION_NUM)  );			
	out_chunkData.push_back( unsigned char(CHUNK_BITS_X) );
	out_chunkData.push_back( unsigned char(CHUNK_BITS_Y) );
	out_chunkData.push_back( unsigned char(CHUNK_BITS_Z) );

	// To save the world seed, I need to break up the data into 4, 8-bit data types to preserve its integrity 
	// ( worldSeed_4, worldSeed_3, worldSeed_2, worldSeed_1 ) will save the worldSeed data (highest - lowest)
	unsigned int mask = 255;					// 0000'0000, 0000'0000, 0000'0000, 1111'1111

	// worldSeed_1 = m_worldSeed & (0000'0000, 0000'0000, 0000'0000, 1111'1111)
	unsigned int worldSeed_1 = m_worldSeed & mask;

	// worldSeed_2 = m_worldSeed & (0000'0000, 0000'0000, 1111'1111, 0000'0000)
	mask = mask << 8;
	unsigned int worldSeed_2 = m_worldSeed & mask;
	worldSeed_2 = worldSeed_2 >> 8;							// Bitshift 8 bits right-ward to bring worldSeed_2 back to (0000'0000, 0000'0000, 0000'0000, 1111'1111)

	// worldSeed_3 = m_worldSeed & (0000'0000, 1111'1111, 0000'0000, 0000'0000)
	mask = mask << 8;
	unsigned int worldSeed_3 = m_worldSeed & mask;
	worldSeed_3 = worldSeed_3 >> 16;							// Bitshift 16 bits right-ward to bring worldSeed_3 back to (0000'0000, 0000'0000, 0000'0000, 1111'1111)

	// worldSeed_4 = m_worldSeed & (1111'1111, 0000'0000, 0000'0000, 0000'0000)
	unsigned int worldSeed_4 = m_worldSeed >> 24;	// Bitshift 24 bits right-ward to bring worldSeed_3 back to (0000'0000, 0000'0000, 0000'0000, 1111'1111)										

	out_chunkData.push_back( unsigned char(worldSeed_1)  );
	out_chunkData.push_back( unsigned char(worldSeed_2)  );
	out_chunkData.push_back( unsigned char(worldSeed_3)  );
	out_chunkData.push_back( unsigned char(worldSeed_4)  );

	int blockType = 0;
	int numBlocks = 0;
	for ( int i = 0; i < MAX_BLOCKS_PER_CHUNK; i++ )
	{
		Block& currentBlock = chunkToSave->m_blockList[i];
		if ( blockType != currentBlock.m_blockTypeIndex || (numBlocks == 255) )
		{
			// Save chunkData if blockType has changed
			out_chunkData.push_back( unsigned char( blockType ) );
			out_chunkData.push_back( unsigned char( numBlocks ) );

			// Start a new RLE sequence
			numBlocks = 0;
			blockType = currentBlock.m_blockTypeIndex;
		}
		
		// numBlocks
		numBlocks++;
	}

	// Save last blockNum and Index
	out_chunkData.push_back( unsigned char( blockType ) );
	out_chunkData.push_back( unsigned char( numBlocks ) );
}

//----------------------------------------------------------------------------------------------------------------------
void World::SaveChunkToFile( Chunk*& chunkToSave )
{
	// Save file by encoding chunk data then copying that data to file
	std::vector<char> out_chunkData;
	EncodeChunkData( out_chunkData, chunkToSave );
	std::string chunkName = Stringf( "Saves/Chunk(%i,%i).chunk", chunkToSave->m_chunkCoords.x, chunkToSave->m_chunkCoords.y );
	WriteBinaryBufferToFile( out_chunkData, chunkName );
}

//----------------------------------------------------------------------------------------------------------------------
void World::InitializeIndoorLightInfluenceLevels( Chunk* currentChunk )
{
	// Loop through all blocks in currentBlock
	for ( int blockIndex = 0; blockIndex < MAX_BLOCKS_PER_CHUNK; blockIndex++ )
	{
		// Mark currentBlockType DIRTY IF it is a "glowstone"	// Glowstone is "10"
		Block& currentBlock = currentChunk->m_blockList[blockIndex];
		if ( currentBlock.m_blockTypeIndex == 10 )	
		{
			BlockIterator blockIter = BlockIterator( currentChunk, blockIndex );
			MarkLightingDirty( blockIter );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void World::InitializeOutdoorLightInfluenceLevels( Chunk* currentChunk )
{
	MarkSkyBlocks( currentChunk );
}

//----------------------------------------------------------------------------------------------------------------------
void World::ProcessAllDirtyLighting()
{
	while ( !m_dirtyLightBlockList.empty() )
	{
		// 1. pop the front one
		// Get the front-most item (dirty block) in the list
		BlockIterator blockIter = m_dirtyLightBlockList.front();
		m_dirtyLightBlockList.pop_front();

		// Debug code
//		IntVec3 worldBlockCoords = GetWorldBlockCoordsFromBlockIter( blockIter );
//		if ( worldBlockCoords == IntVec3( -32, -1, 65 ) )
//		{
//			int x = 0;
//		}
//		if ( worldBlockCoords == IntVec3( -32, -2, 65 ) )
//		{
//			int x = 0;
//		}
		ProcessDirtyLightBlock( blockIter );
	}
}

//----------------------------------------------------------------------------------------------------------------------
int World::ComputeBlockCorrectIndoorLight( BlockIterator neighborBlock, BlockIterator currentBlock )
{
	// Check if the blockIters are valid
	if ( !neighborBlock.IsBlockIndexValid() || !currentBlock.IsBlockIndexValid() )
	{
		return 0;
	}

	BlockDef blockDef = neighborBlock.GetBlock()->GetBlockDef();

	// Get myLightValue
	int myIndoorLightValue = 0;
	if ( blockDef.m_indoorEmissionLight > 0 )
	{
		myIndoorLightValue = (blockDef.m_indoorEmissionLight - 1);
	}
	
	// Get neighbor blocks' highest light influence values
	int neighborBrightestIndoorLightValue = GetBrightestNeighborIndoorLightValue( currentBlock );
	
	// Compare myLightValue with neighborBrightestLightValue
	if ( myIndoorLightValue < neighborBrightestIndoorLightValue )
	{
		// Set correct light value to (neighborHighestLightValue - 1)
		myIndoorLightValue = (neighborBrightestIndoorLightValue - 1);
	}

	return myIndoorLightValue;
}

//----------------------------------------------------------------------------------------------------------------------
int World::ComputeBlockCorrectOutdoorLight( BlockIterator neighborBlock, BlockIterator currentBlock )
{
	// Check if the blockIters are valid
	if ( !neighborBlock.IsBlockIndexValid() || !currentBlock.IsBlockIndexValid() )
	{
		return 0;
	}

	// Check if this block is NOT opaque 
	BlockDef blockDef = neighborBlock.GetBlock()->GetBlockDef();

	int myOutdoorLightValue = 0;
	if ( neighborBlock.GetBlock()->CanBlockSeeTheSky() && !blockDef.m_isOpaque )
	{
		myOutdoorLightValue = MAX_LIGHT_INFLUENCE;
		return myOutdoorLightValue;
	}

	// Get neighbor blocks' highest light influence values
	int neighborBrightestOutdoorLightValue = GetBrightestNeighborOutdoorLightValue( neighborBlock );

	// Compare myLightValue with neighborBrightestLightValue
	if ( myOutdoorLightValue > neighborBrightestOutdoorLightValue )
	{
		// Set to correct light value
		myOutdoorLightValue = (neighborBrightestOutdoorLightValue - 1);
	}

	return myOutdoorLightValue;
}

//----------------------------------------------------------------------------------------------------------------------
void World::ProcessNextDirtyLightBlock()
{
	if ( !m_dirtyLightBlockList.empty() )
	{
		// 1. pop the front one
		// Get the front-most item (dirty block) in the list
		BlockIterator blockIter = m_dirtyLightBlockList.front();
		m_dirtyLightBlockList.pop_front();
		ProcessDirtyLightBlock( blockIter );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void World::MarkLightingDirty( BlockIterator blockIterator )
{
	if ( !blockIterator.IsBlockIndexValid() )
	{
		return;
	}

	Block* block = blockIterator.GetBlock();
	if ( block == nullptr )
	{
		return;
	}

	// Return if this block is already marked Light dirty
	if ( block->IsLightDirty() )
	{
		return;
	}

	block->SetLightDirty();
	m_dirtyLightBlockList.push_back( blockIterator );
}

//----------------------------------------------------------------------------------------------------------------------
void World::UndirtyAllBlocksInChunk()
{
}

//----------------------------------------------------------------------------------------------------------------------
void World::MarkSkyBlocks( Chunk* currentChunk )
{
	// Loop through all blocks in currentBlock
	for ( int blockIndexY = 0; blockIndexY < CHUNK_SIZE_Y; blockIndexY++ )
	{
		for ( int blockIndexX = 0; blockIndexX < CHUNK_SIZE_X; blockIndexX++ )
		{
			// Get index from position
			int currentBlockIndex = GetBlockIndexFromLocalBlockCoords( IntVec3( blockIndexX, blockIndexY, CHUNK_MAX_INDEX_Z ) );

//			if ( currentChunk->m_chunkCoords == IntVec2( -2, -1 ) )
//			{
//				if ( blockIndexX == 0 && blockIndexY == 15 )
//				{
//					int x = 0;
//				}
//			}

			// Check if currentBlock is NOT opaque 
			while ( true )
			{
				// Break out of infinite loop if currentBlockIndex is negative
				if ( currentBlockIndex < 0 )
				{
					break;
				}

				Block& currentBlock	= currentChunk->m_blockList[currentBlockIndex];
				if ( currentBlock.GetBlockDef().m_isOpaque )
				{
					break;
				}

//				IntVec3 localBlockCoords = currentChunk->GetLocalBlockCoordsFromIndex( currentBlockIndex );
				if ( !currentBlock.GetBlockDef().m_isOpaque )
				{
					// Set current block to SKY
					currentBlock.SetIsSky();
				}

	//				//----------------------------------------------------------------------------------------------------------------------
	//				// Mark non sky, non opaque horizontal neighbors light dirty
	//				//----------------------------------------------------------------------------------------------------------------------
	//				BlockIterator currentBlockIter			= BlockIterator( currentChunk, currentBlockIndex );
	//				BlockIterator eastNeighborBlockIter		= currentBlockIter.GetEastNeighborBlock();
	//				BlockIterator westNeighborBlockIter		= currentBlockIter.GetWestNeighborBlock();
	//				BlockIterator northNeighborBlockIter	= currentBlockIter.GetNorthNeighborBlock(); 
	//				BlockIterator southNeighborBlockIter	= currentBlockIter.GetSouthNeighborBlock();
	//
	//				Block* eastNeighborBlock		= currentBlockIter.GetEastNeighborBlock().GetBlock();
	//				Block* westNeighborBlock		= currentBlockIter.GetWestNeighborBlock().GetBlock();
	//				Block* northNeighborBlock		= currentBlockIter.GetNorthNeighborBlock().GetBlock();
	//				Block* southNeighborBlock		= currentBlockIter.GetSouthNeighborBlock().GetBlock();
	//				
	//				IntVec3 worldBlockCoords = GetWorldBlockCoordsFromBlockIter( currentBlockIter );
	//				if ( worldBlockCoords == IntVec3( -32, -1, 65 ) )
	//				{
	//					int x = 0;
	//				}
	//				if ( worldBlockCoords == IntVec3( -32, -2, 65 ) )
	//				{
	//					int x = 0;
	//				}
	//
	//				if ( eastNeighborBlock != nullptr && !eastNeighborBlock->CanBlockSeeTheSky() )
	//				{
	//					MarkLightingDirtyIfBlockIsNotOpqaue( eastNeighborBlockIter );
	//				}
	//				if ( westNeighborBlock != nullptr && !westNeighborBlock->CanBlockSeeTheSky() )
	//				{
	//					MarkLightingDirtyIfBlockIsNotOpqaue( westNeighborBlockIter );
	//				}
	//				if ( northNeighborBlock != nullptr && !northNeighborBlock->CanBlockSeeTheSky() )
	//				{
	//					MarkLightingDirtyIfBlockIsNotOpqaue( northNeighborBlockIter );
	//				}
	//				if ( southNeighborBlock != nullptr && !southNeighborBlock->CanBlockSeeTheSky() )
	//				{
	//					MarkLightingDirtyIfBlockIsNotOpqaue( southNeighborBlockIter );
	//				}

				// Go downwards on Z axis
				currentBlockIndex -= CHUNK_BLOCKS_PER_LAYER;
			}
		}	
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Mark non sky, non opaque horizontal neighbors light dirty
	//----------------------------------------------------------------------------------------------------------------------
	for ( int blockY = 0; blockY < CHUNK_SIZE_Y; blockY++ )
	{
		for ( int blockX = 0; blockX < CHUNK_SIZE_X; blockX++ )
		{		
			// Get index from position
			int currentBlockIndex = GetBlockIndexFromLocalBlockCoords( IntVec3( blockX, blockY, CHUNK_MAX_INDEX_Z ) );

			// Check if currentBlock is NOT opaque 
			while ( true )
			{
				Block& currentBlock = currentChunk->m_blockList[currentBlockIndex];
				if ( !currentBlock.GetBlockDef().m_isOpaque )
				{
					break;
				}

				// Set light value
				if ( currentBlock.CanBlockSeeTheSky() && !currentBlock.GetBlockDef().m_isOpaque )
				{
					currentBlock.SetOutdoorLightInfluence( MAX_LIGHT_INFLUENCE );

					//----------------------------------------------------------------------------------------------------------------------
					// Mark non sky, non opaque horizontal neighbors light dirty
					//----------------------------------------------------------------------------------------------------------------------
					BlockIterator currentBlockIter			= BlockIterator( currentChunk, currentBlockIndex );
					BlockIterator eastNeighborBlockIter		= currentBlockIter.GetEastNeighborBlock();
					BlockIterator westNeighborBlockIter		= currentBlockIter.GetWestNeighborBlock();
					BlockIterator northNeighborBlockIter	= currentBlockIter.GetNorthNeighborBlock(); 
					BlockIterator southNeighborBlockIter	= currentBlockIter.GetSouthNeighborBlock();

					Block* eastNeighborBlock	= currentBlockIter.GetEastNeighborBlock().GetBlock();
					Block* westNeighborBlock	= currentBlockIter.GetWestNeighborBlock().GetBlock();
					Block* northNeighborBlock	= currentBlockIter.GetNorthNeighborBlock().GetBlock();
					Block* southNeighborBlock	= currentBlockIter.GetSouthNeighborBlock().GetBlock();
				
					IntVec3 worldBlockCoords = GetWorldBlockCoordsFromBlockIter( currentBlockIter );
//					if ( worldBlockCoords == IntVec3( -32, -1, 65 ) )
//					{
//						int x = 0;
//					}
//					if ( worldBlockCoords == IntVec3( -32, -2, 65 ) )
//					{
//						int x = 0;
//					}

					if ( eastNeighborBlock != nullptr && !eastNeighborBlock->CanBlockSeeTheSky() && !eastNeighborBlock->GetBlockDef().m_isOpaque )
					{
						MarkLightingDirtyIfBlockIsNotOpqaue( eastNeighborBlockIter );
					}
					if ( westNeighborBlock != nullptr && !westNeighborBlock->CanBlockSeeTheSky() && westNeighborBlock->GetBlockDef().m_isOpaque )
					{
						MarkLightingDirtyIfBlockIsNotOpqaue( westNeighborBlockIter );
					}
					if ( northNeighborBlock != nullptr && !northNeighborBlock->CanBlockSeeTheSky() && northNeighborBlock->GetBlockDef().m_isOpaque )
					{
						MarkLightingDirtyIfBlockIsNotOpqaue( northNeighborBlockIter );
					}
					if ( southNeighborBlock != nullptr && !southNeighborBlock->CanBlockSeeTheSky() && southNeighborBlock->GetBlockDef().m_isOpaque )
					{
						MarkLightingDirtyIfBlockIsNotOpqaue( southNeighborBlockIter );
					}

					// Go downwards on Z axis
					currentBlockIndex -= CHUNK_BLOCKS_PER_LAYER;
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void World::MarkLightingDirtyIfBlockIsNotOpqaue( BlockIterator blockIter )
{
	Block* currentBlock = blockIter.GetBlock();
	if ( blockIter.m_currentChunk != nullptr )
	{
		if ( !currentBlock->GetBlockDef().m_isOpaque )
		{
			//----------------------------------------------------------------------------------------------------------------------
			// Debug code
			//----------------------------------------------------------------------------------------------------------------------
//			IntVec3 worldBlockCoords = GetWorldBlockCoordsFromBlockIter( blockIter );
//			if ( worldBlockCoords == IntVec3(-32,15,65) )
//			{
//				int x = 0;
//			}
			MarkLightingDirty( blockIter );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void World::ProcessDirtyLightBlock( BlockIterator currentBlock )
{
	// Make sure it still exists
	Block* dirtyBlock = currentBlock.GetBlock();
	if ( dirtyBlock == nullptr )
	{
		return;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// Debug info
	IntVec3 worldBlockCoords	= GetWorldBlockCoordsFromBlockIter( currentBlock );
	IntVec2 chunkCoords			= currentBlock.m_currentChunk->m_chunkCoords;
	IntVec3 localBlockCoords	= currentBlock.m_currentChunk->GetLocalBlockCoordsFromIndex( currentBlock.m_blockIndex );
	

	// 3. Get current light level of this block
	BlockDef const& blockDef	 = dirtyBlock->GetBlockDef();	// get block Def
	int currentIndoorLight		 = dirtyBlock->GetIndoorLightInfluence();
	int currentOutdoorLight		 = dirtyBlock->GetOutdoorLightInfluence();

	// 4. Get neighbors
	BlockIterator northNeighbor	 = currentBlock.GetNorthNeighborBlock(); 
	BlockIterator southNeighbor	 = currentBlock.GetSouthNeighborBlock();
	BlockIterator eastNeighbor	 = currentBlock.GetEastNeighborBlock();
	BlockIterator westNeighbor	 = currentBlock.GetWestNeighborBlock();
	BlockIterator skyNeighbor	 = currentBlock.GetSkyNeighborBlock();
	BlockIterator groundNeighbor = currentBlock.GetGroundNeighborBlock();
	
	// 5. Compute the block's "correct" light influence level 
	int correctIndoorLight   = blockDef.m_indoorEmissionLight;			// get indoor emission light value 
	int correctOutdoorLight  = 0;
	if ( dirtyBlock->CanBlockSeeTheSky() )			
	{
		// Set outdoor light value correctly based on checking if the block is/isn't sky
		correctOutdoorLight = MAX_LIGHT_INFLUENCE;
	}
	if ( !blockDef.m_isOpaque )	
	{
		IncreaseLightLevelToAtLeastOneLessThanNeighbor( correctIndoorLight, correctOutdoorLight, northNeighbor , currentBlock );
		IncreaseLightLevelToAtLeastOneLessThanNeighbor( correctIndoorLight, correctOutdoorLight, southNeighbor , currentBlock );
		IncreaseLightLevelToAtLeastOneLessThanNeighbor( correctIndoorLight, correctOutdoorLight, eastNeighbor  , currentBlock );
		IncreaseLightLevelToAtLeastOneLessThanNeighbor( correctIndoorLight, correctOutdoorLight, westNeighbor  , currentBlock );
		IncreaseLightLevelToAtLeastOneLessThanNeighbor( correctIndoorLight, correctOutdoorLight, skyNeighbor   , currentBlock );
		IncreaseLightLevelToAtLeastOneLessThanNeighbor( correctIndoorLight, correctOutdoorLight, groundNeighbor, currentBlock );
	}

	// 6. Compare if currentLight is != correctLight	
	// If any of our current values are wrong
	if ( currentIndoorLight != correctIndoorLight || currentOutdoorLight != correctOutdoorLight )
	{
		// Correct own light values
		dirtyBlock->SetIndoorLightInfluence(  char(correctIndoorLight ) );
		dirtyBlock->SetOutdoorLightInfluence( char(correctOutdoorLight) );
	
		// Mark neighboring blocks as lighting dirty
		MarkLightingDirtyIfBlockIsNotOpqaue( eastNeighbor   );
		MarkLightingDirtyIfBlockIsNotOpqaue( westNeighbor   );
		MarkLightingDirtyIfBlockIsNotOpqaue( northNeighbor  );
		MarkLightingDirtyIfBlockIsNotOpqaue( southNeighbor  );
		MarkLightingDirtyIfBlockIsNotOpqaue( skyNeighbor	);
		MarkLightingDirtyIfBlockIsNotOpqaue( groundNeighbor );

		// Dirty all of our (non-opaque) neighbors	// 4 horizontal neighbor bocks
//		eastNeighbor.m_currentChunk->m_chunkCoords;
		DirtyChunkMesh( northNeighbor );
		DirtyChunkMesh( southNeighbor );
		DirtyChunkMesh( eastNeighbor  );
		DirtyChunkMesh( westNeighbor  );

		// Mark currentChunk as DIRTY
		DirtyChunkMesh( currentBlock );
//		blockIter.m_currentChunk->m_isMeshDirty = true;
	}

	// Mark this block as NO longer dirty	
	dirtyBlock->SetLightNotDirty();
}

//----------------------------------------------------------------------------------------------------------------------
void World::IncreaseLightLevelToAtLeastOneLessThanNeighbor( int& correctIndoorLight, int& correctOutdoorLight, BlockIterator neighborBlock, BlockIterator currentBlock )
{
	if ( neighborBlock.m_currentChunk == nullptr || !neighborBlock.IsBlockIndexValid() || !currentBlock.IsBlockIndexValid() )
	{
		return;
	}

	// Check all neighbors get highestNeighborIndoorLightValue and highestNeighborOutdoorLightValue
	int highestIndoorLight  = ComputeBlockCorrectIndoorLight(  neighborBlock, currentBlock );
	int highestOutdoorLight = ComputeBlockCorrectOutdoorLight( neighborBlock, currentBlock );

	if ( correctOutdoorLight < highestOutdoorLight )
	{
		correctOutdoorLight = highestOutdoorLight;
	}
	if ( correctIndoorLight < highestIndoorLight )
	{
		correctIndoorLight = highestIndoorLight;
	}
}

//----------------------------------------------------------------------------------------------------------------------
int World::GetBrightestNeighborIndoorLightValue( BlockIterator neighbor )
{
	int correctLightValue = 0;

	// Check all neighbors get highestNeighborIndoorLightValue
	BlockIterator northNeighbor = neighbor.GetNorthNeighborBlock();
	if ( (northNeighbor.m_blockIndex >= 0) && (northNeighbor.m_blockIndex < MAX_BLOCKS_PER_CHUNK) )
	{
		if ( (northNeighbor.m_currentChunk != nullptr) && (northNeighbor.GetBlock() != nullptr) )
		{
			int northIndoorLightInfluence = northNeighbor.GetBlock()->GetIndoorLightInfluence();
			if ( northIndoorLightInfluence > correctLightValue )
			{
				correctLightValue = northIndoorLightInfluence;
			}		
		}
	}
	BlockIterator southNeighbor = neighbor.GetSouthNeighborBlock();
	if ( (southNeighbor.m_blockIndex >= 0) && (southNeighbor.m_blockIndex < MAX_BLOCKS_PER_CHUNK) )
	{
		if ( (southNeighbor.m_currentChunk != nullptr) && (southNeighbor.GetBlock() != nullptr) )
		{
			int southIndoorLightInfluence = southNeighbor.GetBlock()->GetIndoorLightInfluence();
			if ( southIndoorLightInfluence > correctLightValue )
			{
				correctLightValue = southIndoorLightInfluence;
			}
		}
	}
	BlockIterator eastNeighbor = neighbor.GetEastNeighborBlock();
	if ( (eastNeighbor.m_blockIndex >= 0) && (eastNeighbor.m_blockIndex < MAX_BLOCKS_PER_CHUNK) )
	{
		if ( (eastNeighbor.m_currentChunk != nullptr) && (eastNeighbor.GetBlock() != nullptr) )
		{
			int eastIndoorLightInfluence = eastNeighbor.GetBlock()->GetIndoorLightInfluence();
			if ( eastIndoorLightInfluence > correctLightValue )
			{
				correctLightValue = eastIndoorLightInfluence;
			}
		}
	}
	BlockIterator westNeighbor = neighbor.GetWestNeighborBlock();
	if ( (westNeighbor.m_blockIndex >= 0) && (westNeighbor.m_blockIndex < MAX_BLOCKS_PER_CHUNK) )
	{
		if ( (westNeighbor.m_currentChunk != nullptr) && (westNeighbor.GetBlock() != nullptr) )
		{
			int westIndoorLightInfluence = westNeighbor.GetBlock()->GetIndoorLightInfluence();
			if ( westIndoorLightInfluence > correctLightValue )
			{
				correctLightValue = westIndoorLightInfluence;
			}	
		}
	}
	BlockIterator skyNeighbor = neighbor.GetSkyNeighborBlock();
	if ( (skyNeighbor.m_blockIndex >= 0) && (skyNeighbor.m_blockIndex < MAX_BLOCKS_PER_CHUNK) )
	{
		if ( (skyNeighbor.m_currentChunk != nullptr) && (skyNeighbor.GetBlock() != nullptr) )
		{
			int skyIndoorLightInfluence = skyNeighbor.GetBlock()->GetIndoorLightInfluence();
			if ( skyIndoorLightInfluence > correctLightValue )
			{
				correctLightValue = skyIndoorLightInfluence;
			}	
		}
	}
	BlockIterator groundNeighbor = neighbor.GetGroundNeighborBlock();
	if ( (groundNeighbor.m_blockIndex >= 0) && (groundNeighbor.m_blockIndex < MAX_BLOCKS_PER_CHUNK) )
	{
		if ( (groundNeighbor.m_currentChunk != nullptr) && (groundNeighbor.GetBlock() != nullptr) )
		{
			int groundIndoorLightInfluence = groundNeighbor.GetBlock()->GetIndoorLightInfluence();
			if ( groundIndoorLightInfluence > correctLightValue )
			{
				correctLightValue = groundIndoorLightInfluence;
			}	
		}
	}

	return correctLightValue;
}

//----------------------------------------------------------------------------------------------------------------------
int World::GetBrightestNeighborOutdoorLightValue( BlockIterator neighbor )
{
	int correctLightValue = 0;
	 
	// Check all neighbors get highestNeighborOutdoorLightValue
	BlockIterator northNeighbor = neighbor.GetNorthNeighborBlock();
	if ( northNeighbor.IsBlockIndexValid() )
	{
		if ( (northNeighbor.m_currentChunk != nullptr) && (northNeighbor.GetBlock() != nullptr) )
		{
			int northOutdoorLightInfluence = northNeighbor.GetBlock()->GetOutdoorLightInfluence();
			if ( northOutdoorLightInfluence > correctLightValue )
			{
				correctLightValue = northOutdoorLightInfluence;
			}
		}
	}
	BlockIterator southNeighbor = neighbor.GetSouthNeighborBlock();
	if ( northNeighbor.IsBlockIndexValid() )
	{
		if ( (southNeighbor.m_currentChunk != nullptr) && (southNeighbor.GetBlock() != nullptr) )
		{
			int southOutdoorLightInfluence = southNeighbor.GetBlock()->GetOutdoorLightInfluence();
			if ( southOutdoorLightInfluence > correctLightValue )
			{
				correctLightValue = southOutdoorLightInfluence;
			}
		}
	}
	BlockIterator eastNeighbor = neighbor.GetEastNeighborBlock();
	if ( eastNeighbor.IsBlockIndexValid() )
	{
		if ( (eastNeighbor.m_currentChunk != nullptr) && (eastNeighbor.GetBlock() != nullptr) )
		{
			int eastOutdoorLightInfluence = eastNeighbor.GetBlock()->GetOutdoorLightInfluence();
			if ( eastOutdoorLightInfluence > correctLightValue )
			{
				correctLightValue = eastOutdoorLightInfluence;
			}
		}
	}
	BlockIterator westNeighbor = neighbor.GetWestNeighborBlock();
	if ( westNeighbor.IsBlockIndexValid() )
	{
		if ( (westNeighbor.m_currentChunk != nullptr) && (westNeighbor.GetBlock() != nullptr) )
		{
			int westOutdoorLightInfluence = westNeighbor.GetBlock()->GetOutdoorLightInfluence();
			if ( westOutdoorLightInfluence > correctLightValue )
			{
				correctLightValue = westOutdoorLightInfluence;
			}
		}
	}
	BlockIterator skyNeighbor = neighbor.GetSkyNeighborBlock();
	if ( skyNeighbor.IsBlockIndexValid() )
	{
		if ( (skyNeighbor.m_currentChunk != nullptr) && (skyNeighbor.GetBlock() != nullptr) )
		{
			int skyOutdoorLightInfluence = skyNeighbor.GetBlock()->GetOutdoorLightInfluence();
			if ( skyOutdoorLightInfluence > correctLightValue )
			{
				correctLightValue = skyOutdoorLightInfluence;
			}
		}
	}
	BlockIterator groundNeighbor = neighbor.GetGroundNeighborBlock();
	if ( groundNeighbor.IsBlockIndexValid() )
	{
		if ( (groundNeighbor.m_currentChunk != nullptr) && (groundNeighbor.GetBlock() != nullptr) )
		{
			int groundOutdoorLightInfluence = groundNeighbor.GetBlock()->GetOutdoorLightInfluence();
			if ( groundOutdoorLightInfluence > correctLightValue )
			{
				correctLightValue = groundOutdoorLightInfluence;
			}
		}
	}

	return correctLightValue;
}

//----------------------------------------------------------------------------------------------------------------------
void World::DirtyChunkMesh( BlockIterator& blockIter )
{
	if ( blockIter.m_currentChunk != nullptr )
	{
		blockIter.m_currentChunk->m_isMeshDirty = true;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void World::DebugDrawCavesForAllChunks() const
{
	std::vector< Vertex_PCU > caveDebugVerts;
	std::map< IntVec2, Chunk* >::const_iterator chunkIter;
	for ( chunkIter = m_activeChunkList.begin(); chunkIter != m_activeChunkList.end(); chunkIter++ )
	{
		Chunk const* currentChunk = chunkIter->second;
		if ( currentChunk )
		{
			currentChunk->DebugAddVertsForCaves( caveDebugVerts );
		}
	}

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( int( caveDebugVerts.size() ), caveDebugVerts.data() );
//	g_theRenderer->DrawVertexArray( int( g_debugDrawCavesList.size() ), g_debugDrawCavesList.data() );
}

//----------------------------------------------------------------------------------------------------------------------
void World::ToggleDebugRenderingFunctions() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Toggle rendering for debugChunkBoundary verts
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_debugDrawChunkBoundaries )
	{
		std::vector<Vertex_PCU> debugVerts;
		debugVerts.reserve( 2 * 3 * 2 * m_maxNumChunks );
		for ( auto iter = m_activeChunkList.begin(); iter != m_activeChunkList.end(); ++iter )
		{
			Chunk* const & currentChunk = iter->second;
			if ( currentChunk != nullptr )
			{
				currentChunk->AddDebugDrawChunkBoundaryVerts( debugVerts );
			}
		}
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->DrawVertexArray( int(debugVerts.size()), debugVerts.data() );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Toggle rendering for debug blockLightValue verts
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_debugDrawLightValues )
	{
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture( &g_font->GetTexture() );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->DrawVertexArray( int(m_debugTextList.size()), m_debugTextList.data() );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Toggle rendering for debugBlockIterValue verts
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_debugDrawCurrentBlockIter )
	{
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture( &g_font->GetTexture() );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->DrawVertexArray( int( m_debugBlockIterList.size() ), m_debugBlockIterList.data() );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Toggle rendering for debugCaves verts
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_debugDrawCaves )
	{
		DebugDrawCavesForAllChunks();

		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture( &g_font->GetTexture() );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->DrawVertexArray( int( g_debugDrawCavesList.size() ), g_debugDrawCavesList.data() );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Toggle rendering with worldShader / defaultShader
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_debugUseWorldShader )
	{
//		Shader* worldShader = g_theRenderer->CreateOrGetShaderByName( "Data/Shaders/World" );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Toggle rendering debugRaycast
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_debugDrawRaycast )
	{
		if ( !m_raycastResult.m_didImpact )
		{
			return;
		}

		std::vector<Vertex_PCU> debugVerts;

		AddVertsForSphere3D( debugVerts, m_raycastResult.m_impactPos, 0.05f, 16.0f, 32.0f );
		AddVertsForArrow3D( debugVerts, m_raycastResult.m_impactPos, (m_raycastResult.m_impactPos +  m_raycastResult.m_impactNormal), 0.05f, Rgba8::DARK_BLUE );

		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->DrawVertexArray( int( debugVerts.size() ), debugVerts.data() );
	}


}

//----------------------------------------------------------------------------------------------------------------------
void World::AddVertsForLightDebugBlock( std::vector<Vertex_PCU>& verts, BlockIterator blockIter )
{
	Vec3			blockCenterPos		= blockIter.GetBlockCenterInWorldPos();
	float			cellHeight			= 0.05f;
	int				indoorLightValue	= blockIter.GetBlock()->GetIndoorLightInfluence();
	int				outdoorLightValue	= blockIter.GetBlock()->GetOutdoorLightInfluence();
	AddVertsForBillboardText3D( verts, blockCenterPos + Vec3( 0.0f, 0.0f, cellHeight ), cellHeight, Stringf("%d", indoorLightValue  ), Rgba8::PURPLE  );
	AddVertsForBillboardText3D( verts, blockCenterPos - Vec3( 0.0f, 0.0f, cellHeight ), cellHeight, Stringf("%d", outdoorLightValue ), Rgba8::WHITE	  );
}

//----------------------------------------------------------------------------------------------------------------------
void World::AddVertsForBillboardText3D( std::vector<Vertex_PCU>& debugTextList, Vec3 textOrigin, float cellHeight, std::string const& text, Rgba8 textColor, float cellAspect )
{
	Mat44 cameraWorldToLocal	= g_theGame->m_player->m_worldCamera.GetViewMatrix();
	Mat44 cameraLocalToWorld	= cameraWorldToLocal.GetOrthoNormalInverse();
	Vec3  billboardIBasis		= cameraLocalToWorld.GetJBasis3D() * (-1.0f);
	Vec3  billboardJBasis		= cameraLocalToWorld.GetKBasis3D();
	g_font->AddVertsForText3D( debugTextList, textOrigin, billboardIBasis, billboardJBasis, cellHeight, text, textColor, cellAspect );
}

//----------------------------------------------------------------------------------------------------------------------
void World::AddVertsForLightDebugBlocks( std::vector<Vertex_PCU>& verts )
{
	// Calculate neighborhood bounds for rendering block light values 
	Vec3	playerPos					= GetPlayerPos();
	IntVec3 worldBlockCoords			= IntVec3( RoundDownToInt(playerPos.x), RoundDownToInt(playerPos.y), RoundDownToInt(playerPos.z) );
	int		searchRadius				= 1 + RoundDownToInt( m_debugBlockMaxDist );
	IntVec3 minNeighborhoodBlockCoords	= worldBlockCoords - IntVec3( searchRadius, searchRadius, int(m_debugBlockLayers) ); 
	IntVec3 maxNeighborhoodBlockCoords	= worldBlockCoords + IntVec3( searchRadius, searchRadius, int(m_debugBlockLayers) ); 
 
	// Loop through all blocks within range and render only valid blocks
	for ( int globalZ = minNeighborhoodBlockCoords.z; globalZ < maxNeighborhoodBlockCoords.z; globalZ++ )
	{
		for ( int globalY = minNeighborhoodBlockCoords.y; globalY < maxNeighborhoodBlockCoords.y; globalY++ )
		{
			for ( int globalX = minNeighborhoodBlockCoords.x; globalX < maxNeighborhoodBlockCoords.x; globalX++ )
			{
				IntVec3 currentWorldBlockCoords = IntVec3( globalX, globalY, globalZ );
				Vec3 blockCenter; 
				blockCenter.x			= float(currentWorldBlockCoords.x);
				blockCenter.y			= float(currentWorldBlockCoords.y); 
				blockCenter.z			= float(currentWorldBlockCoords.z);
				blockCenter				+= Vec3( 0.5f, 0.5f, 0.5f );
				Vec3 dispMeToBlock		= blockCenter - playerPos;
				float dotProductResult	= DotProduct3D( dispMeToBlock, g_theGame->m_player->m_orientationDegrees.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D() );
				if ( dotProductResult < 0.0f )
				{
					continue;
				}

				float distMeToBlock = GetDistanceSquared3D( blockCenter, playerPos );
				float distMinSquared = m_debugBlockMinDist * m_debugBlockMinDist;
				float distMaxSquared = m_debugBlockMaxDist * m_debugBlockMaxDist;
				if ( distMeToBlock < distMinSquared || distMeToBlock > distMaxSquared )
				{
					continue;
				}

				BlockIterator currentBlockIter = LocateBlock( currentWorldBlockCoords );
				if ( currentBlockIter.m_currentChunk != nullptr && currentBlockIter.GetBlock() != nullptr )
				{
					AddVertsForLightDebugBlock( verts, currentBlockIter );
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void World::AddVertsForDebugBlockIter( std::vector<Vertex_PCU>& verts, BlockIterator blockIter )
{
	Vec3	blockCenterPos		= blockIter.GetBlockCenterInWorldPos();
	float	cellHeight			= 0.05f;
	IntVec2 chunkCoords			= blockIter.m_currentChunk->m_chunkCoords;
	IntVec3 localBlockCoords	= blockIter.m_currentChunk->GetLocalBlockCoordsFromIndex( blockIter.m_blockIndex );

	AddVertsForBillboardText3D( verts, blockCenterPos + Vec3( 0.0f, 0.0f, cellHeight ), cellHeight, Stringf("%d,%d",		 chunkCoords.x,		 chunkCoords.y ), Rgba8::GREEN  );
	AddVertsForBillboardText3D( verts, blockCenterPos - Vec3( 0.0f, 0.0f, cellHeight ), cellHeight, Stringf("%d,%d,%d", localBlockCoords.x, localBlockCoords.y, localBlockCoords.z ), Rgba8::RED );
}

//----------------------------------------------------------------------------------------------------------------------
void World::TestBlockIterGetNeighbor()
{
	Chunk* currentChunk = GetChunkAtCoords( m_currentChunkCoords );
	if ( currentChunk != nullptr )
	{
		BlockIterator blockIter = BlockIterator( currentChunk, m_debugBlockIterCurrentIndex );

		if ( g_theInput->WasKeyJustPressed( 'I' ) )
		{
			m_debugBlockIterList.clear();
			BlockIterator northNeighbor = blockIter.GetNorthNeighborBlock();	
			AddVertsForDebugBlockIter( m_debugBlockIterList, northNeighbor );
			m_currentChunkCoords			= northNeighbor.m_currentChunk->m_chunkCoords;
			m_debugBlockIterCurrentIndex	= northNeighbor.m_blockIndex;
		}
		if ( g_theInput->WasKeyJustPressed( 'J' ) )
		{
			m_debugBlockIterList.clear();
			BlockIterator westNeighbor = blockIter.GetWestNeighborBlock();
			AddVertsForDebugBlockIter( m_debugBlockIterList, westNeighbor );
			m_currentChunkCoords			= westNeighbor.m_currentChunk->m_chunkCoords;
			m_debugBlockIterCurrentIndex	= westNeighbor.m_blockIndex;
		}
		if ( g_theInput->WasKeyJustPressed( 'K' ) )
		{
			m_debugBlockIterList.clear();
			BlockIterator southNeighbor = blockIter.GetSouthNeighborBlock();
			AddVertsForDebugBlockIter( m_debugBlockIterList, southNeighbor );
			m_currentChunkCoords			= southNeighbor.m_currentChunk->m_chunkCoords;
			m_debugBlockIterCurrentIndex	= southNeighbor.m_blockIndex;
		}
		if ( g_theInput->WasKeyJustPressed( 'L' ) )
		{
			m_debugBlockIterList.clear();
			BlockIterator eastNeighbor		= blockIter.GetEastNeighborBlock();
			AddVertsForDebugBlockIter( m_debugBlockIterList, eastNeighbor );
			m_currentChunkCoords			= eastNeighbor.m_currentChunk->m_chunkCoords;
			m_debugBlockIterCurrentIndex	= eastNeighbor.m_blockIndex;
		}
		if ( g_theInput->WasKeyJustPressed( 'U' ) )
		{
			m_debugBlockIterList.clear();
			BlockIterator skyNeighbor		= blockIter.GetSkyNeighborBlock();
			AddVertsForDebugBlockIter( m_debugBlockIterList, skyNeighbor );
			m_currentChunkCoords			= skyNeighbor.m_currentChunk->m_chunkCoords;
			m_debugBlockIterCurrentIndex	= skyNeighbor.m_blockIndex;
		}
		if ( g_theInput->WasKeyJustPressed( 'O' ) )
		{
			m_debugBlockIterList.clear();
			BlockIterator groundNeighbor	= blockIter.GetGroundNeighborBlock();
			AddVertsForDebugBlockIter( m_debugBlockIterList, groundNeighbor );
			m_currentChunkCoords			= groundNeighbor.m_currentChunk->m_chunkCoords;
			m_debugBlockIterCurrentIndex	= groundNeighbor.m_blockIndex;
		}
		if ( g_theInput->WasKeyJustPressed( 'R' ) )
		{
			m_debugBlockIterList.clear();
			m_currentChunkCoords			= m_startingChunkCoords;
			currentChunk					= GetChunkAtCoords( m_currentChunkCoords );
			BlockIterator currentBlockIter	= BlockIterator( currentChunk, m_debugBlockIterStartingIndex );
			AddVertsForDebugBlockIter( m_debugBlockIterList, currentBlockIter );
			m_currentChunkCoords			= currentBlockIter.m_currentChunk->m_chunkCoords;
			m_debugBlockIterCurrentIndex	= currentBlockIter.m_blockIndex;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void World::ForceCreateWorldFolder()
{
	std::string command = Stringf( "mkdir Saves\\World_%u", g_theGame->m_currentWorld->m_worldSeed );
	system( command.c_str() );	// Function to type stuff into the command prompt
}

//----------------------------------------------------------------------------------------------------------------------
void World::RaycastVsBlocks()
{
	// Toggle raycast update position
	if ( g_theInput->WasKeyJustPressed( 'R' ) )
	{
		// Stop updating raycast position
		m_shouldLockRaycast = !m_shouldLockRaycast;
		DebuggerPrintf("raycastStartPos TOGGLED!\n");
	}

	if ( g_debugDrawRaycast )
	{
		Player* player = g_theGame->m_player;
		if ( player != nullptr )
		{
			// Update raycast position
			if ( m_shouldLockRaycast )
			{
				// Do nothing
				DebuggerPrintf( "LOCK RAYCAST!\n" );
			}
			else
			{
				DebuggerPrintf( "UPDATE RAYCAST!\n" );
				m_raycastStartPos	= player->m_position;
				m_fwdNormal			= player->m_orientationDegrees.GetForwardDir_XFwd_YLeft_ZUp();
			}
				
			IntVec2 currentChunkCoords	= GetChunkCoordsFromWorldPos( Vec2( m_raycastStartPos.x, m_raycastStartPos.y ) );
			Chunk* currentChunk			= GetChunkAtCoords( currentChunkCoords );
			if ( currentChunk != nullptr )
			{	
				IntVec3 localBlockCoords				 = GetLocalBlockCoordsFromWorldPos( m_raycastStartPos );
				int currentBlockIndex					 = GetBlockIndexFromLocalBlockCoords( localBlockCoords );
				BlockIterator currentBlockIter			 = BlockIterator( currentChunk, currentBlockIndex );
				m_raycastResult							 = RaycastXYZ( m_raycastStartPos, m_fwdNormal, 8.0f, currentBlockIter	);
				m_raycastResult.m_rayStartPosition.z	-= 0.05f; 
				Vec3 rayEnd								 = m_raycastResult.m_rayStartPosition + (m_raycastResult.m_rayDirection * m_raycastResult.m_rayLength );
				if ( !m_raycastResult.m_didImpact )
				{
					// MISS
					DebugAddWorldArrow( m_raycastResult.m_rayStartPosition, rayEnd, 0.01f, 0.0f, Rgba8::CYAN, Rgba8::CYAN );
//					DebuggerPrintf("miss, fwdNormal: %0.2f, %0.2f, %0.2f\n", fwdNormal.x, fwdNormal.y, fwdNormal.z );
				}
				else if ( m_raycastResult.m_didImpact )
				{
					// HIT
					DebugAddWorldArrow( m_raycastResult.m_rayStartPosition, rayEnd, 0.01f, 0.0f, Rgba8::MAGENTA, Rgba8::MAGENTA );
//					DebuggerPrintf("hit, fwdNormal: %0.2f, %0.2f, %0.2f\n", fwdNormal.x, fwdNormal.y, fwdNormal.z );
				}
			}

		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
GameRaycastResult3D World::RaycastXYZ( Vec3 const& rayStartPos, Vec3 const& rayFwdNormal, float rayMaxDist, BlockIterator currentBlockIter )
{
	// Initialize raycast results
	GameRaycastResult3D raycastHitResult;
	raycastHitResult.m_rayStartPosition = rayStartPos;
	raycastHitResult.m_rayDirection		= rayFwdNormal;
	raycastHitResult.m_rayLength		= rayMaxDist;

	GameRaycastResult3D raycastMissResult;
	raycastMissResult.m_rayStartPosition = rayStartPos;
	raycastMissResult.m_rayDirection	 = rayFwdNormal;
	raycastMissResult.m_rayLength		 = rayMaxDist;

	BlockIterator impactBlockIter = currentBlockIter;

	//----------------------------------------------------------------------------------------------------------------------
	// Calculate X crossing information
	float fwdDistPerXCrossing	= 1.0f / abs( rayFwdNormal.x );
	int tileStepDirectionX		= 0;
	if ( rayFwdNormal.x < 0 )
	{
		tileStepDirectionX = -1;
	}
	else
	{
		tileStepDirectionX = 1;
	}
	float tileX						= floorf( rayStartPos.x );
	float xAtFirstCrossing			= tileX + ( (tileStepDirectionX + 1) / 2.0f ) ;					// X-coordinate of the first x-crossing (Round up east or down west)
	float xDistToFirstXCrossing		= xAtFirstCrossing - rayStartPos.x;								// How far on X we must go until the first x-crossing
	float fwdDistAtNextXCrossing	= fabsf( xDistToFirstXCrossing ) * fwdDistPerXCrossing;			// How far forward we must go until the first x-crossing

	//----------------------------------------------------------------------------------------------------------------------
	// Calculate Y crossing information
	float fwdDistPerYCrossing	= 1.0f / abs( rayFwdNormal.y );
	int tileStepDirectionY		= 0;
	if ( rayFwdNormal.y < 0 )
	{
		tileStepDirectionY = -1;
	}
	else
	{
		tileStepDirectionY = 1;
	}
	float tileY						= floorf( rayStartPos.y );
	float yAtFirstCrossing			= tileY + ( (tileStepDirectionY + 1) / 2.0f ) ;					// Y-coordinate of the first y-crossing (Round up north or down south)
	float yDistToFirstYCrossing		= yAtFirstCrossing - rayStartPos.y;								// How far on Y we must go until the first y-crossing
	float fwdDistAtNextYCrossing	= fabsf( yDistToFirstYCrossing ) * fwdDistPerYCrossing;			// How far forward we must go until the first y-crossing

	//----------------------------------------------------------------------------------------------------------------------
	// Calculate Z crossing information
	float fwdDistPerZCrossing	= 1.0f / abs( rayFwdNormal.z );
	int tileStepDirectionZ		= 0;
	if ( rayFwdNormal.z < 0 )
	{
		tileStepDirectionZ = -1;
	}
	else
	{
		tileStepDirectionZ = 1;
	}
	float tileZ						= floorf( rayStartPos.z );
	float zAtFirstCrossing			= tileZ + ( (tileStepDirectionZ + 1) / 2.0f ) ;					// Z-coordinate of the first z-crossing (Round up skyward or down groundward)
	float zDistToFirstZCrossing		= zAtFirstCrossing - rayStartPos.z;								// How far on Z we must go until the first z-crossing
	float fwdDistAtNextZCrossing	= fabsf( zDistToFirstZCrossing ) * fwdDistPerZCrossing;			// How far forward we must go until the first z-crossing

	//----------------------------------------------------------------------------------------------------------------------
	// Determine next crossing axis (X or Y or Z)
	while ( true )
	{
//		DebuggerPrintf( "WHILE LOOP\n" );

		//----------------------------------------------------------------------------------------------------------------------
		// Step along X-axis IF next closest ray crossing is along X-axis 
		if ( (fwdDistAtNextXCrossing <= fwdDistAtNextYCrossing) && (fwdDistAtNextXCrossing <= fwdDistAtNextZCrossing) )
		{
//			DebuggerPrintf( "X CLOSEST! \n" );

			// Check if raycast missed
			if ( fwdDistAtNextXCrossing > rayMaxDist )
			{
				//  Return raycast MISSED!
				return raycastMissResult;
			}

			// Step along X-axis (east or west) from one block to another
			tileX += tileStepDirectionX;
			if ( tileStepDirectionX < 0 )
			{
				impactBlockIter = impactBlockIter.GetWestNeighborBlock();
			}
			else
			{
				impactBlockIter = impactBlockIter.GetEastNeighborBlock();
			}


			if ( impactBlockIter.IsBlockIndexValid() )
			{
				Block* block = impactBlockIter.GetBlock();
				if ( block != nullptr )
				{
					if ( !IsBlockAtCoordsAir( impactBlockIter ) )
					{
	//					DebuggerPrintf("IMPACT X");

						//  Return raycast HIT!
						raycastHitResult.m_didImpact		= true;
						raycastHitResult.m_impactDist		= fwdDistAtNextXCrossing;
						raycastHitResult.m_impactPos		= rayStartPos + ( rayFwdNormal * raycastHitResult.m_impactDist );
						raycastHitResult.m_impactNormal		= Vec3( float(-tileStepDirectionX), 0.0f, 0.0f );
						raycastHitResult.m_currentBlockIter   = impactBlockIter;
						raycastHitResult.m_impactBlockFaceDir = raycastHitResult.m_impactNormal;
						return raycastHitResult;
					}
				}
			}

			// Keep stepping along X-axis
			fwdDistAtNextXCrossing += fwdDistPerXCrossing;
			if ( ( fwdDistAtNextXCrossing == 0.0f ) && ( fwdDistPerXCrossing == 0.0f ) )
			{
				return raycastMissResult;
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Step along Y-axis IF next closest ray crossing is along Y-axis 
		else if ( (fwdDistAtNextYCrossing < fwdDistAtNextXCrossing) && (fwdDistAtNextYCrossing < fwdDistAtNextZCrossing ) )
		{
//			DebuggerPrintf( "Y CLOSEST! \n" );

			// Check if raycast missed
			if ( fwdDistAtNextYCrossing > rayMaxDist )
			{
				//  Return raycast MISSED!
				return raycastMissResult;
			}

			// Step along Y-axis (north or south ) from one block to another
			tileY += tileStepDirectionY;
			if ( tileStepDirectionY < 0 )
			{
				impactBlockIter = impactBlockIter.GetSouthNeighborBlock();
			}
			else
			{
				impactBlockIter = impactBlockIter.GetNorthNeighborBlock();
			}

			if ( impactBlockIter.IsBlockIndexValid() )
			{
				Block* block = impactBlockIter.GetBlock();
				if ( block != nullptr )
				{
					if ( !IsBlockAtCoordsAir( impactBlockIter ) )
					{
						DebuggerPrintf( "IMPACT Y" );

						//  Return raycast HIT!
						raycastHitResult.m_didImpact		= true;
						raycastHitResult.m_impactDist		= fwdDistAtNextYCrossing;
						raycastHitResult.m_impactPos		= rayStartPos + ( rayFwdNormal * raycastHitResult.m_impactDist );
						raycastHitResult.m_impactNormal		= Vec3( 0.0f, float(-tileStepDirectionY), 0.0f );
						raycastHitResult.m_currentBlockIter   = impactBlockIter;
						raycastHitResult.m_impactBlockFaceDir = raycastHitResult.m_impactNormal;
						return raycastHitResult;
					}
				}
			}

			// Keep stepping along Y-axis
			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
			if ( ( fwdDistAtNextYCrossing == 0.0f ) && ( fwdDistPerYCrossing == 0.0f ) )
			{
				return raycastMissResult;
			}
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Step along Z-axis IF next closest ray crossing is along Z-axis 
		else if ( (fwdDistAtNextZCrossing < fwdDistAtNextXCrossing) && (fwdDistAtNextZCrossing <= fwdDistAtNextYCrossing) )
		{
			DebuggerPrintf( "Z CLOSEST! \n" );

			// Check if raycast missed
			if ( fwdDistAtNextZCrossing > rayMaxDist )
			{
				//  Return raycast MISSED!
				return raycastMissResult;
			}

			// Step along Z-axis (sky or ground) from one block to another
			tileZ += tileStepDirectionZ;
			if ( tileStepDirectionZ < 0 )
			{
				impactBlockIter = impactBlockIter.GetGroundNeighborBlock();
			}
			else
			{
				impactBlockIter = impactBlockIter.GetSkyNeighborBlock();
			}

			if ( impactBlockIter.IsBlockIndexValid() )
			{
				Block* block = impactBlockIter.GetBlock();
				if ( block != nullptr )
				{
					if ( !IsBlockAtCoordsAir( impactBlockIter ) )
					{
						DebuggerPrintf( "IMPACT Z" );

						//  Return raycast HIT!
						raycastHitResult.m_didImpact		= true;
						raycastHitResult.m_impactDist		= fwdDistAtNextZCrossing;
						raycastHitResult.m_impactPos		= rayStartPos + ( rayFwdNormal * raycastHitResult.m_impactDist );
						raycastHitResult.m_impactNormal		= Vec3( 0.0f, 0.0f, float(-tileStepDirectionZ) );
						raycastHitResult.m_currentBlockIter   = impactBlockIter;
						raycastHitResult.m_impactBlockFaceDir = raycastHitResult.m_impactNormal;
						return raycastHitResult;
					}
				}
			}

			// Keep stepping along Z-axis
			fwdDistAtNextZCrossing += fwdDistPerZCrossing;
			if ( ( fwdDistAtNextZCrossing == 0.0f ) && ( fwdDistPerZCrossing == 0.0f ) )
			{
				return raycastMissResult;
			}
			
		}

		DebuggerPrintf("x: %0.2f, y: %0.2f, z: %0.2f\n", fwdDistAtNextXCrossing, fwdDistAtNextYCrossing, fwdDistAtNextZCrossing );
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool World::IsBlockAtCoordsAir( BlockIterator currentBlockIter )
{
	bool blockIsAir = false;
	// Check if THIS block is AIR
	// I need to access the blockTypeIndex
	// I need to get my current chunk to ask "what is THIS block's type"
//	BlockIterator currentBlockIter = LocateBlock( worldBlockCoord );
	
	// Check if block is Air type (0 == air)
	if ( currentBlockIter.GetBlock()->m_blockTypeIndex == 0 )
	{
		blockIsAir = true;
	}

	return blockIsAir;	
}

//----------------------------------------------------------------------------------------------------------------------
bool operator<( IntVec2 const& a, IntVec2 const& b )
{
	if ( a.y < b.y )
	{
		return true;
	}
	else if ( b.y < a.y )
	{
		return false;
	}
	else
	{
		return a.x < b.x;
	}
}
