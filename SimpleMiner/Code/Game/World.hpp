#pragma once

#include "Game/BlockIterator.hpp"
#include "Chunk.hpp"

#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <string>
#include <vector>
#include <map>
#include <deque>

//----------------------------------------------------------------------------------------------------------------------
class Texture;
class Chunk;

//----------------------------------------------------------------------------------------------------------------------
struct GameRaycastResult3D : RaycastResult3D
{
	BlockIterator	m_currentBlockIter		= BlockIterator();
	Vec3			m_impactBlockFaceDir	= Vec3::ZERO; 
};

//----------------------------------------------------------------------------------------------------------------------
class World
{
public:
	World();
	~World();
	
	void Update();
	void Render() const;

	// Helper functions
	IntVec2			GetChunkCoordsFromWorldPos( Vec2 worldPos );
	Vec2			GetChunkCenterPosXYFromChunkCoords( IntVec2 chunkCoords );
	Vec3			GetPlayerPos();
	IntVec3			GetLocalBlockCoordsFromWorldPos( Vec3 worldPos );
	int				GetBlockIndexFromLocalBlockCoords( IntVec3 localBlockCoords );
	bool			DoesChunkExist( IntVec2 chunkCoords );
	Chunk*			GetChunkAtCoords( IntVec2 chunkCoords );
	IntVec3			GetWorldBlockCoordsFromBlockIter( BlockIterator blockIter );
	int				GetNumActiveChunks() const;
	BlockIterator	LocateBlock( IntVec3 worldBlockCoords );

	// Player Input
	void EquipBlockType();
	void ChangeBlockTypeAtOrBelowPlayerToAir();
	void PlaceBlockOnGroundBelowPlayerZ();
	
	// Activation and Deactivation 
	Chunk* GetNearestChunkToPlayer();
	void ActivateNearestMissingChunk();
	void DeactivateFurthestExistingChunk();
	void ActivateChunk( Chunk* chunkToActivate );						// Set up neighbor pointers and initialize lighting
	void DeactiveChunkAtCoords( IntVec2 const& chunkCoords );
	bool IsThreadAlreadyWorkingOnThisChunk( IntVec2 chunkCoord );

	// Saving and loading functions
	void EncodeChunkData( std::vector<char>& out_chunkData, Chunk*& chunkToSave );	
	void SaveChunkToFile( Chunk*& chunkToSave );

	// Lighting functions
	void InitializeIndoorLightInfluenceLevels(  Chunk* currentChunk );
	void InitializeOutdoorLightInfluenceLevels( Chunk* currentChunk );
	void ProcessAllDirtyLighting();
	int  ComputeBlockCorrectIndoorLight(  BlockIterator neighborBlock, BlockIterator currentBlock  );
	int  ComputeBlockCorrectOutdoorLight( BlockIterator neighborBlock, BlockIterator currentBlock  );
	void ProcessNextDirtyLightBlock();
	void MarkLightingDirty( BlockIterator blockIterator );
	void UndirtyAllBlocksInChunk();
	void MarkSkyBlocks( Chunk* currentChunk );
	void MarkLightingDirtyIfBlockIsNotOpqaue( BlockIterator blockIter );
	void ProcessDirtyLightBlock( BlockIterator blockIter );
	void IncreaseLightLevelToAtLeastOneLessThanNeighbor( int& correctIndoorLight, int& correctOutdoorLight, BlockIterator neighborBlock, BlockIterator currentBlock );
	int  GetBrightestNeighborIndoorLightValue(  BlockIterator neighbor );
	int  GetBrightestNeighborOutdoorLightValue( BlockIterator neighbor );

	// Mesh Functions
	void DirtyChunkMesh( BlockIterator& blockIter );
//	void DirtyChunkMesh( BlockIterator blockIter );

	// Debug Functions
	void DebugDrawCavesForAllChunks() const;
	void ToggleDebugRenderingFunctions() const;
	void AddVertsForLightDebugBlock( std::vector<Vertex_PCU>& verts, BlockIterator blockIter );
	void AddVertsForBillboardText3D( std::vector<Vertex_PCU>& debugTextList, Vec3 textOrigin, float cellHeight, std::string const& text, Rgba8 textColor, float cellAspect = 0.5f );
	void AddVertsForLightDebugBlocks( std::vector<Vertex_PCU>& verts );
	void AddVertsForDebugBlockIter( std::vector<Vertex_PCU>& verts, BlockIterator blockIter );
	void TestBlockIterGetNeighbor();

	// BlockTemplate Functions
//	BlockTemplate* CreateTreeBlockTemplate();

	void ForceCreateWorldFolder();

	// Raycast Functions
	void RaycastVsBlocks();
	GameRaycastResult3D RaycastXYZ( Vec3 const& rayStartPos, Vec3 const& rayFwdNormal, float rayMaxDist, BlockIterator currentBlockIter );
	bool IsBlockAtCoordsAir( BlockIterator currentBlockIter );

public:
	// Raycast result
	GameRaycastResult3D m_raycastResult			= GameRaycastResult3D();
	bool				m_shouldLockRaycast		= false;
	Vec3				m_raycastStartPos		= Vec3::ZERO;
	Vec3				m_fwdNormal				= Vec3::ZERO;

	// Chunk Status Variables
	std::map<IntVec2, Chunk*>	m_activeChunkList;
	std::vector<Chunk*>			m_inGenerationProgressChunkList;

	// Activation / Deactivation variables
	int  m_chunkActivationRange			= -1;
	int  m_chunkDeactivationRange		= -1;
	bool m_activationOccuredThisFrame	= false;
	bool m_didChunkGetRebuilt			= false;

	// Neighborhood variables
	int m_maxChunksRadiusX	= -1;
	int m_maxChunksRadiusY	= -1;
	int m_maxNumChunks		= -1;

	// Read from Game Config
	bool m_isHiddenSurfaceRemoval = false;
	
	// Lighting
	std::deque<BlockIterator> m_dirtyLightBlockList;

	// Placing blocks
	unsigned char m_equipedBlockType = 10;

	//----------------------------------------------------------------------------------------------------------------------
	// Debug Text variables
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> m_debugTextList;
	float m_debugBlockMinDist = -1.0f;
	float m_debugBlockMaxDist = -1.0f;
	float m_debugBlockLayers  = -1.0f;
	
	//----------------------------------------------------------------------------------------------------------------------
	// Debug BlockIter variables
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> m_debugBlockIterList;
	int						m_debugBlockIterStartingIndex	= 16878;
	int						m_debugBlockIterCurrentIndex	= 16878;
	Chunk*					m_BlockIterCurrentChunk			= nullptr;
	BlockIterator			m_debugBlockIter				= BlockIterator( nullptr, -1 );
	bool					m_doThisOnce					= true;
	bool					m_shouldRepeat2					= true;
	IntVec2					m_currentChunkCoords			= IntVec2( -2, 0 );
	IntVec2					m_startingChunkCoords			= IntVec2( -2, 0 );

	//----------------------------------------------------------------------------------------------------------------------
	// Debug Cave variables
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> g_debugDrawCavesList;

	//----------------------------------------------------------------------------------------------------------------------d
	// Seed Variables
	//----------------------------------------------------------------------------------------------------------------------d
	int m_worldSeed			= 1;
	int m_humiditySeed		= m_worldSeed + 1;
	int m_temperatureSeed	= m_worldSeed + 2;
	int m_hillinessSeed		= m_worldSeed + 3;
	int m_oceannessSeed		= m_worldSeed + 4;
	int m_treeSeed			= m_worldSeed + 5;
	int m_caveSeed			= m_worldSeed + 6;
	int m_cloudSeed			= m_worldSeed + 7;

	// Tree Block Template
//	BlockTemplate* m_treeBlockTemplate;

	bool m_activateOnce = true;

	// World Time & Day
	float m_worldTime = 0.0f;
};

//----------------------------------------------------------------------------------------------------------------------
// IntVec2 < operator
bool operator<( IntVec2 const& a, IntVec2 const& b );