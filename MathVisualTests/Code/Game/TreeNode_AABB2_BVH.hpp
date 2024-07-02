#pragma once

#include "Game/GameConvexObject2D.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"

#include <vector>


//----------------------------------------------------------------------------------------------------------------------
class TreeNode_AABB2_BVH
{
public:
	TreeNode_AABB2_BVH();
	TreeNode_AABB2_BVH( std::vector<GameConvexObject*> gameObjectsHere );
	~TreeNode_AABB2_BVH();

	void UpdateBounds_TreeNode();
	void SplitAndCreateChildNodes( int desiredLevel );
	void RaycastNodes( RaycastResult2D& rayResult, std::vector<GameConvexObject*>& entitiesHere );

public:
	std::vector<GameConvexObject*> m_gameObjectsHere;
	AABB2				m_bounds		= AABB2( Vec2( FLT_MAX, FLT_MAX ), Vec2( -FLT_MAX, -FLT_MAX ) );
	TreeNode_AABB2_BVH* m_leftChild		= nullptr;
	TreeNode_AABB2_BVH* m_rightChild	= nullptr;
};