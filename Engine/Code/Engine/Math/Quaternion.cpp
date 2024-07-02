#include "Engine/Math/Quaternion.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
Quaternion::Quaternion()
{
}


//----------------------------------------------------------------------------------------------------------------------
Quaternion::Quaternion( float initialX, float initialY, float initialZ, float initialW )
	: x ( initialX ), y ( initialY ), z ( initialZ ), w ( initialW )
{
}


//----------------------------------------------------------------------------------------------------------------------
Quaternion const Quaternion::operator*( Quaternion const& rotationToAppend ) const
{
	float realPartOfThisQuat		= w;
	Vec3  imaginaryPartofThisQuat   = Vec3( x, y, z );

	float realPartOfRotor		= rotationToAppend.w;
	Vec3 imaginaryPartofRotor	= Vec3( rotationToAppend.x, rotationToAppend.y, rotationToAppend.z );

	float w1w2		= realPartOfThisQuat * realPartOfRotor;
	float v1DotV2	= DotProduct3D( imaginaryPartofThisQuat, imaginaryPartofRotor );
	Vec3  w1v2		= realPartOfThisQuat * imaginaryPartofRotor;
	Vec3  w2v1		= realPartOfRotor * imaginaryPartofThisQuat;
	Vec3  v1CrossV2 = CrossProduct3D( imaginaryPartofThisQuat, imaginaryPartofRotor );

	float scalar	= w1w2 - v1DotV2;
	Vec3  vector	= w1v2 + w2v1 + v1CrossV2;

	Quaternion result;
	result.x = vector.x;
	result.y = vector.y;
	result.z = vector.z;
	result.w = scalar;

	return result;
}


//----------------------------------------------------------------------------------------------------------------------
Vec3 const Quaternion::operator*( Vec3 const& vectorToRotate ) const
{
	Vec3 vectorPartOfQuat = Vec3( x, y, z );
	Vec3 rotatedResult;
	rotatedResult = ( 2.0f * DotProduct3D( vectorPartOfQuat, vectorToRotate ) * vectorPartOfQuat ) + 
					( ( w * w - DotProduct3D( vectorPartOfQuat, vectorPartOfQuat ) ) * vectorToRotate ) + 
					( 2.0f * w * CrossProduct3D( vectorPartOfQuat, vectorToRotate ) );
	return rotatedResult;
}


//----------------------------------------------------------------------------------------------------------------------
Quaternion const Quaternion::operator*( float scalar ) const
{
	Quaternion scaledQuad;
	scaledQuad.x *= scalar;
	scaledQuad.y *= scalar;
	scaledQuad.z *= scalar;
	scaledQuad.w *= scalar;
	return scaledQuad;
}


//----------------------------------------------------------------------------------------------------------------------
Quaternion const Quaternion::operator+( Quaternion const& quatToAdd ) const
{
	Quaternion newQuat;
	newQuat.x += quatToAdd.x;
	newQuat.y += quatToAdd.y;
	newQuat.z += quatToAdd.z;
	newQuat.w += quatToAdd.w;
	return newQuat;
}


//----------------------------------------------------------------------------------------------------------------------
Quaternion const Quaternion::operator-( Quaternion const& quatToSubtract ) const
{
	Quaternion newQuat;
	newQuat.x -= quatToSubtract.x;
	newQuat.y -= quatToSubtract.y;
	newQuat.z -= quatToSubtract.z;
	newQuat.w -= quatToSubtract.w;
	return newQuat;
}


//----------------------------------------------------------------------------------------------------------------------
void Quaternion::Normalize()
{
	float lengthSquared = (x * x) + (y * y) + (z * z) + (w * w);
	float length		= sqrtf( lengthSquared );
//	if ( CompareIfFloatsAreEqual( length, 0.0f, 0.001f ) )
//	{
//		return Quaternion();
//	}
	float normalize = 1 / length;
	x = x * normalize;
	y = y * normalize;
	z = z * normalize;
	w = w * normalize;
}