#pragma once

#include <string>
#include <stdint.h>
#include <vector>


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
enum BufferEndian
{
	NATIVE,
	LITTLE,
	BIG
};


//----------------------------------------------------------------------------------------------------------------------
class BufferWriter
{
public:
	BufferWriter( std::vector<unsigned char>& buffer, BufferEndian endianMode = LITTLE );
	~BufferWriter();

	// Primitive data types
	void AppendByte			( unsigned char byte	);		
	void AppendBool			( bool boolean			);		
	void AppendChar			( char c				);		
	void AppendFloat		( float f				);
	void AppendDouble		( double d				);
	void AppendShort		( short s				);
	void AppendUnsignedShort( unsigned short us		);
	void AppendInt16		(  int16_t i	);
	void AppendInt32		(  int64_t i	);
	void AppendInt64		(  int64_t i64  );
	void AppendUnsignedInt16( uint16_t ui16	);
	void AppendUnsignedInt32( uint32_t ui32	);
	void AppendUnsignedInt64( uint64_t ui64 );
	// "Advanced" data types
	void AppendVec2		 (		 Vec2 const& vec2		  );
	void AppendVec3		 (		 Vec3 const& vec3		  );
	void AppendVec4		 (		 Vec4 const& vec4		  );
	void AppendIntVec2	 (	  IntVec2 const& intVec2	  );
	void AppendIntVec3	 (	  IntVec3 const& intVec3	  );
	void AppendRgba8	 (		Rgba8 const& color		  );
	void AppendRgb		 (		Rgba8 const& color		  );
	void AppendObb2D	 (		OBB2D const& obb2		  );
	void AppendObb3D	 (		OBB3D const& obb3		  );
	void AppendAABB2	 (		AABB2 const& aabb2		  );
	void AppendAABB3	 (		AABB3 const& aabb3		  );
	void AppendPlane2D	 (	  Plane2D const& plane2D	  );
	void AppendVertexPCU ( Vertex_PCU const& vertex_PCU	  );
	void AppendStringZeroTerminated(   std::string const& string );
	void AppendStringAfter32BitLength( std::string const& string );
	// Misc functions
	void		 OverwriteUint32( size_t writePosOffset, unsigned int u32 );
	void		 SetEndianMode( BufferEndian endianMode );
	BufferEndian GetNativeEndianMode();
	BufferEndian GetEndianMode();
	void		 Reverse2BytesInPlace( unsigned char& dataOf2Bytes );
	void		 Reverse4BytesInPlace( unsigned char& dataOf4Bytes );
	void		 Reverse8BytesInPlace( unsigned char& dataOf8Bytes );

public:
	std::vector<unsigned char>& m_buffer;
	BufferEndian m_endian = NATIVE;
	bool m_isOppositeNativeEndian = false;
};