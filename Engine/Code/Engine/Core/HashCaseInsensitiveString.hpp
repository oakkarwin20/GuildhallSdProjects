#pragma once

#include "Engine/Core/XmlUtils.hpp"

#include <map>
#include <string>


//----------------------------------------------------------------------------------------------------------------------
class HashCaseInsensitiveString
{
public:
	HashCaseInsensitiveString() = default;
	HashCaseInsensitiveString( HashCaseInsensitiveString const& copyFrom ) = default;
	HashCaseInsensitiveString( std::string const& text );
	HashCaseInsensitiveString( char const* text );


public:
	// Accessors
	unsigned int		GetHash() const;
	std::string const&	GetOriginalString() const;
	char const*			c_str() const;

	static unsigned int CalculateHashForText( char const* text );
	static unsigned int CalculateHashForText( std::string const& text );

	// Operators
	bool operator<( HashCaseInsensitiveString const& compareRHS )	const;		// RHS = "right hand side" of the argument  
	bool operator>( HashCaseInsensitiveString const& compareRHS )	const;		
	bool operator==( HashCaseInsensitiveString const& compareRHS )	const;	
	bool operator!=( HashCaseInsensitiveString const& compareRHS )	const;	 
	bool operator==( std::string const& text )						const;
	bool operator!=( std::string const& text )						const;
	bool operator==( char const* text )								const;
	bool operator!=( char const* text )								const;
	void operator=( HashCaseInsensitiveString const& copyAssign );
	void operator=( std::string const& text );
	void operator=( char const* text );


private:
	std::string  m_originalText;		// NOT default initialized on purpose to avoid allocating memory for the default string, re-allocating memory after might be slow
	unsigned int m_hash = 0;
};
typedef HashCaseInsensitiveString HCIString;