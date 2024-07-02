#pragma once

#include "Engine/Core/BufferWriter.hpp"

#include <string>
#include <stdint.h>


//----------------------------------------------------------------------------------------------------------------------
struct Vec2;
struct Vec3;
struct Vec4;
struct IntVec2;
struct IntVec3;
struct Rgba8;
struct OBB2D;
struct OBB3D;
struct AABB2;
struct AABB3;
class  Plane2D;
struct Vertex_PCU;


//----------------------------------------------------------------------------------------------------------------------
class BufferParser
{
public:
	BufferParser( unsigned char* bufferData, unsigned char bufferSizeInBytes, BufferEndian endianMode = LITTLE );
	~BufferParser();

	// Primitive data types
	unsigned char	ParseByte();
	bool			ParseBool();
	char			ParseChar();
	float			ParseFloat();
	double			ParseDouble();
	short			ParseShort();
	unsigned short	ParseUnsignedShort();
	int32_t			ParseInt32();
	int64_t			ParseInt64();
	uint8_t			ParseUnsignedInt8();
	uint32_t		ParseUnsignedInt32();
	uint64_t		ParseUnsignedInt64();
	// "Advanced" data types
	Vec2			ParseVec2();
	Vec3			ParseVec3();
	Vec4			ParseVec4();
	IntVec2			ParseIntVec2();
	IntVec3			ParseIntVec3();
	Rgba8			ParseRgba8();
	Rgba8			ParseRgb();
	OBB2D			ParseOBB2D();
	OBB3D			ParseOBB3D();
	AABB2			ParseAABB2();
	AABB3			ParseAABB3();
	Plane2D			ParsePlane2D();
	Vertex_PCU		ParseVertexPCU();
	std::string		ParseStringZeroTerminated();
	std::string		ParseStringAfter32BitLength();
	// Misc functions
	BufferEndian	GetEndianMode();
	void			SetEndianMode( BufferEndian endianMode );
	void			Reverse2BytesInPlace( unsigned char& dataOf2Bytes );
	void			Reverse4BytesInPlace( unsigned char& dataOf4Bytes );
	void			Reverse8BytesInPlace( unsigned char& dataOf8Bytes );
	void			JumpToPos( int newOffset );

public:
	unsigned char*	m_bufferStart				= nullptr;
	unsigned int	m_offsetToCurrentByte		= 0;
	unsigned int	m_sizeOfBuffer				= 0;
	BufferEndian	m_endian					= NATIVE;
	bool			m_isOppositeNativeEndian	= false;
};