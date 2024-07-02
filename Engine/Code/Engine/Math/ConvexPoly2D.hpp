#pragma once

#include <vector>

//----------------------------------------------------------------------------------------------------------------------
struct Vec2;


//-----------------------------------------------------------------------------------------------------------------------
class ConvexPoly2D
{
public:
	ConvexPoly2D();
	ConvexPoly2D( std::vector<Vec2> ccwOrderedPoints );
	~ConvexPoly2D();

	bool						IsPointInside	( Vec2 const& point		);
	Vec2 						GetNearestPoint	( Vec2 const& pointToCheck );
	std::vector<Vec2> const&	GetCcwOrderedPoints() const;

private:
	std::vector<Vec2> m_ccwOrderedPoints;
};