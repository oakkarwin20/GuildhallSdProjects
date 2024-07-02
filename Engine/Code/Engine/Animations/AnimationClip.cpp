#include "Engine/Animations/AnimationClip.hpp"
#include "Engine/Math/MathUtils.hpp"


//----------------------------------------------------------------------------------------------------------------------
AnimationClip::AnimationClip()
{
}


//----------------------------------------------------------------------------------------------------------------------
AnimationPose AnimationClip::SampleAnimPoseAtTime( float sampleTime, AnimationPose const& animationBindPose, bool isAnimationLooping )
{
	if ( isAnimationLooping && (sampleTime > m_endTime) )
	{
		sampleTime = m_startTime;
	}

	AnimationPose animationPoseAtTime;
	// Specify any time and get an animated pose AT THAT TIME
	for ( int jointIndex = 0; jointIndex < m_jointCurveList.size(); jointIndex++ )
	{
		JointAnimationCurve const& curJointAnimCurve = m_jointCurveList[jointIndex];
		AnimationJoint curAnimJoint = animationBindPose.m_jointList[jointIndex];

		// Position
		int numPosKeyframes = int( curJointAnimCurve.m_positionKeyframeList.size() );
		for ( int posKeyframeIndex = 0; posKeyframeIndex < numPosKeyframes; posKeyframeIndex++ )
		{
			// Update Cur joint's position at elapsed time
			Vec3Keyframe const& curJointPosKeyframe = curJointAnimCurve.m_positionKeyframeList[posKeyframeIndex];
			if ( sampleTime > curJointPosKeyframe.m_keyframeTime )
			{
				Vec3Keyframe const& lastKeyframe = curJointPosKeyframe;
				if ( posKeyframeIndex == ( numPosKeyframes - 1 ) )
				{
					// If cur index is the last pose in the curve, no need to blend
					curAnimJoint.m_localToParentTransform.m_position = curJointPosKeyframe.m_data;
				}
				else
				{
					// If there's more poses after this frame, we need to blend between this frame's pose and next frame's pose
					Vec3Keyframe const& nextKeyframe				 = curJointAnimCurve.m_positionKeyframeList[posKeyframeIndex + 1];
					float				fractionWithinRange			 = GetFractionWithinRange( sampleTime, lastKeyframe.m_keyframeTime, nextKeyframe.m_keyframeTime );
					Vec3				lerpPos						 = Interpolate( curJointPosKeyframe.m_data, nextKeyframe.m_data, fractionWithinRange );
					curAnimJoint.m_localToParentTransform.m_position = lerpPos;
				}
			}
		}

		// Rotation
		int numRotKeyframes = int( curJointAnimCurve.m_rotationKeyframeList.size() );
		for ( int rotKeyframeIndex = 0; rotKeyframeIndex < numRotKeyframes; rotKeyframeIndex++ )
		{
			// Update Cur joint's rotation at elapsed time
			QuaternionKeyframe const& curJointRotKeyframe = curJointAnimCurve.m_rotationKeyframeList[ rotKeyframeIndex ];
			if ( sampleTime > curJointRotKeyframe.m_keyframeTime )
			{
				curAnimJoint.m_localToParentTransform.m_rotation = curJointRotKeyframe.m_data;
			}
		}


		//----------------------------------------------------------------------------------------------------------------------
		// Blending quats not working
		// #ToDo: Fix quat lerping
		//----------------------------------------------------------------------------------------------------------------------
		// Rotation
// 		int numRotKeyframes = int( curJointAnimCurve.m_rotationKeyframeList.size() );
// //		for ( int rotKeyframeIndex = 0; rotKeyframeIndex < numRotKeyframes - 1; rotKeyframeIndex++ )
// 		for ( int rotKeyframeIndex = 0; rotKeyframeIndex < numRotKeyframes; rotKeyframeIndex++ )
// 		{
// 			// Update Cur joint's rotation at elapsed time
// 			QuaternionKeyframe const& curJointRotKeyframe = curJointAnimCurve.m_rotationKeyframeList[ rotKeyframeIndex ];
// //			QuaternionKeyframe const& nextKeyframe		  = curJointAnimCurve.m_rotationKeyframeList[ rotKeyframeIndex + 1 ];
// //			if ( sampleTime > curJointRotKeyframe.m_keyframeTime && sampleTime < nextKeyframe.m_keyframeTime )
// 			if ( sampleTime > curJointRotKeyframe.m_keyframeTime )
// 			{
// // 				curAnimJoint.m_localToParentTransform.m_rotation = curJointRotKeyframe.m_data;
// 				QuaternionKeyframe const& lastKeyframe = curJointRotKeyframe;
// 				if ( rotKeyframeIndex == ( numRotKeyframes - 1 ) )
// 				{
// 					// If cur index is the last pose in the curve, no need to blend
// 					curAnimJoint.m_localToParentTransform.m_rotation = curJointRotKeyframe.m_data;
// 				}
// 				else
// 				{
// 					// If there's more poses after this frame, we need to blend between this frame's pose and next frame's pose
//  					QuaternionKeyframe const& nextKeyframe			 = curJointAnimCurve.m_rotationKeyframeList[ rotKeyframeIndex + 1];
// 					float				fractionWithinRange			 = GetFractionWithinRange( sampleTime, lastKeyframe.m_keyframeTime, nextKeyframe.m_keyframeTime );
// 					Quaternion  		lerpRotation				 = Interpolate( curJointRotKeyframe.m_data, nextKeyframe.m_data, fractionWithinRange );
// 					curAnimJoint.m_localToParentTransform.m_rotation = lerpRotation;
// 				}
// 			}
// 		}

		// Scale
		int numScaleKeyframes = int( curJointAnimCurve.m_scaleKeyframeList.size() );
		for ( int scaleKeyframeIndex = 0; scaleKeyframeIndex < numScaleKeyframes; scaleKeyframeIndex++ )
		{
			// Update Cur joint's scale at elapsed time
			Vec3Keyframe const& curJointScaleKeyframe = curJointAnimCurve.m_scaleKeyframeList[ scaleKeyframeIndex ];
			if ( sampleTime > curJointScaleKeyframe.m_keyframeTime )
			{
				Vec3Keyframe const& lastKeyframe = curJointScaleKeyframe;
				if ( scaleKeyframeIndex == ( numRotKeyframes - 1 ) )
				{
					// If cur index is the last pose in the curve, no need to blend
					curAnimJoint.m_localToParentTransform.m_scale = curJointScaleKeyframe.m_data;
				}
				else
				{
					// If there's more poses after this frame, we need to blend between this frame's pose and next frame's pose
					Vec3Keyframe const& nextKeyframe			  = curJointAnimCurve.m_scaleKeyframeList[ scaleKeyframeIndex + 1];
					float				fractionWithinRange		  = GetFractionWithinRange( sampleTime, lastKeyframe.m_keyframeTime, nextKeyframe.m_keyframeTime );
					Vec3				lerpScale				  = Interpolate( curJointScaleKeyframe.m_data, nextKeyframe.m_data, fractionWithinRange );
					curAnimJoint.m_localToParentTransform.m_scale = lerpScale;
				}
			}
		}
		animationPoseAtTime.m_jointList.push_back( curAnimJoint );
	}
	return animationPoseAtTime;
}


//----------------------------------------------------------------------------------------------------------------------
void AnimationClip::ComputeStartEndTime()
{
	m_startTime = 0.0f;
	m_endTime	= 0.0f;

	// Loop through all curves and get the longest end time
	for ( int i = 0; i < m_jointCurveList.size(); i++ )
	{
		JointAnimationCurve const& curJointAnimCurve = m_jointCurveList[i];
		int maxIndex	 = int( curJointAnimCurve.m_positionKeyframeList.size() ) - 1;
		if ( maxIndex > 0 )
		{
			float posKeyTime = curJointAnimCurve.m_positionKeyframeList[ maxIndex ].m_keyframeTime;
			if ( posKeyTime > m_endTime )
			{
				m_endTime = posKeyTime;
			}
		}
		maxIndex		 = int( curJointAnimCurve.m_rotationKeyframeList.size() ) - 1;
		if ( maxIndex > 0 )
		{
			float rotKeyTime = curJointAnimCurve.m_rotationKeyframeList[ maxIndex ].m_keyframeTime;
			if ( rotKeyTime > m_endTime )
			{
				m_endTime = rotKeyTime;
			}
		}
		maxIndex		   = int( curJointAnimCurve.m_scaleKeyframeList.size() ) - 1;
		if ( maxIndex > 0 )
		{
			float scaleKeyTime = curJointAnimCurve.m_scaleKeyframeList[ maxIndex ].m_keyframeTime;
			if ( scaleKeyTime > m_endTime )
			{
				m_endTime = scaleKeyTime;
			}
		}
	}
}
