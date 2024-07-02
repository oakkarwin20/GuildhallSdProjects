#include "Engine/Core/BufferParser.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/OBB2D.hpp"
#include "Engine/Math/OBB3D.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/BufferWriter.hpp"
#include "Engine/Math/MathUtils.hpp"


//----------------------------------------------------------------------------------------------------------------------
BufferParser::BufferParser( unsigned char* const bufferData, unsigned char bufferSizeInBytes, BufferEndian endianMode )
{
	m_bufferStart  = bufferData;
	m_sizeOfBuffer = bufferSizeInBytes;
	SetEndianMode( endianMode );
}


//----------------------------------------------------------------------------------------------------------------------
BufferParser::~BufferParser()
{
	m_bufferStart = nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
unsigned char BufferParser::ParseByte()
{
	// Increment offsetToCurrentByte after parsing (by parsed data size, byte in this case) to keep track of the "index" in the bufferList
	unsigned char parsedByte = m_bufferStart[ m_offsetToCurrentByte ];
	m_offsetToCurrentByte++;
	return parsedByte;
}


//----------------------------------------------------------------------------------------------------------------------
bool BufferParser::ParseBool()
{
	unsigned char* dataStart = &m_bufferStart[ m_offsetToCurrentByte ];
	bool* parsedBool	= reinterpret_cast<bool*>(dataStart);
	m_offsetToCurrentByte++;
	return *parsedBool;
}


//----------------------------------------------------------------------------------------------------------------------
char BufferParser::ParseChar()
{
	unsigned char* pointerToCharStart = m_bufferStart + m_offsetToCurrentByte;
	char* parsedChar				  = reinterpret_cast<char*>( pointerToCharStart );
	m_offsetToCurrentByte			 += sizeof( unsigned char );
	return *parsedChar;
}


//----------------------------------------------------------------------------------------------------------------------
float BufferParser::ParseFloat()
{
	unsigned char* pointerToFloatStart	 = m_bufferStart + m_offsetToCurrentByte;
	float* parsedFloat					 = reinterpret_cast<float* const>( pointerToFloatStart );
	m_offsetToCurrentByte				+= sizeof( float );
	if ( m_isOppositeNativeEndian )
	{
		Reverse4BytesInPlace( *pointerToFloatStart );
	}
	return *parsedFloat;
}


//----------------------------------------------------------------------------------------------------------------------
double BufferParser::ParseDouble()
{
	unsigned char* pointerToDoubleStart = m_bufferStart + m_offsetToCurrentByte;
	double*		   parsedDouble			= reinterpret_cast<double*>( pointerToDoubleStart );
	m_offsetToCurrentByte				+= sizeof( double );
	if ( m_isOppositeNativeEndian )
	{
		Reverse8BytesInPlace( *pointerToDoubleStart );
	}
	return *parsedDouble;
}


//----------------------------------------------------------------------------------------------------------------------
short BufferParser::ParseShort()
{
	unsigned char*  pointerToShortStart  = m_bufferStart + m_offsetToCurrentByte;
	short*			parsedShort			 = reinterpret_cast<short*>( pointerToShortStart );
	m_offsetToCurrentByte				+= sizeof( short );
	if ( m_isOppositeNativeEndian )
	{
		Reverse2BytesInPlace( *pointerToShortStart );
	}
	return *parsedShort;
}


//----------------------------------------------------------------------------------------------------------------------
unsigned short BufferParser::ParseUnsignedShort()
{
	unsigned char* pointerToUsStart  = m_bufferStart + m_offsetToCurrentByte;
	unsigned short* parsedUS		 = reinterpret_cast<unsigned short*>( pointerToUsStart );
	m_offsetToCurrentByte			+= sizeof( unsigned short );
	if ( m_isOppositeNativeEndian )
	{
		Reverse2BytesInPlace( *pointerToUsStart );
	}
	return *parsedUS;
}


//----------------------------------------------------------------------------------------------------------------------
int32_t BufferParser::ParseInt32()
{
	unsigned char*	pointerToIntStart	= m_bufferStart + m_offsetToCurrentByte;
	int32_t*		parsedInt			= reinterpret_cast<int32_t*>( pointerToIntStart );
	m_offsetToCurrentByte				+= sizeof( int32_t );
	if ( m_isOppositeNativeEndian )
	{
		Reverse4BytesInPlace( *pointerToIntStart );
	}
	return *parsedInt;
}


//----------------------------------------------------------------------------------------------------------------------
int64_t BufferParser::ParseInt64()
{
	unsigned char*	pointerToInt64Start	= m_bufferStart + m_offsetToCurrentByte;
	int64_t*		parsedInt64			= reinterpret_cast<int64_t*>( pointerToInt64Start );
	m_offsetToCurrentByte				+= sizeof( int64_t );
	if ( m_isOppositeNativeEndian )
	{
		Reverse8BytesInPlace( *pointerToInt64Start );
	}
	return *parsedInt64;
}


//----------------------------------------------------------------------------------------------------------------------
uint8_t BufferParser::ParseUnsignedInt8()
{
	unsigned char* pointerToUiStart  = m_bufferStart + m_offsetToCurrentByte;
	uint8_t*	   parsedUI			 = reinterpret_cast<uint8_t*>( pointerToUiStart );
	m_offsetToCurrentByte			+= sizeof( uint8_t );
	return *parsedUI;
}


//----------------------------------------------------------------------------------------------------------------------
uint32_t BufferParser::ParseUnsignedInt32()
{
	unsigned char* pointerToUiStart  = m_bufferStart + m_offsetToCurrentByte;
	uint32_t*	   parsedUI			 = reinterpret_cast<uint32_t*>( pointerToUiStart );
	m_offsetToCurrentByte			+= sizeof( uint32_t );
	if ( m_isOppositeNativeEndian )
	{
		Reverse4BytesInPlace( *pointerToUiStart );
	}
	return *parsedUI;
}


//----------------------------------------------------------------------------------------------------------------------
uint64_t BufferParser::ParseUnsignedInt64()
{
	unsigned char*	pointerToUi64Start	 = m_bufferStart + m_offsetToCurrentByte;
	uint64_t*		parsedUI64			 = reinterpret_cast<uint64_t*>( pointerToUi64Start );
	m_offsetToCurrentByte				+= sizeof( uint64_t );
	if ( m_isOppositeNativeEndian )
	{
		Reverse8BytesInPlace( *pointerToUi64Start );
	}
	return *parsedUI64;
}


//----------------------------------------------------------------------------------------------------------------------
Vec2 BufferParser::ParseVec2()
{
	float x		= ParseFloat();
	float y		= ParseFloat();
	Vec2 vec2	= Vec2( x, y );
	return vec2;
}


//----------------------------------------------------------------------------------------------------------------------
Vec3 BufferParser::ParseVec3()
{
	float x		= ParseFloat();
	float y		= ParseFloat();
	float z		= ParseFloat();
	Vec3 vec3	= Vec3( x, y, z );
	return vec3;
}


//----------------------------------------------------------------------------------------------------------------------
Vec4 BufferParser::ParseVec4()
{
	float x		= ParseFloat();
	float y		= ParseFloat();
	float z		= ParseFloat();
	float w		= ParseFloat();
	Vec4 vec4	= Vec4( x, y, z, w );
	return vec4;
}


//----------------------------------------------------------------------------------------------------------------------
IntVec2 BufferParser::ParseIntVec2()
{
	int x			= ParseInt32();
	int y			= ParseInt32();
	IntVec2 intVec2	= IntVec2( x, y );
	return intVec2;
}


//----------------------------------------------------------------------------------------------------------------------
IntVec3 BufferParser::ParseIntVec3()
{
	int x			= ParseInt32();
	int y			= ParseInt32();
	int z			= ParseInt32();
	IntVec3 intVec3	= IntVec3( x, y, z );
	return intVec3;
}


//----------------------------------------------------------------------------------------------------------------------
Rgba8 BufferParser::ParseRgba8()
{
	unsigned char r	= ParseByte();
	unsigned char g	= ParseByte();
	unsigned char b	= ParseByte();
	unsigned char a	= ParseByte();
	Rgba8 color		= Rgba8( r, g, b, a );
	return color;
}


//----------------------------------------------------------------------------------------------------------------------
Rgba8 BufferParser::ParseRgb()
{
	unsigned char r	= ParseByte();
	unsigned char g	= ParseByte();
	unsigned char b	= ParseByte();
	unsigned char a	= ParseByte();
	Rgba8 color		= Rgba8( r, g, b, a );
	return color;
}


//----------------------------------------------------------------------------------------------------------------------
OBB2D BufferParser::ParseOBB2D()
{
	Vec2 center  = ParseVec2();
	Vec2 iBasis  = ParseVec2();	
	Vec2 halfDim = ParseVec2();
	OBB2D  obb2	 = OBB2D( center, iBasis, halfDim );
	return obb2;
}


//----------------------------------------------------------------------------------------------------------------------
OBB3D BufferParser::ParseOBB3D()
{
	Vec3 center			= ParseVec3();
	Vec3 iBasis			= ParseVec3();	
	Vec3 jBasisNormal	= ParseVec3();
	Vec3 kBasisNormal	= ParseVec3();
	Vec3 halfDim		= ParseVec3();
	OBB3D  obb3			= OBB3D( center, iBasis, jBasisNormal, kBasisNormal, halfDim );
	return obb3;
}


//----------------------------------------------------------------------------------------------------------------------
AABB2 BufferParser::ParseAABB2()
{
	Vec2 mins = ParseVec2();
	Vec2 maxs = ParseVec2();
	AABB2 box = AABB2( mins, maxs );
	return box;
}


//--------------------------------------------------------------------f--------------------------------------------------
AABB3 BufferParser::ParseAABB3()
{
	Vec3 mins = ParseVec3();
	Vec3 maxs = ParseVec3();
	AABB3 box = AABB3( mins, maxs );
	return box;
}


//----------------------------------------------------------------------------------------------------------------------
Plane2D BufferParser::ParsePlane2D()
{
	Vec2  normal		 = ParseVec2();
	float distFromOrigin = ParseFloat();
	Plane2D plane2D		 = Plane2D( normal, distFromOrigin );
	return plane2D;
}


//----------------------------------------------------------------------------------------------------------------------
Vertex_PCU BufferParser::ParseVertexPCU()
{
	Vec3  pos				= ParseVec3();
	Rgba8 color				= ParseRgba8();
	Vec2  uv				= ParseVec2();
	Vertex_PCU vertexPCU	= Vertex_PCU( pos, color, uv );
	return vertexPCU;
}


//----------------------------------------------------------------------------------------------------------------------
std::string BufferParser::ParseStringZeroTerminated()
{
	std::string parsedString;
	unsigned char currentChar = ParseChar();
	unsigned char  nullChar	  = '\0';
	while ( currentChar != nullChar )
	{
		parsedString.push_back( currentChar );
		currentChar = ParseChar();
	}
	return parsedString;
}


//----------------------------------------------------------------------------------------------------------------------
std::string BufferParser::ParseStringAfter32BitLength()
{
	std::string parsedString;
	int			  sizeOfString  = ParseInt32();
	unsigned char currentChar;
	for ( int i = 0; i < sizeOfString; i++ )
	{
		currentChar = ParseChar();
		parsedString.push_back( currentChar );
	}
	return parsedString;
}


//----------------------------------------------------------------------------------------------------------------------
BufferEndian BufferParser::GetEndianMode()
{
	unsigned int   endianTest = 0x12345678;
	unsigned char* firstByte = reinterpret_cast<unsigned char*>( &endianTest );
	if ( firstByte[ 0 ] == 0x78 )
	{
		return LITTLE;
	}
	else if ( firstByte[ 0 ] == 0x12 )
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
void BufferParser::SetEndianMode( BufferEndian endianMode )
{
	BufferEndian curEndian = GetEndianMode();
	m_endian = curEndian;
	if ( m_endian != endianMode )
	{
		m_isOppositeNativeEndian = true;
	}
	m_endian = endianMode;
}


//----------------------------------------------------------------------------------------------------------------------
void BufferParser::Reverse2BytesInPlace( unsigned char& dataOf2Bytes )
{
	unsigned char* dataStart = &dataOf2Bytes;
	SwapValueOfTwoVariables( dataStart[ 0 ], dataStart[ 1 ] );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferParser::Reverse4BytesInPlace( unsigned char& dataOf4Bytes )
{
	unsigned char* dataStart = &dataOf4Bytes;
	SwapValueOfTwoVariables( dataStart[ 0 ], dataStart[ 3 ] );
	SwapValueOfTwoVariables( dataStart[ 1 ], dataStart[ 2 ] );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferParser::Reverse8BytesInPlace( unsigned char& dataOf8Bytes )
{
	unsigned char* dataStart = &dataOf8Bytes;
	SwapValueOfTwoVariables( dataStart[ 0 ], dataStart[ 7 ] );
	SwapValueOfTwoVariables( dataStart[ 1 ], dataStart[ 6 ] );
	SwapValueOfTwoVariables( dataStart[ 2 ], dataStart[ 5 ] );
	SwapValueOfTwoVariables( dataStart[ 3 ], dataStart[ 4 ] );
}


//----------------------------------------------------------------------------------------------------------------------
void BufferParser::JumpToPos( int newOffset )
{
	m_offsetToCurrentByte = newOffset;
}
