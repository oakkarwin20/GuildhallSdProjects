#include "Game/BlockIterator.hpp"
#include "Game/Chunk.hpp"
#include "Game/World.hpp"
#include "Game/Game.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
BlockIterator::BlockIterator()
{
}

//----------------------------------------------------------------------------------------------------------------------
BlockIterator::BlockIterator( Chunk* chunk, int blockIndex )
{
	m_currentChunk  = chunk;
	m_blockIndex	= blockIndex;
}

//----------------------------------------------------------------------------------------------------------------------
BlockIterator::~BlockIterator()
{
}

//----------------------------------------------------------------------------------------------------------------------
Block* BlockIterator::GetBlock() const
{
	if ( m_currentChunk == nullptr || m_currentChunk->m_blockList == nullptr )
	{
		return nullptr;
	}

	GUARANTEE_OR_DIE( m_blockIndex >= 0 && m_blockIndex < MAX_BLOCKS_PER_CHUNK, "Bad index on block iter" );

	Block* currentBlock = &m_currentChunk->m_blockList[m_blockIndex];	
	return currentBlock;
}

//----------------------------------------------------------------------------------------------------------------------
Vec3 BlockIterator::GetBlockCenterInWorldPos()
{
//	Vec3 chunkMins	= m_currentChunk->m_chunkWorldBounds.m_mins;
//	Vec3 blockMins	= chunkMins + m_currentChunk->GetLocalBlockCoordsFromIndex( m_blockIndex ); 

	IntVec3 localBlockCoords	= m_currentChunk->GetLocalBlockCoordsFromIndex( m_blockIndex );

	// Get block world position
	Vec3 worldBlockPosition;  
	worldBlockPosition.x		= float( localBlockCoords.x + (m_currentChunk->m_chunkCoords.x * CHUNK_SIZE_X) );
	worldBlockPosition.y		= float( localBlockCoords.y + (m_currentChunk->m_chunkCoords.y * CHUNK_SIZE_Y) );
	worldBlockPosition.z		= float( localBlockCoords.z );

	// Get block center position
	worldBlockPosition.x		= worldBlockPosition.x + BLOCK_HALF_SIZE;	
	worldBlockPosition.y		= worldBlockPosition.y + BLOCK_HALF_SIZE;	
	worldBlockPosition.z		= worldBlockPosition.z + BLOCK_HALF_SIZE;	

	return worldBlockPosition;
}

//----------------------------------------------------------------------------------------------------------------------
BlockIterator BlockIterator::GetNorthNeighborBlock()
{
	// Check if currentChunk exists
	if ( m_currentChunk == nullptr || m_currentChunk->m_blockList == nullptr ) 
	{
		// Return Invalid blockIter
		return BlockIterator( m_currentChunk, -1 );
	}

	// Get currentBlock's indexX
	int localY = (m_blockIndex >> CHUNK_BITS_X) & CHUNK_MASK_X;
	if ( localY == CHUNK_MAX_INDEX_Y )
	{
		return BlockIterator( m_currentChunk->m_northNeighbor, (m_blockIndex - CHUNK_MASK_Y) );
	}
	else
	{
//		return BlockIterator( m_currentChunk, m_blockIndex + CHUNK_MAX_INDEX_Y );
		return BlockIterator( m_currentChunk, m_blockIndex + CHUNK_SIZE_Y );
	}

//	//----------------------------------------------------------------------------------------------------------------------
//	// Initial Attempt
//	//----------------------------------------------------------------------------------------------------------------------
//	BlockIterator blockIterator;
//	IntVec3 localBlockCoords	 = m_currentChunk->GetLocalBlockCoordsFromIndex( m_blockIndex );
//	IntVec3 neighborBlockCoords	 = localBlockCoords;
//
//	if ( localBlockCoords.y == CHUNK_MAX_INDEX_Y )
//	{
//		// Reset Y position
//		neighborBlockCoords.y = 0;
//
//		// Calculate and return blockIterator with the new index and neighbor Chunk*
//		blockIterator.m_blockIndex	 = g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords( neighborBlockCoords );
//		blockIterator.m_currentChunk = m_currentChunk->m_northNeighbor;
//		return blockIterator;
//	}
//	
//	// Increment Y position
//	localBlockCoords.y++;
//
//	// Calculate and return blockIterator with the new index but same Chunk*
//	blockIterator.m_blockIndex	 = g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords( localBlockCoords );
//	blockIterator.m_currentChunk = m_currentChunk;
//	return blockIterator;
}

//----------------------------------------------------------------------------------------------------------------------
BlockIterator BlockIterator::GetSouthNeighborBlock()
{
	// Check if currentChunk exists
	if ( m_currentChunk == nullptr || m_currentChunk->m_blockList == nullptr )
	{
		// Return Invalid blockIter
		return BlockIterator( m_currentChunk, -1 );
	}

	// Get currentBlock's indexX
//	int localY = ( m_blockIndex >> CHUNK_BITS_X ) & CHUNK_BITS_Y;
	int localY = ( m_blockIndex >> CHUNK_BITS_X ) & CHUNK_MASK_X;	// Should be chunkMaskY
	// If localX is at edge of chunk boundary
	if ( localY == 0 )
	{
		// Return block at south neighbor chunk
		return BlockIterator( m_currentChunk->m_southNeighbor, ( m_blockIndex + CHUNK_MASK_Y ) );
	}
	else
	{
		// Increment current block index
//		return BlockIterator( m_currentChunk, m_blockIndex - CHUNK_MAX_INDEX_Y );
		return BlockIterator( m_currentChunk, m_blockIndex - CHUNK_SIZE_Y );
	}
	
	//----------------------------------------------------------------------------------------------------------------------
	// Initial Attempt
	//----------------------------------------------------------------------------------------------------------------------
//	BlockIterator blockIterator;
//	IntVec3 localBlockCoords	 = m_currentChunk->GetLocalBlockCoordsFromIndex( m_blockIndex );
//	IntVec3 neighborBlockCoords	 = localBlockCoords;
//
//	if ( localBlockCoords.y == 0 )
//	{
//		// Set Y position to MAX_INDEX
//		neighborBlockCoords.y = CHUNK_MAX_INDEX_Y;
//
//		// Calculate and return blockIterator with the new index and neighbor Chunk*
//		blockIterator.m_blockIndex	 = g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords( neighborBlockCoords );
//		blockIterator.m_currentChunk = m_currentChunk->m_southNeighbor;
//		return blockIterator;
//	}
//
//	// Decrement Y position
//	localBlockCoords.y--;
//
//	// Calculate and return blockIterator with the new index but same Chunk*
//	blockIterator.m_blockIndex	 = g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords( localBlockCoords );
//	blockIterator.m_currentChunk = m_currentChunk;
//	return blockIterator;
}

//----------------------------------------------------------------------------------------------------------------------
BlockIterator BlockIterator::GetEastNeighborBlock()
{	
	// Check if currentChunk exists
	if ( m_currentChunk == nullptr || m_currentChunk->m_blockList == nullptr ) 
	{
		// Return Invalid blockIter
		return BlockIterator( m_currentChunk, -1 );
	}

	// Get currentBlock's indexX
	int localX = m_blockIndex & CHUNK_MASK_X;
	// If localX is at edge of chunk boundary
	if ( localX == CHUNK_MAX_INDEX_X )
	{
		// Return block at east neighbor chunk
		return BlockIterator( m_currentChunk->m_eastNeighbor, (m_blockIndex - CHUNK_MAX_INDEX_X) );		
	}
	else
	{
		// Increment current block index
		return BlockIterator( m_currentChunk, m_blockIndex + 1 );
	}

	//	//----------------------------------------------------------------------------------------------------------------------
	//	// My previous attempt
	//	//----------------------------------------------------------------------------------------------------------------------
	//	BlockIterator blockIterator;
	//	IntVec3 localBlockCoords	 = m_currentChunk->GetLocalBlockCoordsFromIndex( m_blockIndex );
	//	IntVec3 neighborBlockCoords	 = localBlockCoords;
	//	
	//	if ( localBlockCoords.x == CHUNK_MAX_INDEX_X )
	//	{
	//		// Reset X position
	//		neighborBlockCoords.x = 0;
	//
	//		// Calculate and return blockIterator with the new index and neighbor Chunk*
	//		blockIterator.m_blockIndex	 = g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords( neighborBlockCoords );
	//		blockIterator.m_currentChunk = m_currentChunk->m_eastNeighbor;
	//		return blockIterator;
	//	}
	//	
	//	// Increment X position
	//	localBlockCoords.x++;
	//
	//	// Calculate and return blockIterator with the new index but same Chunk*
	//	blockIterator.m_blockIndex	 = g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords( localBlockCoords );
	//	blockIterator.m_currentChunk = m_currentChunk;
	//	return blockIterator;
}

//----------------------------------------------------------------------------------------------------------------------
BlockIterator BlockIterator::GetWestNeighborBlock()
{
	// Check if currentChunk exists
	if ( m_currentChunk == nullptr || m_currentChunk->m_blockList == nullptr )
	{
		// Return Invalid blockIter
		return BlockIterator( m_currentChunk, -1 );
	}

	// Get currentBlock's indexX
	int localX = m_blockIndex & CHUNK_MASK_X;
	// If localX is at edge of chunk boundary
	if ( localX == 0 )
	{
		// Return block at west neighbor chunk
//		return BlockIterator( m_currentChunk->m_westNeighbor, ( m_blockIndex + CHUNK_MAX_INDEX_X ) );
		return BlockIterator( m_currentChunk->m_westNeighbor, ( m_blockIndex + CHUNK_MAX_INDEX_Y ) );
//		return BlockIterator( m_currentChunk->m_westNeighbor, ( m_blockIndex + CHUNK_SIZE_X ) );
	}
	else
	{
		// Increment current block index
		return BlockIterator( m_currentChunk, m_blockIndex - 1 );
	}

//	//----------------------------------------------------------------------------------------------------------------------
//	//----------------------------------------------------------------------------------------------------------------------
//	BlockIterator blockIterator;
//	IntVec3 localBlockCoords	 = m_currentChunk->GetLocalBlockCoordsFromIndex( m_blockIndex );
//	IntVec3 neighborBlockCoords	 = localBlockCoords;
//
//	if ( localBlockCoords.x == 0 )
//	{
//		// Set X position to MAX_INDEX
//		neighborBlockCoords.x = CHUNK_MAX_INDEX_X;
//
//		// Calculate and return blockIterator with the new index and neighbor Chunk*
//		blockIterator.m_blockIndex	 = g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords( neighborBlockCoords );
//		blockIterator.m_currentChunk = m_currentChunk->m_westNeighbor;
//		return blockIterator;
//	}
//	
//	// Decrement X position
//	localBlockCoords.x--;
//
//	// Calculate and return blockIterator with the new index but same Chunk*
//	blockIterator.m_blockIndex	 = g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords( localBlockCoords );
//	blockIterator.m_currentChunk = m_currentChunk;
//	return blockIterator;
}

//----------------------------------------------------------------------------------------------------------------------
BlockIterator BlockIterator::GetSkyNeighborBlock()
{
	if ( m_currentChunk == nullptr || m_currentChunk->m_blockList == nullptr )
	{
		return BlockIterator( nullptr, -1 );
	}

	int localZ = ( m_blockIndex >> (CHUNK_BITS_X + CHUNK_BITS_Y) );
	if ( localZ == CHUNK_MAX_INDEX_Z )
	{
		// return INVALID BlockIter since localZ is above chunk MAX Z
		return BlockIterator( nullptr, -1 );
	}
	else
	{
		return BlockIterator( m_currentChunk, m_blockIndex + CHUNK_BLOCKS_PER_LAYER );
	}

//	//----------------------------------------------------------------------------------------------------------------------
//	// My initial attempt
//	//----------------------------------------------------------------------------------------------------------------------
//	BlockIterator blockIterator;
//	IntVec3 localBlockCoords = m_currentChunk->GetLocalBlockCoordsFromIndex( m_blockIndex );
//	if ( localBlockCoords.z == CHUNK_MAX_INDEX_Z )
//	{
//		// Calculate and return blockIterator with the new index and neighbor Chunk*
//		blockIterator.m_blockIndex	 = -1;
//		blockIterator.m_currentChunk = nullptr;
//		return blockIterator;
//	}
//
//	// Increment Z position
//	localBlockCoords.z++;
//
//	// Calculate and return blockIterator with the new index but same Chunk*
//	blockIterator.m_blockIndex	 = g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords( localBlockCoords );
//	blockIterator.m_currentChunk = m_currentChunk;
//	return blockIterator;
}

//----------------------------------------------------------------------------------------------------------------------
BlockIterator BlockIterator::GetGroundNeighborBlock()
{
	if ( m_currentChunk == nullptr || m_currentChunk->m_blockList == nullptr )
	{
		return BlockIterator( nullptr, -1 );
	}

	int localZ = ( m_blockIndex >> (CHUNK_BITS_X + CHUNK_BITS_Y) );
	if ( localZ == 0 )
	{
		// return INVALID BlockIter since localZ is below chunk MIN Z
		return BlockIterator( nullptr, -1 );
	}
	else
	{
		return BlockIterator( m_currentChunk, m_blockIndex - CHUNK_BLOCKS_PER_LAYER );
	}

//	//----------------------------------------------------------------------------------------------------------------------
//	// My initial attempt
//	//----------------------------------------------------------------------------------------------------------------------
//	BlockIterator blockIterator;
//	IntVec3 localBlockCoords = m_currentChunk->GetLocalBlockCoordsFromIndex( m_blockIndex );
//	if ( localBlockCoords.z == 0 )
//	{
//		// Calculate and return blockIterator with the new index and neighbor Chunk*
//		blockIterator.m_blockIndex	 = -1;
//		blockIterator.m_currentChunk = nullptr;
//		return blockIterator;
//	}
//
//	// Decrement Z position
//	localBlockCoords.z--;
//
//	// Calculate and return blockIterator with the new index but same Chunk*
//	blockIterator.m_blockIndex	 = g_theGame->m_currentWorld->GetBlockIndexFromLocalBlockCoords( localBlockCoords );
//	blockIterator.m_currentChunk = m_currentChunk;
//	return blockIterator;
}

//----------------------------------------------------------------------------------------------------------------------
bool BlockIterator::IsBlockIndexValid() const
{
	if ( ( m_blockIndex >= 0 ) && ( m_blockIndex < MAX_BLOCKS_PER_CHUNK ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}
