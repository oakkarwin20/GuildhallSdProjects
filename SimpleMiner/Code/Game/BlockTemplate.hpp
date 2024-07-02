#pragma once

#include "Game//GameCommon.hpp"	

#include "Engine/Math/IntVec3.hpp"

//----------------------------------------------------------------------------------------------------------------------
struct BlockTemplateEntry
{
	BlockTemplateEntry( int blockType, IntVec3 const& localOffset );

	int		m_blockType   = 0;
	IntVec3 m_localOffset = IntVec3::ZERO;
};

//----------------------------------------------------------------------------------------------------------------------
class BlockTemplate
{
	BlockTemplate( std::string const& newBlockTemplateName );
	~BlockTemplate();

public:
	static void					InitializeBlockTemplate();
	static BlockTemplate const* GetTemplateByName( std::string const& templateName );
	
private:
	static BlockTemplate*		CreateNewBlockTemplate( std::string const& newBlockTemplateName );

public:
	std::string						m_templateName;
	std::vector<BlockTemplateEntry> m_blockTemplateEntryList;

private:
	static std::vector<BlockTemplate*> s_blockTemplateList;
};