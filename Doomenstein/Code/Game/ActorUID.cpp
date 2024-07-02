#include "Game/ActorUID.hpp"

//----------------------------------------------------------------------------------------------------------------------
ActorUID::ActorUID( unsigned int salt, unsigned int index )
{
	// Perform bitwise operations
//	unsigned int mask = 0xFFFF0000;							// Mask to clear left 16 bits
//	unsigned int saltShifted = ( salt << 16 ) & mask;		// Shift salt left 16 bits and mask out left 16 bits	
//	unsigned int shiftedSalt = salt << 16;					// Shift salt left 16 bits and mask out left 16 bits
//	m_saltAndIndexData = shiftedSalt | index;
//	index &= mask;											// Mask out left 16 bits of index
//	m_saltAndIndexData = saltShifted | index;				// Perform bitwise OR with shifted salt and masked index

// Perform bitwise operations
	unsigned int shiftedSalt = salt << 16;					// Shift salt left 16 bits
	m_saltAndIndexData		 = shiftedSalt | index;			// Or operator between shiftedSalt and Index
}

//----------------------------------------------------------------------------------------------------------------------
ActorUID::ActorUID()
{
	m_saltAndIndexData = INVALID;
}

//----------------------------------------------------------------------------------------------------------------------
ActorUID::~ActorUID()
{
}

//----------------------------------------------------------------------------------------------------------------------
unsigned int ActorUID::GetIndex() const
{
	if ( m_saltAndIndexData != INVALID )
	{
		unsigned int index;
		index = m_saltAndIndexData << 16;
		index = m_saltAndIndexData >> 16; 
		return index;
	}

	return INVALID;
}

//----------------------------------------------------------------------------------------------------------------------
bool ActorUID::IsValid()
{
	if ( m_saltAndIndexData != INVALID )
	{
		return true;
	}
	return false;
}
