#include "Game/GameCommon.hpp"

//----------------------------------------------------------------------------------------------------------------------
// Debug globals
//----------------------------------------------------------------------------------------------------------------------
bool g_debugUseWhiteTexture			= false;
bool g_debugDrawChunkBoundaries		= false;
bool g_debugDrawChunkStates			= false;
bool g_debugDrawLightValues			= false;
bool g_debugDrawCurrentBlockIter	= false;
bool g_debugDrawCaves				= false;
bool g_debugUseWorldShader			= true;
bool g_debugDrawRaycast				= true;

extern ConstantBuffer* g_simpleMinerCBO = nullptr;


//----------------------------------------------------------------------------------------------------------------------
void DebugDrawRing()
{
}

//----------------------------------------------------------------------------------------------------------------------
void DebugDrawLine( Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color )
{
	int triangles = 2;
	int verts	= triangles * 3; 
	float halfThickness = thickness / 2;

	Vec2 length = end - start;
	Vec2 normalizedVec = length.GetNormalized();
	Vec2 getForwardNormal = normalizedVec * halfThickness;
	Vec2 getLeftNormal = getForwardNormal.GetRotated90Degrees();
	
	Vec2 EL = end + getForwardNormal + getLeftNormal;
	Vec2 ER = end + getForwardNormal - getLeftNormal;
	Vec2 SL = start - getForwardNormal + getLeftNormal;
	Vec2 SR = start - getForwardNormal - getLeftNormal;

	Vertex_PCU drawLine[] = 
	{
		Vertex_PCU(Vec3(SR.x, SR.y, 0.0f), color, Vec2(0.0f, 0.0f)), //	Triangle A point A
		Vertex_PCU(Vec3(ER.x, ER.y, 0.0f), color, Vec2(0.0f, 0.0f)), //	Triangle A point B
		Vertex_PCU(Vec3(EL.x, EL.y, 0.0f), color, Vec2(0.0f, 0.0f)), //	Triangle A point C

		Vertex_PCU(Vec3(SR.x, SR.y, 0.0f), color, Vec2(0.0f, 0.0f)), //	Triangle B point A
		Vertex_PCU(Vec3(EL.x, EL.y, 0.0f), color, Vec2(0.0f, 0.0f)), //	Triangle B point B
		Vertex_PCU(Vec3(SL.x, SL.y, 0.0f), color, Vec2(0.0f, 0.0f))  //	Triangle B point C
	};

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( verts, drawLine );
}