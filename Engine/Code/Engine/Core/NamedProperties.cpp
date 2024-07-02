#include "Engine/Core/NamedProperties.hpp"


//----------------------------------------------------------------------------------------------------------------------
void NamedPropertyBase::NamedProperties()
{
}


//----------------------------------------------------------------------------------------------------------------------
NamedPropertyBase::~NamedPropertyBase()
{
}


//----------------------------------------------------------------------------------------------------------------------
void NamedProperties::SetValue( std::string const& keyName, char const* defaultValue )
{
	HCIString key		   = HCIString( keyName );
	m_keyValuePairs[ key ] = new NamedPropertyOfType<std::string>( defaultValue );
}


//----------------------------------------------------------------------------------------------------------------------
std::string NamedProperties::GetValue( std::string const& keyName, char const* defaultValue )
{
	HCIString key = HCIString( keyName );
	std::map<HCIString, NamedPropertyBase*>::iterator found = m_keyValuePairs.find( key );
	if ( found == m_keyValuePairs.end() )
	{
		return defaultValue;
	}

	NamedPropertyBase* property = found->second;
	NamedPropertyOfType<std::string>* typedProperty = dynamic_cast<NamedPropertyOfType<std::string>*>(property);
	if ( typedProperty == nullptr )
	{
		ERROR_RECOVERABLE( "Asked for value of incorrect type!" );
		return defaultValue;
	}
	return typedProperty->m_value;
}


//----------------------------------------------------------------------------------------------------------------------
void NamedProperties::SetValue( char const* keyName, char const* defaultValue )
{
	HCIString key		   = HCIString( keyName );
	m_keyValuePairs[ key ] = new NamedPropertyOfType<std::string>( defaultValue );
}


//----------------------------------------------------------------------------------------------------------------------
std::string NamedProperties::GetValue( char const* keyName, char const* defaultValue )
{
	HCIString key = HCIString( keyName );
	std::map<HCIString, NamedPropertyBase*>::iterator found = m_keyValuePairs.find( key );
	if ( found == m_keyValuePairs.end() )
	{
		return defaultValue;
	}

	NamedPropertyBase* property = found->second;
	NamedPropertyOfType<std::string>* typedProperty = dynamic_cast< NamedPropertyOfType<std::string>* >(property);
	if ( typedProperty == nullptr )
	{
		ERROR_RECOVERABLE( "Asked for value of incorrect type!" );
		return defaultValue;
	}
	return typedProperty->m_value;
}