#pragma once

#include "Engine/Core/XmlUtils.hpp"

#include <map>
#include <string>

//----------------------------------------------------------------------------------------------------------------------
struct Rgba8;
struct Vec2;
struct Vec3;
struct IntVec2;

//----------------------------------------------------------------------------------------------------------------------
class NamedStrings
{ 
public:
	NamedStrings()  = default;
	~NamedStrings() = default;

	void			PopulateFromXmlElementAttributes( XmlElement const& element );
	void			SetValue( std::string const& keyName, std::string	const&	newValue );
	std::string		GetValue( std::string const& keyName, std::string	const&	defaultValue ) const;
	bool			GetValue( std::string const& keyName, bool					defaultValue ) const;
	int				GetValue( std::string const& keyName, int					defaultValue ) const;
	float 			GetValue( std::string const& keyName, float					defaultValue ) const;
	std::string		GetValue( std::string const& keyName, char			const*	defaultValue ) const;
	Rgba8			GetValue( std::string const& keyName, Rgba8			const&	defaultValue ) const;
	Vec2			GetValue( std::string const& keyName, Vec2			const&	defaultValue ) const;
	Vec3			GetValue( std::string const& keyName, Vec3			const&	defaultValue ) const;
	IntVec2			GetValue( std::string const& keyName, IntVec2		const&	defaultValue ) const;

private:
	std::map<std::string, std::string>		m_keyValuePairs;
};

//////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------------------------------------
// Templates
//----------------------------------------------------------------------------------------------------------------------
// Templates are NOT code, they are NOT compiled, they are recipes for generating code


//----------------------------------------------------------------------------------------------------------------------
// Build process
//----------------------------------------------------------------------------------------------------------------------
// - Preprocessor does text operations
//		1. #defines do a (word-processing) text substitution
//		2. #includes copy-paste entire documents (usually .HPPs) into other documents (usually CPPs)
// - Template expansion (aka. Template instantiation) step-- template recipes generate C++ code
// 3. Compile each .CPP separately into an .OBJ file
// 4. Link all .OBJs together into a single .EXE


// The compiler throws an error from the code generated from Templates, the compiler does throw errors for the Template
// 
// template< typename T >
// class DataValueWrapper	// Recipe name
// {
// 	T m_value;
// };
// 
// DataValueWrapper<int> tList;


//////////////////////////////////////////////////////////////////////////
// //----------------------------------------------------------------------------------------------------------------------
// class NamedPropertyBase
// {
// 	friend class NamedProperties;
// 
// protected:
// 	virtual ~NamedPropertyBase();
// };
// 
// 
// //----------------------------------------------------------------------------------------------------------------------
// template< typename T>
// class NamedPropertyOfType : public NamedPropertyBase
// {
// protected:
// 	NamedPropertyOfType( T const& value )
// 	{
// 		m_value = value;
// 	}
// 
// protected:
// 	T m_value;
// };
// 
// 
// //----------------------------------------------------------------------------------------------------------------------
// class NamedProperties
// {
// public:
// 	template< typename T>
// 	void SetValue( std::string const& keyName, T const& value );
// 
// 	template< typename T>
// 	T GetValue( std::string const& keyName, T const& defaultValue );
// 
// 
// public:
// 	std::map< std::string, NamedPropertyBase* > m_keyValuePairs;
// };
// 
// 
// //----------------------------------------------------------------------------------------------------------------------
// template< typename T>
// void NamedProperties::SetValue( std::string const& keyName, T const& value )
// {
// 	m_keyValuePairs[ keyName ] = new NamedPropertyOfType< T >( value ); 
// }
// 
// //----------------------------------------------------------------------------------------------------------------------
// template<typename T>
// T NamedProperties::GetValue( std::string const& keyName, T const& defaultValue )
// {
// 	std::map< std::string, NamedPropertyBase* >::iterator found = m_keyValuePairs.find( keyName );
// 	if ( found == m_keyValuePairs.end() )
// 	{
// 		// Key was not found! Return default value instead
// 		return defaultValue;
// 	}
// 
// 	// If found, verify that data type is correct!
// 	NamedPropertyBase* property = found->second;
// 	NamedPropertyOfType< T >* typedProperty = dynamic_cast< NamedPropertyOfType<T>* >( property );
// 	if ( !typedProperty )
// 	{
// 		// Key was found but value was not the correct data type
// 		ERROR_RECOVERABLE( Stringf( "NamedProperties::GetValue asked for a value of the wrong type" ) );
// 		return defaultValue;
// 	}
// 
// 	// If we got this far, then we have found a value of the correct data type
// 	return found->second;
// }


 
// //----------------------------------------------------------------------------------------------------------------------
// // Function ptrs
// //----------------------------------------------------------------------------------------------------------------------
// typedef float (*BasicMathFuncPtr)( float, float );
// 
// 
// float AddFloats( float a, float b );
// 
// float (*funcPtr1)( float a, float f ) = nullptr;
// BasicMathFuncPtr funcPtr2 = nullptr; 
// 
// funcPtr1 = AddFloats;
// funcPtr2 = AddFloats;