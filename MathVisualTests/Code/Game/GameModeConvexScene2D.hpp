#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameConvexObject2D.hpp"

#include "Engine/Core/BufferWriter.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/ConvexHull2D.hpp"

//----------------------------------------------------------------------------------------------------------------------
// asdf*	pointer						
// *asdf	dereference pointer			
// asdf&	reference to a value		
// &asdf	address of variable			
//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------
struct BoundingDiscObject2D
{
	Vec2  m_discCenter  = Vec2::ZERO;
	float m_radius		= -1.0f;
};


//----------------------------------------------------------------------------------------------------------------------
enum ChunkTypeGHCS
{
	INVALID_IGNORE		= 0,
	SCENE_INFO			= 0x01,
	CONVEX_POLY_2D		= 0x02,
	CONVEX_HULL_2D		= 0x80,
	BOUNDING_DISCS_2D	= 0x81,
	BOUNDING_AABB2		= 0x82,
	AABB2_TREE			= 0x83,
};


//----------------------------------------------------------------------------------------------------------------------
struct TocEntryGHCS
{
	TocEntryGHCS( ChunkTypeGHCS chunkType, uint32_t chunkOffset, uint32_t chunkTotalSize )
	{
		m_chunkType		 = chunkType;
		m_chunkOffset	 = chunkOffset;
		m_chunkTotalSize = chunkTotalSize;
	}
	
	ChunkTypeGHCS	m_chunkType			= ChunkTypeGHCS::INVALID_IGNORE;
	uint32_t		m_chunkOffset		= 0;	// chunkOffsetFromBufferStart
	uint32_t		m_chunkTotalSize	= 0;
};


//----------------------------------------------------------------------------------------------------------------------
class TreeNode_AABB2_BVH;
class BufferParser;
class BufferWriter;


//----------------------------------------------------------------------------------------------------------------------
enum OptimizationType
{
	NONE,
	NARROW_DISC,
	BROAD_BVH,
	NUM_TYPES,
};


//----------------------------------------------------------------------------------------------------------------------
class GameModeConvexScene2D : public GameModeBase
{
public:
	GameModeConvexScene2D();
	virtual ~GameModeConvexScene2D();

	virtual void Startup();
	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Reshuffle();
	virtual void Shutdown();

	void UpdateGameCamera_ConvexScene2D();
	void RenderWorldObjects() const;
	void RenderUIObjects() const;

	// Game input and debug functions
	void UpdateGeneralInput( float deltaSeconds );
	void UpdateHotkeysInput( float deltaSeconds );
	void UpdatePauseQuitAndSlowMo();

	// Raycasts
	void Update1000Rays();
	void UpdateRaycastResult();
	void Raycast_Poly_NoOptimizations();
	void Raycast_NarrowPhase_Disc();
	void Raycast_BroadPhase_BVH();

	// Game Convex Object
	void SelectConvexObject( GameConvexObject*& gameConvex, Vec2 const& testPoint );
	std::string const GetEnumAsString() const;

	//----------------------------------------------------------------------------------------------------------------------
	// AABB2 Tree Node Functions
	//----------------------------------------------------------------------------------------------------------------------
	void RegenerateBVH();
	void ToggleRegenerateBVH();
	
	// Loading and saving chunks
	void ParseConvexSceneFromFile( std::vector<unsigned char>& buffer );
	// Append 
	void AppendHeader	( BufferWriter& bufferWriter );
	void AppendTOC		( BufferWriter& bufferWriter ); 
	void AppendChunkData( BufferWriter& bufferWriter, ChunkTypeGHCS chunkType );

	// Parse specific chunk types
	void								ParseConvexSceneInfo ( BufferParser& bufferParser, uint32_t chunkLocation, uint32_t chunkTotalSize );
	std::vector<ConvexPoly2D>			ParseConvexPolys2D	 ( BufferParser& bufferParser, uint32_t chunkLocation, uint32_t chunkTotalSize );
	std::vector<BoundingDiscObject2D>	ParseBoundingDisc2D	 ( BufferParser& bufferParser, uint32_t chunkLocation, uint32_t chunkTotalSize );
	TreeNode_AABB2_BVH* 				ParseAABBTree		 ( BufferParser& bufferParser, uint32_t chunkLocation, uint32_t chunkTotalSize );
//	void ParseBoundingAABB2	 ( uint32_t chunkLocation, uint32_t chunkTotalSize );


	// Parsing & Writing test functions
	void AppendTestFileBufferData( BufferWriter& bufWrite, BufferEndian endianMode );
	void ParseTestFileBufferData ( BufferParser& bufParse, BufferEndian endianMode );

public:
	std::vector<GameConvexObject*> m_gameConvexObjectList;
	GameConvexObject* m_selectedConvexObject	= nullptr;
	GameConvexObject* m_hoveredConvexObject		= nullptr;
//	GameConvexObject  m_testConvexObject		= GameConvexObject(); 

	// Debug Hotkeys
	bool m_debugRenderBoundingDisc_F1	= false;
	bool m_renderConvexOpaque_F2		= true;
	bool m_debugRenderBoundsBVH_F4		= true;
	bool m_randomizeConvexShapes_F8		= true;
	OptimizationType m_optimizationType	= NONE;
	bool m_regenerateBVH				= false;

	//----------------------------------------------------------------------------------------------------------------------
	// Camera Variables
	Camera m_worldCamera_ConvexScene2D;
	Camera m_UiCamera_ConvexScene2D;

	BitmapFont* m_textFont = nullptr;

	//----------------------------------------------------------------------------------------------------------------------
	// Raycast variables
	RaycastResult2D		m_rayConvexResult2D;
	Vec2				m_rayStartPos			= Vec2( WORLD_CENTER_X + 5.0f, WORLD_CENTER_Y - 20.0f );
	Vec2				m_rayEndPos				= m_rayStartPos + Vec2( 40.0f, 30.0f );
	float				m_rayLength				= ( m_rayEndPos - m_rayStartPos ).GetLength();
	Vec2				m_rayFwd				= ( m_rayEndPos - m_rayStartPos ).GetNormalized();
	float				m_arrowSize				= 2.0f;
	float				m_arrowThickness		= 0.5f;
	bool				m_didAABB2Impact 		= false;
	Vec2				m_updatedImpactPos		= Vec2( -1.0f, -1.0f );
	Vec2				m_updatedImpactNormal	= Vec2( -1.0f, -1.0f );

	Rgba8 m_rayDefaultColor		 = Rgba8::GREEN;
	Rgba8 m_rayImpactDistColor	 = Rgba8::RED;
	Rgba8 m_rayAfterImpactColor  = Rgba8::GRAY;
	Rgba8 m_rayImpactDiscColor	 = Rgba8::WHITE;
	Rgba8 m_rayImpactNormalColor = Rgba8::YELLOW;
	
	// Debug rays
	std::vector<RaycastResult2D> m_debugRayResultsList;
	int   m_numRandRays			= 1024;
	float m_timeBeforeDebugRays = 0.0f;
	float m_timeAfterDebugRays  = 0.0f;

	// TreeNodes
	TreeNode_AABB2_BVH* m_rootTreeNode = nullptr;

	// World bounds
//	AABB2 m_parsedWorldBounds		= AABB2::ZERO_TO_ONE;
//	bool  m_shouldChangeWorldBounds	= false;
	float m_rayImpactDiscRadius     = 1.0f;
	float m_rayImpactNormalScalar   = 5.0f;

	std::vector<TocEntryGHCS*> m_tocEntryList;
	unsigned int m_tocLocation = 0;
};