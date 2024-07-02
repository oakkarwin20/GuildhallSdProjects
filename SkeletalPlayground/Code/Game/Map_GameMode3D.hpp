#pragma once

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class GameMode3D;
class VertexBuffer;
class IndexBuffer;
class FoodManager;

//----------------------------------------------------------------------------------------------------------------------
struct Block
{
	Block( AABB3 aabb3, bool isWalkable, bool isClimbable = false )
		: m_aabb3( aabb3 )
		, m_isWalkable( isWalkable )
		, m_isClimbable( isClimbable )
	{
	}
	~Block()
	{
	}

	AABB3	m_aabb3			= AABB3( Vec3::ZERO, Vec3::ZERO );
	bool	m_isWalkable	= true;
	bool	m_isClimbable	= false;
};


//----------------------------------------------------------------------------------------------------------------------
class Map_GameMode3D
{
public:
	Map_GameMode3D( GameMode3D* game );
	~Map_GameMode3D();

	void Update( float deltaSeconds );
	void Render() const;

	void RenderBlocks( std::vector<Vertex_PCU>& verts ) const;
	void InitializeBlocks();
	void InitDynamicTerrain();
	void UpdateDynamicTerrain( float deltaSeconds );

public:
	GameMode3D*					m_gameMode3D	= nullptr;
	VertexBuffer*				m_vbo			= nullptr;
	IndexBuffer*				m_ibo			= nullptr;
	std::vector<Vertex_PCU>		m_planeVerts;
	std::vector<unsigned int>	m_indexList;
	float						m_minFloorHeight = -2.0f;
	float						m_maxFloorHeight =  2.0f;
	FoodManager*				m_foodManager	 = nullptr;
	float						m_noiseTime		 = 0.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Block Objects
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Block*> m_blockList; 
	// Floors
//	Block* m_floor_NE	= new Block( AABB3(	  10.0f,   10.0f, 0.0f, 400.0f, 1500.0f,   1.0f ), true );
	Block* m_floor_NW	= new Block( AABB3( -200.0f,    0.0f, 0.0f,   0.0f,  200.0f,  14.0f ), true );
	Block* m_floor_SE	= new Block( AABB3(    1.0f, -200.0f, 0.0f, 200.0f,    0.0f,  10.0f ), true );
	Block* m_floor_SW	= new Block( AABB3( -200.0f, -200.0f, 0.0f,   0.0f,    0.0f,  12.0f ), true );
	// Boxes
//	Block* m_box1_E		= new Block( AABB3(    0.0f,    0.0f, 0.0f,  50.0f,   50.0f,  50.0f ), true );
//	Block* m_box2_NE	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  50.0f,   50.0f,  50.0f ), true );
//	Block* m_box3_SE	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  50.0f,   50.0f,  50.0f ), true );
//	Block* m_box4_SE	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  50.0f,   50.0f,  50.0f ), true );
//	// Stairs
//	Block* m_stairs1	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
//	Block* m_stairs2	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
//	Block* m_stairs3	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
//	Block* m_stairs4	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
//	Block* m_stairs5	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
//	Block* m_stairs6	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
//	Block* m_stairs7	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
//	Block* m_stairs8	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
//	Block* m_stairs9	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
//	Block* m_stairs10	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  10.0f,   50.0f,  50.0f ), true );
//	// Slopes
//	Block* m_slope_SE	= new Block( AABB3(    0.0f,    0.0f, 0.0f,  20.0f,   50.0f,  50.0f ), true );
//	// Elevator
//	Block* m_elevator_1 = new Block( AABB3(    0.0f,    0.0f, 0.0f, 100.0f,  100.0f,   1.0f ), true );
//	Block* m_elevator_2 = new Block( AABB3(    0.0f,    0.0f, 0.0f, 100.0f,  100.0f,   1.0f ), true );
//	// Cliff
//	Block* m_cliff		= new Block( AABB3(    0.0f,    0.0f, 0.0f, 100.0f,  100.0f, 100.0f ), false, true );
};