#include "Engine/SkeletalSystem/IK_Joint3D.hpp"
#include "Engine/SkeletalSystem/IK_Chain3D.hpp"
#include "Engine/SkeletalSystem/CreatureBase.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//----------------------------------------------------------------------------------------------------------------------
IK_Joint3D::IK_Joint3D( int index, Vec3 startPos, float length, IK_Chain3D* IK_Chain, JointConstraintType jointConstraintType, EulerAngles orientation, FloatRange yawConstraints, FloatRange pitchConstraints, FloatRange rollConstraints, IK_Joint3D* parent )
	: m_jointIndex( index )
	, m_jointPos_LS( startPos )
	, m_distToChild( length )
	, m_ikChain( IK_Chain )
	, m_jointConstraintType( jointConstraintType )
	, m_eulerAnglesGoal_LS( orientation )
	, m_yawConstraints_LS( yawConstraints )
	, m_pitchConstraints_LS( pitchConstraints )
	, m_rollConstraints_LS( rollConstraints )
	, m_parent( parent )
{
	if ( m_parent == nullptr )
	{
		m_jointPos_MS = m_jointPos_LS;
	}
}


//----------------------------------------------------------------------------------------------------------------------
IK_Joint3D::~IK_Joint3D()
{
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::InitMesh_Cylinder( Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color )
{
	AddVertsForCapsule3D_TaperedEllipse( m_meshList_TBN, start, end, radius, radius, Vec2(1.0f, 1.0f), Vec2(1.0f, 1.0f), color );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::InitMesh_CylinderTapered( Vec3 const& start, Vec3 const& end, float startRadius, float endRadius, Rgba8 const& color )
{
	AddVertsForCapsule3D_TaperedEllipse( m_meshList_TBN, start, end, startRadius, endRadius, Vec2( 1.0f, 1.0f ), Vec2( 1.0f, 1.0f ), color );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::InitMesh_CapsuleTaperedEllipse( Vec3 const& start, Vec3 const& end, float startRadius, float endRadius, Vec2 const& startRadiusScalarXY, Vec2 const& endRadiusScalarXY, Rgba8 const& color, float numSlices, float numStacks )
{
	AddVertsForCapsule3D_TaperedEllipse( m_meshList_TBN, start, end, startRadius, endRadius, startRadiusScalarXY, endRadiusScalarXY, color, AABB2::ZERO_TO_ONE, numSlices, numStacks );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::InitMesh_Sphere( Vec3 const& center, float radius, Rgba8 const& color )
{
	AddVertsForSphere3D( m_meshList_TBN, center, radius, 16.0f, 16.0f, color );
}


//----------------------------------------------------------------------------------------------------------------------
// void IK_Joint3D::InitMesh_Arrow( Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color )
// {
// 	if ( m_child == nullptr )
// 	{
// 		//		ERROR_RECOVERABLE( "Attempting to render a mesh with NO child" );
// 		return;
// 	}
// 	AddVertsForArrow3D( m_meshList_TBN, start, end, radius, color );
// }


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::RenderIJK_MS( std::vector<Vertex_PCU>& verts, float length, float arrowThickness ) const
{
	Vec3 fwdDir, leftDir, upDir;
	m_eulerAnglesGoal_LS.GetAsVectors_XFwd_YLeft_ZUp( fwdDir, leftDir, upDir );
	Vec3 fwdPos		= m_jointPos_MS + ( fwdDir				* length );
	Vec3 leftPos	= m_jointPos_MS + ( leftDir				* length );
	Vec3 upPos		= m_jointPos_MS + ( upDir				* length );
	AddVertsForArrow3D( verts, m_jointPos_MS, fwdPos,	arrowThickness, Rgba8::RED	 );		// Fwd
	AddVertsForArrow3D( verts, m_jointPos_MS, leftPos,	arrowThickness, Rgba8::GREEN );		// Left
	AddVertsForArrow3D( verts, m_jointPos_MS, upPos,	arrowThickness, Rgba8::BLUE  );		// Up
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::SetConstraints_YPR( FloatRange yawConstraints, FloatRange pitchConstraints, FloatRange rollConstraints )
{
	// Make sure min constraints are within 
// 	while ( yawConstraints.m_min < -180.0f )
// 	{
// 		yawConstraints.m_min += 360.0f;
// 	}
// 	while ( yawConstraints.m_min > 180.0f )
// 	{
// 		yawConstraints.m_min -= 360.0f;
// 	}

	// Make sure max constraints and less than the min
	while ( yawConstraints.m_max < yawConstraints.m_min )
	{
		yawConstraints.m_max += 360.0f;
	}

	m_yawConstraints_LS		= yawConstraints;
	m_pitchConstraints_LS	= pitchConstraints;
	m_rollConstraints_LS	= rollConstraints;
}


//----------------------------------------------------------------------------------------------------------------------
EulerAngles IK_Joint3D::GetEulerFromFwdDir( Vec3 const& fwdDir )
{
	EulerAngles newEulerAngles;
	if ( m_matrixToEulerPreference == FWD_TEMPORAL )
	{
		float sign = 1.0f;
		if ( fwdDir.x < 0.0f )
		{
			// Preserve the negative sign since we lose it 
			// by squaring the length when calculating "projectedFwdOnPlaneXY"
			sign = -1.0f;
		}
		float projectedFwdOnPlaneXY	= sqrtf( ( fwdDir.x * fwdDir.x ) + ( fwdDir.y * fwdDir.y ) );
		bool  isFacingWorldZ		= CompareIfFloatsAreEqual( projectedFwdOnPlaneXY, 0.0f, 0.0001f );
		if ( isFacingWorldZ )
		{
			// Handle Gimbal lock edge case
			// This issue occurs when our "fwdDir" is facing world -Z or Z+
			// Since Y is around the "world" Z axis and not the "kBasis", this issue occurs.
			// When this happens, we need to compute EulerAngles from our dirVectors differently
			float yawDegrees	= 0.0f;
			float pitchDegrees	= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
			float rollDegrees	= 0.0f;
			newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		else
		{
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Compute both cases of YPR (yawPreferred & pitchPreferred)
			// 2. Compute and choose "nearest" euler
			// 3. Compare both cases of euler and choose most similar to prevEuler solution
			//----------------------------------------------------------------------------------------------------------------------
			// 1a. Compute Yaw preferred solution
			float yawDegrees_y		= Atan2Degrees(  fwdDir.y, fwdDir.x );
			float pitchDegrees_y	= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
			// 1b. Compute pitch preferred solution
			float yawDegrees_p		= yawDegrees_y;
			float pitchDegrees_p	= pitchDegrees_y;
			if ( sign < 0.0f )
			{
				// Re-compute pitch calculations
				projectedFwdOnPlaneXY  *= -1.0f;
				yawDegrees_p			= yawDegrees_y;
				pitchDegrees_p			= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
				if ( yawDegrees_y >= 90.0f )
				{
					yawDegrees_p -= 180.0f;
				}
				else if ( yawDegrees_y <= -90.0f )
				{
					yawDegrees_p += 180.0f;
				}
			}

			//----------------------------------------------------------------------------------------------------------------------
			// 2. Convert YPR to vectors (leftDir AND upDir comparison)
			//----------------------------------------------------------------------------------------------------------------------
			EulerAngles yawPreferredEuler	= EulerAngles( yawDegrees_y, pitchDegrees_y, 0.0f );
			EulerAngles pitchPreferredEuler = EulerAngles( yawDegrees_p, pitchDegrees_p, 0.0f );
			Vec3 fwd_y, left_y, up_y;
			Vec3 fwd_p, left_p, up_p;
			yawPreferredEuler  .GetAsVectors_XFwd_YLeft_ZUp( fwd_y, left_y, up_y );
			pitchPreferredEuler.GetAsVectors_XFwd_YLeft_ZUp( fwd_p, left_p, up_p );
			// 3. Dot product vectors to prev solution
			float leftSimilarity_y	= DotProduct3D( m_leftDir_lastFrame, left_y );
			float leftSimilarity_p	= DotProduct3D( m_leftDir_lastFrame, left_p );
			float upSimilarity_y	= DotProduct3D( m_upDir_lastFrame,   up_y	);
			float upSimilarity_p	= DotProduct3D( m_upDir_lastFrame,   up_p	);
			float similarity_y		= leftSimilarity_y + upSimilarity_y; 
			float similarity_p		= leftSimilarity_p + upSimilarity_p; 

		//----------------------------------------------------------------------------------------------------------------------
		// Handle edge case: if yaw OR pitch solutions are bad, recompute pitch based on prev euler
		//----------------------------------------------------------------------------------------------------------------------
// 		bool yawSolutionIsBad	= CompareIfFloatsAreEqual( similarity_y, -2.0f, 0.1f );
// 		bool pitchSolutionIsBad = CompareIfFloatsAreEqual( similarity_p, -2.0f, 0.1f );
// 		if ( yawSolutionIsBad || pitchSolutionIsBad )
// 		{
// 			// Compute new pitch over 90
//  			float pitchOffset	= 90.0f - pitchDegrees_y;
//  			pitchDegrees_y		= 90.0f + pitchOffset;
// 			// Compute new yaw over 90 
// // 			float yawOffset		= 90.0f - yawDegrees_y;
// // 			yawDegrees_y		= 180.0f + yawOffset;
//  			yawDegrees_y		= m_euler_LastFrame.m_yawDegrees + yawDegrees_y;
// 			yawPreferredEuler	= EulerAngles( yawDegrees_y, pitchDegrees_y, 0.0f );
// 			// Re-compare directions AGAIN
// 			yawPreferredEuler  .GetAsVectors_XFwd_YLeft_ZUp( fwd_y, left_y, up_y );
// 			pitchPreferredEuler.GetAsVectors_XFwd_YLeft_ZUp( fwd_p, left_p, up_p );
// 			leftSimilarity_y	= DotProduct3D( m_leftDir_lastFrame, left_y );
// 			leftSimilarity_p	= DotProduct3D( m_leftDir_lastFrame, left_p );
// 			upSimilarity_y		= DotProduct3D( m_upDir_lastFrame,   up_y	);
// 			upSimilarity_p		= DotProduct3D( m_upDir_lastFrame,   up_p	);
// 			similarity_y		= leftSimilarity_y + upSimilarity_y; 
// 			similarity_p		= leftSimilarity_p + upSimilarity_p; 
// 		}


			// 4. Return euler most "similar" to prev solution
			if ( similarity_y >= similarity_p )
			{
				// yawPreferred solution is more similar to the newLeft and newUp 
				newEulerAngles		= EulerAngles( yawDegrees_y, pitchDegrees_y, 0.0f );
				m_leftDir_lastFrame	= left_y;
				m_upDir_lastFrame	= up_y;
				m_eulerLS_LastFrame	= newEulerAngles;
			}
			else // if ( similarity_y < similarity_p )
			{
				// pitchPreferred solution is more similar to the newLeft and newUp 
				newEulerAngles		= EulerAngles( yawDegrees_p, pitchDegrees_p, 0.0f );
				m_leftDir_lastFrame	= left_p;
				m_upDir_lastFrame	= up_p;
				m_eulerLS_LastFrame	= newEulerAngles;
			}
		}
		return newEulerAngles;
	}
	else if ( m_matrixToEulerPreference == YAW_PREFERRED )
	{
		float sign = 1.0f;
		if ( fwdDir.x < 0.0f )
		{
			// Preserve the negative sign since we lose it 
			// by squaring the length when calculating "projectedFwdOnPlaneXY"
			sign = -1.0f;
		}
		float projectedFwdOnPlaneXY	= sqrtf( ( fwdDir.x * fwdDir.x ) + ( fwdDir.y * fwdDir.y ) );
		bool  isFacingWorldZ		= CompareIfFloatsAreEqual( projectedFwdOnPlaneXY, 0.0f, 0.0001f );
		if ( isFacingWorldZ )
		{
			// Handle Gimbal lock edge case
			// This issue occurs when our "fwdDir" is facing world -Z or Z+
			// Since Y is around the "world" Z axis and not the "kBasis", this issue occurs.
			// When this happens, we need to compute EulerAngles from our dirVectors differently
			float yawDegrees	= 0.0f;
			float pitchDegrees	= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
			float rollDegrees	= 0.0f;
			newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		else
		{
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Compute both cases of YPR (yawPreferred & pitchPreferred)
			// 2. Compute and choose "nearest" euler
			// 3. Compare both cases of euler and choose most similar to prevEuler solution
			//----------------------------------------------------------------------------------------------------------------------
			// 1a. Compute Yaw preferred solution
			float yawDegrees			= Atan2Degrees(  fwdDir.y, fwdDir.x );
			float pitchDegrees			= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
 			if ( sign < 0.0f )
 			{
 				// Re-compute pitch calculations
 				projectedFwdOnPlaneXY  *= -1.0f;
 				pitchDegrees			= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
				if ( pitchDegrees >= 90.0f )
				{
					pitchDegrees = 180.0f - pitchDegrees;
//					pitchDegrees -= 180.0f;
				}
				else if ( pitchDegrees <= -90.0f )
				{
					pitchDegrees = -180.0f - pitchDegrees;
//					pitchDegrees += 180.0f;
				}
			}
			float rollDegrees = 0.0f;
			newEulerAngles = EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		return newEulerAngles;
	}
	else if ( m_matrixToEulerPreference == PITCH_PREFERRED )
	{
		float sign = 1.0f;
		if ( fwdDir.x < 0.0f )
		{
			// Preserve the negative sign since we lose it 
			// by squaring the length when calculating "projectedFwdOnPlaneXY"
			sign = -1.0f;
		}
		float projectedFwdOnPlaneXY = sqrtf( ( fwdDir.x * fwdDir.x ) + ( fwdDir.y * fwdDir.y ) );
		bool  isFacingWorldZ		= CompareIfFloatsAreEqual( projectedFwdOnPlaneXY, 0.0f, 0.0001f );
		if ( isFacingWorldZ )
		{
			// Handle Gimbal lock edge case
			// This issue occurs when our "fwdDir" is facing world -Z or Z+
			// Since Y is around the "world" Z axis and not the "kBasis", this issue occurs.
			// When this happens, we need to compute EulerAngles from our dirVectors differently
			float yawDegrees	= 0.0f;
			float pitchDegrees	= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
			float rollDegrees	= 0.0f;
			newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		else
		{
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Compute both cases of YPR (yawPreferred & pitchPreferred)
			// 2. Compute and choose "nearest" euler
			// 3. Compare both cases of euler and choose most similar to prevEuler solution
			//----------------------------------------------------------------------------------------------------------------------
			// 1a. Compute Yaw preferred solution
			float yawDegrees			= Atan2Degrees(  fwdDir.y, fwdDir.x );
			float pitchDegrees			= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
			if ( sign < 0.0f )
			{
				// Re-compute pitch calculations
				projectedFwdOnPlaneXY  *= -1.0f;
				pitchDegrees			= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
				if ( yawDegrees >= 90.0f )
				{
					yawDegrees -= 180.0f;
				}
				else if ( yawDegrees <= -90.0f )
				{
					yawDegrees += 180.0f;
				}
			}
			float rollDegrees = 0.0f;
			newEulerAngles	  = EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		return newEulerAngles;
	}
	else if ( m_matrixToEulerPreference == UNMODIFIED )
	{
		float projectedFwdOnPlaneXY = sqrtf( ( fwdDir.x * fwdDir.x ) + ( fwdDir.y * fwdDir.y ) );
		bool  isFacingWorldZ = CompareIfFloatsAreEqual( projectedFwdOnPlaneXY, 0.0f, 0.0001f );
		if ( isFacingWorldZ )
		{
			// Handle Gimbal lock edge case
			// This issue occurs when our "fwdDir" is facing world -Z or Z+
			// Since Y is around the "world" Z axis and not the "kBasis", this issue occurs.
			// When this happens, we need to compute EulerAngles from our dirVectors differently
			float yawDegrees	= 0.0f;
			float pitchDegrees	= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
			float rollDegrees	= 0.0f;
			newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		else
		{
			// 1a. Compute Yaw preferred solution
			float yawDegrees    = Atan2Degrees(  fwdDir.y, fwdDir.x );
			float pitchDegrees  = Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
			float rollDegrees	= 0.0f;
			newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		return newEulerAngles;
	}
	ERROR_AND_DIE( "Unspecified matrix to euler preference" );
}


//----------------------------------------------------------------------------------------------------------------------
EulerAngles IK_Joint3D::GetEulerFromFwdAndLeft( Vec3 const& fwdDir, Vec3 const& leftDir )
{
	m_eulerLS_LastFrame = m_eulerAnglesGoal_LS;
	EulerAngles newEulerAngles;
	if ( m_matrixToEulerPreference == FWD_LEFT_TEMPORAL )
	{
		float sign = 1.0f;
		if ( fwdDir.x < 0.0f )
		{
			// Preserve the negative sign since we lose it 
			// by squaring the length when calculating "projectedFwdOnPlaneXY"
			sign = -1.0f;
		}
		float projectedFwdOnPlaneXY	= sqrtf( ( fwdDir.x * fwdDir.x ) + ( fwdDir.y * fwdDir.y ) );
		bool  isFacingWorldZ		= CompareIfFloatsAreEqual( projectedFwdOnPlaneXY, 0.0f, 0.0001f );
		if ( isFacingWorldZ )
		{
			// Handle Gimbal lock edge case
			// This issue occurs when our "fwdDir" is facing world -Z or Z+
			// Since Y is around the "world" Z axis and not the "kBasis", this issue occurs.
			// When this happens, we need to compute EulerAngles from our dirVectors differently
			float yawDegrees	= 0.0f;
			float pitchDegrees	= Atan2Degrees	( -fwdDir.z, projectedFwdOnPlaneXY );
			Vec3 upKbasis		= CrossProduct3D(  fwdDir,	 leftDir );
			upKbasis.Normalize();
			float rollDegrees	= Atan2Degrees( -upKbasis.y, leftDir.y );
			newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		else
		{
// 			// Default version (yaw preferred?)
// 			float yawDegrees				= Atan2Degrees  ( fwdDir.y, fwdDir.x			  );
// 			float pitchDegrees				= Atan2Degrees  (-fwdDir.z, projectedFwdOnPlaneXY );
// 			Vec3 upKbasis					= CrossProduct3D( fwdDir,	leftDir				  );
// 			upKbasis.Normalize();
// 			float rollDegrees				= Atan2Degrees( leftDir.z, upKbasis.z );

// 			// Unrolled version, attempt 1 (Pitch preferred?)
// 			float yawDegrees				= Atan2Degrees( fwdDir.y, fwdDir.x );
// 			projectedFwdOnPlaneXY			*= sign;	// reapply the sign before computing pitch
// 			float pitchDegrees				= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
// 			Vec3 upKbasis					= CrossProduct3D( fwdDir, leftDir );
// 			upKbasis.Normalize();
// 			float rollDegrees				= Atan2Degrees( leftDir.z, upKbasis.z );
// 			if ( fabsf(pitchDegrees) > 90.0f )
// 			{
// 				yawDegrees  += 180.0f;
// 				rollDegrees += 180.0f;
// 			}

			//----------------------------------------------------------------------------------------------------------------------
			// Unrolled version, attempt 2 (Compare against history, actually temporal?)
			//----------------------------------------------------------------------------------------------------------------------
 			// 1. Compute yawPref version
 			float yawDegrees_yPref			= Atan2Degrees( fwdDir.y, fwdDir.x );
 			float pitchDegrees_yPref		= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
 			Vec3 upKbasis					= CrossProduct3D( fwdDir, leftDir );
 			upKbasis.Normalize();
 			float rollDegrees_yPref			= Atan2Degrees( leftDir.z, upKbasis.z );
 			EulerAngles euler_yPref			= EulerAngles( yawDegrees_yPref, pitchDegrees_yPref, rollDegrees_yPref );
 
 			// 2. Compute pitchPref version
 			float yawDegrees_pPref			= Atan2Degrees( fwdDir.y, fwdDir.x );
 			projectedFwdOnPlaneXY			*= sign;	// reapply the sign before computing pitch
 			float pitchDegrees_pPref		= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
 			float rollDegrees_pPref			= Atan2Degrees( leftDir.z, upKbasis.z );
 			if ( fabsf(pitchDegrees_pPref) > 90.0f )
 			{
 				yawDegrees_pPref  += 180.0f;
 				rollDegrees_pPref += 180.0f;
 			}
 			EulerAngles euler_pPref			= EulerAngles( yawDegrees_pPref, pitchDegrees_pPref, rollDegrees_pPref );
 			
 			// 3. Compare for similarity
 			// Yaw similarity
 			float yawDifference_yPref	= fabsf( m_eulerLS_LastFrame.m_yawDegrees   - yawDegrees_yPref	 );
 			float pitchDifference_yPref = fabsf( m_eulerLS_LastFrame.m_pitchDegrees - pitchDegrees_yPref );
 			float rollDifference_yPref	= fabsf( m_eulerLS_LastFrame.m_rollDegrees	- rollDegrees_yPref  );
 			float totalDifference_yPref = yawDifference_yPref + pitchDifference_yPref + rollDifference_yPref; 
 			// Pitch similarity
 			float yawDifference_pPref	= fabsf( m_eulerLS_LastFrame.m_yawDegrees   - yawDegrees_pPref	 );
 			float pitchDifference_pPref = fabsf( m_eulerLS_LastFrame.m_pitchDegrees - pitchDegrees_pPref );
 			float rollDifference_pPref	= fabsf( m_eulerLS_LastFrame.m_rollDegrees	- rollDegrees_pPref  );
 			float totalDifference_pPref = yawDifference_pPref + pitchDifference_pPref + rollDifference_pPref; 
 
 			// 4. Choose best option
 			if ( totalDifference_pPref < totalDifference_yPref )
 			{
 				// Pitch is most similar
 				newEulerAngles = EulerAngles( yawDegrees_pPref, pitchDegrees_pPref, rollDegrees_pPref );
 			}
 			else  
 			{
 				// Yaw is most similar
 				newEulerAngles = EulerAngles( yawDegrees_yPref, pitchDegrees_yPref, rollDegrees_yPref );
 			}

			// Squirrel's approach, diagram made sense geometrically but the results were incorrect
// 			float projectedLeftOnPlaneXY	= sqrtf ( (leftDir.x * leftDir.x) + (leftDir.y * leftDir.y) );
// 			float rollDegrees				= Atan2Degrees( leftDir.z, projectedLeftOnPlaneXY );
// 			if ( leftDir.y < 0.0f )
// 			{
// 				projectedLeftOnPlaneXY *= -1.0f;
// 				rollDegrees				= Atan2Degrees( leftDir.z, projectedLeftOnPlaneXY );
// 				if ( rollDegrees >= 90.0f )
// 				{
// 					rollDegrees -= 180.0f;
// 				}
// 				else if ( rollDegrees <= -90.0f )
// 				{
// 					rollDegrees += 180.0f;
// 				}
// 			}
//  		if ( sign < 0.0f )
//  		{
//  			// Re-compute pitch calculations
//  			projectedFwdOnPlaneXY  *= -1.0f;
//  			pitchDegrees			= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
//   			if ( yawDegrees >= 90.0f )
//   			{
//   				yawDegrees -= 180.0f;
//   			}
//   			else if ( yawDegrees <= -90.0f )
//   			{
//   				yawDegrees += 180.0f;
//   			}
//  		}
//			newEulerAngles = EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		return newEulerAngles;
	}
	else if ( m_matrixToEulerPreference == FWD_LEFT_ALTERNATE )
	{
		float sign = 1.0f;
		if ( fwdDir.x < 0.0f )
		{
			// Preserve the negative sign since we lose it 
			// by squaring the length when calculating "projectedFwdOnPlaneXY"
			sign = -1.0f;
		}
		float projectedFwdOnPlaneXY	= sqrtf( ( fwdDir.x * fwdDir.x ) + ( fwdDir.y * fwdDir.y ) );
		bool  isFacingWorldZ		= CompareIfFloatsAreEqual( projectedFwdOnPlaneXY, 0.0f, 0.0001f );
		if ( isFacingWorldZ )
		{
			// Handle Gimbal lock edge case
			// This issue occurs when our "fwdDir" is facing world -Z or Z+
			// Since Y is around the "world" Z axis and not the "kBasis", this issue occurs.
			// When this happens, we need to compute EulerAngles from our dirVectors differently
			float yawDegrees	= 0.0f;
			float pitchDegrees	= Atan2Degrees	( -fwdDir.z, projectedFwdOnPlaneXY );
			Vec3 upKbasis		= CrossProduct3D(  fwdDir,	 leftDir );
			upKbasis.Normalize();
			float rollDegrees	= Atan2Degrees( -upKbasis.y, leftDir.y );
			newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		else
		{
			//----------------------------------------------------------------------------------------------------------------------
			// Attempt 3, first compute yawPreferred and pitchPreferred results and choose the most similar to last frame.
			//			  Then, compare that result with with 8 base versions (+-Y, +-P, +-R) * 27 alternatives 
			//----------------------------------------------------------------------------------------------------------------------
// 			// 1. Compute yawPref version
// 			float yawDegrees_yPref			= Atan2Degrees( fwdDir.y, fwdDir.x );
// 			float pitchDegrees_yPref		= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
// 			Vec3 upKbasis					= CrossProduct3D( fwdDir, leftDir );
// 			upKbasis.Normalize();
// 			float rollDegrees_yPref			= Atan2Degrees( leftDir.z, upKbasis.z );
// 			EulerAngles euler_yPref			= EulerAngles( yawDegrees_yPref, pitchDegrees_yPref, rollDegrees_yPref );
// 
// 			// 2. Compute pitchPref version
// 			float yawDegrees_pPref			= Atan2Degrees( fwdDir.y, fwdDir.x );
// 			projectedFwdOnPlaneXY			*= sign;	// reapply the sign before computing pitch
// 			float pitchDegrees_pPref		= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
// 			float rollDegrees_pPref			= Atan2Degrees( leftDir.z, upKbasis.z );
// 			if ( fabsf(pitchDegrees_pPref) > 90.0f )
// 			{
// 				yawDegrees_pPref  += 180.0f;
// 				rollDegrees_pPref += 180.0f;
// 			}
// 			EulerAngles euler_pPref			= EulerAngles( yawDegrees_pPref, pitchDegrees_pPref, rollDegrees_pPref );
// 
// 			// 3. Compare for similarity
// 			// Yaw similarity
// 			float yawDifference_yPref	= fabsf( m_eulerLS_LastFrame.m_yawDegrees   - yawDegrees_yPref	 );
// 			float pitchDifference_yPref = fabsf( m_eulerLS_LastFrame.m_pitchDegrees - pitchDegrees_yPref );
// 			float rollDifference_yPref	= fabsf( m_eulerLS_LastFrame.m_rollDegrees	- rollDegrees_yPref  );
// 			float totalDifference_yPref = yawDifference_yPref + pitchDifference_yPref + rollDifference_yPref; 
// 			// Pitch similarity
// 			float yawDifference_pPref	= fabsf( m_eulerLS_LastFrame.m_yawDegrees   - yawDegrees_pPref	 );
// 			float pitchDifference_pPref = fabsf( m_eulerLS_LastFrame.m_pitchDegrees - pitchDegrees_pPref );
// 			float rollDifference_pPref	= fabsf( m_eulerLS_LastFrame.m_rollDegrees	- rollDegrees_pPref  );
// 			float totalDifference_pPref = yawDifference_pPref + pitchDifference_pPref + rollDifference_pPref; 
// 
// 			// 4. Choose best option
// 			// 2. Set eulerBest as the default solution
// 			EulerAngles eulerComputed = EulerAngles();
// 			if ( totalDifference_pPref < totalDifference_yPref )
// 			{
// 				// Pitch is most similar
// 				eulerComputed = EulerAngles( yawDegrees_pPref, pitchDegrees_pPref, rollDegrees_pPref );
// 			}
// 			else  
// 			{
// 				// Yaw is most similar
// 				eulerComputed = EulerAngles( yawDegrees_yPref, pitchDegrees_yPref, rollDegrees_yPref );
// 			}
// 			EulerAngles eulerBest = eulerComputed;
// 
// 			//----------------------------------------------------------------------------------------------------------------------
// 			// (Try alternatives and choose most similar to history)
// 			// 1. Compute default Euler yaw preferred solution
// 			// 2. Set best as default
// 			// 3. Create the altEulerList
// 			// 4. Loop through alternatives and update the best Euler if a more similar solution is found
// 			//		try alternatives 
// 			// 5. the answer SHOULD be the best alias
// 			//----------------------------------------------------------------------------------------------------------------------
// 
// 			// Init baseEulerList, (2 +-yaw * 2 +-pitch * 2 +-roll ) = 8 total variations.
// 			std::vector<EulerAngles> baseEulerList;
// 			baseEulerList.emplace_back( EulerAngles(  180.0f, -180.0f,  180.0f ) );		// 0 +-+
// 			baseEulerList.emplace_back( EulerAngles(  180.0f,  180.0f,  180.0f ) );		// 1 +++
// 			baseEulerList.emplace_back( EulerAngles( -180.0f,  180.0f,  180.0f ) );		// 2 -++
// 			baseEulerList.emplace_back( EulerAngles( -180.0f, -180.0f,  180.0f ) );		// 3 --+
// 			baseEulerList.emplace_back( EulerAngles( -180.0f, -180.0f, -180.0f ) );		// 4 ---
// 			baseEulerList.emplace_back( EulerAngles(  180.0f, -180.0f, -180.0f ) );		// 5 +--
// 			baseEulerList.emplace_back( EulerAngles(  180.0f,  180.0f, -180.0f ) );		// 6 ++-
// 			baseEulerList.emplace_back( EulerAngles( -180.0f,  180.0f, -180.0f ) );		// 7 -+-
// 
// 
// 			// 3. Init the altEulerList, (3 +-yaw * 3 +-pitch * 3 +-roll) = 27 total variations.
// 			std::vector<EulerAngles> altEulerList;
// 			altEulerList.emplace_back( EulerAngles(	   0.0f,	0.0f,    0.0f ) );		// 0
// 			altEulerList.emplace_back( EulerAngles(  360.0f,	0.0f,    0.0f ) );		// 1
// 			altEulerList.emplace_back( EulerAngles( -360.0f,	0.0f,    0.0f ) );		// 2
// 			altEulerList.emplace_back( EulerAngles(    0.0f,  360.0f,    0.0f ) );		// 3
// 			altEulerList.emplace_back( EulerAngles(    0.0f, -360.0f,    0.0f ) );		// 4
// 			altEulerList.emplace_back( EulerAngles(    0.0f,	0.0F,  360.0f ) );		// 5
// 			altEulerList.emplace_back( EulerAngles(    0.0f,	0.0F, -360.0f ) );		// 6
// 			altEulerList.emplace_back( EulerAngles(  360.0f,  360.0f,    0.0f ) );		// 7
// 			altEulerList.emplace_back( EulerAngles( -360.0f,  360.0f,    0.0f ) );		// 8
// 			altEulerList.emplace_back( EulerAngles(  360.0f, -360.0f,    0.0f ) );		// 9
// 			altEulerList.emplace_back( EulerAngles( -360.0f, -360.0f,    0.0f ) );		// 10
// 			altEulerList.emplace_back( EulerAngles(  360.0f,    0.0f,  360.0f ) );		// 11
// 			altEulerList.emplace_back( EulerAngles( -360.0f,    0.0f,  360.0f ) );		// 12
// 			altEulerList.emplace_back( EulerAngles(  360.0f,    0.0f, -360.0f ) );		// 13
// 			altEulerList.emplace_back( EulerAngles( -360.0f,    0.0f, -360.0f ) );		// 14
// 			altEulerList.emplace_back( EulerAngles(	   0.0f,  360.0f,  360.0f ) );		// 15
// 			altEulerList.emplace_back( EulerAngles(	   0.0f, -360.0f,  360.0f ) );		// 16
// 			altEulerList.emplace_back( EulerAngles(	   0.0f,  360.0f, -360.0f ) );		// 17
// 			altEulerList.emplace_back( EulerAngles(	   0.0f, -360.0f, -360.0f ) );		// 18
// 			altEulerList.emplace_back( EulerAngles(	 360.0f,  360.0f,  360.0f ) );		// 19
// 			altEulerList.emplace_back( EulerAngles(	-360.0f,  360.0f,  360.0f ) );		// 20
// 			altEulerList.emplace_back( EulerAngles(	 360.0f, -360.0f,  360.0f ) );		// 21
// 			altEulerList.emplace_back( EulerAngles(	 360.0f,  360.0f, -360.0f ) );		// 22
// 			altEulerList.emplace_back( EulerAngles(	-360.0f, -360.0f,  360.0f ) );		// 23
// 			altEulerList.emplace_back( EulerAngles(	-360.0f, -360.0f, -360.0f ) );		// 24
// 			altEulerList.emplace_back( EulerAngles(	 360.0f, -360.0f, -360.0f ) );		// 25
// 			altEulerList.emplace_back( EulerAngles(	-360.0f,  360.0f, -360.0f ) );		// 26
// 
// 
// 			// 4. Loop through baseEulerList and try every combination of the altEulerList
// 			float bestEulerDifference = 180.0f;
// 			for ( int baseEulerIndex = 0; baseEulerIndex < baseEulerList.size(); baseEulerIndex++ )
// 			{
// 				EulerAngles& baseEuler = baseEulerList[baseEulerIndex];
// 				for ( int altEulerIndex = 0; altEulerIndex < altEulerList.size(); altEulerIndex++ )
// 				{
// 					// 5. Compute alternate Euler by combining the computedEuler with the curAltEuler
// 					EulerAngles& curAltEuler	= altEulerList[ altEulerIndex ];
// 					float signYaw   			= 1.0f;
// 					float signPitch 			= 1.0f;
// 					float signRoll				= 1.0f;
// 					if ( baseEuler.m_yawDegrees < 0.0f )
// 					{
// 						signYaw = signYaw * -1.0f;
// 					}
// 					if ( baseEuler.m_pitchDegrees < 0.0f )
// 					{
// 						signPitch = signPitch * -1.0f;
// 					}
// 					if ( baseEuler.m_rollDegrees < 0.0f )
// 					{
// 						signRoll = signRoll * -1.0f;
// 					}
// 
// 					float yawAlias				= ( baseEuler.m_yawDegrees   + ( eulerComputed.m_yawDegrees   * signYaw   ) ) + curAltEuler.m_yawDegrees;
// 					float pitchAlias			= ( baseEuler.m_pitchDegrees + ( eulerComputed.m_pitchDegrees * signPitch ) ) + curAltEuler.m_pitchDegrees; 
// 					float rollAlias				= ( baseEuler.m_rollDegrees  + ( eulerComputed.m_rollDegrees  * signRoll  ) ) + curAltEuler.m_rollDegrees; 
// 					EulerAngles eulerAlias		= EulerAngles( yawAlias, pitchAlias, rollAlias );
// 
// 					// 6. Compute aliased Euler by subtracting from the last frame's Euler
// 					float yawDifference			= fabsf( m_eulerLS_LastFrame.m_yawDegrees   - yawAlias   );
// 					float pitchDifference		= fabsf( m_eulerLS_LastFrame.m_pitchDegrees - pitchAlias );
// 					float rollDifference		= fabsf( m_eulerLS_LastFrame.m_rollDegrees	- rollAlias	 );
// 					float curEulerDifference	= yawDifference + pitchDifference + rollDifference;
// 
// 					// 7. Compare for similarity by checking the difference between curAlias and last frame's Euler
// 					if ( curEulerDifference < bestEulerDifference )
// 					{
// 						// Pitch is most similar
// 						eulerBest			= eulerAlias;
// 						bestEulerDifference	= curEulerDifference;
// 					}
// 				}
// 			}
// 			newEulerAngles = eulerBest;

 			// Attempt 2, approach with 8 base versions (+-Y, +-P, +-R) + 27 alternatives
 			//----------------------------------------------------------------------------------------------------------------------
 			// (Try alternatives and choose most similar to history)
 			// 1. Compute default Euler yaw preferred solution
 			// 2. Set best as default
 			// 3. Create the altEulerList
 			// 4. Loop through alternatives and update the best euler if a more similar solution is found
 			//		try alternatives 
 			// 5. the answer SHOULD be the best alias
 			//----------------------------------------------------------------------------------------------------------------------
 			// 1. Compute yawPref version
 			float yawDegrees			= Atan2Degrees( fwdDir.y, fwdDir.x );
 			float pitchDegrees			= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
 			Vec3 upKbasis				= CrossProduct3D( fwdDir, leftDir );
 			upKbasis.Normalize();
 			float rollDegrees			= Atan2Degrees( leftDir.z, upKbasis.z );
 			// 2. Set eulerBest as the default solution
 			EulerAngles eulerComputed	= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
 			EulerAngles eulerBest		= eulerComputed;
 
 			// Init baseEulerList, (2 +-yaw * 2 +-pitch * 2 +-roll ) = 8 total variations.
 			std::vector<EulerAngles> baseEulerList;
 			baseEulerList.emplace_back( EulerAngles(  180.0f, -180.0f,  180.0f ) );		// 0 +-+
 			baseEulerList.emplace_back( EulerAngles(  180.0f,  180.0f,  180.0f ) );		// 1 +++
 			baseEulerList.emplace_back( EulerAngles( -180.0f,  180.0f,  180.0f ) );		// 2 -++
 			baseEulerList.emplace_back( EulerAngles( -180.0f, -180.0f,  180.0f ) );		// 3 --+
 			baseEulerList.emplace_back( EulerAngles( -180.0f, -180.0f, -180.0f ) );		// 4 ---
 			baseEulerList.emplace_back( EulerAngles(  180.0f, -180.0f, -180.0f ) );		// 5 +--
 			baseEulerList.emplace_back( EulerAngles(  180.0f,  180.0f, -180.0f ) );		// 6 ++-
 			baseEulerList.emplace_back( EulerAngles( -180.0f,  180.0f, -180.0f ) );		// 7 -+-
 
 			// 3. Init the altEulerList, (3 +-yaw * 3 +-pitch * 3 +-roll) = 27 total variations.
 			std::vector<EulerAngles> altEulerList;
 			altEulerList.emplace_back( EulerAngles(	   0.0f,	0.0f,    0.0f ) );		// 0
 			altEulerList.emplace_back( EulerAngles(  360.0f,	0.0f,    0.0f ) );		// 1
 			altEulerList.emplace_back( EulerAngles( -360.0f,	0.0f,    0.0f ) );		// 2
 			altEulerList.emplace_back( EulerAngles(    0.0f,  360.0f,    0.0f ) );		// 3
 			altEulerList.emplace_back( EulerAngles(    0.0f, -360.0f,    0.0f ) );		// 4
 			altEulerList.emplace_back( EulerAngles(    0.0f,	0.0F,  360.0f ) );		// 5
 			altEulerList.emplace_back( EulerAngles(    0.0f,	0.0F, -360.0f ) );		// 6
 			altEulerList.emplace_back( EulerAngles(  360.0f,  360.0f,    0.0f ) );		// 7
 			altEulerList.emplace_back( EulerAngles( -360.0f,  360.0f,    0.0f ) );		// 8
 			altEulerList.emplace_back( EulerAngles(  360.0f, -360.0f,    0.0f ) );		// 9
 			altEulerList.emplace_back( EulerAngles( -360.0f, -360.0f,    0.0f ) );		// 10
 			altEulerList.emplace_back( EulerAngles(  360.0f,    0.0f,  360.0f ) );		// 11
 			altEulerList.emplace_back( EulerAngles( -360.0f,    0.0f,  360.0f ) );		// 12
 			altEulerList.emplace_back( EulerAngles(  360.0f,    0.0f, -360.0f ) );		// 13
 			altEulerList.emplace_back( EulerAngles( -360.0f,    0.0f, -360.0f ) );		// 14
 			altEulerList.emplace_back( EulerAngles(	   0.0f,  360.0f,  360.0f ) );		// 15
 			altEulerList.emplace_back( EulerAngles(	   0.0f, -360.0f,  360.0f ) );		// 16
 			altEulerList.emplace_back( EulerAngles(	   0.0f,  360.0f, -360.0f ) );		// 17
 			altEulerList.emplace_back( EulerAngles(	   0.0f, -360.0f, -360.0f ) );		// 18
 			altEulerList.emplace_back( EulerAngles(	 360.0f,  360.0f,  360.0f ) );		// 19
 			altEulerList.emplace_back( EulerAngles(	-360.0f,  360.0f,  360.0f ) );		// 20
 			altEulerList.emplace_back( EulerAngles(	 360.0f, -360.0f,  360.0f ) );		// 21
 			altEulerList.emplace_back( EulerAngles(	 360.0f,  360.0f, -360.0f ) );		// 22
 			altEulerList.emplace_back( EulerAngles(	-360.0f, -360.0f,  360.0f ) );		// 23
 			altEulerList.emplace_back( EulerAngles(	-360.0f, -360.0f, -360.0f ) );		// 24
 			altEulerList.emplace_back( EulerAngles(	 360.0f, -360.0f, -360.0f ) );		// 25
 			altEulerList.emplace_back( EulerAngles(	-360.0f,  360.0f, -360.0f ) );		// 26
 
 			// 4. Loop through baseEulerList and try every combination of the altEulerList
 			float bestEulerDifference = 180.0f;
 			for ( int baseEulerIndex = 0; baseEulerIndex < baseEulerList.size(); baseEulerIndex++ )
 			{
 				EulerAngles& baseEuler = baseEulerList[baseEulerIndex];
 				for ( int altEulerIndex = 0; altEulerIndex < altEulerList.size(); altEulerIndex++ )
 				{
 					// 5. Compute alternate Euler by combining the computedEuler with the curAltEuler
 					EulerAngles& curAltEuler	= altEulerList[ altEulerIndex ];
 					float signYaw   			= 1.0f;
 					float signPitch 			= 1.0f;
 					float signRoll				= 1.0f;
 					if ( baseEuler.m_yawDegrees < 0.0f )
 					{
 						signYaw = signYaw * -1.0f;
 					}
 					if ( baseEuler.m_pitchDegrees < 0.0f )
 					{
 						signPitch = signPitch * -1.0f;
 					}
 					if ( baseEuler.m_rollDegrees < 0.0f )
 					{
 						signRoll = signRoll * -1.0f;
 					}
 
 					float yawAlias				= ( baseEuler.m_yawDegrees   + ( eulerComputed.m_yawDegrees   * signYaw   ) ) + curAltEuler.m_yawDegrees;
 					float pitchAlias			= ( baseEuler.m_pitchDegrees + ( eulerComputed.m_pitchDegrees * signPitch ) ) + curAltEuler.m_pitchDegrees; 
 					float rollAlias				= ( baseEuler.m_rollDegrees  + ( eulerComputed.m_rollDegrees  * signRoll  ) ) + curAltEuler.m_rollDegrees; 
 					EulerAngles eulerAlias		= EulerAngles( yawAlias, pitchAlias, rollAlias );
 
 					// 6. Compute aliased Euler by subtracting from the last frame's Euler
 					float yawDifference			= fabsf( m_eulerLS_LastFrame.m_yawDegrees   - yawAlias   );
 					float pitchDifference		= fabsf( m_eulerLS_LastFrame.m_pitchDegrees - pitchAlias );
 					float rollDifference		= fabsf( m_eulerLS_LastFrame.m_rollDegrees	- rollAlias	 );
 					float curEulerDifference	= yawDifference + pitchDifference + rollDifference;
 
 					// 7. Compare for similarity by checking the difference between curAlias and last frame's Euler
 					if ( curEulerDifference < bestEulerDifference )
 					{
 						// Pitch is most similar
 						eulerBest			= eulerAlias;
 						bestEulerDifference	= curEulerDifference;
 					}
 				}
 			}
 			newEulerAngles = eulerBest;

//  			// Attempt 1, approach with only one base version (+Y,-P,+R) + 27 alternatives
// 			//----------------------------------------------------------------------------------------------------------------------
//  			// (Try alternatives and choose most similar to history)
//  			// 1. Compute default Euler yaw preferred solution
//  			// 2. Set best as default
//  			// 3. Create the altEulerList
//  			// 4. Loop through alternatives and update the best euler if a more similar solution is found
//  			//		try alternatives 
//  			// 5. the answer SHOULD be the best alias
//  			//----------------------------------------------------------------------------------------------------------------------
//  			// 1. Compute yawPref version
//  			float yawDegrees			= Atan2Degrees( fwdDir.y, fwdDir.x );
//  			float pitchDegrees			= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
//  			Vec3 upKbasis				= CrossProduct3D( fwdDir, leftDir );
//  			upKbasis.Normalize();
//  			float rollDegrees			= Atan2Degrees( leftDir.z, upKbasis.z );
//  			// 2. Set eulerBest as the default solution
//  			EulerAngles eulerComputed	= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
//  			EulerAngles eulerBest		= eulerComputed;
//  
//  			// 3. Init the altEulerList, (3 yaw * 3 pitch * 3 roll) variations = 27 total.
//  			std::vector<EulerAngles> altEulerList;
//  			altEulerList.emplace_back( EulerAngles(	 180.0f, -180.0f,  180.0f ) );		// 0	// Custom
// 			altEulerList.emplace_back( EulerAngles(	   0.0f,	0.0f,    0.0f ) );		// 1
// 			altEulerList.emplace_back( EulerAngles(  360.0f,	0.0f,    0.0f ) );		// 2
// 			altEulerList.emplace_back( EulerAngles( -360.0f,	0.0f,    0.0f ) );		// 3
// 			altEulerList.emplace_back( EulerAngles(    0.0f,  360.0f,    0.0f ) );		// 4
// 			altEulerList.emplace_back( EulerAngles(    0.0f, -360.0f,    0.0f ) );		// 5
// 			altEulerList.emplace_back( EulerAngles(    0.0f,	0.0F,  360.0f ) );		// 6
// 			altEulerList.emplace_back( EulerAngles(    0.0f,	0.0F, -360.0f ) );		// 7
// 			altEulerList.emplace_back( EulerAngles(  360.0f,  360.0f,    0.0f ) );		// 8
// 			altEulerList.emplace_back( EulerAngles( -360.0f,  360.0f,    0.0f ) );		// 9
// 			altEulerList.emplace_back( EulerAngles(  360.0f, -360.0f,    0.0f ) );		// 10
// 			altEulerList.emplace_back( EulerAngles( -360.0f, -360.0f,    0.0f ) );		// 11
// 			altEulerList.emplace_back( EulerAngles(  360.0f,    0.0f,  360.0f ) );		// 12
// 			altEulerList.emplace_back( EulerAngles( -360.0f,    0.0f,  360.0f ) );		// 13
// 			altEulerList.emplace_back( EulerAngles(  360.0f,    0.0f, -360.0f ) );		// 14
// 			altEulerList.emplace_back( EulerAngles( -360.0f,    0.0f, -360.0f ) );		// 15
// 			altEulerList.emplace_back( EulerAngles(	   0.0f,  360.0f,  360.0f ) );		// 16
// 			altEulerList.emplace_back( EulerAngles(	   0.0f, -360.0f,  360.0f ) );		// 17
// 			altEulerList.emplace_back( EulerAngles(	   0.0f,  360.0f, -360.0f ) );		// 18
// 			altEulerList.emplace_back( EulerAngles(	   0.0f, -360.0f, -360.0f ) );		// 19
// 			altEulerList.emplace_back( EulerAngles(	 360.0f,  360.0f,  360.0f ) );		// 20
// 			altEulerList.emplace_back( EulerAngles(	-360.0f,  360.0f,  360.0f ) );		// 21
// 			altEulerList.emplace_back( EulerAngles(	 360.0f, -360.0f,  360.0f ) );		// 22
// 			altEulerList.emplace_back( EulerAngles(	 360.0f,  360.0f, -360.0f ) );		// 23
// 			altEulerList.emplace_back( EulerAngles(	-360.0f, -360.0f,  360.0f ) );		// 24
// 			altEulerList.emplace_back( EulerAngles(	-360.0f, -360.0f, -360.0f ) );		// 25
// 			altEulerList.emplace_back( EulerAngles(	 360.0f, -360.0f, -360.0f ) );		// 26
//  			altEulerList.emplace_back( EulerAngles(	-360.0f,  360.0f, -360.0f ) );		// 27
//  
//  			// 4. Loop through alternatives and update the best Euler if a more similar solution is found
//  			float bestEulerDifference = 1040.0f;
//  			for ( int i = 0; i < altEulerList.size(); i++ )
//  			{
// 				// 5. Compute alternate Euler by combining the computedEuler with the curAltEuler
// 				EulerAngles curAltEuler		= altEulerList[i];
// 				float yawAlias				= eulerComputed.m_yawDegrees   + curAltEuler.m_yawDegrees;  
// 				float pitchAlias			= eulerComputed.m_pitchDegrees + curAltEuler.m_pitchDegrees; 
// 				float rollAlias				= eulerComputed.m_rollDegrees  + curAltEuler.m_rollDegrees; 
//  				EulerAngles eulerAlias		= EulerAngles( yawAlias, pitchAlias, rollAlias );
//  				
// 				// Test to incorporate custom solution into the alternate approach
// 				if ( i == 0 )
// 				{
// 					// Modify the base Euler by adding the custom Euler
// 					eulerAlias.m_yawDegrees    = 180.0f + yawDegrees; 
// 					eulerAlias.m_pitchDegrees  = 180.0f - pitchDegrees; 
// 					eulerAlias.m_rollDegrees   = 180.0f + rollDegrees; 
// 					// Update alias values
// 					yawAlias	= eulerAlias.m_yawDegrees;
// 					pitchAlias	= eulerAlias.m_pitchDegrees;
// 					rollAlias	= eulerAlias.m_rollDegrees;
//  					eulerAlias	= EulerAngles( yawAlias, pitchAlias, rollAlias );
// 				}
// 
// 				// 6. Compute aliased Euler by subtracting from the last frame's Euler
//  				float yawDifference			= fabsf( m_eulerLS_LastFrame.m_yawDegrees   - yawAlias   );
// 				float pitchDifference		= fabsf( m_eulerLS_LastFrame.m_pitchDegrees - pitchAlias );
// 				float rollDifference		= fabsf( m_eulerLS_LastFrame.m_rollDegrees	- rollAlias	 );
//  				float curEulerDifference	= yawDifference + pitchDifference + rollDifference;
//  			
//  				// 7. Compare for similarity by checking the difference between curAlias and last frame's Euler
//  				if ( curEulerDifference < bestEulerDifference )
//  				{
//  					// Pitch is most similar
//  					eulerBest			= eulerAlias;
// 					bestEulerDifference	= curEulerDifference;
//  				}
//  			}
// 			newEulerAngles = eulerBest;
		}
		return newEulerAngles;
	}
	else if ( m_matrixToEulerPreference == FWD_LEFT_CUSTOM )	
	{
		float sign = 1.0f;
		if ( fwdDir.x < 0.0f )
		{
			// Preserve the negative sign since we lose it 
			// by squaring the length when calculating "projectedFwdOnPlaneXY"
			sign = -1.0f;
		}
		float projectedFwdOnPlaneXY	= sqrtf( ( fwdDir.x * fwdDir.x ) + ( fwdDir.y * fwdDir.y ) );
		bool  isFacingWorldZ		= CompareIfFloatsAreEqual( projectedFwdOnPlaneXY, 0.0f, 0.0001f );
		if ( isFacingWorldZ )
		{
			// Handle Gimbal lock edge case
			// This issue occurs when our "fwdDir" is facing world -Z or Z+
			// Since Y is around the "world" Z axis and not the "kBasis", this issue occurs.
			// When this happens, we need to compute EulerAngles from our dirVectors differently
			float yawDegrees	= 0.0f;
			float pitchDegrees	= Atan2Degrees	( -fwdDir.z, projectedFwdOnPlaneXY );
			Vec3 upKbasis		= CrossProduct3D(  fwdDir,	 leftDir );
			upKbasis.Normalize();
			float rollDegrees	= Atan2Degrees( -upKbasis.y, leftDir.y );
			newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		else
		{
			//----------------------------------------------------------------------------------------------------------------------
			// (Try alternatives and choose most similar to history)
			// 1. Compute default Euler yaw preferred solution
			// 2. Modify the base Euler by adding the alternative Euler
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Compute yawPref version
			float yawDegrees			= Atan2Degrees( fwdDir.y, fwdDir.x );
			float pitchDegrees			= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
			Vec3 upKbasis				= CrossProduct3D( fwdDir, leftDir );
			upKbasis.Normalize();
			float rollDegrees			= Atan2Degrees( leftDir.z, upKbasis.z );
			EulerAngles eulerDefault	= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );

			// 2. Modify the base Euler by adding the alternative Euler
			eulerDefault.m_yawDegrees    = 180.0f + eulerDefault.m_yawDegrees; 
			eulerDefault.m_pitchDegrees  = 180.0f - eulerDefault.m_pitchDegrees; 
			eulerDefault.m_rollDegrees   = 180.0f + eulerDefault.m_rollDegrees; 
			newEulerAngles				 = eulerDefault;
		}
		return newEulerAngles;
	}

	// Default version converting from left AND up
//	else if ( m_matrixToEulerPreference == FWD_LEFT_DEFAULT )
	else
	{
		float sign = 1.0f;
		if ( fwdDir.x < 0.0f )
		{
			// Preserve the negative sign since we lose it 
			// by squaring the length when calculating "projectedFwdOnPlaneXY"
			sign = -1.0f;
		}
		float projectedFwdOnPlaneXY	= sqrtf( ( fwdDir.x * fwdDir.x ) + ( fwdDir.y * fwdDir.y ) );
		bool  isFacingWorldZ		= CompareIfFloatsAreEqual( projectedFwdOnPlaneXY, 0.0f, 0.0001f );
		if ( isFacingWorldZ )
		{
			// Handle Gimbal lock edge case
			// This issue occurs when our "fwdDir" is facing world -Z or Z+
			// Since Y is around the "world" Z axis and not the "kBasis", this issue occurs.
			// When this happens, we need to compute EulerAngles from our dirVectors differently
			float yawDegrees	= 0.0f;
			float pitchDegrees	= Atan2Degrees	( -fwdDir.z, projectedFwdOnPlaneXY );
			Vec3 upKbasis		= CrossProduct3D(  fwdDir,	 leftDir );
			upKbasis.Normalize();
			float rollDegrees	= Atan2Degrees( -upKbasis.y, leftDir.y );
			newEulerAngles		= EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		else
		{
			float yawDegrees	= Atan2Degrees  ( fwdDir.y, fwdDir.x			  );
			float pitchDegrees	= Atan2Degrees  (-fwdDir.z, projectedFwdOnPlaneXY );
			Vec3 upKbasis		= CrossProduct3D( fwdDir,	leftDir				  );
			upKbasis.Normalize();
			float rollDegrees	= Atan2Degrees( leftDir.z, upKbasis.z );
	// 		float projectedLeftOnPlaneXY	= sqrtf ( (leftDir.x * leftDir.x) + (leftDir.y * leftDir.y) );
	// 		float rollDegrees				= Atan2Degrees( leftDir.z, projectedLeftOnPlaneXY );
	// 		if ( leftDir.y < 0.0f )
	// 		{
	// 			projectedLeftOnPlaneXY *= -1.0f;
	// 			rollDegrees				= Atan2Degrees( leftDir.z, projectedLeftOnPlaneXY );
	// 			if ( rollDegrees >= 90.0f )
	// 			{
	// 				rollDegrees -= 180.0f;
	// 			}
	// 			else if ( rollDegrees <= -90.0f )
	// 			{
	// 				rollDegrees += 180.0f;
	// 			}
	// 		}
	// 
	//  		if ( sign < 0.0f )
	//  		{
	//  			// Re-compute pitch calculations
	//  			projectedFwdOnPlaneXY  *= -1.0f;
	//  			pitchDegrees			= Atan2Degrees( -fwdDir.z, projectedFwdOnPlaneXY );
	//   			if ( yawDegrees >= 90.0f )
	//   			{
	//   				yawDegrees -= 180.0f;
	//   			}
	//   			else if ( yawDegrees <= -90.0f )
	//   			{
	//   				yawDegrees += 180.0f;
	//   			}
	//  		}

			newEulerAngles = EulerAngles( yawDegrees, pitchDegrees, rollDegrees );
		}
		return newEulerAngles;
	}

//	ERROR_AND_DIE( "Matrix to Euler Preference for GetEulerFromFwdAndLeft unspecified!!!" );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::ToggleSingleStep_Forwards()
{
	m_solveSingleStep_Forwards = false;
	if ( m_parent != nullptr )
	{
		// Logic for child
		m_parent->m_solveSingleStep_Forwards = true;
	}
	else
	{
		// Logic for parent
		m_solveSingleStep_Backwards = true;
	}
	m_ikChain->m_breakFABRIK = true;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::ToggleSingleStep_Backwards()
{
	if ( m_child != nullptr )
	{
		m_solveSingleStep_Backwards		     = false;
		m_child->m_solveSingleStep_Backwards = true;
	}
	else
	{
		// Logic for end effector
		m_solveSingleStep_Backwards = false;
		m_solveSingleStep_Forwards  = true;
	}
	m_ikChain->m_breakFABRIK = true;
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::ClampYPR()
{
	m_eulerAnglesGoal_LS.m_yawDegrees	= GetClamped( m_eulerAnglesGoal_LS.m_yawDegrees,	m_yawConstraints_LS.m_min,	m_yawConstraints_LS.m_max	);
	m_eulerAnglesGoal_LS.m_pitchDegrees	= GetClamped( m_eulerAnglesGoal_LS.m_pitchDegrees, m_pitchConstraints_LS.m_min,	m_pitchConstraints_LS.m_max );
	m_eulerAnglesGoal_LS.m_rollDegrees	= GetClamped( m_eulerAnglesGoal_LS.m_rollDegrees,	m_rollConstraints_LS.m_min,	m_rollConstraints_LS.m_max  );
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::ToggleDeadlockDir_Pitch()
{
	if ( m_deadlockDir_Pitch == DEADLOCK_BEND_TOWARD )
	{
		m_deadlockDir_Pitch = DEADLOCK_BEND_AWAY;
	}
	else if ( m_deadlockDir_Pitch == DEADLOCK_BEND_AWAY )
	{
		m_deadlockDir_Pitch = DEADLOCK_BEND_TOWARD;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::ToggleDeadlockDir_Yaw()
{
	if ( m_deadlockDir_Yaw == DEADLOCK_BEND_TOWARD )
	{
		m_deadlockDir_Yaw = DEADLOCK_BEND_AWAY;
	}
	else if ( m_deadlockDir_Yaw == DEADLOCK_BEND_AWAY )
	{
		m_deadlockDir_Yaw = DEADLOCK_BEND_TOWARD;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void IK_Joint3D::LerpChainEuler_CurToGoal( float deltaSeconds, float turnRateYaw, float turnRatePitch, float turnRateRoll )
{
	// Compute turn amount this frame
	float maxYawThisFrame	= turnRateYaw	* deltaSeconds;		// 1.5f degreesThisFrame = 90.0f degreesPerSecond * 0.016f seconds
	float maxPitchThisFrame	= turnRatePitch * deltaSeconds;
	float maxRollThisFrame	= turnRateRoll	* deltaSeconds;
	// Rotate by turn amount toward goal
	m_eulerAnglesCurrent_LS.m_yawDegrees	= TurnTowardsGoal( m_eulerAnglesCurrent_LS.m_yawDegrees,	m_eulerAnglesGoal_LS.m_yawDegrees,	  maxYawThisFrame,  m_yawConstraints_LS   );
	m_eulerAnglesCurrent_LS.m_pitchDegrees	= TurnTowardsGoal( m_eulerAnglesCurrent_LS.m_pitchDegrees,	m_eulerAnglesGoal_LS.m_pitchDegrees, maxPitchThisFrame, m_pitchConstraints_LS );
	m_eulerAnglesCurrent_LS.m_rollDegrees	= TurnTowardsGoal( m_eulerAnglesCurrent_LS.m_rollDegrees,	m_eulerAnglesGoal_LS.m_rollDegrees,  maxRollThisFrame,  m_rollConstraints_LS  );
}


//----------------------------------------------------------------------------------------------------------------------
float IK_Joint3D::TurnTowardsGoal( float currentValue, float goalValue, float maxDelta, FloatRange angleConstraints )
{
	// This displacement might provide an answer greater than 180 degrees, causing the lerp to happen the long way around, to prevent this we do 
	// an additional check below to ensure the displacement is within the [-180, +180 ] range.
	float displacement					= goalValue - currentValue;
	float diplacement_shortWayAround	= displacement;

	// Only avoid lerping "long way around" if joint has NO constraints
	if ( ( angleConstraints.m_min <= -180.0f ) && ( angleConstraints.m_max >= 180.0f ) )	// check if curJoint is unconstrained
	{
		// Ensure displacement is within -180 to +180 range
		while (diplacement_shortWayAround < -180.0f) 
		{
			diplacement_shortWayAround += 360.0f;
		}
		while (diplacement_shortWayAround > 180.0f) 
		{
			diplacement_shortWayAround -= 360.0f;
		}
	}

	// Make sure we only rotate the shortest way around if we still obey the constraints
//	float diplacement_longwayAround		= displacement;
// 	float rotatedValue = 0.0f;
// 	if ( diplacement_shortWayAround < 0.0f )
// 	{
// 		rotatedValue = currentValue - maxDelta;
// 		if ( rotatedValue < angleConstraints.m_min )
// 		{
// 			diplacement_shortWayAround = diplacement_longwayAround;
// 		}
// 	}
// 	else
// 	{
// 		rotatedValue = currentValue + maxDelta;
// 		if ( rotatedValue > angleConstraints.m_max )
// 		{
// 			diplacement_shortWayAround = diplacement_longwayAround;
// 		}
// 	}

	if ( fabsf( diplacement_shortWayAround ) <= maxDelta )
	{
		// Goal is close enough to reach 
		return goalValue;
	}
	else if ( diplacement_shortWayAround < 0.0f )
	{
		// Rotate negative
		return currentValue - maxDelta;
	}
	else // disp > 0.0f
	{
		// Rotate positive
		return currentValue + maxDelta;
	}
}


//----------------------------------------------------------------------------------------------------------------------
// LS = Local Space
// WS = World Space
// MS = Model Space
//----------------------------------------------------------------------------------------------------------------------
Mat44 IK_Joint3D::GetMatrix_LocalToParent( bool useCurEulerAngles_LS ) const
{
	EulerAngles eulerToUse;
	if ( useCurEulerAngles_LS )
	{
		eulerToUse = m_eulerAnglesCurrent_LS;
	}
	else
	{
		eulerToUse = m_eulerAnglesGoal_LS;
	}
	Mat44 localToParentMatrix = eulerToUse.GetAsMatrix_XFwd_YLeft_ZUp();
	localToParentMatrix.SetTranslation3D( m_jointPos_LS );
	return localToParentMatrix;
}


//----------------------------------------------------------------------------------------------------------------------
// Starting from the root's position, apply "offsets" for each child in "relative" IJKTs
// Root to child (currentJoint)
//----------------------------------------------------------------------------------------------------------------------
Mat44 IK_Joint3D::GetMatrix_LocalToModel( Mat44 localToModelMatrix /*= Mat44()*/, bool useCurEulerAngles_LS ) const
{
	for ( int i = 0; i <= m_jointIndex; i++ )
	{
		// Start with the parent's matrix and keep appending until
		// we've reaching "this" IK_Segment
		IK_Joint3D* iteratingJoint	= m_ikChain->m_jointList[i];
		Mat44 localToParentMatrix	= iteratingJoint->GetMatrix_LocalToParent( useCurEulerAngles_LS );
		localToModelMatrix.Append( localToParentMatrix );
	}
	return localToModelMatrix;
}


//----------------------------------------------------------------------------------------------------------------------
Mat44 IK_Joint3D::GetMatrix_LocalToWorld( bool useCurEulerAngles_LS ) const
{
	Mat44 modelToWorldMatrix = m_ikChain->m_eulerAngles_WS.GetAsMatrix_XFwd_YLeft_ZUp();
	modelToWorldMatrix.SetTranslation3D( m_ikChain->m_position_WS );
	modelToWorldMatrix		 = GetMatrix_LocalToModel( modelToWorldMatrix, useCurEulerAngles_LS );
	return modelToWorldMatrix;
}


//----------------------------------------------------------------------------------------------------------------------
// parentModelToWorldMatrix is expressed is WS
Mat44 IK_Joint3D::GetMatrix_LocalToWorld( Mat44 const& parentModelToWorldMatrix, bool useCurEulerAngles_LS ) const
{
	Mat44 modelToWorldMatrix = GetMatrix_LocalToModel( parentModelToWorldMatrix, useCurEulerAngles_LS );
	return modelToWorldMatrix;
}


//----------------------------------------------------------------------------------------------------------------------
Mat44 IK_Joint3D::GetIkChainMatrix_ModelToWorld() const
{
	Mat44 modelToWorldMatrix = m_ikChain->m_eulerAngles_WS.GetAsMatrix_XFwd_YLeft_ZUp();
	modelToWorldMatrix.SetTranslation3D( m_ikChain->m_position_WS );
	return modelToWorldMatrix;
}


//----------------------------------------------------------------------------------------------------------------------
Mat44 IK_Joint3D::GetModelRootMatrix_ModelToWorld() const
{
	Mat44 modelToWorldMatrix = m_ikChain->m_creatureOwner->m_modelRoot->m_eulerAnglesGoal_LS.GetAsMatrix_XFwd_YLeft_ZUp();
	modelToWorldMatrix.SetTranslation3D( m_ikChain->m_position_WS );
	return modelToWorldMatrix;
}


//----------------------------------------------------------------------------------------------------------------------
std::string IK_Joint3D::GetEulerPreference_EnumAsString() const 
{
	std::string enumAsString			= "INVALID";
	MatrixToEulerPreference preference  = m_matrixToEulerPreference;
	if ( preference == FWD_TEMPORAL )
	{
		enumAsString = "FWD TEMPORAL";
	}
	else if ( preference == YAW_PREFERRED )
	{
		enumAsString = "YAW_PREFERRED";
	}
	else if ( preference == PITCH_PREFERRED )
	{
		enumAsString = "PITCH_PREFERRED";
	}
	else if ( preference == FWD_LEFT_TEMPORAL )
	{
		enumAsString = "FWD_LEFT_TEMPORAL";
	}
	return enumAsString;
}
