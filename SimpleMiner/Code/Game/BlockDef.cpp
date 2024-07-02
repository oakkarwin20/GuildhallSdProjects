#include "Game/BlockDef.hpp"

// static std::vector<BlockDef> m_blockDefList;
std::vector<BlockDef> BlockDef::m_blockDefList;

//----------------------------------------------------------------------------------------------------------------------
BlockDef::BlockDef()
{
}

//----------------------------------------------------------------------------------------------------------------------
BlockDef::~BlockDef()
{
}

//----------------------------------------------------------------------------------------------------------------------
void BlockDef::InitializeBlockDefs()
{
	// BlockDefs		Name				Visible	Solid	Opaque	indoorLightValue,		SkySprite		SideSprite	  GroundSprite
	CreateNewBlockDef( "air",				false,	false,	false,				  0,	 IntVec2( 0, 0), IntVec2( 0, 0), IntVec2(0,0) );		// 0
	CreateNewBlockDef( "stone",				true,	true,	true,				  0,	 IntVec2(33,32), IntVec2(33,32), IntVec2(33,32) ); 		// 1
	CreateNewBlockDef( "dirt",				true,	true,	true,				  0,	 IntVec2(32,34), IntVec2(32,34), IntVec2(32,34) ); 		// 2
	CreateNewBlockDef( "grass",				true,	true,	true,				  0,	 IntVec2(32,33), IntVec2(33,33), IntVec2(32,34) ); 		// 3
	CreateNewBlockDef( "coal",				true,	true,	true,				  0,	 IntVec2(63,34), IntVec2(63,34), IntVec2(63,34) ); 		// 4
	CreateNewBlockDef( "iron",				true,	true,	true,				  0,	 IntVec2(63,35), IntVec2(63,35), IntVec2(63,35) ); 		// 5
	CreateNewBlockDef( "gold",				true,	true,	true,				  0,	 IntVec2(63,36), IntVec2(63,36), IntVec2(63,36) ); 		// 6
	CreateNewBlockDef( "diamond",			true,	true,	true,				  0,	 IntVec2(63,37), IntVec2(63,37), IntVec2(63,37) ); 		// 7
	CreateNewBlockDef( "water",				true,	true,	true,				  0,	 IntVec2(32,44), IntVec2(32,44), IntVec2(32,44) ); 		// 8
	CreateNewBlockDef( "cobblestone",		true,	true,	true,				  0,	 IntVec2(35,32), IntVec2(35,32), IntVec2(35,32) ); 		// 9
	CreateNewBlockDef( "glowstone",			true,	true,	true,				 15,	 IntVec2(46,34), IntVec2(46,34), IntVec2(46,34) ); 		// 10
	CreateNewBlockDef( "ice",				true,	true,	true,				  0,	 IntVec2(36,35), IntVec2(36,35), IntVec2(36,35) ); 		// 11
	CreateNewBlockDef( "sand",				true,	true,	true,				  0,	 IntVec2(34,34), IntVec2(34,34), IntVec2(34,34) ); 		// 12
	CreateNewBlockDef( "darkWood",			true,	true,	true,				  0,	 IntVec2(37,33), IntVec2(37,33), IntVec2(37,33) ); 		// 13
	CreateNewBlockDef( "lightWood",			true,	true,	true,				  0,	 IntVec2(36,33), IntVec2(36,33), IntVec2(36,33) ); 		// 14
	CreateNewBlockDef( "leaf",				true,	true,	true,				  0,	 IntVec2(32,35), IntVec2(32,35), IntVec2(32,35) ); 		// 15
	CreateNewBlockDef( "cactus",			true,	true,	true,				  0,	 IntVec2(37,36), IntVec2(37,36), IntVec2(37,36) ); 		// 16
	CreateNewBlockDef( "transparentIce",	true,	true,	true,				  0,	 IntVec2(44,34), IntVec2(44,34), IntVec2(44,34) ); 		// 17
	CreateNewBlockDef( "lighterLeaf",		true,	true,	true,				  0,	 IntVec2(37,37), IntVec2(37,37), IntVec2(37,37) ); 		// 18
//	CreateNewBlockDef( "glowstone",			true,	true,	true,				  0,	 IntVec2(46,34), IntVec2(46,34), IntVec2(46,34) ); 
}

//----------------------------------------------------------------------------------------------------------------------
void BlockDef::CreateNewBlockDef( std::string name, bool isVisible, bool isSolid, bool isOpaque, int indoorLightValue, IntVec2 skySprite, IntVec2 sideSprite, IntVec2 groundSprite )
{
	BlockDef newBlockDef;
	newBlockDef.m_name					= name;
	newBlockDef.m_isVisible				= isVisible;
	newBlockDef.m_isSolid				= isSolid;
	newBlockDef.m_isOpaque				= isOpaque;
	newBlockDef.m_skySprite				= skySprite;
	newBlockDef.m_sideSprite			= sideSprite;
	newBlockDef.m_groundSprite			= groundSprite;
	newBlockDef.m_indoorEmissionLight	= indoorLightValue;
 	BlockDef::m_blockDefList.push_back(newBlockDef);
}
