#include "Game/Game.hpp"
#include "Game/Chunk.hpp"
#include "Game/World.hpp"
#include "Game/Block.hpp"
#include "Game/BlockDef.hpp"
#include "Game/GameCommon.hpp"
#include "Game/BlockTemplate.hpp"

#include "Engine/ThirdParty/Squirrel/Noise/RawNoise.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/ThirdParty/Squirrel/Noise/SmoothNoise.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include <fstream>

#include <thread>
#include <mutex>

//----------------------------------------------------------------------------------------------------------------------
Chunk::Chunk( IntVec2 chunkCoords )
	: m_chunkCoords( chunkCoords )
{
	SetWorldBounds();

	// Reserving chunk verts
	constexpr int VERTS_PER_TRI			= 6;
	constexpr int TRIS_PER_FACE			= 2;
	constexpr int APPROX_VISIBLE_FACES	= 2500;
	constexpr int NUM_VERTS_TO_RESERVE	= VERTS_PER_TRI * TRIS_PER_FACE * APPROX_VISIBLE_FACES;
	m_cpuVerts.reserve( NUM_VERTS_TO_RESERVE );	
														
	// Create all blocks in this chunk
	m_blockList = new Block[ MAX_BLOCKS_PER_CHUNK ];

	// Create Vertex Buffer
	m_gpuVerts = g_theRenderer->CreateVertexBuffer( (sizeof(Vertex_PCU) * 1), sizeof(Vertex_PCU) );			// #Question multiply by 1??

	//----------------------------------------------------------------------------------------------------------------------
	// Load existing Chunk if it exists
	//----------------------------------------------------------------------------------------------------------------------
	m_chunkWasLoadedFromFile = LoadSavedChunkOnDisk();
	if ( !m_chunkWasLoadedFromFile )
	{
		// Only regenerate if chunk was NOT loaded from file
//		RegenerateBlocksInChunk();

		// Generate blocks using worker threads (multi-threading) instead of generating blocks through the main thread
		ChunkGenerateJob* chunkGenerateJob = new ChunkGenerateJob( this,  1 );
		g_theJobSystem->PostNewJob( chunkGenerateJob );

		//----------------------------------------------------------------------------------------------------------------------
		// Add Caves
		//----------------------------------------------------------------------------------------------------------------------
//		AddCaves( g_theGame->m_currentWorld->m_caveSeed );
		//----------------------------------------------------------------------------------------------------------------------
	}
}

//----------------------------------------------------------------------------------------------------------------------
Chunk::~Chunk()
{
	// Delete and null gpuVerts
	if ( m_gpuVerts != nullptr )
	{
		delete m_gpuVerts;
		m_gpuVerts = nullptr;
	}

	// Null neighbor pointers
	m_northNeighbor = nullptr;
	m_southNeighbor = nullptr;
	m_eastNeighbor	= nullptr;
	m_westNeighbor	= nullptr;

	delete[] m_blockList;
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::Update()
{
	if ( m_isMeshDirty				&& 
		 m_northNeighbor != nullptr &&
		 m_southNeighbor != nullptr &&
		 m_eastNeighbor  != nullptr &&
		 m_westNeighbor  != nullptr 
	   )
	{
		RebuildVertexes();
		g_theGame->m_currentWorld->m_didChunkGetRebuilt = true;
	}
//	RebuildVertexes();		// Debug code for caves
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::Render() const
{
	if ( !m_cpuVerts.empty() )
	{
		g_theRenderer->DrawVertexBuffer( m_gpuVerts, int(m_cpuVerts.size() ) );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::AddDebugDrawChunkBoundaryVerts( std::vector<Vertex_PCU>& verts ) const
{
	Vec3 SW = Vec3( m_chunkWorldBounds.m_mins.x + 0.5f, m_chunkWorldBounds.m_mins.y + 0.5f, m_chunkWorldBounds.m_maxs.z );
	Vec3 SE = Vec3( m_chunkWorldBounds.m_maxs.x - 0.5f, m_chunkWorldBounds.m_mins.y + 0.5f, m_chunkWorldBounds.m_maxs.z );
	Vec3 NE = Vec3( m_chunkWorldBounds.m_maxs.x - 0.5f, m_chunkWorldBounds.m_maxs.y - 0.5f, m_chunkWorldBounds.m_maxs.z );
	Vec3 NW = Vec3( m_chunkWorldBounds.m_mins.x + 0.5f, m_chunkWorldBounds.m_maxs.y - 0.5f, m_chunkWorldBounds.m_maxs.z );
	AddVertsForQuad3D( verts, SW, SE, NE, NW, Rgba8::DARK_GRAY );
	AddVertsForQuad3D( verts, SW, NW, NE, SE, Rgba8::DARK_GRAY );

}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::AddVertsForBlock3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color, AABB2 const& skyUVs, AABB2 const& sideUVs, AABB2 const& groundUVs, int blockIndex, IntVec3 blockCoords ) 
{
	// Calculate verts
	Vec3 ESB = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z );		// ESB
	Vec3 ENB = Vec3( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z );		// ENB	
	Vec3 ENT = Vec3( bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z );		// ENT	
	Vec3 EST = Vec3( bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z );		// EST

	Vec3 WNB = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z );		// WNB	
	Vec3 WSB = Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z );		// WSB	
	Vec3 WST = Vec3( bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z );		// WST	
	Vec3 WNT = Vec3( bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z );		// WNT

	//----------------------------------------------------------------------------------------------------------------------
	// Filter hidden blocks to omit / render based on "m_isHiddenSurfaceRemoval"
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theGame->m_currentWorld->m_isHiddenSurfaceRemoval )
	{
		// Calculate cardinal blockIndex
		int eastBlockIndex		= blockIndex + 1;
		int westBlockIndex		= blockIndex - 1;
		int northBlockIndex		= blockIndex + CHUNK_SIZE_X;
		int southBlockIndex		= blockIndex - CHUNK_SIZE_X;
		int groundBlockIndex	= blockIndex - CHUNK_BLOCKS_PER_LAYER;
		int skyBlockIndex		= blockIndex + CHUNK_BLOCKS_PER_LAYER;

		// Reset blockIndex if out of bounds of blockDefList	
		if ( eastBlockIndex < 0 || eastBlockIndex >= MAX_BLOCKS_PER_CHUNK )
		{
			eastBlockIndex = MAX_BLOCKS_PER_CHUNK - 1;
		}
		if ( westBlockIndex < 0 || westBlockIndex >= MAX_BLOCKS_PER_CHUNK )
		{
			westBlockIndex = MAX_BLOCKS_PER_CHUNK - 1;
		}
		if ( northBlockIndex < 0 || northBlockIndex >= MAX_BLOCKS_PER_CHUNK )
		{
			northBlockIndex = MAX_BLOCKS_PER_CHUNK - 1;
		}
		if ( southBlockIndex < 0 || southBlockIndex >= MAX_BLOCKS_PER_CHUNK )
		{
			southBlockIndex = MAX_BLOCKS_PER_CHUNK - 1;
		}
		if ( groundBlockIndex < 0 || groundBlockIndex >= MAX_BLOCKS_PER_CHUNK )
		{
			groundBlockIndex = MAX_BLOCKS_PER_CHUNK - 1;
		}
		if ( skyBlockIndex < 0 || skyBlockIndex >= MAX_BLOCKS_PER_CHUNK )
		{
			skyBlockIndex = MAX_BLOCKS_PER_CHUNK - 1;
		}

		// Get blockTypeIndex
//		int eastBlockTypeIndex		= m_blockList[eastBlockIndex  ].m_blockTypeIndex;
//		int westBlockTypeIndex		= m_blockList[westBlockIndex  ].m_blockTypeIndex;
//		int northBlockTypeIndex		= m_blockList[northBlockIndex ].m_blockTypeIndex;
//		int southBlockTypeIndex		= m_blockList[southBlockIndex ].m_blockTypeIndex;
//		int groundBlockTypeIndex	= m_blockList[groundBlockIndex].m_blockTypeIndex;
//		int skyBlockTypeIndex		= m_blockList[skyBlockIndex	  ].m_blockTypeIndex;	

		//----------------------------------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------------------------------
		// Code printing useful info when debugging
//		BlockIterator currentBlockIter	= BlockIterator( this, blockIndex );
//		int currentBlockTypeIndex		= currentBlockIter.GetBlock()->m_blockTypeIndex;
//		IntVec3 localBlockCoords		= GetLocalBlockCoordsFromIndex( blockIndex );
//		IntVec3 worldBlockCoords		= GetWorldBlockCoordsFromLocalBlockCoords( localBlockCoords );
//		if ( currentBlockTypeIndex == 10 )	// 10 = glowstone
//		if ( currentBlockTypeIndex == 8 && worldBlockCoords == IntVec3(-15,-15,64) )	//  8 = water
//		if ( worldBlockCoords == IntVec3(-15,-15,64) )	//  8 = water
//		{
//			int x = 5;
//		}
		//----------------------------------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------------------------------------------
		// Full Hidden Surface Removal
		//----------------------------------------------------------------------------------------------------------------------
		BlockIterator currentBlockIter	= BlockIterator( this, blockIndex );
		Block* eastNeighbor				= currentBlockIter.GetEastNeighborBlock().GetBlock();
		Block* westNeighbor				= currentBlockIter.GetWestNeighborBlock().GetBlock();
		Block* northNeighbor			= currentBlockIter.GetNorthNeighborBlock().GetBlock();
		Block* southNeighbor			= currentBlockIter.GetSouthNeighborBlock().GetBlock();
		Block* skyNeighbor				= currentBlockIter.GetSkyNeighborBlock().GetBlock();
		Block* groundNeighbor			= currentBlockIter.GetGroundNeighborBlock().GetBlock();
		
//		BlockIterator eastNeighbor		= BlockIterator( m_eastNeighbor, eastBlockIndex );
//		BlockIterator westNeighbor		= BlockIterator( m_westNeighbor, westBlockIndex );
//		BlockIterator northNeighbor		= BlockIterator( m_northNeighbor, northBlockIndex );
//		BlockIterator southNeighbor		= BlockIterator( m_southNeighbor, southBlockIndex );
//		BlockIterator skyNeighbor		= BlockIterator( this, skyBlockIndex );
//		BlockIterator groundNeighbor	= BlockIterator( this, groundBlockIndex );

		// Only addVerts for face if neighbor is NOT opaque OR currentQuad faces chunk edge/boundary
//		Rgba8 tint = Rgba8::WHITE;
//		if ( !BlockDef::m_blockDefList[eastBlockTypeIndex].m_isOpaque || localBlockCoords.x == (CHUNK_SIZE_X - 1) )
//		if ( !eastNeighbor.GetBlock()->GetBlockDef().m_isOpaque )
//		{
//			Rgba8 tint = GetColorFromLightInfluence( eastNeighbor, currentBlockIter );		// East
//			AddVertsForQuad3D( verts, ESB, ENB, ENT, EST, tint, sideUVs );						
//		}
////		if ( !BlockDef::m_blockDefList[westBlockTypeIndex].m_isOpaque || localBlockCoords.x == 0 )
//		if ( !westNeighbor.GetBlock()->GetBlockDef().m_isOpaque )
//		{
//			Rgba8 tint = GetColorFromLightInfluence( westNeighbor, currentBlockIter );
//			AddVertsForQuad3D( verts, WNB, WSB, WST, WNT, tint, sideUVs );						// West
//		}
////		if ( !BlockDef::m_blockDefList[northBlockTypeIndex].m_isOpaque || localBlockCoords.y == (CHUNK_SIZE_Y - 1) )
//		if ( !northNeighbor.GetBlock()->GetBlockDef().m_isOpaque )
//		{
//			Rgba8 tint = GetColorFromLightInfluence( northNeighbor, currentBlockIter );
//			AddVertsForQuad3D( verts, ENB, WNB, WNT, ENT, tint, sideUVs );						// North
//		}
////		if ( !BlockDef::m_blockDefList[southBlockTypeIndex].m_isOpaque || localBlockCoords.y == 0 )
//		if ( !southNeighbor.GetBlock()->GetBlockDef().m_isOpaque )
//		{
//			Rgba8 tint = GetColorFromLightInfluence( southNeighbor, currentBlockIter );
//			AddVertsForQuad3D( verts, WSB, ESB, EST, WST, tint, sideUVs );						// South
//		}
////		if ( !BlockDef::m_blockDefList[skyBlockTypeIndex].m_isOpaque ||	localBlockCoords.z == (CHUNK_SIZE_Z - 1) )
//		if ( !skyNeighbor.GetBlock()->GetBlockDef().m_isOpaque )
//		{
//			Rgba8 tint = GetColorFromLightInfluence( skyNeighbor, currentBlockIter );
//			AddVertsForQuad3D( verts, WST, EST, ENT, WNT, tint, skyUVs );						// Sky
//		}
//// 		if ( localBlockCoords.z > 0  && !BlockDef::m_blockDefList[groundBlockTypeIndex].m_isOpaque )
//		if ( !groundNeighbor.GetBlock()->GetBlockDef().m_isOpaque )
//		{
//			Rgba8 tint = GetColorFromLightInfluence( groundNeighbor, currentBlockIter );
//			AddVertsForQuad3D( verts, WNB, ENB, ESB, WSB, tint, groundUVs );					// Ground
//		}


		// Only addVerts for face if neighbor is NOT opaque OR currentQuad faces chunk edge/boundary
		if ( !IsBlockOpaque( eastNeighbor ) )
		{
			Rgba8 tint = eastNeighbor->GetColorFromLightInfluence();							// East
			AddVertsForQuad3D( verts, ESB, ENB, ENT, EST, tint, sideUVs );						
		}
		if ( !IsBlockOpaque( westNeighbor ) )
		{
			Rgba8 tint = westNeighbor->GetColorFromLightInfluence();
			AddVertsForQuad3D( verts, WNB, WSB, WST, WNT, tint, sideUVs );						// West
		}
		if ( !IsBlockOpaque( northNeighbor ) )
		{
			Rgba8 tint = northNeighbor->GetColorFromLightInfluence();
			AddVertsForQuad3D( verts, ENB, WNB, WNT, ENT, tint, sideUVs );						// North
		}
		if ( !IsBlockOpaque( southNeighbor ) )
		{
			Rgba8 tint = southNeighbor->GetColorFromLightInfluence();
			AddVertsForQuad3D( verts, WSB, ESB, EST, WST, tint, sideUVs );						// South
		}
		if ( !IsBlockOpaque( skyNeighbor ) )
		{
			Rgba8 tint = skyNeighbor->GetColorFromLightInfluence();
			AddVertsForQuad3D( verts, WST, EST, ENT, WNT, tint, skyUVs );						// Sky
		}
		if ( !IsBlockOpaque( groundNeighbor ) )
		{
			Rgba8 tint = groundNeighbor->GetColorFromLightInfluence();
			AddVertsForQuad3D( verts, WNB, ENB, ESB, WSB, tint, groundUVs );					// Ground
		}
	}
	else
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Render all faces (include hidden faces)
		//----------------------------------------------------------------------------------------------------------------------
		AddVertsForQuad3D( verts, ESB, ENB, ENT, EST, color, sideUVs   );		// East
		AddVertsForQuad3D( verts, WNB, WSB, WST, WNT, color, sideUVs   );		// West
		AddVertsForQuad3D( verts, ENB, WNB, WNT, ENT, color, sideUVs   );		// North
		AddVertsForQuad3D( verts, WSB, ESB, EST, WST, color, sideUVs   );		// South
		AddVertsForQuad3D( verts, WST, EST, ENT, WNT, color, skyUVs	   );		// Sky
		AddVertsForQuad3D( verts, WNB, ENB, ESB, WSB, color, groundUVs );		// Ground
	}
}

//----------------------------------------------------------------------------------------------------------------------
int Chunk::GetIndexFromSpriteCoord( IntVec2 spriteCoords ) const
{
	// Get spriteIndex from spriteCoords
	int	yOffset	= spriteCoords.y * SPRITESHEET_GRID_LAYOUT_X;	// 64 = size of blockSpriteSheet maxU
	int	index	= spriteCoords.x + yOffset;						// Calculate index
	return index;
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::RebuildVertexes()
{
	// Clear verts
	m_cpuVerts.clear();
	
	// Loop through blockList and render blocks != air
	for ( int blockIndex = 0; blockIndex < (MAX_BLOCKS_PER_CHUNK - 1); blockIndex++ )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Calculate spriteCoords then create block
		//----------------------------------------------------------------------------------------------------------------------

		// Get blockTypeIndex for currentBlock 
		int blockTypeIndex	= m_blockList[blockIndex].m_blockTypeIndex;

		// Reset blockTypeIndex if out of bounds of blockDefList	
		if ( blockTypeIndex >= BlockDef::m_blockDefList.size() )
		{
			// Reset to airType 
			blockTypeIndex = 0;	
		}

		// Check and skip if currentBlock !isVisible
		if ( !BlockDef::m_blockDefList[blockTypeIndex].m_isVisible )
		{
			continue;
		}

		// Get spriteCoords based on block type
		IntVec2 skySpriteCoords		= BlockDef::m_blockDefList[blockTypeIndex].m_skySprite;
		IntVec2 sideSpriteCoords	= BlockDef::m_blockDefList[blockTypeIndex].m_sideSprite;
		IntVec2 groundSpriteCoords	= BlockDef::m_blockDefList[blockTypeIndex].m_groundSprite;

		// Get index from SpriteCoords
		int skyIndex	= GetIndexFromSpriteCoord( skySpriteCoords	  );
		int sideIndex	= GetIndexFromSpriteCoord( sideSpriteCoords	  );
		int groundIndex	= GetIndexFromSpriteCoord( groundSpriteCoords );

		// Create empty spriteUVs
		AABB2 out_skyUV;
		AABB2 out_sideUVs; 
		AABB2 out_groundUVs;

		// Get spriteUVs based on index
		g_theGame->m_blockSpriteSheet->GetSpriteUVs(	 out_skyUV.m_mins,	    out_skyUV.m_maxs, static_cast<int>(skyIndex)	);
		g_theGame->m_blockSpriteSheet->GetSpriteUVs(   out_sideUVs.m_mins,	  out_sideUVs.m_maxs, static_cast<int>(sideIndex)   );
		g_theGame->m_blockSpriteSheet->GetSpriteUVs( out_groundUVs.m_mins,  out_groundUVs.m_maxs, static_cast<int>(groundIndex) );

		//----------------------------------------------------------------------------------------------------------------------
		// Calculate X, Y, Z positions
		IntVec3 position = GetLocalBlockCoordsFromIndex( blockIndex );

		// Debug test code
//		if ( position.z == 70 )
//		if ( blockTypeIndex == 10 )
//		{
//			int x = 1;
//		}

		// Create block
		AABB3 localBlockBounds;
		localBlockBounds.m_mins = Vec3( float(position.x) + m_chunkWorldBounds.m_mins.x,	float(position.y) + m_chunkWorldBounds.m_mins.y,	 float(position.z) + m_chunkWorldBounds.m_mins.z  );
		localBlockBounds.m_maxs = Vec3(				   localBlockBounds.m_mins.x + 1.0f,				   localBlockBounds.m_mins.y + 1.0f,					 localBlockBounds.m_mins.z + 1.0f );
		AddVertsForBlock3D( m_cpuVerts, localBlockBounds, Rgba8::WHITE, out_skyUV, out_sideUVs, out_groundUVs, blockIndex, position );
	}

	// Send verts from CPU to GPU 
	g_theRenderer->Copy_CPU_To_GPU( m_cpuVerts.data(), sizeof(m_cpuVerts[0]) * int( m_cpuVerts.size() ), m_gpuVerts, sizeof(Vertex_PCU) );

	// Set dirty flag to false once verts have been re-built
	m_isMeshDirty = false;
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::SetWorldBounds()
{
	m_chunkWorldBounds.m_mins.x = static_cast<float>( m_chunkCoords.x * CHUNK_SIZE_X );
	m_chunkWorldBounds.m_mins.y = static_cast<float>( m_chunkCoords.y * CHUNK_SIZE_Y );
	m_chunkWorldBounds.m_mins.z = 0.0f;

	m_chunkWorldBounds.m_maxs.x = static_cast<float>( m_chunkWorldBounds.m_mins.x + CHUNK_SIZE_X );
	m_chunkWorldBounds.m_maxs.y = static_cast<float>( m_chunkWorldBounds.m_mins.y + CHUNK_SIZE_Y );
	m_chunkWorldBounds.m_maxs.z = CHUNK_SIZE_Z; 
}

//----------------------------------------------------------------------------------------------------------------------
IntVec3 Chunk::GetLocalBlockCoordsFromIndex( int index )
{
	// Calculate X, Y, Z positions using bitShift && bitMask
	IntVec3 localPosition;
	localPosition.x = index & CHUNK_MAX_INDEX_X;
	localPosition.y = ( index >> CHUNK_BITS_X ) & CHUNK_MAX_INDEX_Y;
	localPosition.z = index >> ( CHUNK_BITS_X + CHUNK_BITS_Y );
	return localPosition;
}

//----------------------------------------------------------------------------------------------------------------------
IntVec3 Chunk::GetWorldBlockCoordsFromLocalBlockCoords( IntVec3 localBlockCoords )
{
	int worldY					= (m_chunkCoords.y * CHUNK_SIZE_Y) + localBlockCoords.y;
	int worldX					= (m_chunkCoords.x * CHUNK_SIZE_Y) + localBlockCoords.x;
	IntVec3 worldBlockCoords	= IntVec3( worldX, worldY, localBlockCoords.z );
	return worldBlockCoords;
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::RegenerateBlocksInChunk()
{
	// Initialize local RNG for THIS chunk
	RandomNumberGenerator localRNG;
	unsigned int chunkSeed = Get2dNoiseUint( m_chunkCoords.x, m_chunkCoords.y, g_theGame->m_currentWorld->m_worldSeed );
	localRNG.SetSeed( chunkSeed );

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize common chunk variables
	float mountainHeight				= 60.0f;
	float riverDepth					= 5.0f;
	float mountainElevation				= mountainHeight + riverDepth;
	float riverFloorHeight			    = CHUNK_SEA_LEVEL - riverDepth;
	float maxNumBlocksToReduceSeaLevel  = 15;
	for ( int columnLocalY = 0; columnLocalY < CHUNK_SIZE_Y; columnLocalY++ )
	{
		int columnGlobalY = columnLocalY + ( CHUNK_SIZE_Y * m_chunkCoords.y );

		for ( int columnLocalX = 0; columnLocalX < CHUNK_SIZE_X; columnLocalX++ )
		{
			int columnGlobalX = columnLocalX + ( CHUNK_SIZE_X * m_chunkCoords.x );
			
			//----------------------------------------------------------------------------------------------------------------------
			// Calculate column index 
			//----------------------------------------------------------------------------------------------------------------------
			int columnIndex = columnLocalX + ( CHUNK_SIZE_X * columnLocalY );

			//----------------------------------------------------------------------------------------------------------------------
			// Calculate grass height for current column
			//----------------------------------------------------------------------------------------------------------------------
			// Scale	= numBlocks or "distance" per noise re-calculation
			// Octave	= "amplitude" or "range of frequency" of noise
			//----------------------------------------------------------------------------------------------------------------------
			
			//----------------------------------------------------------------------------------------------------------------------
			// Hilli-ness
			//----------------------------------------------------------------------------------------------------------------------
			// Compute hilliness noise and range map [0,1]
			float hillinessNoise	= Compute2dPerlinNoise( float(columnGlobalX), float(columnGlobalY), 150.0f, 3, 0.5f, 2.0f, true, g_theGame->m_currentWorld->m_hillinessSeed ); 
			float hillinessScale	= 0.5f + ( 0.5f * hillinessNoise );	// positionalOffset + ( amplitude * hillinessNoise );	// RangeMap [0,1]
//			hillinessScale			= SmoothStep3( hillinessScale );	// Lerp groundZ changes 
//			hillinessScale			*= 90.0f;
			
			// Apply hilli-ness
			mountainHeight			= 60.0f;
			mountainHeight			*= hillinessScale;
			mountainElevation		= mountainHeight + riverDepth;

//			// Compute terrainNoise based on hilliness "factor"
//			float terrainNoise		= fabsf( Compute2dPerlinNoise( float(columnGlobalX), float(columnGlobalY), 100.0f, 6, 0.5f, 2.0f, true, g_theGame->m_currentWorld->m_worldSeed ) );	
//			terrainNoise			= hillinessScale * terrainNoise;
//			int groundHeightZ		= riverFloorHeight + int(terrainNoise);	// riverFloorHeight = (GROUND_HEIGHT_Z - 7)

			// Compute terrain height 
			float terrainNoise		= fabsf( Compute2dPerlinNoise( float(columnGlobalX), float(columnGlobalY), 200.0f, 7, 0.5f, 2.0f, true, g_theGame->m_currentWorld->m_worldSeed   ) );	
			terrainNoise			= SmoothStep5( terrainNoise );
			terrainNoise			= mountainElevation * terrainNoise;
			int groundHeightZ		= int(riverFloorHeight) + int(terrainNoise);	

			//----------------------------------------------------------------------------------------------------------------------
			// Ocean-ness 
			//----------------------------------------------------------------------------------------------------------------------
			// Compute ocean-ness at this column
			float oceanNoise						= fabsf( Compute2dPerlinNoise( float( columnGlobalX ), float( columnGlobalY ), 50.0f, 3, 0.5f, 2.0f, true, g_theGame->m_currentWorld->m_oceannessSeed ) );
			oceanNoise								= SmoothStep3( oceanNoise );
			m_oceanNoiseForEachColumn[columnIndex]	= oceanNoise;

			// Compute numBlocks to reduce groundHeight based on ocean-ness at this column
			float numBlocksToReduceGroundHeightThisColumn				= RangeMapClamped( oceanNoise, m_minOceanicThreshold, 1.0f, 0.0f, maxNumBlocksToReduceSeaLevel );
			groundHeightZ												-= int(numBlocksToReduceGroundHeightThisColumn);	// Ocean-ness pulls down groundHeight 
			m_groundPulledDownByOceanHeightForEachColumn[columnIndex]	= groundHeightZ;

			// This logic will loop through the chunk in 2D, for EACH column, then save those positions
			// And when I generate the trees, I use these positions as my TreeLocalZ positions
			// My Z positions are not wrong, but I have a bug where there are leaves at the "edge" of the chunk boudaries


			//----------------------------------------------------------------------------------------------------------------------
			// Set grass height for current column 
			//----------------------------------------------------------------------------------------------------------------------
			m_grassHeightsForEachColumn[columnIndex] = groundHeightZ;

//			//----------------------------------------------------------------------------------------------------------------------
//			// Trees
//			//----------------------------------------------------------------------------------------------------------------------
//			float treeNoise									  = Get2dNoiseZeroToOne( columnGlobalX, columnGlobalY, g_theGame->m_currentWorld->m_treeSeed );
//			m_treeNoiseForEachColumn[columnIndex]			  = treeNoise;
//			float forestDensityNoise						  = Compute2dPerlinNoise( float(columnGlobalX), float(columnGlobalY), 750.0f, 6, 0.5f, 2.0f, true, g_theGame->m_currentWorld->m_humiditySeed );
//			float forestDensityNoiseThisColumn				  = 0.5f + ( 0.5f * forestDensityNoise );
//			float minTreeNoiseThreshold						  = RangeMapClamped( forestDensityNoiseThisColumn, 0.5f, 0.8f, 1.0f, 0.9f );
//			m_forestNoiseForEachColumn[columnIndex]			  = forestDensityNoiseThisColumn;
//			m_minTreeNoiseThresholdForEachColumn[columnIndex] = minTreeNoiseThreshold;
//			if ( treeNoise == air && treeNoise > minTreeNoiseThreshold && localZ == ( groundHeightZ + 1 ) )
//			{
// 			   if ( groundHeightZ > CHUNK_SEA_LEVEL )
// 			   {
//					// Save off this tree position into a vector of tree positions
// 
//					// Set this block to tree	
// 
//					// Check if what type of tree should be here based on humidity 
//					if ( humidityAtThisColumn < humidityThreshold )
// 					{
// 						// Set this block type to other tree blockDef type
//					}
// 
// 
// 			   }
// 
//			}
// After setting all blocks in this chunk
// Loop through list of tree positions AND spawnBlockTemplateAtLocalCoords( templateType, localtreeOriginList[i] );

			//----------------------------------------------------------------------------------------------------------------------

			//----------------------------------------------------------------------------------------------------------------------
			// Temperature
			//----------------------------------------------------------------------------------------------------------------------
			// Compute temperature for this column
			float temperatureNoise					= Compute2dPerlinNoise( float(columnGlobalX), float(columnGlobalY), 50.0f, 3, 0.5f, 2.0f, true, g_theGame->m_currentWorld->m_temperatureSeed );
			float temperatureThisColumn				=  0.5f + ( 0.5f * temperatureNoise );
			temperatureThisColumn					+= 0.01f * Get2dNoiseNegOneToOne( columnGlobalX, columnGlobalY, g_theGame->m_currentWorld->m_temperatureSeed );
			m_temperatureForEachColumn[columnIndex]	= temperatureThisColumn;

			//----------------------------------------------------------------------------------------------------------------------
			// Humidity
			//----------------------------------------------------------------------------------------------------------------------
			// Compute humidity for this column
			float humidityNoise						= Compute2dPerlinNoise( float(columnGlobalX), float(columnGlobalY), 100.0f, 4, 0.5f, 2.0f, true, g_theGame->m_currentWorld->m_humiditySeed );
			float humidityThisColumn				= 0.5f + ( 0.5f * humidityNoise );
			m_humidityForEachColumn[columnIndex]	= humidityThisColumn;		

			//----------------------------------------------------------------------------------------------------------------------
			// Cave noise
			//----------------------------------------------------------------------------------------------------------------------
//			int caveSeed = g_theGame->m_currentWorld->m_caveSeed;
//			float caveNoise = Compute3dPerlinNoise( float(columnGlobalX), float(columnGlobalY), float(localZ), 50.0f, 4, 0.5f, 2.0f, true, caveSeed );
//			caveNoise		+= 0.1f * Get3dNoiseNegOneToOne( columnGlobalX, columnGlobalY, localZ, caveSeed );
//			m_caveNoiseForEachColumn[columnIndex] = caveNoise;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Determine and assign currentBlockType(s) per block
	//----------------------------------------------------------------------------------------------------------------------
	for ( int localZ = 0; localZ < CHUNK_SIZE_Z; localZ++ )
	{
		for ( int localY = 0; localY < CHUNK_SIZE_Y; localY++ )
		{
			for ( int localX = 0; localX < CHUNK_SIZE_X; localX++ )
			{
				// Calculate grass height at current column
				int columnIndex					= localX + ( CHUNK_SIZE_X * localY );
				int grassHeightAtCurrentColumn	= m_grassHeightsForEachColumn[columnIndex];

				// Calculate randNumDirtBlocks below groundHeight
				int randNumDirtBlocks	= localRNG.RollRandomIntInRange( 3, 4 );
				int dirtBlocksMinZ		= (grassHeightAtCurrentColumn - randNumDirtBlocks);

				//----------------------------------------------------------------------------------------------------------------------
				// Assign block types 
				//----------------------------------------------------------------------------------------------------------------------
				unsigned char blockType = 0;
				// Set blocks below grass && above dirt to dirt UNLESS humidity in this column sets the blocks to sand
				if ( localZ < grassHeightAtCurrentColumn && localZ >= dirtBlocksMinZ )
				{
					// Set to dirt
					blockType = 2;

					// Create sand blocks beneath grass blocks
					// Set dirt blocks to sand if this biome is humid enough
					if ( m_humidityForEachColumn[columnIndex] <= m_minHumidityThreshold )
					{
						int minSandHeight = grassHeightAtCurrentColumn - m_minSandHeightThreshold;
						if ( localZ < grassHeightAtCurrentColumn && localZ >= minSandHeight )
						{
							// 12 is sand
							blockType = 12;
						}
					}
				}
				else if ( localZ < dirtBlocksMinZ )
				{
					// Calculate random chance to be coal, iron, gold, or diamond
					bool isCoal			= localRNG.RollRandomChance(0.05f );
					bool isIron			= localRNG.RollRandomChance(0.02f );
					bool isGold			= localRNG.RollRandomChance(0.005f);
					bool isDiamond		= localRNG.RollRandomChance(0.001f);
					bool isGlowstone	= localRNG.RollRandomChance(0.001f);

					if ( isGlowstone )
					{
						// Set blockType glowstone and mark as dirty to re-calculate lighting
						blockType		= 10;
						int blockIndex  = g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords(IntVec3(localX, localY, localZ) );
						m_blockList[blockIndex].SetIsDirty();
					}
					if ( isDiamond )
					{
						// Set to diamond
						blockType = 7;
					}
					else if ( isGold )
					{
						// Set to gold
						blockType = 6;
					}
					else if ( isIron )
					{
						// Set to iron
						blockType = 5;
					}
					else if ( isCoal )
					{
						// Set to coal
						blockType = 4;
					}
					else
					{
						// Set to stone
						blockType = 1;
					}
				}
				else if ( localZ == grassHeightAtCurrentColumn )
				{
					// Set to grass
					blockType = 3;

					// Set grass blocks at GRASS HEIGHT to sand if humidity is below minThreshold
					// Creates breaches and "desert" areas
					if ( m_humidityForEachColumn[columnIndex] <= m_minHumidityThreshold )
					{
						// 12 is sand
						blockType = 12;
					}
					// Create "Sandy Beaches"
					// Replace grass blocks at SEA LEVEL to sand blocks if humidity is below maxThreshold
					else if ( localZ == CHUNK_SEA_LEVEL )
					{
						if ( m_humidityForEachColumn[columnIndex] <= m_moderateHumidityThreshold )
						{
							// 12 is sand
							blockType = 12;
						}
					}

					// Randomly place a few glowstones
					bool isGlowstone = localRNG.RollRandomChance( 0.1f );
					if ( isGlowstone )
					{
						blockType = 10;
					}
				}
				// Any blocks that would otherwise be “air” but whose z block coordinate is <= CHUNK_HEIGHT/2 is “water” OR "ice" depending on temperature threshold.		// (I.e., sea-level)
				else if ( (localZ >= riverFloorHeight) && (localZ <= CHUNK_SEA_LEVEL) ) 
				{
					// Create river
					// Set blockType to water
					blockType = 8;

					if ( m_temperatureForEachColumn[columnIndex] >= m_minIceTempThreshold )
					{
						// Set to water
						blockType = 8;
					}
					else	// If temp is super cold
					{
						float tempForThisColumn							= m_temperatureForEachColumn[columnIndex];
						float numBlocksToFreezeBelowGrassHeight			= RangeMap( tempForThisColumn, m_minIceTempThreshold, 0.0f, 1.0f, 5.0f );
						float minIceBlockZCoordAtThisColumn				= CHUNK_SEA_LEVEL - numBlocksToFreezeBelowGrassHeight;
//						int	  numBlocksBetweenSeaLevelAndGrassHeight	= CHUNK_SEA_LEVEL - grassHeightAtCurrentColumn;		// Chunk_Half_Height == SeaLevel
//						float numBlocksToFreezeBelowGrassHeight			= RangeMap( tempForThisColumn, m_minIceTempThreshold, 0.0f, 1.0f, float(numBlocksBetweenSeaLevelAndGrassHeight) );

//						float tempForThisColumn					= temperatureForEachColumn[columnIndex];
//						float maxNumIceBlocksBelowGrassHeight	= 12.0f;
//						float numIceBlocksThisColumn			= RangeMapClamped( tempForThisColumn, 0.0f, m_minIceTempThreshold, maxNumIceBlocksBelowGrassHeight, 0.0f );
//						float minIceBlockZCoordAtThisColumn		= CHUNK_SEA_LEVEL - numIceBlocksThisColumn;

						if ( localZ >= minIceBlockZCoordAtThisColumn )
						{
							// Set to ice
							blockType = 11;
						}
						else
						{
							// Set to water
							blockType = 8;
						}
					}
				}

				// Any blocks that would otherwise be “air” but whose z block coordinate is <= CHUNK_HEIGHT/2 is “water” instead.		// (I.e., sea-level)
//				else if ( (localZ > grassHeightAtCurrentColumn) && (localZ <= CHUNK_SEA_LEVEL) )
//				{ 
//					if ( m_temperatureForEachColumn[columnIndex] >= m_minIceTempThreshold )
//					{
//						// Set to water
//						blockType = 8;
//					}
//					else
//					{
//						float tempForThisColumn							= m_temperatureForEachColumn[columnIndex];
//						float numBlocksToFreezeBelowGrassHeight			= RangeMap( tempForThisColumn, m_minIceTempThreshold, 0.0f, 1.0f, 5.0f );
//						float minIceBlockZCoordAtThisColumn				= CHUNK_SEA_LEVEL - numBlocksToFreezeBelowGrassHeight;
//
////						float tempForThisColumn							= m_temperatureForEachColumn[columnIndex];
//						int	  numBlocksBetweenSeaLevelAndGrassHeight	= CHUNK_SEA_LEVEL - grassHeightAtCurrentColumn;		// Chunk_Half_Height == SeaLevel
//						float numBlocksToFreezeBelowGrassHeight			= RangeMap( tempForThisColumn, m_minIceTempThreshold, 0.0f, 1.0f, float(numBlocksBetweenSeaLevelAndGrassHeight) );
//						float numBlocksToFreezeBelowGrassHeight			= RangeMap( tempForThisColumn, m_minIceTempThreshold, 0.0f, 1.0f, 5.0f );
//						float minIceBlockZCoordAtThisColumn				= CHUNK_SEA_LEVEL - numBlocksToFreezeBelowGrassHeight;
//
////						float tempForThisColumn					= temperatureForEachColumn[columnIndex];
//						float maxNumIceBlocksBelowGrassHeight	= 12.0f;
//						float numIceBlocksThisColumn			= RangeMapClamped( tempForThisColumn, 0.0f, m_minIceTempThreshold, maxNumIceBlocksBelowGrassHeight, 0.0f );
//						float minIceBlockZCoordAtThisColumn		= CHUNK_SEA_LEVEL - numIceBlocksThisColumn;
//
//						if ( localZ >= minIceBlockZCoordAtThisColumn )
//						{
//							// Set to ice
//							blockType = 11;
//						}
//						else
//						{
//							// Set to water
//							blockType = 8;
//						}
//					}
//				}

//				else if ( m_treeNoiseForEachColumn[columnIndex] > m_minTreeNoiseThresholdForEachColumn[columnIndex] ) 
//				{
//					// Create neighbors (5x5 grid) mins and maxs 
//					IntVec2 currentTreeLocalBlockCoordsXY	= IntVec2( localX, localY );
//					IntVec2 treeNeighborBlockCoordsMaxs		= currentTreeLocalBlockCoordsXY + IntVec2( 2, 2 );
//					IntVec2 treeNeighborBlockCoordsMins		= currentTreeLocalBlockCoordsXY - IntVec2( 2, 2 );
//
//					// Loop through all treeNeighbors and check (5x5 grid), if (currentTreeNoise value < any neighborTreeNoise value)
//						// True: continue // Don't generate currentTree
//					float currentTreeNoise			= m_treeNoiseForEachColumn[columnIndex];
//					bool thisTreeShouldBeGenerated	= true;
//					for ( int treeNeighborWorldY = treeNeighborBlockCoordsMins.y; treeNeighborWorldY <= treeNeighborBlockCoordsMaxs.y; treeNeighborWorldY++ )
//					{  
//						for ( int treeNeighborWorldX = treeNeighborBlockCoordsMins.x; treeNeighborWorldX <= treeNeighborBlockCoordsMaxs.x; treeNeighborWorldX++ )
//						{
//							// Get appropriate column index based on currentTreeWorldCoords
//							int currentColumnIndex = treeNeighborWorldX + ( CHUNK_SIZE_X * treeNeighborWorldY );
//							if ( currentColumnIndex > 255 || currentColumnIndex < 0 )
//							{
//								continue;
//							}
//
//							float neighborTreeNoise = m_treeNoiseForEachColumn[currentColumnIndex];
//							if ( currentTreeNoise < neighborTreeNoise )
//							{
//								thisTreeShouldBeGenerated = false;
//								continue;
//							}
//						}
//					}
//					if ( thisTreeShouldBeGenerated )
//					{
//						// Else: generate this true with the logic below
//	 				   if ( grassHeightAtCurrentColumn >= CHUNK_SEA_LEVEL )
//	 				   {
//						   if ( localZ <= ( grassHeightAtCurrentColumn + 1 ) )
//						   {							
//	//							// Set this block to tree	
	//							blockType = 13;	// 13 is darkWood
	//							// Check if what type of tree should be here based on humidity 
	//							if ( m_humidityForEachColumn[columnIndex] <= m_minTreeTypeHumidityThreshold )
	//								{
	//									// Set this block type to other tree blockDef type
	//								blockType = 14;	// 14 is lightWood
	//							}
//							
//								// Save off this tree position into a vector of tree positions
////								m_treeToGenerateInLocalPositionList.push_back( IntVec3(localX, localY, localZ) );
//						   }
//	 				   }
//					}
//				}
//				// After setting all blocks in this chunk
//				// Loop through list of tree positions AND spawnBlockTemplateAtLocalCoords( templateType, localtreeOriginList[i] );

				else if ( localZ > grassHeightAtCurrentColumn )
				{
					// Set to air
					blockType = 0;	
				}

				//----------------------------------------------------------------------------------------------------------------------
				// Test code for creating caves
				//----------------------------------------------------------------------------------------------------------------------
				
				int columnGlobalX = localX + ( CHUNK_SIZE_X * m_chunkCoords.x );
				int columnGlobalY = localY + ( CHUNK_SIZE_Y * m_chunkCoords.y );

				int caveSeed							= g_theGame->m_currentWorld->m_caveSeed;
				float caveNoise							= Compute3dPerlinNoise( float(columnGlobalX), float(columnGlobalY), float(localZ), 50.0f, 4, 0.5f, 2.0f, true, caveSeed );
				caveNoise								+= 0.1f * Get3dNoiseNegOneToOne( columnGlobalX, columnGlobalY, localZ, caveSeed );
				m_caveNoiseForEachColumn[columnIndex]	= caveNoise;		

				int cloudSeed							= g_theGame->m_currentWorld->m_cloudSeed;
				float cloudNoise						= Compute3dPerlinNoise( float(columnGlobalX), float(columnGlobalY), float(localZ), 50.0f, 6, 0.5f, 2.0f, true, cloudSeed );
				cloudNoise								+= 0.1f * Get3dNoiseNegOneToOne( columnGlobalX, columnGlobalY, localZ, cloudSeed );
				m_caveNoiseForEachColumn[columnIndex]	= cloudNoise;		

//				if ( localZ > ( grassHeightAtCurrentColumn + 15) && 
//					 localZ < ( grassHeightAtCurrentColumn + 55) && 
//					 cloudNoise > m_minCloudNoiseThresholdForThisColumn )
				if ( localZ > 105  && 
					 localZ < 125 && 
					cloudNoise > m_minCloudNoiseThresholdForThisColumn )
				{
					// create clouds
//					m_cloudToGenerateInLocalPositionList.push_back( IntVec3( localX, localY, localZ ) );
//					m_cloudToGenerateInLocalPositionList.push_back( IntVec3( 0, 0, 120 ) );
//					blockType = 0;
//					blockType = 11;
				}

				//----------------------------------------------------------------------------------------------------------------------

				// Assign current block's type
				int blockIndex							  = localX + (CHUNK_SIZE_X * localY) + (CHUNK_BLOCKS_PER_LAYER * localZ);
				m_blockList[blockIndex].m_blockTypeIndex  = blockType;
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Test code for trees
	//----------------------------------------------------------------------------------------------------------------------
	// Create a larger grid of tree noise to check if any trees positions/leaves cross boundaries into MY chunk
	constexpr int MAX_TREE_RADIUS				= 2;	// CHUNK MAX X + WIDTH OF TREE AFTER CHUNK MAX AND CHUNK MINS X
	constexpr int EXTRA_BLOCK_ACROSS_BOUNDARY	= 1;
	constexpr int BLOCKS_RIGHT_OF_CHUNK			= MAX_TREE_RADIUS + EXTRA_BLOCK_ACROSS_BOUNDARY;
	constexpr int BLOCKS_BELOW_CHUNK			= MAX_TREE_RADIUS + EXTRA_BLOCK_ACROSS_BOUNDARY;
	constexpr int BLOCKS_ABOVE_CHUNK			= MAX_TREE_RADIUS + EXTRA_BLOCK_ACROSS_BOUNDARY;
	constexpr int BLOCKS_LEFT_OF_CHUNK			= MAX_TREE_RADIUS + EXTRA_BLOCK_ACROSS_BOUNDARY;

	constexpr int TREE_GRID_SIZE_X				= CHUNK_SIZE_X + BLOCKS_RIGHT_OF_CHUNK + BLOCKS_LEFT_OF_CHUNK;	
	constexpr int TREE_GRID_SIZE_Y				= CHUNK_SIZE_Y + BLOCKS_ABOVE_CHUNK + BLOCKS_BELOW_CHUNK;	
	constexpr int TREE_GRID_COUNT				= TREE_GRID_SIZE_X * TREE_GRID_SIZE_Y;
	
	//----------------------------------------------------------------------------------------------------------------------
	// Generate tree noise
	//----------------------------------------------------------------------------------------------------------------------
	float treeNoiseThisColumn[ TREE_GRID_COUNT ] = {};
	for ( int treeY = 0; treeY < TREE_GRID_SIZE_Y; treeY++ )
	{
		for ( int treeX = 0; treeX < TREE_GRID_SIZE_X; treeX++ )
		{
			int chunkMinsWorldX						= m_chunkCoords.x * CHUNK_SIZE_X;
			int chunkMinsWorldY						= m_chunkCoords.y * CHUNK_SIZE_Y;
			int treeSpaceMinsWorldX					= chunkMinsWorldX - BLOCKS_LEFT_OF_CHUNK;
			int treeSpaceMinsWorldY					= chunkMinsWorldY - BLOCKS_BELOW_CHUNK;
			int worldX								= treeSpaceMinsWorldX + treeX;
			int worldY								= treeSpaceMinsWorldY + treeY;
			float treeNoiseIndex					= Get2dNoiseZeroToOne( worldX, worldY, g_theGame->m_currentWorld->m_treeSeed );
			int treeSpaceIndex						= treeX + ( treeY * TREE_GRID_SIZE_X );
			treeNoiseThisColumn[treeSpaceIndex]		= treeNoiseIndex;
		}
	}

	std::vector<IntVec2> treeLocalXYPositionColumnList;
	std::vector<IntVec3> treeSpawnLocalCoordsToDoList;

	// Scan through the tree noise grid looking for values that are higher than their neighbors
	for ( int treeY = 1; treeY <= (TREE_GRID_SIZE_Y - 1); treeY++ )
	{
		for ( int treeX = 1; treeX <= (TREE_GRID_SIZE_X - 1); treeX++ )
		{
//			int worldBlockX							= (m_chunkCoords.x * CHUNK_SIZE_X ) + treeX - TREE_GRID_SIZE_X - 1;
//			int worldBlockY							= (m_chunkCoords.y * CHUNK_SIZE_Y ) + treeY - TREE_GRID_SIZE_Y - 1;
			int worldBlockX							= (m_chunkCoords.x * CHUNK_SIZE_X ) + treeX - BLOCKS_LEFT_OF_CHUNK;
			int worldBlockY							= (m_chunkCoords.y * CHUNK_SIZE_Y ) + treeY - BLOCKS_BELOW_CHUNK;
			float forestNoise						= 0.5f + 0.5f * Compute2dPerlinNoise( float(worldBlockX), float(worldBlockY), 100.0f, 1, 0.5f, 2.0f, true, g_theGame->m_currentWorld->m_treeSeed );
			float minTreeNoiseThresholdToSpawnHere  = RangeMapClamped( forestNoise, 0.5f, 1.0f, 1.0f, 0.9f );

			// Get currentTreeNoiseIndex
			int myTreeNoiseIndex = treeX + ( treeY * TREE_GRID_SIZE_X );
			float myTreeNoise	 = treeNoiseThisColumn[myTreeNoiseIndex];
			if ( myTreeNoise < minTreeNoiseThresholdToSpawnHere )
			{
				// Skip this tree if higher than treeThreshold
				continue;
			}
			// (treeX,treeY) is what we're checking for "is THIS a tree"?
			// But in order to know that, we have to see if the value at 
			// (treeX,treeY) is GREATER THAN the other 8 neighbor tree noise values
			// So we need to for-loop through the 8 nearby neighbors; if any of them 
			// has a tree noise equal or greater than ours, just stop (no tree here)
			
			// Check neighbor trees to see if currentTree has the highestTreeValue
			// Only generate if currentTree has the highestTreeValue
			bool isHighest = true;
			for ( int neighborTreeY = (treeY - 1); neighborTreeY < (treeY + 1); neighborTreeY++ )
			{
				for ( int neighborTreeX = (treeX - 1); neighborTreeX < (treeX + 1); neighborTreeX++ )
				{
					if ( neighborTreeX == treeX && neighborTreeY == treeY )
					{
						// Don't check MYSELF
						 continue;
					}

					int neighborTreeNoiseIndex = neighborTreeX + ( TREE_GRID_SIZE_X * neighborTreeY );
					if ( treeNoiseThisColumn[neighborTreeNoiseIndex] >= myTreeNoise )
					{
						isHighest = false;
						break;
					}
				}
				if ( !isHighest )
				{
					break;
				}
			}
			if ( isHighest )
			{
				int treeLocalChunkX = treeX - BLOCKS_LEFT_OF_CHUNK;		// ( MAX_TREE_RADIUS + 1 );	// e.g. treeX = 2 means localX = -1		// localTreePositionInChunkSpace 
				int treeLocalChunkY = treeY - BLOCKS_BELOW_CHUNK;		// ( MAX_TREE_RADIUS + 1 );	// e.g. treeY = 5 means localX =  2	

				// Determine grass height at THIS column
				int chunkColumnIndex = treeLocalChunkX + ( treeLocalChunkY * CHUNK_SIZE_X );
				if ( (chunkColumnIndex >= 0) && (chunkColumnIndex <= (MAX_BLOCKS_PER_CHUNK - 1) ) )
				{
					int grassHeightAtCurrentColumn	 = m_grassHeightsForEachColumn[chunkColumnIndex];
					if ( grassHeightAtCurrentColumn >= CHUNK_SEA_LEVEL )
					{
						int treeLocalZ = grassHeightAtCurrentColumn + 1;
						treeLocalXYPositionColumnList.push_back( IntVec2( treeLocalChunkX, treeLocalChunkY ) );
						m_treeToGenerateInLocalPositionList.push_back( IntVec3( treeLocalChunkX, treeLocalChunkY, treeLocalZ ) );		// This is the correct answer
//						m_treeToGenerateInLocalPositionList.push_back( IntVec3( treeLocalChunkX, treeLocalChunkY, 120 ) );	// hard coded for debugging
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Loop through m_treePositionList and create treeBlockTemplates
	//----------------------------------------------------------------------------------------------------------------------
	for ( int treePosIndex = 0; treePosIndex < m_treeToGenerateInLocalPositionList.size(); treePosIndex++ )
	{
		IntVec3 treeLocalBlockCoords = GetLocalBlockCoordsFromIndex( treePosIndex );
//		IntVec3 treeLocalBlockCoords = m_treeToGenerateInLocalPositionList[treePosIndex];
		int columnIndex = treeLocalBlockCoords.x + ( CHUNK_SIZE_X * treeLocalBlockCoords.y );
		if ( *m_temperatureForEachColumn < m_coldTreeTemperature )
		{
			SpawnBlockTemplateAtLocalCoords( "spruceTree", m_treeToGenerateInLocalPositionList[treePosIndex] );
		}
		else
		{
			if ( &m_humidityForEachColumn[columnIndex] <= m_humidityForEachColumn )
			{
				SpawnBlockTemplateAtLocalCoords( "cactus", m_treeToGenerateInLocalPositionList[treePosIndex] );
			}
			else if ( &m_humidityForEachColumn[columnIndex] > m_humidityForEachColumn )
			{
				SpawnBlockTemplateAtLocalCoords( "oakTree", m_treeToGenerateInLocalPositionList[treePosIndex] );
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Loop through m_cloudPositionList and create cloudBlockTemplates
	//----------------------------------------------------------------------------------------------------------------------
//	RandomNumberGenerator rng;
//	float randX = rng.RollRandomFloatInRange( 0.0f, CHUNK_MAX_INDEX_X );
//	float randY = rng.RollRandomFloatInRange( 0.0f, CHUNK_MAX_INDEX_Y );
//	float randZ = rng.RollRandomFloatInRange( 95.0f, CHUNK_MAX_INDEX_Z );
//	m_cloudToGenerateInLocalPositionList.push_back( IntVec3( randX, randY, randZ ) );		// Debug code for testing
	m_cloudToGenerateInLocalPositionList.push_back( IntVec3( 8, 8, 120 ) );					// Debug code for testing
	for ( int cloudPosIndex = 0; cloudPosIndex < m_cloudToGenerateInLocalPositionList.size(); cloudPosIndex++ )
	{
		SpawnBlockTemplateAtLocalCoords( "cloud", m_cloudToGenerateInLocalPositionList[cloudPosIndex] );									//  #ToDo: Fix cloudBlockTemplate logic (not rendering across chunks)
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Create Caves 
	//----------------------------------------------------------------------------------------------------------------------
	AddCaves( g_theGame->m_currentWorld->m_caveSeed );

	// Does not need to be saved until modified later on
	m_needsSaving = false;

	// The mesh needs to be rebuilt now that we actually have blocks
	m_isMeshDirty = true;
}

//----------------------------------------------------------------------------------------------------------------------
bool Chunk::IsBlockIndexValid( int index ) const
{
	if ( index >= 0 )
	{
		if ( index < MAX_BLOCKS_PER_CHUNK )
		{
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool Chunk::IsBlockOpaque( Block const* currentBlock ) const
{
	if ( currentBlock == nullptr )
	{
		return true; 
	}

	BlockDef const& blockDef = currentBlock->GetBlockDef();
	return blockDef.m_isOpaque;
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::PlaceAirCube( IntVec3 localBlockCoords, int cubeRadius )
{
	if ( !IsInBoundsLocal( localBlockCoords ) )
	{
		return;
	}

	// Get neighborhood mins and maxs
	IntVec3 neighborhoodMins = IntVec3( localBlockCoords.x - cubeRadius, localBlockCoords.y - cubeRadius, localBlockCoords.z - cubeRadius );
	IntVec3 neighborhoodMax  = IntVec3( localBlockCoords.x + cubeRadius, localBlockCoords.y + cubeRadius, localBlockCoords.z + cubeRadius );

	// Loop through "neighborhood" and set all blocks inside to air
	for ( int localZ = neighborhoodMins.z; localZ <= neighborhoodMax.z; localZ++ )
	{
		for ( int localY = neighborhoodMins.y; localY <= neighborhoodMax.y; localY++ )
		{
			for ( int localX = neighborhoodMins.x; localX <= neighborhoodMax.x; localX++ )
			{
				int blockIndex = g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords( IntVec3( localX, localY, localZ ) );

				if ( !IsBlockIndexValid( blockIndex ) )
				{
					continue;
				}

				m_blockList[blockIndex].m_blockTypeIndex = 0;	// 0 is airBlockType
//				m_blockList[blockIndex].m_blockTypeIndex = 7;	// 7 is diamondBlockType
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::CarveAABB3D( Vec3 worldBlockCenter, Vec3 halfDimensions )
{
//	BlockDef::GetBlockDefIDByName( "Air" );		
	Vec3  currentChunkMins = m_chunkWorldBounds.m_mins;
	AABB3 carveWorldBounds = AABB3( worldBlockCenter - halfDimensions, worldBlockCenter + halfDimensions );
	AABB3 carveLocalBounds = AABB3( carveWorldBounds.m_mins - currentChunkMins, carveWorldBounds.m_maxs - currentChunkMins );
	int   minLocalX		   = RoundDownToInt( carveLocalBounds.m_mins.x );
	int   maxLocalX		   = RoundDownToInt( carveLocalBounds.m_maxs.x );
	int   minLocalY		   = RoundDownToInt( carveLocalBounds.m_mins.y );
	int   maxLocalY		   = RoundDownToInt( carveLocalBounds.m_maxs.y );
	int   minLocalZ		   = RoundDownToInt( carveLocalBounds.m_mins.z );
	int   maxLocalZ		   = RoundDownToInt( carveLocalBounds.m_maxs.z );
	for ( int localZ = minLocalZ; localZ <= maxLocalZ; localZ++ )
	{
		// Skip if Z is out of bounds
		if ( localZ < 0 || localZ > CHUNK_MAX_INDEX_Z )
		{
			continue;
		}

		for ( int localY = minLocalY; localY <= maxLocalY; localY++ )
		{
			// Skip if Y is out of bounds
			if ( localY < 0 || localY > CHUNK_MAX_INDEX_Y )
			{
				continue;
			}

			for ( int localX = minLocalX; localX <= maxLocalX; localX++ )
			{
				// Skip if X is out of bounds
				if ( localX < 0 || localX > CHUNK_MAX_INDEX_X )
				{
					continue;
				}

				SetBlockTypeAtLocalBlockCoords( IntVec3( localX, localY, localZ ), 0 );		// 0 is airType;
			}
		}
	}

//	minLocalX				= Clamp( minLocalX, 0, CHUNK_MAX_INDEX_X );
//	maxLocalX				= Clamp( minLocalX, 0, CHUNK_MAX_INDEX_X );
//	minLocalY				= Clamp( minLocalY, 0, CHUNK_MAX_INDEX_Y );
//	maxLocalY				= Clamp( minLocalY, 0, CHUNK_MAX_INDEX_Y );
//	minLocalZ				= Clamp( minLocalZ, 0, CHUNK_MAX_INDEX_Z );
//	maxLocalZ				= Clamp( minLocalZ, 0, CHUNK_MAX_INDEX_Z );

}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::CarveCapsule3D( Vec3 capsuleWorldStart, Vec3 capsuleWorldEnd, float capsuleRadius )
{
	float minX = ( capsuleWorldStart.x < capsuleWorldEnd.x ) ? capsuleWorldStart.x - capsuleRadius : capsuleWorldEnd.x   - capsuleRadius;
	float minY = ( capsuleWorldStart.y < capsuleWorldEnd.y ) ? capsuleWorldStart.y - capsuleRadius : capsuleWorldEnd.y   - capsuleRadius;
	float minZ = ( capsuleWorldStart.z < capsuleWorldEnd.z ) ? capsuleWorldStart.z - capsuleRadius : capsuleWorldEnd.z   - capsuleRadius;
	float maxX = ( capsuleWorldStart.x < capsuleWorldEnd.x ) ? capsuleWorldEnd.x   + capsuleRadius : capsuleWorldStart.x + capsuleRadius;
	float maxY = ( capsuleWorldStart.y < capsuleWorldEnd.y ) ? capsuleWorldEnd.y   + capsuleRadius : capsuleWorldStart.y + capsuleRadius;
	float maxZ = ( capsuleWorldStart.z < capsuleWorldEnd.z ) ? capsuleWorldEnd.z   + capsuleRadius : capsuleWorldStart.z + capsuleRadius;
	AABB3 capsuleWorldBounds( minX, minY, minZ, maxX, maxY, maxZ );

	if ( !DoAABB3DOverlap( capsuleWorldBounds, m_chunkWorldBounds ) )
	{
		return;
	}

	Vec3  currentChunkMins = m_chunkWorldBounds.m_mins;
	AABB3 capsuleLocalBounds( capsuleWorldBounds.m_mins - currentChunkMins, capsuleWorldBounds.m_maxs - currentChunkMins );
//	int minLocalX = RoundDownToInt( capsuleWorldBounds.m_mins.x );
//	int maxLocalX = RoundDownToInt( capsuleWorldBounds.m_maxs.x );
//	int minLocalY = RoundDownToInt( capsuleWorldBounds.m_mins.y );
//	int maxLocalY = RoundDownToInt( capsuleWorldBounds.m_maxs.y );
//	int minLocalZ = RoundDownToInt( capsuleWorldBounds.m_mins.z );
//	int maxLocalZ = RoundDownToInt( capsuleWorldBounds.m_maxs.z );

	int minLocalX = RoundDownToInt( capsuleLocalBounds.m_mins.x );
	int maxLocalX = RoundDownToInt( capsuleLocalBounds.m_maxs.x );
	int minLocalY = RoundDownToInt( capsuleLocalBounds.m_mins.y );
	int maxLocalY = RoundDownToInt( capsuleLocalBounds.m_maxs.y );
	int minLocalZ = RoundDownToInt( capsuleLocalBounds.m_mins.z );
	int maxLocalZ = RoundDownToInt( capsuleLocalBounds.m_maxs.z );
	for ( int localZ = minLocalZ; localZ <= maxLocalZ; localZ++ )
	{
		// Skip if Z is out of bounds
		if ( localZ < 0 || localZ > CHUNK_MAX_INDEX_Z )
		{
			continue;
		}
		float blockCenterWorldZ = currentChunkMins.z + float( localZ ) + 0.5f;

		for ( int localY = minLocalY; localY <= maxLocalY; localY++ )
		{
			// Skip if Y is out of bounds
			if ( localY < 0 || localY > CHUNK_MAX_INDEX_Y )
			{
				continue;
			}
			float blockCenterWorldY = currentChunkMins.y + float( localY ) + 0.5f;

			for ( int localX = minLocalX; localX<= maxLocalX; localX++ )
			{
				// Skip if X is out of bounds
				if ( localX < 0 || localX > CHUNK_MAX_INDEX_X )
				{
					continue;
				}
				float blockCenterWorldX = currentChunkMins.x + float( localX ) + 0.5f;
				Vec3  blockCenterWorld	=  Vec3( blockCenterWorldX, blockCenterWorldY, blockCenterWorldZ );
				if ( IsPointInsideCapsule3D( blockCenterWorld, capsuleWorldStart, capsuleWorldEnd, capsuleRadius ) )
				{
					// Only carve blocks if 
//					SetBlockTypeAtLocalBlockCoords( IntVec3( localX, localY, localZ ), 7 );		// 0 is airType;
					SetBlockTypeAtLocalBlockCoords( IntVec3( localX, localY, localZ ), 0 );		// 0 is airType;
				}				
			}
		}
	}

//	float minX = boneWorldStart.x - radius;
//	if ( boneWorldEnd.x < boneWorldStart.x )
//	{
//		// Check if boneEnd is on the LEFT of the capsule
//		minX = boneWorldEnd.x - radius;
//	}

}

// Old code commented out before creating a better version in Block::GetColorFromLightInfluence
//----------------------------------------------------------------------------------------------------------------------
// Rgba8 Chunk::GetColorFromLightInfluence( BlockIterator neighbor, BlockIterator currentBlockIter )
// { 
// 	int currentIndoorLightInfluence  = currentBlockIter.GetBlock()->GetIndoorLightInfluence();
// 	int currentOutdoorLightInfluence = currentBlockIter.GetBlock()->GetOutdoorLightInfluence();
// 
// 	if ( !IsBlockIndexValid( neighbor.m_blockIndex ) )
// 	{
// 		Rgba8 tint = Rgba8::WHITE;
// 		return tint;
// 	}
// 
// 	int	indoorLightInfluence = neighbor.GetBlock()->GetIndoorLightInfluence();
// 	if ( currentIndoorLightInfluence > indoorLightInfluence )
// 	{
// 		indoorLightInfluence = currentIndoorLightInfluence;
// 	}
// 	float indoorTint = RangeMapClamped( float(indoorLightInfluence), 0.0f, 15.0f, 0.0f, 255.0f );
// 	
// 	int	outdoorLightInfluence = neighbor.GetBlock()->GetOutdoorLightInfluence();
// 	if ( currentOutdoorLightInfluence > outdoorLightInfluence )
// 	{
// 		outdoorLightInfluence = currentOutdoorLightInfluence;
// 	}
// 	float outdoorTint = RangeMapClamped( float(outdoorLightInfluence), 0.0f, 15.0f, 0.0f, 255.0f );
// 
// //	UNUSED( outdoorTint );
// //	UNUSED( indoorTint  );
// 
// 	// Get color based on combined values from indoor and outdoor light influences
// //	Rgba8 tint					= Rgba8( char(outdoorTint), char(indoorTint), 127, 255 );
// //	Rgba8 tint					= Rgba8( char(outdoorTint),				   0, 127, 255 );
// //	Rgba8 tint					= Rgba8(			   255,	char(indoorTint), 255, 255 );
// 	Rgba8 tint					= Rgba8(			   255,				 255, 255, 255 );
// 	return tint;
// 
// //	BlockDef blockDef				= neighbor.GetBlock()->GetBlockDef();
// //	int		 indoorLightInfluence	= g_theGame->m_currentWorld->GetBrightestNeighborIndoorLightValue( neighbor );
// //	float	 indoorTint				= RangeMapClamped( float(indoorLightInfluence), 0.0f, 15.0f, 0.0f, 255.0f );
// //	int		 outdoorLightInfluence	= g_theGame->m_currentWorld->GetBrightestNeighborOutdoorLightValue( neighbor );
// //	float	 outdoorTint			= RangeMapClamped( float(outdoorLightInfluence), 0.0f, 15.0f, 0.0f, 255.0f );
// //	Rgba8    tint					= Rgba8( char(outdoorTint), char(indoorTint), 127, 255 );
// //	return tint;
// }

//----------------------------------------------------------------------------------------------------------------------
bool Chunk::LoadSavedChunkOnDisk()
{
	std::string filepPath = GetFilePath();		// I.e., "Saves/Chunk(%i, %i).chunk"

	std::vector<unsigned char> bufferData;
	bool wasSuccessful = FileReadToBuffer( bufferData, filepPath );

	if ( !wasSuccessful  )
	{
		return wasSuccessful;
	}

	// 4CC = 4 character code // I.e., GChK
	unsigned char identifier4CC_1	= bufferData[0];
	unsigned char identifier4CC_2	= bufferData[1];
	unsigned char identifier4CC_3	= bufferData[2];
	unsigned char identifier4CC_4	= bufferData[3];
	unsigned char versionNum		= bufferData[4];
	unsigned char bitSizeX			= bufferData[5];
	unsigned char bitSizeY			= bufferData[6];
	unsigned char bitSizeZ			= bufferData[7];
	unsigned char worldSeed_1		= bufferData[8];
	unsigned char worldSeed_2		= bufferData[9];
	unsigned char worldSeed_3		= bufferData[10];
	unsigned char worldSeed_4		= bufferData[11];


	// I need to validate that (worldSeed_4, worldSeed_3, worldSeed_2, worldSeed_1) combined ==  m_worldSeed
	// I need to break out m_worldSeed into 4 parts and compare (worldSeed_4, worldSeed_3, worldSeed_2, worldSeed_1)
	unsigned int mask			= 255;							// (0000'0000, 0000'0000, 0000'0000, 1111'1111) 
	unsigned int worldSeed		= g_theGame->m_currentWorld->m_worldSeed;
	// worldSeed_1
	unsigned char worldSeed_P1	= unsigned char( worldSeed & mask );			// worldSeed_P1 = (0000'0000, 0000'0000, 0000'0000, 1111'1111) 
	// worldSeed_2
	mask						= mask << 8;					// Mask			= (0000'0000, 0000'0000, 1111'1111, 0000'0000) 
	unsigned char worldSeed_P2	= unsigned char( ( worldSeed & mask ) >> 8 );	// worldSeed_P2 = (0000'0000, 0000'0000, 1111'1111, 0000'0000) >> (0000'0000, 0000'0000, 0000'0000, 1111'1111) 
	// worldSeed_3
	mask						= mask << 8;					// Mask			= (0000'0000, 1111'1111, 0000'0000, 0000'0000) 
	unsigned char worldSeed_P3	= unsigned char( ( worldSeed & mask ) >> 16 );	// worldSeed_P3 = (0000'0000, 1111'1111, 0000'0000, 0000'0000) >> (0000'0000, 0000'0000, 1111'1111, 0000'0000) >> (0000'0000, 0000'0000, 0000'0000, 1111'1111)
	// worldSeed_4
	mask						= mask << 8;					// Mask			= (1111'1111, 0000'0000, 0000'0000, 0000'0000) 
	unsigned char worldSeed_P4	= unsigned char( ( worldSeed & mask ) >> 24 );	// worldSeed_P4 = (1111'1111, 0000'0000, 0000'0000, 0000'0000) >> (0000'0000, 1111'1111, 0000'0000, 0000'0000) >> (0000'0000, 0000'0000, 1111'1111, 0000'0000) >> (0000'0000, 0000'0000, 0000'0000, 1111'1111)


	// Validate file header
	GUARANTEE_OR_DIE( identifier4CC_1	 ==			 'G', "First identifier INVALID" );
	GUARANTEE_OR_DIE( identifier4CC_2	 ==			 'C', "First identifier INVALID" );
	GUARANTEE_OR_DIE( identifier4CC_3	 ==			 'H', "First identifier INVALID" );
	GUARANTEE_OR_DIE( identifier4CC_4	 ==			 'K', "First identifier INVALID" );
	GUARANTEE_OR_DIE( versionNum		 ==	 VERSION_NUM, "Game version INVALID"	 );
	GUARANTEE_OR_DIE( bitSizeX			 ==	CHUNK_BITS_X, "Bit sizeX INVALID"		 );
	GUARANTEE_OR_DIE( bitSizeY			 ==	CHUNK_BITS_Y, "Bit sizeY INVALID"		 );
	GUARANTEE_OR_DIE( bitSizeZ			 ==	CHUNK_BITS_Z, "Bit sizeZ INVALID"		 );
	GUARANTEE_OR_DIE( worldSeed_4		 ==	worldSeed_P4, "worldSeed_4 INVALID"		 );
	GUARANTEE_OR_DIE( worldSeed_3		 ==	worldSeed_P3, "worldSeed_3 INVALID"		 );
	GUARANTEE_OR_DIE( worldSeed_2		 ==	worldSeed_P2, "worldSeed_2 INVALID"		 );
	GUARANTEE_OR_DIE( worldSeed_1		 ==	worldSeed_P1, "worldSeed_1 INVALID"		 );
//	GUARANTEE_OR_DIE( worldSeed			 ==	g_theGame->m_currentWorld->m_worldSeed, "worldSeed INVALID"		   );

	// Parse rest of data as RLE compressed block sequences, alternating block type and (num OR frequency)
	int blockIndex = 0;
	for ( int bufferIndex = 8; bufferIndex < bufferData.size(); bufferIndex += 2 )
//	for ( int bufferIndex = 12; bufferIndex < bufferData.size(); bufferIndex += 2 )
	{
		unsigned char blockType			= bufferData[bufferIndex];
		unsigned char numBlocksOfType	= bufferData[bufferIndex + 1];

		// Create and set block num and type for specified blockNum
		for ( int i = 0; i < numBlocksOfType; i++ )
		{
			m_blockList[blockIndex].m_blockTypeIndex = blockType;
			blockIndex++;
		}
	}
	
	// Check correct numBlocks exist within this chunk
	GUARANTEE_OR_DIE( blockIndex ==  MAX_BLOCKS_PER_CHUNK, Stringf("%d / %d blocks loaded at Chunk(%d, %d)", blockIndex, MAX_BLOCKS_PER_CHUNK, m_chunkCoords.x, m_chunkCoords.y) );

	// Set mesh dirty flag true for this chunk 
	m_isMeshDirty = true;

	return true;

	//----------------------------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------------------------
	// 	bool chunkWasLoadedFromFile = false;
	// 
	// 	std::ifstream file;
	// 	std::string chunkName = Stringf( "Saves/Chunk(%i, %i).chunk", m_chunkCoords.x, m_chunkCoords.y );
	// 	file.open( chunkName );
	// 	file.close();
	// 
	// 	if ( file )
	// 	{
	// 		// Take data from buffer and convert to numBlocks to set as block type
	// 		std::vector<char> savedBlockList;
	// 		ReadFileToBinaryBuffer( savedBlockList, chunkName );
	// //		ReadFileToBinaryBuffer( file., chunkName );
	// 
	// 		// Loop through savedBlockList and assign specific numBlocks as specified type
	// 		for ( int i = 0; i < savedBlockList.size(); i++ )
	// 		{
	// 			// Skip the first 7th items in the list 4CC + XYZ sizes
	// 
	// 			// create num blocks of type
	// 			// add to m_blockList
	// 		}
	// 
	// 		chunkWasLoadedFromFile = true;
	// 	}
	// 
	// 	return chunkWasLoadedFromFile;		   sizeY			
}					  		

//----------------------------------------------------------------------------------------------------------------------
std::string Chunk::GetFilePath()
{
	std::string filePath = Stringf( "Saves/Chunk(%i,%i).chunk", m_chunkCoords.x, m_chunkCoords.y );
	return filePath;
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::SpawnBlockTemplateAtLocalCoords( std::string const& blockTemplateName, IntVec3 localOrigin )
{
//	BlockTemplate* blockTemplate = BlockTemplate::GetBlockTemplateByName( blockTemplateName );
//	GUARANTEE_OR_DIE( blockTemplate != nullptr, Stringf("ERROR, block template does not exist '%s' "), blockTemplateName );

	// loop through block template list
	// get local position based + local origin
	// SetBlockType( localCoords, blockTemplistList.blockType );

	BlockTemplate const* treeBlockTemplate = BlockTemplate::GetTemplateByName( blockTemplateName );
//	BlockTemplate const* treeBlockTemplate = g_theGame->m_currentWorld->CreateTreeBlockTemplate();
	GUARANTEE_OR_DIE( treeBlockTemplate != nullptr, Stringf("No such block template named '%s'", blockTemplateName.c_str() ) );
	if ( treeBlockTemplate != nullptr )
	{
		for ( int i = 0; i < treeBlockTemplate->m_blockTemplateEntryList.size(); i++ )
		{
			BlockTemplateEntry const& blockToSpawn	= treeBlockTemplate->m_blockTemplateEntryList[i];
			IntVec3 localCoords						= localOrigin + blockToSpawn.m_localOffset;
			int localIndex							= blockToSpawn.m_blockType;
			SetBlockTypeAtLocalBlockCoords( localCoords, localIndex );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::SetBlockTypeAtLocalBlockCoords( IntVec3 localCoords, int blockType )
{
	for ( int i = 0; i < MAX_BLOCKS_PER_CHUNK; i++ )
	{
		IntVec3 localBlockCoords = GetLocalBlockCoordsFromIndex( i );
		if ( localBlockCoords == localCoords )
		{
			m_blockList[i].m_blockTypeIndex = unsigned char (blockType);	
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::SetBlockTypeAtLocalBlockCoords( int localBlockX, int localBlockY, int localBlockZ, int blockType )
{
	bool isIntBounds = IsInBoundsLocal( localBlockX, localBlockY, localBlockZ );
	if ( isIntBounds )
	{
		SetBlockTypeAtLocalBlockCoords( IntVec3( localBlockX, localBlockY, localBlockZ ), blockType ); 
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool Chunk::IsInBoundsLocal( int localBlockX, int localBlockY, int localBlockZ )
{
	if ( localBlockX >= 0 && localBlockX <= CHUNK_SIZE_X &&
		 localBlockY >= 0 && localBlockY <= CHUNK_SIZE_Y &&
		 localBlockZ >= 0 && localBlockZ <= CHUNK_SIZE_Z
		)
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool Chunk::IsInBoundsLocal( IntVec3 localBlockCoords )
{
	bool isInbounds = IsInBoundsLocal( localBlockCoords.x, localBlockCoords.y, localBlockCoords.z );
	return isInbounds;
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::AddCaves( unsigned int worldCaveSeed )
{
	// Set various cave generation constants
	constexpr float CHANCE_FOR_CAVE_TO_START_IN_A_CHUNK = 0.2f;
	constexpr float CAVE_MAX_DIST_BLOCKS				= 30.0f;
	constexpr float CAVE_STEP_LENGTH_BLOCKS				= 20.0f;
	constexpr float CAVE_MAX_TURN_DEGREES				= 15.0f;
	constexpr float CAVE_MAX_DIVE_DEGREES				= 10.0f;
	constexpr int   MAX_CAVE_STEPS						= 25;

	// Calculate how big of a search area we'll need to check chunks in to detect for caves
	constexpr float CHUNK_WIDTH					= float( CHUNK_SIZE_X < CHUNK_SIZE_Y ? CHUNK_SIZE_X : CHUNK_SIZE_Y );
	constexpr float CAVE_SEARCH_RADIUS_CHUNKS	= 1.0f + ( CAVE_MAX_DIST_BLOCKS / CHUNK_WIDTH );

	// Search all chunks in a large rectangular region around us for possible cave start locations
	IntVec2 chunkSearchMins = m_chunkCoords - IntVec2( int(CAVE_SEARCH_RADIUS_CHUNKS), int(CAVE_SEARCH_RADIUS_CHUNKS) );
	IntVec2 chunkSearchMaxs = m_chunkCoords + IntVec2( int(CAVE_SEARCH_RADIUS_CHUNKS), int(CAVE_SEARCH_RADIUS_CHUNKS) );
	for ( int chunkY = chunkSearchMins.y; chunkY <= chunkSearchMaxs.y; chunkY++ )
	{
		for ( int chunkX = chunkSearchMins.x; chunkX<= chunkSearchMaxs.y; chunkX++ )
		{
			float caveOriginNoise = Get2dNoiseZeroToOne( chunkX, chunkY, worldCaveSeed );
			if ( caveOriginNoise < CHANCE_FOR_CAVE_TO_START_IN_A_CHUNK )
			{
				// Make a note that "a cave definitely starts somewhere in this chunk"
				CaveInfo currentCave = CaveInfo( IntVec2(chunkX, chunkY) );
				m_caveInfoList.push_back( currentCave );
			}
		}
	}

	// For each cave originating nearby, determine its exact start location, then start wandering 
	for ( int i = 0; i < m_caveInfoList.size(); i++ )
	{
		// Create a (mostly) unique seed for THIS cave, and create a private RNG based on that seed 
		CaveInfo& currentCave			= m_caveInfoList[i];
		unsigned int currentCaveSeed	= Get2dNoiseUint( currentCave.m_startChunkCoords.x, currentCave.m_startChunkCoords.y, worldCaveSeed );	 // #TODO: come up with a seed consistent for this cave
		RandomNumberGenerator caveRNG( currentCaveSeed );

		// Pick a random starting position within the cave's starting chunk (which is probably not me, "the current chunk")
		AABB3 startChunkBounds				= GetChunkBoundsForChunkCoords( currentCave.m_startChunkCoords );
		Vec3  crawlWorldPosition;
		crawlWorldPosition.x				= caveRNG.RollRandomFloatInRange( startChunkBounds.m_mins.x, startChunkBounds.m_maxs.x );
		crawlWorldPosition.y				= caveRNG.RollRandomFloatInRange( startChunkBounds.m_mins.y, startChunkBounds.m_maxs.y );
		crawlWorldPosition.z				= caveRNG.RollRandomFloatInRange( 30.0f, 50.0f );
		currentCave.m_startWorldBlockPos	= crawlWorldPosition;

		EulerAngles crawlOrientation;
		crawlOrientation.m_yawDegrees	= caveRNG.RollRandomFloatInRange( 0.0f, 360.0f );
		crawlOrientation.m_pitchDegrees = 30.0f;

		// Note the starting position, then start crawling
		for ( int crawlStep = 0; crawlStep < MAX_CAVE_STEPS; crawlStep++ )
		{
			Vec3 forwardDisp				 = CAVE_STEP_LENGTH_BLOCKS * crawlOrientation.GetForwardDir_XFwd_YLeft_ZUp();
			crawlWorldPosition				+= forwardDisp;
			crawlOrientation.m_yawDegrees	+= caveRNG.RollRandomFloatInRange( -CAVE_MAX_TURN_DEGREES, CAVE_MAX_TURN_DEGREES );
//			crawlOrientation.m_pitchDegrees	+= caveRNG.RollRandomFloatInRange( -CAVE_MAX_DIVE_DEGREES, CAVE_MAX_DIVE_DEGREES);
			crawlOrientation.m_pitchDegrees	 = 89.0f * Compute1dPerlinNoise( float(crawlStep), 10.0f, 3, 0.5f, 2.0f, true, currentCaveSeed );

//			IntVec3 localBlockCoords = g_theGame->m_currentWorld->GetLocalBlockCoordsFromWorldPos( crawlWorldPosition );	
//			PlaceAirCube( localBlockCoords, 1 );

			currentCave.m_caveNodePositionList.push_back( crawlWorldPosition );

			// #DebugCode
//			PlaceAirCube( IntVec3( 8, 8, 55 ), 1 );
//			PlaceAirCube( IntVec3( 8, 8, 85 ), 1 );

//			// Carve out THIS block into "Ice"
//			// How do I get THIS block?
//			Vec3 currentWorldBlockPos		= currentCave.m_caveNodePositionList[crawlStep];
//			IntVec3 currentLocalBlockCoords	= g_theGame->m_currentWorld->GetLocalBlockCoordsFromWorldPos( currentWorldBlockPos );
//			int currentBlockIndex			= g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords( currentLocalBlockCoords );
//			m_blockList[currentBlockIndex].m_blockTypeIndex = 11;
		}

//		IntVec3 localBlockCoords = g_theGame->m_currentWorld->GetLocalBlockCoordsFromWorldPos( crawlWorldPosition );
//		PlaceAirCube( localBlockCoords, 1 );
	}

	// Get a cave noise value based on chunk coords for each chunk in a huge region around me
	// Each chunk with a noise value > some threshold has a cave that STARTS in that chunk
	// Each cave will also have a mostly-unique seed number for its RNG to make it different
	// Each cave's RNG will be based on raw noise of its chunk coords
	// Generate each cave carving blocks as it goes; only MY blocks will actually be carved

//	CarveAABB3D( Vec3( 10.0f, 10.0f, 20.0f ), Vec3( 12.0f, 7.0f, 4.0f ) );
//	CarveAABB3D( Vec3( 12.0f, 12.0f, 20.0f ), Vec3( 13.0f, 5.0f, 6.0f ) );
//	CarveAABB3D( Vec3( 14.0f, 15.0f, 20.0f ), Vec3( 14.0f, 3.0f, 8.0f ) );
//	CarveAABB3D( Vec3( 16.0f, 14.0f, 20.0f ), Vec3( 15.0f, 5.0f, 4.0f ) );
//	CarveAABB3D( Vec3( 18.0f, 11.0f, 20.0f ), Vec3( 16.0f, 6.0f, 4.0f ) );
//
//	CarveAABB3D( Vec3( 210.0f, 110.0f, 20.0f ), Vec3( 12.0f, 37.0f, 24.0f ) );
//	CarveAABB3D( Vec3( 212.0f, 112.0f, 20.0f ), Vec3( 13.0f, 35.0f, 26.0f ) );
//	CarveAABB3D( Vec3( 214.0f, 115.0f, 20.0f ), Vec3( 14.0f, 33.0f, 28.0f ) );
//	CarveAABB3D( Vec3( 216.0f, 114.0f, 20.0f ), Vec3( 15.0f, 35.0f, 24.0f ) );
//	CarveAABB3D( Vec3( 218.0f, 111.0f, 20.0f ), Vec3( 16.0f, 36.0f, 24.0f ) );

//	CarveCapsule3D( Vec3( 0.0f, 0.0f, 80.0f ), Vec3(  8.0f, 0.0f, 80.0f ), 5.0f ); 
//	CarveCapsule3D( Vec3( 8.0f, 0.0f, 30.0f ), Vec3( 16.0f, 8.0f, 30.0f ), 5.0f ); 

	for ( int i = 0; i < m_caveInfoList.size(); i++ ) 
	{
//		CaveInfo const& currentCave = m_caveStartChunkCoordsList[i];
		CaveInfo const& currentCave = m_caveInfoList[i];
		IntVec2 chunkCoords			= currentCave.m_startChunkCoords;
		Vec3 chunkMins( float( chunkCoords.x * CHUNK_SIZE_X), float( chunkCoords.y * CHUNK_SIZE_Y ), 0.0f );

		Vec3 startPos = currentCave.m_startWorldBlockPos;
		for ( int caveStep = 0; caveStep < currentCave.m_caveNodePositionList.size(); caveStep++ )
		{
			Vec3 caveNodePos = currentCave.m_caveNodePositionList[caveStep];
			CarveCapsule3D( startPos, caveNodePos, 2.0f );

			startPos = caveNodePos;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Chunk::DebugAddVertsForCaves( std::vector<Vertex_PCU>& verts ) const
{
//	for ( int i = 0; i < m_caveStartChunkCoordsList.size(); i++ ) 
	for ( int i = 0; i < m_caveInfoList.size(); i++ ) 
	{
//		CaveInfo const& currentCave = m_caveStartChunkCoordsList[i];
		CaveInfo const& currentCave = m_caveInfoList[i];
		IntVec2 chunkCoords			= currentCave.m_startChunkCoords;
		Vec3 chunkMins( float( chunkCoords.x * CHUNK_SIZE_X), float( chunkCoords.y * CHUNK_SIZE_Y ), 0.0f );
		Vec3 markerPos = chunkMins + Vec3( 8.0f, 8.0f, 80.0f );
//		Vec3 markerPos = m_chunkWorldBounds.GetCenter() + Vec3( 0.0f, 0.0f, 40.0f );
		AddvertsforCube3D( verts, markerPos, 0.25f, Rgba8::RED );

		for ( int caveStep = 0; caveStep < currentCave.m_caveNodePositionList.size(); caveStep++ )
		{
			Vec3 caveNodePos = currentCave.m_caveNodePositionList[caveStep];
			AddvertsforCube3D( verts, caveNodePos, 0.25f, Rgba8::CYAN );
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
AABB3 Chunk::GetChunkBoundsForChunkCoords( IntVec2 const& chunkCoords )
{
	float minX = chunkCoords.x * float( CHUNK_SIZE_X );
	float minY = chunkCoords.y * float( CHUNK_SIZE_Y );
	float minZ = 0.0f;
	float maxX = minX + float( CHUNK_SIZE_X );
	float maxY = minY + float( CHUNK_SIZE_Y );
	float maxZ = float( CHUNK_SIZE_Z );
	return AABB3( Vec3(minX, minY, minZ), Vec3(maxX, maxY, maxZ) );
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// Threads
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

// 	Job* jobToDo = nullptr;
// 	g_workQueueMutex.Lock();
// 	if ( !g_workQueue.empty() )
// 	{
// 		jobToDo = g_workQueue.popFront();
// 	}
// 	g_workQueueMutex.unlock();
// 	jobToDo->Execute();
// 
// // Enum ChunkStatus
// enum ChunkStatus
// {
// 	CHUNK_STATUS_GENERATING,
// 	CHUNK_STATUS_NUM,
// };
// 
// std::atomic<ChunkStatus> g_chunkStatus;
// 
// std::mutex			g_jobsQueuedWaitingForWorkerMutex;	// Mutex = mutual exclusion			// "Mutex is the pumpkin"
// std::queue<Job*>	g_jobsQueuedWaitingForWorker;	
// 
// g_jobsQueuedWaitingForWorkerMutex.lock();		//	
// g_jobsQueuedWaitingForWorkerMutex.unlock();		//	
// 
// // Thread safety
// std::atomic<int> g_numThreads = 0;
// 
// //----------------------------------------------------------------------------------------------------------------------
// void ThreadMain( int threadID, std::string name )
// {
// 
// }
// 
// // Create thread
// std::thread* thread1 = new std::thread( ThreadMain, 1, "Oak" );
// 
// thread1->join();		// Merges current thread back with main thread
// 
// delete thread1;

//----------------------------------------------------------------------------------------------------------------------
// Simple Miner game code for generating chunk
//----------------------------------------------------------------------------------------------------------------------
// class ChunkGenerateJob : public Job
// {
// 	ChunkGenerateJob( Chunk* chunk )
// 		: m_chunk( chunk )
// 	{}
// 
// 	Chunk* m_chunk = nullptr;
// }
// 
// //----------------------------------------------------------------------------------------------------------------------
// class ChunkGenerateJob::Execute()
// {
// 	m_chunk->GenerateBlocks();
// }
// 
//  //----------------------------------------------------------------------------------------------------------------------
// class ChunkGenerateJob : public Job
// {
//  virtual void Execute() override;
// 
// };

//----------------------------------------------------------------------------------------------------------------------
//	Job* jobToDo = jobSystem->GetJobToWorkOn();
//	if ( jobToDo != null )
//	{
//		jobToDo->Execute();
//	}

//----------------------------------------------------------------------------------------------------------------------
//	Job* jobSystem::GetJobToWorkOn()
//  {
//		mutext lock()
//		check if jobList.size() != 0
//		get job (pop front)
//		mutext unlock()
//	}

// getNearestChunkToActivate
// Chunk* newChunk = Chunk();
// add this job to the list of jobs 
//----------------------------------------------------------------------------------------------------------------------
//	Job* jobToDo = jobSystem->GetJobToWorkOn();
//	if ( jobToDo != null )
//	{
//		jobToDo->Execute();
//	}
	
//----------------------------------------------------------------------------------------------------------------------
//	Job* jobSystem::GetJobToWorkOn()
//  {
//		mutext lock()
//		check if jobList.size() != 0
//		get job (pop front)
//		mutext unlock()
//	}

// getNearestChunkToActivate
// Chunk* newChunk = Chunk();
// add this job to the list of jobs

//----------------------------------------------------------------------------------------------------------------------
void ChunkGenerateJob::Execute()
{
	m_chunk->RegenerateBlocksInChunk();
}
