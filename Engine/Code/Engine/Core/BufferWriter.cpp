#include "Engine/Core/BufferWriter.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2D.hpp"
#include "Engine/Math/OBB3D.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"


//----------------------------------------------------------------------------------------------------------------------
BufferWriter::BufferWriter( std::vector<unsigned char>& buffer, BufferEndian endianMode )
	: m_buffer ( buffer )
{
	SetEndianMode( endianMode );
}


//----------------------------------------------------------------------------------------------------------------------
BufferWriter::~BufferWriter()
{
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendByte( unsigned char byte )
{
	m_buffer.push_back( byte );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendBool( bool boolean )
{
	m_buffer.push_back( boolean );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendChar( char c )
{
	m_buffer.push_back( c );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendFloat( float f )
{
	float* addressOfFloat_F = &f;
	unsigned char* addressOfFloatAsByteArray = reinterpret_cast<unsigned char*>( addressOfFloat_F );
	AppendByte( addressOfFloatAsByteArray[0] );		// We are saving the value at the addressOfFloatAsByteArray + some offset
	AppendByte( addressOfFloatAsByteArray[1] );
	AppendByte( addressOfFloatAsByteArray[2] );
	AppendByte( addressOfFloatAsByteArray[3] );
	if ( m_isOppositeNativeEndian )
	{
		Reverse4BytesInPlace( *addressOfFloatAsByteArray );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendDouble( double d )
{
	// Guarantee that double is 8 bytes in size?

	double* addressOfDouble_D = &d;
	unsigned char* addressOfDoubleAsByteArray = reinterpret_cast<unsigned char*>( addressOfDouble_D );
	AppendByte( addressOfDoubleAsByteArray[0] );
	AppendByte( addressOfDoubleAsByteArray[1] );
	AppendByte( addressOfDoubleAsByteArray[2] );
	AppendByte( addressOfDoubleAsByteArray[3] );
	AppendByte( addressOfDoubleAsByteArray[4] );
	AppendByte( addressOfDoubleAsByteArray[5] );
	AppendByte( addressOfDoubleAsByteArray[6] );
	AppendByte( addressOfDoubleAsByteArray[7] );
	if ( m_isOppositeNativeEndian )
	{
		Reverse8BytesInPlace( *addressOfDoubleAsByteArray );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendShort( short s )
{
	short* addressOfShort_S						= &s;
	unsigned char* addressOfShortAsByteArray	= reinterpret_cast<unsigned char*>( addressOfShort_S );
	m_buffer.push_back( addressOfShortAsByteArray[0] );
	m_buffer.push_back( addressOfShortAsByteArray[1] );
	if ( m_isOppositeNativeEndian )
	{
		Reverse2BytesInPlace( *addressOfShortAsByteArray );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendUnsignedShort( unsigned short us )
{
	unsigned short* addressOfUnsignedShort_US			= &us;
	unsigned char*  addressOfUnsignedShortAsByteArray	= reinterpret_cast<unsigned char*>( addressOfUnsignedShort_US );
	m_buffer.push_back( addressOfUnsignedShortAsByteArray[ 0 ] );
	m_buffer.push_back( addressOfUnsignedShortAsByteArray[ 1 ] );
	if ( m_isOppositeNativeEndian )
	{
		Reverse2BytesInPlace( *addressOfUnsignedShortAsByteArray );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendInt16( int16_t i )
{
	// How do I know if an int is 2 or 4 bytes?
	int16_t* addressOfInt_I = &i;
	unsigned char* addressOfIntAsByteArray = reinterpret_cast<unsigned char*>( addressOfInt_I );
	m_buffer.push_back( addressOfIntAsByteArray[ 0 ] );
	m_buffer.push_back( addressOfIntAsByteArray[ 1 ] );
	m_buffer.push_back( addressOfIntAsByteArray[ 2 ] );
	m_buffer.push_back( addressOfIntAsByteArray[ 3 ] );
	if ( m_isOppositeNativeEndian )
	{
		Reverse4BytesInPlace( *addressOfIntAsByteArray );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendInt32( int64_t i )
{
	// How do I know if an int is 2 or 4 bytes?
	int64_t*			addressOfInt_I			= &i;
	unsigned char*		addressOfIntAsByteArray	= reinterpret_cast<unsigned char*>( addressOfInt_I );
	m_buffer.push_back( addressOfIntAsByteArray[ 0 ] );
	m_buffer.push_back( addressOfIntAsByteArray[ 1 ] );
	m_buffer.push_back( addressOfIntAsByteArray[ 2 ] );
	m_buffer.push_back( addressOfIntAsByteArray[ 3 ] );
	if ( m_isOppositeNativeEndian )
	{
		Reverse4BytesInPlace( *addressOfIntAsByteArray );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendInt64( int64_t int64 )
{
	int64_t*			addressOfInt64_UI				= &int64 ;
	unsigned char*		addressOfUnsignedIntAsByteArray	= reinterpret_cast<unsigned char*>( addressOfInt64_UI );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 0 ] );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 1 ] );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 2 ] );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 3 ] );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 4 ] );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 5 ] );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 6 ] );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 7 ] );
	if ( m_isOppositeNativeEndian )
	{
		Reverse8BytesInPlace( *addressOfUnsignedIntAsByteArray );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendUnsignedInt16( uint16_t ui16 )
{
	// How do I know if an int is 2 or 4 bytes?
	uint16_t* addressOfUnsignedInt_UI = &ui16;
	unsigned char* addressOfUnsignedIntAsByteArray = reinterpret_cast<unsigned char*>( addressOfUnsignedInt_UI );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 0 ] );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 1 ] );
	if ( m_isOppositeNativeEndian )
	{
		Reverse2BytesInPlace( *addressOfUnsignedIntAsByteArray );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendUnsignedInt32( uint32_t ui32 )
{
	// How do I know if an int is 2 or 4 bytes?
	uint32_t*			addressOfUnsignedInt_UI			= &ui32;
	unsigned char*		addressOfUnsignedIntAsByteArray	= reinterpret_cast<unsigned char*>( addressOfUnsignedInt_UI );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 0 ] );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 1 ] );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 2 ] );
	m_buffer.push_back( addressOfUnsignedIntAsByteArray[ 3 ] );
	if ( m_isOppositeNativeEndian )
	{
		Reverse4BytesInPlace( *addressOfUnsignedIntAsByteArray );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendUnsignedInt64( uint64_t ui64 )
{
	uint64_t*			addressOfUnsignedInt64_UI			= &ui64;
	unsigned char*		addressOfUnsignedInt64AsByteArray	= reinterpret_cast<unsigned char*>( addressOfUnsignedInt64_UI );
	m_buffer.push_back( addressOfUnsignedInt64AsByteArray[ 0 ] );
	m_buffer.push_back( addressOfUnsignedInt64AsByteArray[ 1 ] );
	m_buffer.push_back( addressOfUnsignedInt64AsByteArray[ 2 ] );
	m_buffer.push_back( addressOfUnsignedInt64AsByteArray[ 3 ] );
	m_buffer.push_back( addressOfUnsignedInt64AsByteArray[ 4 ] );
	m_buffer.push_back( addressOfUnsignedInt64AsByteArray[ 5 ] );
	m_buffer.push_back( addressOfUnsignedInt64AsByteArray[ 6 ] );
	m_buffer.push_back( addressOfUnsignedInt64AsByteArray[ 7 ] );
	if ( m_isOppositeNativeEndian )
	{
		Reverse8BytesInPlace( *addressOfUnsignedInt64AsByteArray );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendVec2( Vec2 const& vec2 )
{
	AppendFloat( vec2.x );
	AppendFloat( vec2.y );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendVec3( Vec3 const& vec3 )
{
	AppendFloat( vec3.x );
	AppendFloat( vec3.y );
	AppendFloat( vec3.z );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendVec4( Vec4 const& vec4 )
{
	AppendFloat( vec4.x );
	AppendFloat( vec4.y );
	AppendFloat( vec4.z );
	AppendFloat( vec4.w );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendIntVec2( IntVec2 const& intVec2 )
{
	AppendInt32( intVec2.x );
	AppendInt32( intVec2.y );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendIntVec3( IntVec3 const& intVec3 )
{
	AppendInt32( intVec3.x );
	AppendInt32( intVec3.y );
	AppendInt32( intVec3.z );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendRgba8( Rgba8 const& color )
{
	AppendByte( color.r );
	AppendByte( color.g );
	AppendByte( color.b );
	AppendByte( color.a );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendRgb( Rgba8 const& color )
{
	AppendByte( color.r );
	AppendByte( color.g );
	AppendByte( color.b );
	AppendByte( 255 );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendObb2D( OBB2D const& obb2 )
{
	AppendVec2( obb2.m_center		  );
	AppendVec2( obb2.m_iBasisNormal	  );
	AppendVec2( obb2.m_halfDimensions );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendObb3D( OBB3D const& obb3 )
{
	AppendVec3( obb3.m_center		  );
	AppendVec3( obb3.m_iBasisNormal	  );
	AppendVec3( obb3.m_jBasisNormal	  );
	AppendVec3( obb3.m_kBasisNormal	  );
	AppendVec3( obb3.m_halfDimensions );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendAABB2( AABB2 const& aabb2 )
{
	AppendVec2( aabb2.m_mins );
	AppendVec2( aabb2.m_maxs );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendAABB3( AABB3 const& aabb3 )
{
	AppendVec3( aabb3.m_mins );
	AppendVec3( aabb3.m_maxs );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendPlane2D( Plane2D const& plane2D )
{
	AppendVec2 ( plane2D.m_normal			);
	AppendFloat( plane2D.m_distFromOrigin	);
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendVertexPCU( Vertex_PCU const& vertex_PCU )
{
	AppendVec3 ( vertex_PCU.m_position		);
	AppendRgba8( vertex_PCU.m_color			);
	AppendVec2 ( vertex_PCU.m_uvTexCoords	);
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendStringZeroTerminated( std::string const& string )
{
	for ( int i = 0; i < string.length(); i++ )
	{
		AppendChar( string[i] );
	}
	AppendChar( '\0' ); 
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::AppendStringAfter32BitLength( std::string const& string )
{
	unsigned int sizeOfString = unsigned int( string.length() );
	AppendUnsignedInt32( sizeOfString  );
	for ( unsigned int i = 0; i < sizeOfString; i++ )
	{
		AppendChar( string[i] );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::OverwriteUint32( size_t writePosOffset, unsigned int u32 )
{
	unsigned char* posToWriteData	= &m_buffer[writePosOffset];
	unsigned char* u32AsByteArray	= reinterpret_cast<unsigned char*>( &u32 );
	posToWriteData[0]				= u32AsByteArray[0];
	posToWriteData[1]				= u32AsByteArray[1];
	posToWriteData[2]				= u32AsByteArray[2];
	posToWriteData[3]				= u32AsByteArray[3];
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::SetEndianMode( BufferEndian endianMode )
{
	BufferEndian curEndian = GetNativeEndianMode();
	m_endian = curEndian;
	if ( m_endian != endianMode )
	{
		m_isOppositeNativeEndian = true;
	}
	m_endian = endianMode;
}


//----------------------------------------------------------------------------------------------------------------------
BufferEndian BufferWriter::GetNativeEndianMode()
{
	unsigned int   endianTest = 0x12345678;
	unsigned char* firstByte  = reinterpret_cast<unsigned char*>( &endianTest );
	if ( firstByte[0] == 0x78 )
	{
		return LITTLE;
	}
	else if ( firstByte[0] == 0x12 )
	{
		return BIG;
	}
	else
	{
		// Should return error instead?
		return NATIVE;
	}
}


//----------------------------------------------------------------------------------------------------------------------
BufferEndian BufferWriter::GetEndianMode()
{
	return m_endian;
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::Reverse2BytesInPlace( unsigned char& dataOf2Bytes )
{
	unsigned char* dataStart = &dataOf2Bytes;
	SwapValueOfTwoVariables( dataStart[0], dataStart[1] );
}

//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::Reverse4BytesInPlace( unsigned char& dataOf4Bytes )
{
	unsigned char* dataStart = &dataOf4Bytes;
	SwapValueOfTwoVariables( dataStart[0], dataStart[3] );
	SwapValueOfTwoVariables( dataStart[1], dataStart[2] );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferWriter::Reverse8BytesInPlace( unsigned char& dataOf8Bytes )
{
	unsigned char* dataStart = &dataOf8Bytes;
	SwapValueOfTwoVariables( dataStart[ 0 ], dataStart[ 7 ] );
	SwapValueOfTwoVariables( dataStart[ 1 ], dataStart[ 6 ] );
	SwapValueOfTwoVariables( dataStart[ 2 ], dataStart[ 5 ] );
	SwapValueOfTwoVariables( dataStart[ 3 ], dataStart[ 4 ] );
}


//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// inline void Reverse4BytesInPlace( void* ptrTo32Bit )
// {
// 	unsigned int u = *(unsigned int);
// }
// 
// 
// // GetNativeEndian
// unsigned int endianTest = 0x12345678;
// unsigned char firstByte = *reinterpret_cast<unsigned char>( endianTest );
// 
// 
// void OverwriteUint32( size_t writePosOffset, unsigned int u32 )
// {
// 	unsigned char* bufferStart = m_buffer.start;
// 	unsigned int writePosition = (unsigned int*)( buffer + writePosOffset );
// 	if ( oppositeEndian )
// 	{
// 		Reverse4BytesInPlace( &u32 );
// 	}
// 	*writePosition = u32;
// }
// 
// ParseChunk_ConvexPoly
// ParseChunk_Footer
// 
// parse sceneInfoChunk first
// 
// ParseAndValidate4CC( BufferParser& bufferParser, char characterToValidate )
// {
// 		
// }