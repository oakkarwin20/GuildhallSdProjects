#include "Game/BlockTemplate.hpp"

std::vector<BlockTemplate*> BlockTemplate::s_blockTemplateList;

//----------------------------------------------------------------------------------------------------------------------
BlockTemplate::BlockTemplate(  std::string const& newBlockTemplateName )
{
	m_templateName = newBlockTemplateName;
}

//----------------------------------------------------------------------------------------------------------------------
BlockTemplate::~BlockTemplate()
{
}

//----------------------------------------------------------------------------------------------------------------------
void BlockTemplate::InitializeBlockTemplate()
{
	// Get block type IDs ahead of time for building templates
	//	0	= "air"			
	//	1	= "stone"			
	//	2	= "dirt"			
	//	3	= "grass"			
	//	4	= "coal"			
	//	5	= "iron"			
	//	6	= "gold"			
	//	7	= "diamond"		
	//	8	= "water"			
	//	9	= "cobblestone"	
	//	10	= "glowstone"		
	//  13  = "ice"			
	//  12  = "sand"			
	//  13  = "darkWood"		
	//  14  = "lightWood"		
	//  15  = "leaf"			
	//  16  = "cactus"		
	//  17  = "transparentIce"
	//  18  = "lighterLeaf"

	//----------------------------------------------------------------------------------------------------------------------
	// Create oakTree blockTemplate and add to blockTemplateList
	//----------------------------------------------------------------------------------------------------------------------
	BlockTemplate* oakTree = CreateNewBlockTemplate( "oakTree" );
	oakTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 15, IntVec3( 0, 0, 3) ) );		// oakTreeTop 
	oakTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 15, IntVec3( 0, 1, 2) ) );		// oakTreeLeftArm
	oakTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 15, IntVec3( 0,-1, 2) ) );		// oakTreeRightArm
	oakTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 15, IntVec3( 0, 0, 2) ) );		// oakTreeUpperBody
	oakTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 15, IntVec3( 1, 0, 2) ) );		// oakTreeUpperBodyForward
	oakTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 15, IntVec3(-1, 0, 2) ) );		// oakTreeUpperBodyBack
	oakTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 13, IntVec3( 0, 0, 1) ) );		// oakTreeLowerMiddle
	oakTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 13, IntVec3( 0, 0, 0) ) );		// oakTreeBottom
	s_blockTemplateList.push_back( oakTree );

	//----------------------------------------------------------------------------------------------------------------------
	// Create cactus blockTemplate and add to blockTemplateList
	//----------------------------------------------------------------------------------------------------------------------
	BlockTemplate* cactus = CreateNewBlockTemplate( "cactus" );
	cactus->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 16, IntVec3( 0, 0, 3) ) );		// cactusTop 
	cactus->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 16, IntVec3( 0, 0, 2) ) );		// cactusUpperBody
	cactus->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 16, IntVec3( 0, 0, 1) ) );		// cactusLowerBody
	cactus->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 16, IntVec3( 0, 0, 0) ) );		// cactusBottom
	s_blockTemplateList.push_back( cactus );

	//----------------------------------------------------------------------------------------------------------------------
	// SpruceTree blockTemplate and add to blockTemplateList
	//----------------------------------------------------------------------------------------------------------------------
	BlockTemplate* spruceTree = CreateNewBlockTemplate( "spruceTree" );
	spruceTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 14, IntVec3(  0,  0, 3 ) ) );		// spruceTreeTop 
	spruceTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 18, IntVec3(  0,  1, 3 ) ) );		// spruceTreeLeftArm
	spruceTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 18, IntVec3(  0, -1, 3 ) ) );		// spruceTreeRightArm
	spruceTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 14, IntVec3(  0,  0, 2 ) ) );		// spruceTreeUpperBody
	spruceTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 18, IntVec3(  1,  0, 3 ) ) );		// spruceTreeUpperBodyForward
	spruceTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 18, IntVec3( -1,  0, 3 ) ) );		// spruceTreeUpperBodyBack
	spruceTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 14, IntVec3(  0,  0, 1 ) ) );		// spruceTreeLowerMiddle
	spruceTree->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 14, IntVec3(  0,  0, 0 ) ) );		// spruceTreeBottom
	s_blockTemplateList.push_back( spruceTree );

	//----------------------------------------------------------------------------------------------------------------------
	// Create cactus blockTemplate and add to blockTemplateList			// #Note: Order goes "left to right" (sameY, negativeX to positiveX)
	//----------------------------------------------------------------------------------------------------------------------
	BlockTemplate* cloud = CreateNewBlockTemplate( "cloud" );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -3,  0, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -2,  0, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -1,  0, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  0,  0, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  1,  0, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  2,  0, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  3,  0, 0 ) ) );

	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -2, -1, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -1, -1, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  0, -1, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  1, -1, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  2, -1, 0 ) ) );

	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -2,  1, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -1,  1, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  0,  1, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  1,  1, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  2,  1, 0 ) ) );

	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -1, -2, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  0, -2, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  1, -2, 0 ) ) );

	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -1,  2, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  0,  2, 0 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  1,  2, 0 ) ) );

	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -2,  0, 1 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -1,  0, 1 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -0,  0, 1 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  1,  0, 1 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  2,  0, 1 ) ) );

	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -1,  1, 1 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  0,  1, 1 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  1,  1, 1 ) ) );
	
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3( -1, -1, 1 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  0, -1, 1 ) ) );
	cloud->m_blockTemplateEntryList.push_back( BlockTemplateEntry( 11, IntVec3(  1, -1, 1 ) ) );
	
	s_blockTemplateList.push_back( cloud );
}

//----------------------------------------------------------------------------------------------------------------------
BlockTemplate const* BlockTemplate::GetTemplateByName( std::string const& templateName )
{
	for ( int i = 0; i < s_blockTemplateList.size(); i++ )
	{
		if ( s_blockTemplateList[i]->m_templateName == templateName )
		{
			return s_blockTemplateList[i];
		}
	}

	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
BlockTemplate* BlockTemplate::CreateNewBlockTemplate( std::string const& newBlockTemplateName )
{
	BlockTemplate* blockTemplate = new BlockTemplate( newBlockTemplateName );
	s_blockTemplateList.push_back( blockTemplate );
	return blockTemplate;
}

//----------------------------------------------------------------------------------------------------------------------
BlockTemplateEntry::BlockTemplateEntry( int blockType, IntVec3 const& localOffset )
{
	m_blockType   = blockType;
	m_localOffset =	localOffset;
}
