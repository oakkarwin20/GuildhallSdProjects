#include "Game/GameMode_Animations.hpp"
#include "Game/Map_Animations.hpp"


//----------------------------------------------------------------------------------------------------------------------
Map_Animations::Map_Animations()
{
	m_floorBounds = AABB3( Vec3(-100.0f, -100.0f, -1.0f), Vec3(100.0f, 100.0f, 1.0f) );
	// #ToDo: addVerts once on startup using vertexBuffer instead of pushing verts every frame
}


//----------------------------------------------------------------------------------------------------------------------
Map_Animations::~Map_Animations()
{
}


//----------------------------------------------------------------------------------------------------------------------
void Map_Animations::Startup()
{
}


//----------------------------------------------------------------------------------------------------------------------
void Map_Animations::Shutdown()
{
}


//----------------------------------------------------------------------------------------------------------------------
void Map_Animations::Update( float deltaSeconds )
{
}


//----------------------------------------------------------------------------------------------------------------------
void Map_Animations::Render( std::vector<Vertex_PCU>& verts ) const
{
	AddVertsForAABB3D( verts, m_floorBounds, Rgba8::MAGENTA );
}
