#pragma once

#include "Engine/Core/HashCaseInsensitiveString.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <map>
#include <string>


//----------------------------------------------------------------------------------------------------------------------
class NamedPropertyBase
{
public:
	void NamedProperties();
	virtual ~NamedPropertyBase();
};


//----------------------------------------------------------------------------------------------------------------------
template< typename T>
class NamedPropertyOfType : public NamedPropertyBase
{
	friend class NamedProperties;	// NamedProperties accesses protected data from NamedPropertyOfType

protected:
	NamedPropertyOfType( T const& value )
	{
		m_value = value;
	}

protected:
	T m_value;
};


//----------------------------------------------------------------------------------------------------------------------
class NamedProperties
{
public:
	template< typename T >
	void SetValue( std::string const& keyName, T const& value );
	
	template< typename T >
	T			GetValue( std::string const& keyName, T    const& defaultValue );
	void		SetValue( std::string const& keyName, char const* defaultValue );
	std::string GetValue( std::string const& keyName, char const* defaultValue );
	
	template< typename T >
	void		SetValue( char const* keyName, T const& value );
	
	template< typename T >
	T			GetValue( char const* keyName, T	const& defaultValue );
	void		SetValue( char const* keyName, char const* defaultValue );
	std::string GetValue( char const* keyName, char const* defaultValue );

private:
	std::map< HCIString, NamedPropertyBase* > m_keyValuePairs;

};

//----------------------------------------------------------------------------------------------------------------------
template<typename T>
inline void NamedProperties::SetValue( std::string const& keyName, T const& defaultValue )
{
	HCIString key		   = HCIString( keyName );
	m_keyValuePairs[ key ] = new NamedPropertyOfType<T>( defaultValue );
}

//----------------------------------------------------------------------------------------------------------------------
template<typename T>
inline T NamedProperties::GetValue( std::string const& keyName, T const& defaultValue )
{
	HCIString key = HCIString( keyName );
	std::map<HCIString, NamedPropertyBase*>::iterator found = m_keyValuePairs.find( key );
	if ( found == m_keyValuePairs.end() )
	{
		return defaultValue;
	}

	NamedPropertyBase* property = found->second;
	NamedPropertyOfType<T>* typedProperty = dynamic_cast< NamedPropertyOfType<T>* >(property);
	if ( typedProperty == nullptr )
	{
		ERROR_RECOVERABLE( "Asked for value of incorrect type!" );
		return defaultValue;
	}
	return typedProperty->m_value;
}

//----------------------------------------------------------------------------------------------------------------------
template<typename T>
inline void NamedProperties::SetValue( char const* keyName, T const& value )
{
	HCIString key			= HCIString( keyName );
	m_keyValuePairs[ key ]	= new NamedPropertyOfType< T >( value );
}

//----------------------------------------------------------------------------------------------------------------------
template<typename T>
inline T NamedProperties::GetValue( char const* keyName, T const& defaultValue )
{
	HCIString key = HCIString( keyName );
	std::map<HCIString, NamedPropertyBase*>::iterator found = m_keyValuePairs.find( key );
	if ( found == m_keyValuePairs.end() )
	{
		return defaultValue;
	}

	NamedPropertyBase* property = found->second;
	NamedPropertyOfType<T>* typedProperty = dynamic_cast<NamedPropertyOfType<T>*>(property);
	if ( typedProperty == nullptr )
	{
		ERROR_RECOVERABLE( "Asked for value of incorrect type!" );
		return defaultValue;
	}
	return typedProperty->m_value;
}