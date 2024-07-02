#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/HashCaseInsensitiveString.hpp"


//----------------------------------------------------------------------------------------------------------------------
HashCaseInsensitiveString::HashCaseInsensitiveString( std::string const& text )
	: m_originalText( text )
	, m_hash( CalculateHashForText( text ) )
{
	m_originalText = text;
}


//----------------------------------------------------------------------------------------------------------------------
HashCaseInsensitiveString::HashCaseInsensitiveString( char const* text )
	: m_originalText( text )
	, m_hash( CalculateHashForText( text ) )
{
}


//----------------------------------------------------------------------------------------------------------------------
unsigned int HashCaseInsensitiveString::GetHash() const
{
	return m_hash;
}


//----------------------------------------------------------------------------------------------------------------------
std::string const& HashCaseInsensitiveString::GetOriginalString() const
{
	return m_originalText;
}


//----------------------------------------------------------------------------------------------------------------------
char const* HashCaseInsensitiveString::c_str() const
{
	return m_originalText.c_str();
}


//----------------------------------------------------------------------------------------------------------------------
unsigned int HashCaseInsensitiveString::CalculateHashForText( char const* text )
{
	unsigned int hash = 0;
	for ( char const* readPos = text; *readPos != '\0'; readPos++ )
	{
		hash *= 31;
		hash += unsigned int( tolower( *readPos ) );	// Overflow might occur when adding to the string, which is good since we get unique hash values
	}
	return hash;
}


//----------------------------------------------------------------------------------------------------------------------
unsigned int HashCaseInsensitiveString::CalculateHashForText( std::string const& text )
{
	unsigned int newHash = CalculateHashForText( text.c_str() );
	return newHash;
}


//----------------------------------------------------------------------------------------------------------------------
bool HashCaseInsensitiveString::operator<( HashCaseInsensitiveString const& compareRHS ) const
{
	if ( m_hash < compareRHS.m_hash )
	{
		return true;
	}
	else if ( m_hash > compareRHS.m_hash )
	{
		return false;
	}
	else
	{
		// Hash values are equal
		unsigned int comparison = _stricmp( m_originalText.c_str(), compareRHS.m_originalText.c_str() );
		if ( comparison < 0 )
		{
			return true;
		}
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool HashCaseInsensitiveString::operator>( HashCaseInsensitiveString const& compareRHS ) const
{
	if ( m_hash > compareRHS.m_hash )
	{
		return true;
	}
	else if ( m_hash < compareRHS.m_hash )
	{
		return false;
	}
	else
	{
		// Hash values are equal
		unsigned int comparison = _stricmp( m_originalText.c_str(), compareRHS.m_originalText.c_str() );
		if ( comparison > 0 )
		{
			return true;
		}
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool HashCaseInsensitiveString::operator==( HashCaseInsensitiveString const& compareRHS ) const
{
	if ( m_hash != compareRHS.m_hash )
	{
		return false;
	}
	else
	{
		// Hash values are equal
		unsigned int comparison = _stricmp( m_originalText.c_str(), compareRHS.m_originalText.c_str() );
		if ( comparison == 0 )
		{
			// Strings are equal
			return true;
		}
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool HashCaseInsensitiveString::operator!=( HashCaseInsensitiveString const& compareRHS ) const
{
	return !( *this == compareRHS );
}


//----------------------------------------------------------------------------------------------------------------------
bool HashCaseInsensitiveString::operator==( std::string const& text ) const
{
	// Hash values are equal
	unsigned int comparison = _stricmp( m_originalText.c_str(), text.c_str() );
	if ( comparison == 0 )
	{
		// Strings are equal
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool HashCaseInsensitiveString::operator!=( std::string const& text ) const
{
	// Hash values are equal
	unsigned int comparison = _stricmp( m_originalText.c_str(), text.c_str() );
	if ( comparison == 0 )
	{
		// Strings are equal
		return false;
	}
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool HashCaseInsensitiveString::operator==( char const* text ) const
{
	// Hash values are equal
	unsigned int comparison = _stricmp( m_originalText.c_str(), text );
	if ( comparison == 0 )
	{
		// Strings are equal
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool HashCaseInsensitiveString::operator!=( char const* text ) const
{
	// Hash values are equal
	unsigned int comparison = _stricmp( m_originalText.c_str(), text );
	if ( comparison == 0 )
	{
		// Strings are equal
		return false;
	}
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
void HashCaseInsensitiveString::operator=( HashCaseInsensitiveString const& copyAssign )
{
	m_originalText	= copyAssign.m_originalText;
	m_hash			= copyAssign.m_hash;
}


//----------------------------------------------------------------------------------------------------------------------
void HashCaseInsensitiveString::operator=( std::string const& text )
{
	m_originalText	= text;
	m_hash			= CalculateHashForText( m_originalText );
}


//----------------------------------------------------------------------------------------------------------------------
void HashCaseInsensitiveString::operator=( char const* text )
{
	m_originalText = text;
	m_hash			= CalculateHashForText( m_originalText );
}