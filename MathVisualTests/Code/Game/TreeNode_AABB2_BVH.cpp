#include "Game/TreeNode_AABB2_BVH.hpp"

//----------------------------------------------------------------------------------------------------------------------
TreeNode_AABB2_BVH::TreeNode_AABB2_BVH()
{
}


//----------------------------------------------------------------------------------------------------------------------
TreeNode_AABB2_BVH::TreeNode_AABB2_BVH( std::vector<GameConvexObject*> gameObjectsHere )
{
	m_gameObjectsHere = gameObjectsHere;
}


//----------------------------------------------------------------------------------------------------------------------
TreeNode_AABB2_BVH::~TreeNode_AABB2_BVH()
{
	delete m_leftChild;
	delete m_rightChild;
	m_leftChild  = nullptr;
	m_rightChild = nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
void TreeNode_AABB2_BVH::UpdateBounds_TreeNode()
{
	for ( int i = 0; i < m_gameObjectsHere.size(); i++ )
	{
		GameConvexObject const* curConvexObject = m_gameObjectsHere[i];
		// 1. Loop through all convexObjects, find the min and max bounds
		Vec2  curDiscCenter = curConvexObject->GetBoundingDiscCenter();
		float curDiscRadius = curConvexObject->GetBoundingDiscRadius();
		float treeEdgeMinX  = curDiscCenter.x - curDiscRadius;
		float treeEdgeMinY  = curDiscCenter.y - curDiscRadius;
		float treeEdgeMaxX  = curDiscCenter.x + curDiscRadius;
		float treeEdgeMaxY  = curDiscCenter.y + curDiscRadius;
		if ( treeEdgeMinX < m_bounds.m_mins.x )
		{
			m_bounds.m_mins.x = treeEdgeMinX;
		}
		if ( treeEdgeMinY < m_bounds.m_mins.y )
		{
			m_bounds.m_mins.y = treeEdgeMinY;
		}
		if ( treeEdgeMaxX > m_bounds.m_maxs.x )
		{
			m_bounds.m_maxs.x = treeEdgeMaxX;
		}
		if ( treeEdgeMaxY > m_bounds.m_maxs.y )
		{
			m_bounds.m_maxs.y = treeEdgeMaxY;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void TreeNode_AABB2_BVH::SplitAndCreateChildNodes( int desiredLevel )
{
	if ( m_gameObjectsHere.empty() || desiredLevel <= 0 )
	{
		return;
	}
	UpdateBounds_TreeNode();

	std::vector<GameConvexObject*> leftList;
	std::vector<GameConvexObject*> rightList;
	// Split objects left or right relative to the boundsMiddle 
	for ( int i = 0; i < m_gameObjectsHere.size(); i++ )
	{
		GameConvexObject* curObject = m_gameObjectsHere[i];
		Vec2 discCenter = curObject->GetBoundingDiscCenter();
		float widthX	= m_bounds.m_maxs.x - m_bounds.m_mins.x;
		float widthY	= m_bounds.m_maxs.y - m_bounds.m_mins.y;
		float midPoint;
		if ( widthX > widthY )
		{
			// Cut on width
			midPoint = widthX * 0.5f;
			midPoint = m_bounds.m_mins.x + midPoint;
			if ( discCenter.x < midPoint )
			{
				leftList.push_back( curObject );
			}
			else
			{
				rightList.push_back( curObject );
			}
		}
		else
		{
			// Cut on height
			midPoint = widthY * 0.5f;
			midPoint = m_bounds.m_mins.y + midPoint;
			if ( discCenter.y < midPoint )
			{
				leftList.push_back( curObject );
			}
			else
			{
				rightList.push_back( curObject );
			}
		}
	}

	delete m_leftChild;
	delete m_rightChild;
	m_leftChild  = nullptr;
	m_rightChild = nullptr;
 	if ( !leftList.empty() || !rightList.empty() )
	{
		--desiredLevel;
		int currentLevel = desiredLevel;
		if ( currentLevel == 0 )
		{
			return;
		}

		if ( !leftList.empty() )
		{
			m_leftChild  = new TreeNode_AABB2_BVH( leftList );
			m_leftChild->SplitAndCreateChildNodes( currentLevel );
		}
		if ( !rightList.empty() )
		{
			m_rightChild = new TreeNode_AABB2_BVH( rightList );
			m_rightChild->SplitAndCreateChildNodes( currentLevel );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void TreeNode_AABB2_BVH::RaycastNodes( RaycastResult2D& rayResult, std::vector<GameConvexObject*>& entitiesHere )
{
	RaycastResult2D	rayResult_LeftChild		= rayResult;
	RaycastResult2D	rayResult_RightChild	= rayResult;	

	// 1. Raycast against "ourselves"
	rayResult = RaycastVsAABB2D( rayResult.m_rayStartPos, rayResult.m_rayForwardNormal, rayResult.m_rayMaxLength, m_bounds );

	if ( rayResult.m_didImpact )
	{
		// 2. Raycast against children if they exist
		if ( m_leftChild )
		{
			m_leftChild->RaycastNodes( rayResult_LeftChild, entitiesHere );
		}
		if ( m_rightChild )
		{
			m_rightChild->RaycastNodes( rayResult_RightChild, entitiesHere );
		}
	}

	if ( m_leftChild == nullptr && m_rightChild == nullptr )
	{
		if ( rayResult.m_didImpact )
		{
			for ( int i = 0; i < m_gameObjectsHere.size(); i++ )
			{
				GameConvexObject* curObjectHere = m_gameObjectsHere[i];
				entitiesHere.push_back( curObjectHere );
			}
		}
		return;
	}


	// 1. Ray against curTree, if hit
	// 2. Ray against child, if they exist
	// 3. if children are null AND raycast hit, get all "entitesHere" 
}
