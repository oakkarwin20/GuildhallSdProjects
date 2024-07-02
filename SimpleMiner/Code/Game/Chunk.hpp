#pragma once

#include "Game/Block.hpp"
#include "Game/BlockIterator.hpp"

#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <vector>
#include <string>

//----------------------------------------------------------------------------------------------------------------------
// Chunk Variables
constexpr int CHUNK_BITS_X = 4;
constexpr int CHUNK_BITS_Y = 4;
constexpr int CHUNK_BITS_Z = 7;

// Chunk Dimensions
constexpr int CHUNK_SIZE_X				= (1 << CHUNK_BITS_X);
constexpr int CHUNK_SIZE_Y				= (1 << CHUNK_BITS_Y);
constexpr int CHUNK_SIZE_Z				= (1 << CHUNK_BITS_Z);
constexpr int CHUNK_BLOCKS_PER_LAYER	= CHUNK_SIZE_X * CHUNK_SIZE_Y;
constexpr int CHUNK_HALF_HEIGHT			= CHUNK_SIZE_Z / 2;
constexpr int CHUNK_SEA_LEVEL			= CHUNK_HALF_HEIGHT;
constexpr int CHUNK_MAX_INDEX_X			= (CHUNK_SIZE_X - 1);		// Subtract 1 since this value is used for indexing
constexpr int CHUNK_MAX_INDEX_Y			= (CHUNK_SIZE_Y - 1);		// Subtract 1 since this value is used for indexing
constexpr int CHUNK_MAX_INDEX_Z			= (CHUNK_SIZE_Z - 1);		// Subtract 1 since this value is used for indexing
constexpr int MAX_BLOCKS_PER_CHUNK		= CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z;

// Chunk Bitmask
constexpr int CHUNK_MASK_X = CHUNK_SIZE_X - 1;
constexpr int CHUNK_MASK_Y = CHUNK_MASK_X << CHUNK_BITS_Y;
constexpr int CHUNK_MASK_Z = CHUNK_MASK_Y << CHUNK_BITS_Z;

// Chunk BitShift 
constexpr int CHUNK_BITSHIFT_X = 0;								// zzzzzzzyyyyXXXX	// Shift 0 bits to put XXXX in bottom					// (I.e., stay the same)
constexpr int CHUNK_BITSHIFT_Y = CHUNK_BITS_X;					// 0000zzzzzzzYYYY	// Shift 4 x-bits to put YYYY in bottom					// (I.e., move past "length" of x-bits)
constexpr int CHUNK_BITSHIFT_Z = CHUNK_BITS_X + CHUNK_BITS_Y;	// 00000000ZZZZZZZ	// Shift 8 (x-bits && y-bits) to put ZZZZZZZ in bottom	// (I.e., move past "length" of x-bits AND y-bits)

// CHUNK VERSION NUMBER
constexpr int VERSION_NUM = 1;

//----------------------------------------------------------------------------------------------------------------------
class Texture;
class VertexBuffer;

//----------------------------------------------------------------------------------------------------------------------
enum ChunkStatus
{
	CHUNK_STATUS_NEW,			// Constructed but not queued for work yet
	CHUNK_STATUS_QUEUED,		// Queued, waiting to be claimed by a worker thread
	CHUNK_STATUS_WORKING,		// Claimed by a worker thread who is currently executing it
	CHUNK_STATUS_COMPLETED,		// Completed, placed into the completed list for the main thread
	CHUNK_STATUS_RETRIEVED,		// Retrieved by the main thread, retired from the Job system
};

//----------------------------------------------------------------------------------------------------------------------
class ChunkGenerateJob : public Job
{
public:
	ChunkGenerateJob( Chunk* chunk, int sleepMS )
		: m_chunk( chunk )
		, m_sleepMS( sleepMS )
	{}

	virtual void Execute() override;

	Chunk*						m_chunk			= nullptr;
	std::atomic<ChunkStatus>	m_jobStatus		= CHUNK_STATUS_NEW;
	int							m_sleepMS		= 0;
};

//----------------------------------------------------------------------------------------------------------------------
struct CaveInfo
{
	CaveInfo( IntVec2 chunkCoords )
	{
		m_startChunkCoords = chunkCoords;
	}

	IntVec2				m_startChunkCoords			= IntVec2::ZERO;
	Vec3				m_startWorldBlockPos		= Vec3::ZERO;
	std::vector<Vec3>	m_caveNodePositionList;
};

//----------------------------------------------------------------------------------------------------------------------
class Chunk
{
public:
	Chunk( IntVec2 chunkCoords );
	~Chunk();

	void Update();
	void Render() const;

	void		AddDebugDrawChunkBoundaryVerts( std::vector<Vertex_PCU>& verts ) const;
	void		AddVertsForBlock3D( std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color, AABB2 const& skyUVs, AABB2 const& sideUVs, AABB2 const& groundUVs, int blockIndex, IntVec3 blockCoords );
	int			GetIndexFromSpriteCoord( IntVec2 spriteCoords ) const;
	void		RebuildVertexes();
	void		SetWorldBounds();
	IntVec3		GetLocalBlockCoordsFromIndex( int index );
	IntVec3		GetWorldBlockCoordsFromLocalBlockCoords( IntVec3 localBlockCoords );;
	void		RegenerateBlocksInChunk();
	bool		IsBlockIndexValid( int index ) const;
	bool		IsBlockOpaque( Block const* currentBlock ) const;
	void		PlaceAirCube( IntVec3 localBlockCoords, int cubeRadius );
	void		CarveAABB3D( Vec3 worldCenter, Vec3 halfDimensions );
	void		CarveCapsule3D( Vec3 boneWorldStart, Vec3 boneWorldEnd, float radius );

	// Lighting
	Rgba8 GetColorFromLightInfluence( BlockIterator neighbor, BlockIterator currentBlockIter );

	// Loading Function
	bool		LoadSavedChunkOnDisk();
	std::string GetFilePath();

	// BlockTemplate functions
	void SpawnBlockTemplateAtLocalCoords( std::string const& blockTemplateName, IntVec3 localOrigin );
	void SetBlockTypeAtLocalBlockCoords( IntVec3 localCoords, int blockType );
	void SetBlockTypeAtLocalBlockCoords( int localBlockX, int localBlockY, int localBlockZ, int blockType );
	bool IsInBoundsLocal( int localBlockX, int localBlockY, int localBlockZ );
	bool IsInBoundsLocal( IntVec3 localBlockCoords );

	void			AddCaves( unsigned int caveSeed );
	void			DebugAddVertsForCaves( std::vector<Vertex_PCU>& verts ) const;
	static AABB3	GetChunkBoundsForChunkCoords( IntVec2 const& chunkCoords );

public:
	//----------------------------------------------------------------------------------------------------------------------
	// Core Variables
	//----------------------------------------------------------------------------------------------------------------------
	Block*					m_blockList			= nullptr;
	AABB3					m_chunkWorldBounds	= AABB3( Vec3::ZERO, Vec3::ZERO );	
	IntVec2					m_chunkCoords		= IntVec2::ZERO;
	std::vector<Vertex_PCU> m_cpuVerts;
	VertexBuffer*			m_gpuVerts			= nullptr;

	//----------------------------------------------------------------------------------------------------------------------
	// Re-generation flags
	//----------------------------------------------------------------------------------------------------------------------
	bool m_isMeshDirty	= false;	
	bool m_needsSaving	= false;	

	//----------------------------------------------------------------------------------------------------------------------
	// Loading
	bool m_chunkWasLoadedFromFile = false;

	//----------------------------------------------------------------------------------------------------------------------
	// Chunk Neighbor pointers
	//----------------------------------------------------------------------------------------------------------------------
	Chunk* m_northNeighbor	= nullptr;
	Chunk* m_southNeighbor	= nullptr;
	Chunk* m_eastNeighbor	= nullptr;
	Chunk* m_westNeighbor	= nullptr;

	//----------------------------------------------------------------------------------------------------------------------
	// Relevant World Generation Variables that uses Perlin noise
	//----------------------------------------------------------------------------------------------------------------------
	float m_minIceTempThreshold				= 0.4f;
	float m_minOceanicThreshold				= 0.45f;
	float m_minHumidityThreshold			= 0.4f;
	float m_minTreeTypeHumidityThreshold	= 0.45f;
	float m_coldTreeTemperature				= 0.3f;
	float m_moderateHumidityThreshold 		= 0.7f;
	int	  m_minSandHeightThreshold 			= 3;

	//----------------------------------------------------------------------------------------------------------------------
	// Biome factors for each column
	//----------------------------------------------------------------------------------------------------------------------
	// Noise for each column
	int				 	   m_grassHeightsForEachColumn[CHUNK_BLOCKS_PER_LAYER] = {};
	float					m_temperatureForEachColumn[CHUNK_BLOCKS_PER_LAYER] = {};
	float					   m_humidityForEachColumn[CHUNK_BLOCKS_PER_LAYER] = {};
	float					  m_treeNoiseForEachColumn[CHUNK_BLOCKS_PER_LAYER] = {};
	float					m_forestNoiseForEachColumn[CHUNK_BLOCKS_PER_LAYER] = {};
	float					 m_oceanNoiseForEachColumn[CHUNK_BLOCKS_PER_LAYER] = {};
	float					  m_caveNoiseForEachColumn[CHUNK_BLOCKS_PER_LAYER] = {};
	
	//----------------------------------------------------------------------------------------------------------------------
	// Min noise threshold for each column
	//----------------------------------------------------------------------------------------------------------------------
	float		  m_minTreeNoiseThresholdForEachColumn[CHUNK_BLOCKS_PER_LAYER] = {};
	int	  m_groundPulledDownByOceanHeightForEachColumn[CHUNK_BLOCKS_PER_LAYER] = {};

	//----------------------------------------------------------------------------------------------------------------------
	// Positions of all trees to be generated in this Chunk
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<IntVec3> m_treeToGenerateInLocalPositionList;

	//----------------------------------------------------------------------------------------------------------------------
	// Caves
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<CaveInfo> m_caveInfoList;
//	std::vector<IntVec2> m_caveStartChunkCoordsList;
	
	//----------------------------------------------------------------------------------------------------------------------
	// Cloud block templates
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<IntVec3> m_cloudToGenerateInLocalPositionList;
	float m_minCloudNoiseThresholdForThisColumn = 0.7f;
};

