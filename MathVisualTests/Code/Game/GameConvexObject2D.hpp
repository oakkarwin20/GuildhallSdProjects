#pragma once

#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/ConvexHull2D.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"


//----------------------------------------------------------------------------------------------------------------------
class GameConvexObject
{
public:
	GameConvexObject();
	GameConvexObject( ConvexPoly2D convexPoly2D, Vec2 const& boundingDiscCenter, float boundingDiscRadius );
	~GameConvexObject();

	// Getter Functions
	ConvexPoly2D const& GetConvexPoly() const;
	ConvexHull2D const& GetConvexHull() const;
	Vec2		 const& GetBoundingDiscCenter() const;
	float		 const& GetBoundingDiscRadius() const;

	// Utilities
	void ComputeOffsetsToPoint( Vec2 const& testPoint );
	void Translate( Vec2 const& cursorPos, float deltaSeconds );
	void Rotate( Vec2 const& pivotPoint, float rotateDegrees, float deltaSeconds );
	void Scale ( Vec2 const& pivotPoint, float scaleAmount, float deltaSeconds );
	void RandomizeConvexPosAndShape();
	void RandomizeConvexPolyShape( Vec2 const& discCenter, float discRadius );
	void DebugRenderBoundingDisc( std::vector<Vertex_PCU>& verts, float thickness, Rgba8 const& color ) const;

private:
	ConvexPoly2D m_convexPoly2D	= ConvexPoly2D();
	ConvexHull2D m_convexHull2D	= ConvexHull2D( m_convexPoly2D );
	Vec2  m_boundingDiscCenter	= Vec2::ZERO;
	float m_boundingDiscRadius	= 1.0f;

public:
	std::vector<Vec2> m_offsetPointsList;
	Vec2  m_offset_discCenter = Vec2::ZERO;
};