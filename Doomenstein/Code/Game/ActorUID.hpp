#pragma once

//----------------------------------------------------------------------------------------------------------------------
class ActorUID
{
public:
	ActorUID( unsigned int salt, unsigned int index );
	ActorUID();
	~ActorUID();

	unsigned int GetIndex() const;
	bool		 IsValid();

public:
	static const unsigned int INVALID = static_cast<unsigned int>( -1 );
	unsigned int m_saltAndIndexData	  = INVALID;
};