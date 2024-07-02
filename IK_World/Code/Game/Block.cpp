#include "Game/Block.hpp"
#include "Game/Chunk.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//----------------------------------------------------------------------------------------------------------------------
Block::Block()
{
}

//----------------------------------------------------------------------------------------------------------------------
Block::~Block()
{
}

//----------------------------------------------------------------------------------------------------------------------
bool Block::CanBlockSeeTheSky()
{
    return (m_blockBitFlags & BLOCK_CAN_SEE_SKY) != 0;
}

//----------------------------------------------------------------------------------------------------------------------
void Block::SetIsSky()
{
    m_blockBitFlags |= BLOCK_CAN_SEE_SKY;       // "Set these bits"
}

//----------------------------------------------------------------------------------------------------------------------
void Block::SetIsNotSky()
{
	m_blockBitFlags &= ~BLOCK_CAN_SEE_SKY;      // "Clear these bits"
}

//----------------------------------------------------------------------------------------------------------------------
void Block::SetIsDirty()
{
	m_blockBitFlags |= BLOCK_IS_IN_DIRTY_LIGHT_QUEUE;       // "Set these bits"		// Activates this flag
}

//----------------------------------------------------------------------------------------------------------------------
void Block::SetIsNotDirty()
{
	m_blockBitFlags &= ~BLOCK_IS_IN_DIRTY_LIGHT_QUEUE;      // "Clear these bits"	// Deactivate this flag
}

//----------------------------------------------------------------------------------------------------------------------
BlockDef Block::GetBlockDef() const
{
	BlockDef blockDef = BlockDef::m_blockDefList[m_blockTypeIndex];
	return blockDef;
}

//----------------------------------------------------------------------------------------------------------------------
bool Block::IsLightDirty() const
{
    return (m_blockBitFlags & BLOCK_IS_IN_DIRTY_LIGHT_QUEUE) != 0;
}

//----------------------------------------------------------------------------------------------------------------------
void Block::SetLightDirty()
{
	m_blockBitFlags |= BLOCK_IS_IN_DIRTY_LIGHT_QUEUE;       // "Set these bits"		// Activates this flag
}

//----------------------------------------------------------------------------------------------------------------------
void Block::SetLightNotDirty()
{
	m_blockBitFlags &= ~BLOCK_IS_IN_DIRTY_LIGHT_QUEUE;       // "Clear these bits"		// Deactivates this flag
}

//----------------------------------------------------------------------------------------------------------------------
int Block::GetOutdoorLightInfluence() const
{
	// m_blockLighting has indoor light influence (0-15) in low 4 bits:
	// 00001111 = indoor light influence bits (0-15) = 15
	// 11110000 = outdoor light influence bits (0-15, shifted over by 4 bits)
	return m_blockLightInfluence >> CHUNK_BITS_X;
}

//----------------------------------------------------------------------------------------------------------------------
int Block::GetIndoorLightInfluence() const
{
    // m_blockLighting has indoor light influence (0-15) in low 4 bits:
    // 00001111 = indoor light influence bits (0-15) = 15
    // 11110000 = outdoor light influence bits (0-15, shifted over by 4 bits)
    return m_blockLightInfluence & INDOOR_LIGHTING_BIT_MASK;
}

//----------------------------------------------------------------------------------------------------------------------
void Block::SetOutdoorLightInfluence( unsigned char newOutdoorLightInfluence )
{
	GUARANTEE_OR_DIE( newOutdoorLightInfluence <= MAX_LIGHT_INFLUENCE, "Illegal light influence value" );

	// 1. Start with:                                 OOOOIIII (outdoor light influnece in high nibble; indoor low)
	// 2. Destroy old values, preserve indoor bits,:  OOOOIIII
	m_blockLightInfluence &= ~OUTDOOR_LIGHTING_BIT_MASK;        // shorthand for "Clear my bits"

	// 3. Set new outdoor bits: OOOONNNN ( NNNN = new indoor light influence (0-15)
	m_blockLightInfluence |= (newOutdoorLightInfluence << INDOOR_LIGHTING_BITS );
}

//----------------------------------------------------------------------------------------------------------------------
void Block::SetIndoorLightInfluence( unsigned char newIndoorLightInfluence )
{
    GUARANTEE_OR_DIE( newIndoorLightInfluence <= MAX_LIGHT_INFLUENCE, "Illegal light influence value" );

    // 1. Start with:      OOOOIIII (outdoor light influnece in high nibble; indoor low)
    // 2. Destroy indoor:  OOOO0000
    m_blockLightInfluence &= ~INDOOR_LIGHTING_BIT_MASK;
    // 3. Set indoor bits: OOOONNNN ( NNNN = new indoor light influence (0-15)
    m_blockLightInfluence |= newIndoorLightInfluence;
}

//----------------------------------------------------------------------------------------------------------------------
Rgba8 Block::GetColorFromLightInfluence() const
{
	// Get indoor & outdoor light influences each in [0,15]
	int currentOutdoorLightInfluence = GetOutdoorLightInfluence();
	int currentIndoorLightInfluence  = GetIndoorLightInfluence();

	// Range-map them into [0,255] and set them into the red& green color channels
	float outdoorTint = RangeMapClamped( float( currentOutdoorLightInfluence ), 0.0f, 15.0f, 0.0f, 255.0f );
	float indoorTint  = RangeMapClamped( float( currentIndoorLightInfluence  ), 0.0f, 15.0f, 0.0f, 255.0f );

	// Temp hack
//	outdoorTint = 127;
	
	// Get color based on combined values from indoor and outdoor light influences
	Rgba8 tint = Rgba8( unsigned char( outdoorTint),   unsigned char(indoorTint), 127, 255 );
//	Rgba8 tint = Rgba8( unsigned char( outdoorTint),		  unsigned char( 0 ), 127, 255 );
//	Rgba8 tint = Rgba8(			 unsigned char( 0 ), unsigned char( indoorTint ), 127, 255 );
//	Rgba8 tint = Rgba8(		   unsigned char( 255 ),		unsigned char( 255 ), 255, 255 );
	return tint;
}