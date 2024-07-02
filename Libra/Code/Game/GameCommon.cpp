#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
void DebugDrawRing( Vec2 const& center, float radius, float thickness, Rgba8 const& color )
{
	float halfThickness = 0.5f * thickness;
	float innerRadius = radius - halfThickness;
	float outerRadius = radius + halfThickness;

	constexpr int NUM_SIDES = 32;
	constexpr int NUM_TRIS = 2 * NUM_SIDES;
	constexpr int NUM_VERTS = 3 * NUM_TRIS;

	Vertex_PCU verts[ NUM_VERTS ];

	constexpr float DEGREES_PER_SIDE = 360.0f / static_cast<float>( NUM_SIDES );

	for ( int sideNum = 0; sideNum < NUM_SIDES; sideNum++ )
	{
		float startDegrees = DEGREES_PER_SIDE * static_cast<float>( sideNum );
		float endDegrees = DEGREES_PER_SIDE * static_cast<float>( sideNum + 1 );

		float cosStart = CosDegrees( startDegrees );
		float sinStart = SinDegrees( startDegrees );
		float cosEnd = CosDegrees( endDegrees );
		float sinEnd = SinDegrees( endDegrees );
		
		Vec3 innerStartPos( center.x + innerRadius * cosStart, center.y + innerRadius * sinStart, 0.0f);
		Vec3 outerStartPos( center.x + outerRadius * cosStart, center.y + outerRadius * sinStart, 0.0f);
		Vec3 outerEndPos( center.x + outerRadius * cosEnd, center.y + outerRadius * sinEnd, 0.0f);
		Vec3 innerEndPos( center.x + innerRadius * cosEnd, center.y + innerRadius * sinEnd, 0.0f);

		int vertIndexA = ( 6 * sideNum ) + 0;
		int vertIndexB = ( 6 * sideNum ) + 1;
		int vertIndexC = ( 6 * sideNum ) + 2;
		int vertIndexD = ( 6 * sideNum ) + 3;
		int vertIndexE = ( 6 * sideNum ) + 4;
		int vertIndexF = ( 6 * sideNum ) + 5;

		verts[ vertIndexA ].m_position = innerEndPos;
		verts[ vertIndexB ].m_position = innerStartPos;
		verts[ vertIndexC ].m_position = outerStartPos;

		verts[ vertIndexD ].m_position = innerEndPos;
		verts[ vertIndexE ].m_position = outerStartPos;
		verts[ vertIndexF ].m_position = outerEndPos;

		verts[vertIndexA].m_color = color;
		verts[vertIndexB].m_color = color;
		verts[vertIndexC].m_color = color;

		verts[vertIndexD].m_color = color;
		verts[vertIndexE].m_color = color;
		verts[vertIndexF].m_color = color;
	}

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray(NUM_VERTS, verts);
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
	g_theRenderer->DrawVertexArray( verts, drawLine );
}