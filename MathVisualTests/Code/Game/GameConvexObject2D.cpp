#include "Game/GameConvexObject2D.hpp"
#include "Game/App.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


//----------------------------------------------------------------------------------------------------------------------
GameConvexObject::GameConvexObject()
{
}


//----------------------------------------------------------------------------------------------------------------------
GameConvexObject::GameConvexObject( ConvexPoly2D convexPoly2D, Vec2 const& boundingDiscCenter, float boundingDiscRadius )
{
	m_convexPoly2D		 = convexPoly2D;
 	m_convexHull2D		 = ConvexHull2D( convexPoly2D );
 	m_boundingDiscCenter = boundingDiscCenter;
 	m_boundingDiscRadius = boundingDiscRadius;
}


//----------------------------------------------------------------------------------------------------------------------
GameConvexObject::~GameConvexObject()
{
}


//----------------------------------------------------------------------------------------------------------------------
ConvexPoly2D const& GameConvexObject::GetConvexPoly() const
{
	return m_convexPoly2D;
}


//----------------------------------------------------------------------------------------------------------------------
ConvexHull2D const& GameConvexObject::GetConvexHull() const
{
	return m_convexHull2D;
}


//----------------------------------------------------------------------------------------------------------------------
Vec2 const& GameConvexObject::GetBoundingDiscCenter() const
{
	return m_boundingDiscCenter;
}


//----------------------------------------------------------------------------------------------------------------------
float const& GameConvexObject::GetBoundingDiscRadius() const
{
	return m_boundingDiscRadius;
}


//----------------------------------------------------------------------------------------------------------------------
// This function computes offsets (disps) from convexPoints to a testPoint
//----------------------------------------------------------------------------------------------------------------------
void GameConvexObject::ComputeOffsetsToPoint( Vec2 const& testPoint )
{
	// 1. Compute each convexPoly point's offset from cursorPos
	m_offsetPointsList.clear();
	std::vector<Vec2> const& orderedPoints = m_convexPoly2D.GetCcwOrderedPoints();
	for ( int i = 0; i < orderedPoints.size(); i++ )
	{
		Vec2 const& curPoint = orderedPoints[i];
		Vec2 curOffset		 = curPoint - testPoint;
		m_offsetPointsList.push_back( curOffset );
	}
	// Compute offset boundingDisc & radius offset to testPoint
	m_offset_discCenter = m_boundingDiscCenter - testPoint;
}


//----------------------------------------------------------------------------------------------------------------------
void GameConvexObject::Translate( Vec2 const& cursorPos, float deltaSeconds )
{
	// Multiply by delta seconds to make changes frame rate independent
//	float rotatePercentage = rotateDegrees * deltaSeconds;

	//----------------------------------------------------------------------------------------------------------------------
	// 1. Compute offsets dispCursorToPolyPoint
	// 2. Apply offsets to cursor pos every frame
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vec2> newPointsList;
	for ( int i = 0; i < m_offsetPointsList.size(); i++ )
	{
		Vec2 newPos = cursorPos + m_offsetPointsList[i];
		newPointsList.push_back( newPos );
	}
	// Update member variables
	m_convexPoly2D		 = ConvexPoly2D( newPointsList );
	m_convexHull2D		 = ConvexHull2D( m_convexPoly2D );
	m_boundingDiscCenter = cursorPos + m_offset_discCenter;
}


//----------------------------------------------------------------------------------------------------------------------
void GameConvexObject::Rotate( Vec2 const& pivotPoint, float rotateDegrees, float deltaSeconds )
{
	// Multiply by delta seconds to make changes frame rate independent
	float rotatePercentage = rotateDegrees * deltaSeconds;

	ComputeOffsetsToPoint( pivotPoint );
	std::vector<Vec2> newPointsList = m_offsetPointsList;
	for ( int i = 0; i < newPointsList.size(); i++ )
	{
		Vec2& curOffset	 = newPointsList[i];
		Vec2 dir		 = curOffset.GetNormalized();
		float length	 = curOffset.GetLength();
		dir		 		 = dir.GetRotatedDegrees( rotatePercentage );
		curOffset		 = pivotPoint + ( dir * length );
	}
	// Rotate discCenter
	Vec2 dir				 = m_offset_discCenter.GetNormalized();
	dir						 = dir.GetRotatedDegrees( rotatePercentage );
	float length			 = m_offset_discCenter.GetLength();
	Vec2 rotatedDiscCenter	 = pivotPoint + ( dir * length );
	// Update member variables
	m_convexPoly2D			 = ConvexPoly2D( newPointsList );
	m_convexHull2D			 = ConvexHull2D( m_convexPoly2D );
	m_boundingDiscCenter	 = rotatedDiscCenter;
}


//----------------------------------------------------------------------------------------------------------------------
void GameConvexObject::Scale( Vec2 const& pivotPoint, float scaleAmount, float deltaSeconds )
{
	// Multiply by delta seconds to make changes frame rate independent
	float scalePercentage = scaleAmount * deltaSeconds;
	scalePercentage		  =  GetClamped( scalePercentage, -1.0f, 1.0f );

	ComputeOffsetsToPoint( pivotPoint );
	std::vector<Vec2> newPointsList = m_offsetPointsList;
	for ( int i = 0; i < newPointsList.size(); i++ )
	{
		Vec2& curOffset	  = newPointsList[i];
		Vec2  scalar	  = curOffset * scalePercentage;
		curOffset		 += pivotPoint + scalar;
	}
	// Compute scaled boundingDiscCenter offset
	Vec2 scalar				 = m_offset_discCenter * scalePercentage;
	m_offset_discCenter		+= pivotPoint + scalar;
	// Compute scaled radius
	float scaledRadius		 = m_boundingDiscRadius * scalePercentage;
	// Update member variables
	m_convexPoly2D			 = ConvexPoly2D( newPointsList );
	m_convexHull2D			 = ConvexHull2D( m_convexPoly2D );
	m_boundingDiscCenter	 = m_offset_discCenter;
	m_boundingDiscRadius	+= scaledRadius;
}


//----------------------------------------------------------------------------------------------------------------------
void GameConvexObject::RandomizeConvexPosAndShape()
{
	float randDiscCenterX;
	float randDiscCenterY;
	float randRadius;
	// 1. Compute rand numbers for new convexPoly positions
	if ( m_shouldChangeWorldBounds )
	{
		randDiscCenterX = g_theRNG->RollRandomFloatInRange( g_parsedWorldBounds.m_mins.x + 10.0f, g_parsedWorldBounds.m_maxs.x - 10.0f );
		randDiscCenterY = g_theRNG->RollRandomFloatInRange( g_parsedWorldBounds.m_mins.y +  5.0f, g_parsedWorldBounds.m_maxs.y -  5.0f );
		randRadius		= g_theRNG->RollRandomFloatInRange( 2.0f, 5.0f );
	}
	else
	{
 		randDiscCenterX	= g_theRNG->RollRandomFloatInRange( 10.0f, 190.0f );
 		randDiscCenterY	= g_theRNG->RollRandomFloatInRange(  5.0f,  95.0f );
		randRadius		= g_theRNG->RollRandomFloatInRange(  3.0f,  10.0f );
	}
	m_boundingDiscCenter	= Vec2( randDiscCenterX, randDiscCenterY );
	// Create new poly at new discCenter with points from offsetList
	RandomizeConvexPolyShape( m_boundingDiscCenter, randRadius );
}


//----------------------------------------------------------------------------------------------------------------------
void GameConvexObject::RandomizeConvexPolyShape( Vec2 const& discCenter, float discRadius )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Pick a disc center, loop from 0 degrees till > 360 degrees with rand deltas per loop. 
	// Choose endPoints as convexPoly points
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vec2> newOrderedPoints;
	float curDegrees = 0.0f;
	// 1. Generate first point
	Vec2 curPoint = discCenter + ( Vec2::X_FWD * discRadius );
	newOrderedPoints.push_back( curPoint );
	// 2. Pre-compute variables and increment curDegrees once
	float minRange	= 35.0f;
	float maxRange	= 135.0f;
	float randDelta	= g_theRNG->RollRandomFloatInRange( minRange, maxRange );
	curDegrees	   += randDelta;
	// Loop till we have a fully shaped convex poly
	while ( curDegrees < 360.0f )
	{
		Vec2 dir		= Vec2::MakeFromPolarDegrees( curDegrees, discRadius );
		dir.Normalize(); 
		curPoint		= discCenter + ( dir * discRadius );
		newOrderedPoints.push_back( curPoint );
		// Only increment curDegrees at the end so the loop can check break if
		// curDegrees has exceeded 360 degrees
		randDelta		= g_theRNG->RollRandomFloatInRange( minRange, maxRange );
		curDegrees	   += randDelta;
	}
	// Update member variables
	m_convexPoly2D		 = ConvexPoly2D( newOrderedPoints );
	m_convexHull2D		 = ConvexHull2D( m_convexPoly2D );
	m_boundingDiscCenter = discCenter;
	m_boundingDiscRadius = discRadius;
}


//----------------------------------------------------------------------------------------------------------------------
void GameConvexObject::DebugRenderBoundingDisc( std::vector<Vertex_PCU>& verts, float thickness, Rgba8 const& color ) const
{
	AddVertsForRing2D( verts, m_boundingDiscCenter, m_boundingDiscRadius, thickness, color );
}
