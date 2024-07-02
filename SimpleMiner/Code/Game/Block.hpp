#pragma once

#include "Game/BlockDef.hpp"
#include "Engine/Core/Rgba8.hpp"

//----------------------------------------------------------------------------------------------------------------------
// Bitmask (Bitmask-ing is the process of covering all the "unwanted" parts of the bits)
constexpr unsigned char INDOOR_LIGHTING_BIT_MASK  = 0b00001111;		// 0b00001111 = 15 in binary
constexpr unsigned char OUTDOOR_LIGHTING_BIT_MASK = 0b11110000;			
constexpr unsigned char MAX_LIGHT_INFLUENCE		  = 15;			
constexpr unsigned char INDOOR_LIGHTING_BITS	  = 4;

//----------------------------------------------------------------------------------------------------------------------
// Bitflags
constexpr unsigned char BLOCK_CAN_SEE_SKY				= 0b00000001;
constexpr unsigned char BLOCK_IS_IN_DIRTY_LIGHT_QUEUE	= 0b00000010;
constexpr unsigned char BLOCK_IS_OPAQUE					= 0b00000100;
constexpr unsigned char BLOCK_IS_SOLID					= 0b00001000;
constexpr unsigned char BLOCK_IS_VISIBLE				= 0b00010000;

//----------------------------------------------------------------------------------------------------------------------
class Block
{
public:
	Block(); 
	~Block();

	bool CanBlockSeeTheSky();
	void SetIsSky();
	void SetIsNotSky();

	bool IsLightDirty() const;
	void SetLightDirty();
	void SetLightNotDirty();
	void SetIsDirty();
	void SetIsNotDirty();

	BlockDef GetBlockDef() const;

	int  GetOutdoorLightInfluence() const;
	int  GetIndoorLightInfluence()  const;
	void SetOutdoorLightInfluence( unsigned char newOutdoorLightInfluence );
	void SetIndoorLightInfluence(  unsigned char newIndoorLightInfluence  );

	Rgba8 GetColorFromLightInfluence() const;

public:
	unsigned char m_blockTypeIndex			= static_cast<unsigned char>(0);
	unsigned char m_blockLightInfluence		= static_cast<unsigned char>(0);
	unsigned char m_blockBitFlags			= static_cast<unsigned char>(0);
};