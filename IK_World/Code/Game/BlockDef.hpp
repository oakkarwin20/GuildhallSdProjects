#pragma once

#include "Engine/Math/IntVec2.hpp"

#include <vector>
#include <string>

//----------------------------------------------------------------------------------------------------------------------
class BlockDef
{
public:
	BlockDef();
	~BlockDef();

	static void InitializeBlockDefs();
	static void CreateNewBlockDef( std::string name, bool isVisible, bool isSolid, bool isOpaque, int indoorLightValue, IntVec2 skySprite, IntVec2 sideSprite, IntVec2 groundSprite );	
	
//	int GetBlockDefByName( std::string const& blockName );
//	static void InitializeBlockTemplates();

public:
	static std::vector<BlockDef> m_blockDefList;


	// Block name
	std::string m_name;

	// Block traits
	bool m_isVisible	= false;
	bool m_isSolid		= false;
	bool m_isOpaque		= false;

	// UV spriteCoords
	IntVec2 m_skySprite		= IntVec2::ZERO;
	IntVec2 m_sideSprite	= IntVec2::ZERO;
	IntVec2 m_groundSprite	= IntVec2::ZERO;

	int m_indoorEmissionLight = 0;
};