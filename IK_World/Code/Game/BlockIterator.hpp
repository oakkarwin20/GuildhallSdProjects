#pragma once

#include "Engine/Math/Vec3.hpp"

//----------------------------------------------------------------------------------------------------------------------
constexpr float BLOCK_HALF_SIZE = 0.5f;

//----------------------------------------------------------------------------------------------------------------------
class Chunk;
class Block;

//----------------------------------------------------------------------------------------------------------------------
class BlockIterator
{
public:
	BlockIterator();
	BlockIterator( Chunk* chunk, int blockIndex );
	~BlockIterator();

	Block*	GetBlock() const;
	Vec3	GetBlockCenterInWorldPos();

	BlockIterator GetNorthNeighborBlock();
	BlockIterator GetSouthNeighborBlock();
	BlockIterator GetEastNeighborBlock();
	BlockIterator GetWestNeighborBlock();
	BlockIterator GetSkyNeighborBlock();
	BlockIterator GetGroundNeighborBlock();

	bool IsBlockIndexValid() const;

public:
	Chunk*	m_currentChunk	= nullptr;
	int		m_blockIndex	= -1;
};