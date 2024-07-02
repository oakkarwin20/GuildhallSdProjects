#pragma once


struct Vec3;


//----------------------------------------------------------------------------------------------------------------------
struct Quaternion
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

	Quaternion();	// Identity
	Quaternion( float x, float y, float z, float w );

	// Append rotation 
	Quaternion const operator*( Quaternion const& rotationToAppend  ) const;
	Vec3 const		 operator*( Vec3 const& vectorToRotate )		  const;
	Quaternion const operator*( float scalar )						  const;
	Quaternion const operator+( Quaternion const& quatToAdd			) const;
	Quaternion const operator-( Quaternion const& quatToSubtract	) const;

	void Normalize();
};
