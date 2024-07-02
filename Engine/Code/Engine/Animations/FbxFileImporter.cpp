#include "Engine/Animations/AnimationClip.hpp"
#include "Engine/Animations/FbxFileImporter.hpp"
#include "Engine/Animations/AnimationPose.hpp" 
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Game/EngineBuildPreferences.hpp"
#if defined( ENGINE_ENABLE_FBXSDK )

#define FBXSDK_SHARED
#include "fbxsdk.h"

#include <queue>
#include <stack>


//----------------------------------------------------------------------------------------------------------------------
float SCALE_OF_THE_FBX_SCENE = 4.0f;


void AddJointNodeToAnimationPose( FbxNode* fbxNode, int parentIndex, AnimationPose& pose );
void InitializeFBX( std::string const& fileName, FbxScene** fbxScene, FbxManager** fbxManager );
void DestroyFBX   ( FbxScene** fbxScene, FbxManager** fbxManager );


//----------------------------------------------------------------------------------------------------------------------
void FbxFileImporter::LoadAnimationPose( std::string const& fileName, AnimationPose& newAnimationPose )
{
    FbxScene*   fbxScene    = nullptr;
    FbxManager* fbxManager  = nullptr;
    InitializeFBX( fileName, &fbxScene, &fbxManager );

    // Get the skeleton root node
    FbxNode* rootNode	= fbxScene->GetRootNode();
    int		 childCount = rootNode->GetChildCount();
    struct FbxJoint
    {
        int		 m_jointID  = -1;
        int		 m_parentID = -1;
        FbxNode* m_fbxNode  = nullptr;
    };
    std::stack<FbxJoint> jointStackList;
    for ( int i = 0; i < childCount; i++ )
    {
        FbxNode*				childNode		  = rootNode->GetChild( i );
        //FbxNodeAttribute::EType nodeAttributeType = childNode->GetNodeAttribute()->GetAttributeType();
        //		DebugPrintFbxNodeAttributeType( nodeAttributeType );

        bool isSkeletonNode = childNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::EType::eSkeleton;
        if ( isSkeletonNode )
        {
            FbxJoint curJoint; 
            curJoint.m_jointID  =  0;
            curJoint.m_parentID = -1;
            curJoint.m_fbxNode  = childNode;
            jointStackList.push( curJoint );
            break;
        }
    }

    // Get child skeletal joints
    while ( !jointStackList.empty() )
    {
        // skeletal data
        FbxJoint curJoint   = jointStackList.top();
        AddJointNodeToAnimationPose( curJoint.m_fbxNode, curJoint.m_parentID, newAnimationPose );
        curJoint.m_jointID  = int(newAnimationPose .m_jointList.size() - 1);
        jointStackList.pop();

        int childCount2 = curJoint.m_fbxNode->GetChildCount();
        for ( int i = 0; i < childCount2; i++ )
        {
            FbxNode* childNode = curJoint.m_fbxNode->GetChild( i );

            FbxJoint newJoint;
            newJoint.m_parentID = curJoint.m_jointID;
            newJoint.m_fbxNode  = childNode;
            jointStackList.push( newJoint );
        }        
    }

    DestroyFBX( &fbxScene, &fbxManager );

	// Breath first search approach 
// 	Init FBX
// 		FbxManager* fbxManager = FbxManager::Create();
// 	
// 		// define what to load from the fbx file, default is all the data
// 		FbxIOSettings* ioSettings = FbxIOSettings::Create( fbxManager, IOSROOT );
// 		fbxManager->SetIOSettings( ioSettings );
// 	
// 		// load file
// 		FbxImporter* importer	  = FbxImporter::Create( fbxManager, "Importer" );
// 		bool		 importStatus = importer->Initialize( fileName.c_str(), -1, fbxManager->GetIOSettings() );
// 		FbxScene*	 fbxScene	  = nullptr;
// 		if ( importStatus )
// 		{
// 			fbxScene = FbxScene::Create( fbxManager, "FbxScene" );
// 			importer->Import( fbxScene );
// 			importer->Destroy();
// 		}
// 		else   // error occurred
// 		{
// 			DebuggerPrintf( "Call to FbxImporter::Initialize() Failed. \n" );
// 			DebuggerPrintf( "Error returned: %s\n\n", importer->GetStatus().GetErrorString() );
// 			return;
// 		}
// 	
// 		// Change the axis (basis) system in fbx scene
// 		FbxAxisSystem defaultAxisSystem = fbxScene->GetGlobalSettings().GetAxisSystem();
// 		FbxAxisSystem sdAxisSystem		= FbxAxisSystem( FbxAxisSystem::EUpVector::eZAxis, FbxAxisSystem::eParityEven, FbxAxisSystem::eRightHanded );
// 		if ( defaultAxisSystem != sdAxisSystem )
// 		{
// 			sdAxisSystem.DeepConvertScene( fbxScene );
// 		}
// 	
// 		// Change the scale of the fbx scene
// 		FbxSystemUnit defaultUnit = fbxScene->GetGlobalSettings().GetSystemUnit();
// 		FbxSystemUnit sdUnit	  = FbxSystemUnit( SCALE_OF_THE_FB_SCENE );
// 		sdUnit.ConvertScene( fbxScene );
// 	 
// 	 	// Get the skeleton root node
// 	 	FbxNode* rootNode	= fbxScene->GetRootNode();
// 	 	int		 childCount = rootNode->GetChildCount();
// 	 	struct Joint
// 	 	{
// 	 		int		 m_jointID  = -1;
// 	 		int		 m_parentID = -1;
// 	 		FbxNode* m_fbxNode  = nullptr;
// 	 	};
// 	 	std::queue<Joint> jointQueueList;
// 	 	for ( int i = 0; i < childCount; i++ )
// 	 	{
// 	 		FbxNode*				childNode		  = rootNode->GetChild( i );
// 	 		FbxNodeAttribute::EType nodeAttributeType = childNode->GetNodeAttribute()->GetAttributeType();
// 	 //		DebugPrintFbxNodeAttributeType( nodeAttributeType );
// 	 
// 	 		bool isSkeletonNode = childNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::EType::eSkeleton;
// 	 		if ( isSkeletonNode )
// 	 		{
// 	 			Joint curJoint; 
// 	 			curJoint.m_jointID  =  0;
// 	 			curJoint.m_parentID = -1;
// 	 			curJoint.m_fbxNode  = childNode;
// 	 			jointQueueList.push( curJoint );
// 	 			break;
// 	 		}
// 	 	}
// 	 
// 	 	// Get child skeletal joints
// 	    int index = 0;
// 	 	while ( !jointQueueList.empty() )
// 	 	{
// 	 		// skeletal data
// 	 		Joint& curJoint	   = jointQueueList.front();
// 			AddJointNodeToPose( curJoint.m_fbxNode, curJoint.m_parentID, outPose );
// 	     curJoint.m_jointID = outPose.GetNumberOfJoints() - 1;
// 	        jointNames.push_back( curJoint.m_fbxNode->GetName() );
// 	        curJoint.m_jointID = index;
// 	 
// 	 		for ( int i = 0; i < curJoint.m_fbxNode->GetChildCount(); i++ )
// 	 		{
// 	 			FbxNode* childNode = curJoint.m_fbxNode->GetChild( i );
// 	 
// 	 			Joint newJoint;
// 	 			newJoint.m_parentID = curJoint.m_jointID;
// 	 			newJoint.m_fbxNode  = childNode;
// 	            jointQueueList.push( newJoint );
// 	 		}
// 	 		jointQueueList.pop();
// 	        index++;
// 	 	}
// 	
// 		fbxScene->Destroy();
// 		fbxManager->Destroy();
}

//----------------------------------------------------------------------------------------------------------------------
// Note: This function was used for TESTING, mostly.
// bindAnimationPose is the reference to use. It has all joints with names. Use the names
// to get all curve data per joint
// - elapsedAnimationTime is how far into the animation we've played the curve
//----------------------------------------------------------------------------------------------------------------------
void FbxFileImporter::LoadCurveForAllJointsAtElapsedTime( std::string const& fileName, AnimationPose& newAnimationPose, AnimationPose const& bindAnimationPose, float elapsedAnimationTime )
{
    FbxScene*   fbxScene    = nullptr;
    FbxManager* fbxManager  = nullptr;
    InitializeFBX( fileName, &fbxScene, &fbxManager );
   
    // Load curve data (all keyframes in a curve) per joint
    
    // Load the first animation stack
    // Ensure AT LEAST one animation stack exists
    int numFbxAnimationStacks = fbxScene->GetSrcObjectCount<FbxAnimStack>(); 
    if ( numFbxAnimationStacks <= 0 )
    {
//        ERROR_RECOVERABLE( Stringf( "No animation stack found, %s", fileName.c_str() ) );
        return;
    }

    // Get animation stack
    FbxAnimStack* fbxAnimStack = fbxScene->GetSrcObject<FbxAnimStack>();
    // Get animation layer from animation stack
    int numFbxAnimLayers = fbxAnimStack->GetMemberCount<FbxAnimLayer>();
    if ( numFbxAnimLayers <= 0 )
    {
//        ERROR_RECOVERABLE( "No animation layers, %s", fileName.c_str() );
        return;
    }

    FbxNode*                            rootNode            = fbxScene->GetRootNode();
    FbxAnimLayer*                       fbxAnimLayer        = fbxAnimStack->GetMember<FbxAnimLayer>();
    std::vector<AnimationJoint> const&  bindAnimJointList   = bindAnimationPose.m_jointList;
    for ( int jointIndex = 0; jointIndex < bindAnimJointList.size(); jointIndex++ )
    {
        AnimationJoint const&  curJoint     = bindAnimJointList[jointIndex];
        std::string const&     curJointName = curJoint.m_jointName; 
        FbxNode*               fbxJointNode = rootNode->FindChild( curJointName.c_str() );
        if ( fbxJointNode )
        {
            // Save all curve data of curJoint
            FbxAnimCurve* positionAnimCurve = fbxJointNode->LclTranslation.GetCurve( fbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X );
            FbxAnimCurve* rotationAnimCurve = fbxJointNode->LclRotation   .GetCurve( fbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X );
            FbxAnimCurve*    scaleAnimCurve = fbxJointNode->LclScaling    .GetCurve( fbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X );
            int           numPositionKeys   = 0;
            int           numRotationKeys   = 0;
            int           numScaleKeys      = 0;
            Transform     newJointTransform = curJoint.m_localToParentTransform;
            FbxTime       fbxTime;
            fbxTime.SetMilliSeconds( FbxLongLong(elapsedAnimationTime) );
            FbxAMatrix    fbxAMatrix        = fbxJointNode->EvaluateLocalTransform( fbxTime );
            if ( positionAnimCurve )
            {
                numPositionKeys         = positionAnimCurve->KeyGetCount();
                FbxVector4 fbxPosition  = fbxAMatrix.GetT();

                // position
                Vec3 position;
                position.x           = static_cast<float>( fbxPosition[0] );
                position.y           = static_cast<float>( fbxPosition[1] );
                position.z           = static_cast<float>( fbxPosition[2] );
                newJointTransform.m_position = position;
            }
            if ( rotationAnimCurve )
            {
                numRotationKeys           = rotationAnimCurve->KeyGetCount();
                FbxQuaternion fbxRotation = fbxAMatrix.GetQ();

                // rotation as Euler angles X, Y, Z
                Quaternion orientation;
                orientation.x        = static_cast<float>( fbxRotation[0] );
                orientation.y        = static_cast<float>( fbxRotation[1] );
                orientation.z        = static_cast<float>( fbxRotation[2] );
                orientation.w        = static_cast<float>( fbxRotation[3] );
                newJointTransform.m_rotation = orientation;
            }
            if ( scaleAnimCurve )
            {
                numScaleKeys        = scaleAnimCurve->KeyGetCount();
                FbxVector4 fbxScale = fbxAMatrix.GetS();

                // scale
                Vec3 scale;
                scale.x            = static_cast<float>( fbxScale[0] );
                scale.y            = static_cast<float>( fbxScale[1] );
                scale.z            = static_cast<float>( fbxScale[2] );
                newJointTransform.m_scale   = scale;
            }

            AnimationJoint newAnimJoint;
            newAnimJoint.m_jointName                = curJointName;
			newAnimJoint.m_parentJointIndex         = bindAnimJointList[ jointIndex ].m_parentJointIndex;
			newAnimJoint.m_localToParentTransform   = newJointTransform;
            newAnimationPose.m_jointList.push_back( newAnimJoint );
            DebuggerPrintf( "numPosKeys %d, numRotKeys %d, numScaleKeys %d, jointName %s\n", numPositionKeys, numRotationKeys, numScaleKeys, curJointName.c_str() );
        }
    }

    DestroyFBX( &fbxScene, &fbxManager );
}


//----------------------------------------------------------------------------------------------------------------------
void FbxFileImporter::LoadAnimationClip( std::string const& fileName, AnimationClip& animationClip, AnimationPose const& bindAnimationPose )
{
    FbxScene*   fbxScene    = nullptr;
    FbxManager* fbxManager  = nullptr;
    InitializeFBX( fileName, &fbxScene, &fbxManager );

    // Load the first animation stack
    // Ensure AT LEAST one animation stack exists
    int numFbxAnimationStacks = fbxScene->GetSrcObjectCount<FbxAnimStack>(); 
    if ( numFbxAnimationStacks <= 0 )
    {
//        ERROR_RECOVERABLE( Stringf( "No animation stack found, %s", fileName.c_str() ) );
        return;
    }

    // Get animation stack
    FbxAnimStack* fbxAnimStack = fbxScene->GetSrcObject<FbxAnimStack>();
    // Get animation layer from animation stack
    int numFbxAnimLayers = fbxAnimStack->GetMemberCount<FbxAnimLayer>();
    if ( numFbxAnimLayers <= 0 )
    {
//        ERROR_RECOVERABLE( "No animation layers, %s", fileName.c_str() );
        return;
    }

    FbxNode*                            rootNode            = fbxScene->GetRootNode();
    FbxAnimLayer*                       fbxAnimLayer        = fbxAnimStack->GetMember<FbxAnimLayer>();
    std::vector<AnimationJoint> const&  bindAnimJointList   = bindAnimationPose.m_jointList;
    for ( int jointIndex = 0; jointIndex < bindAnimJointList.size(); jointIndex++ )
    {
        AnimationJoint const&  curJoint     = bindAnimJointList[jointIndex];
        std::string const&     curJointName = curJoint.m_jointName; 
        FbxNode*               fbxJointNode = rootNode->FindChild( curJointName.c_str() );
        if ( fbxJointNode )
        {
            // Save all curve data of curJoint
            FbxAnimCurve* positionAnimCurve = fbxJointNode->LclTranslation.GetCurve( fbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X );
            FbxAnimCurve* rotationAnimCurve = fbxJointNode->LclRotation   .GetCurve( fbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X );
            FbxAnimCurve*    scaleAnimCurve = fbxJointNode->LclScaling    .GetCurve( fbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X );
            int           numPositionKeys   = 0;
            int           numRotationKeys   = 0;
            int           numScaleKeys      = 0;
            Transform     newJointTransform = curJoint.m_localToParentTransform;

            JointAnimationCurve jointAnimationCurve;
            if ( positionAnimCurve )
            {
                numPositionKeys = positionAnimCurve->KeyGetCount();
                jointAnimationCurve.m_positionKeyframeList.reserve( numPositionKeys );
                for ( int posKeyIndex = 0; posKeyIndex < numPositionKeys; posKeyIndex++ )
                {
                    FbxTime    fbxTime          = positionAnimCurve->KeyGetTime( posKeyIndex );
                    FbxAMatrix fbxTransform     = fbxJointNode->EvaluateLocalTransform( fbxTime );
                    FbxVector4 fbxPosition      = fbxTransform.GetT();

                    // position
                    Vec3Keyframe posKeyframe;
                    float      timeMiliSeconds  = float( fbxTime.GetMilliSeconds() );
                    posKeyframe.m_keyframeTime  = timeMiliSeconds;
                    posKeyframe.m_data.x        = static_cast<float>( fbxPosition[0] );
                    posKeyframe.m_data.y        = static_cast<float>( fbxPosition[1] );
                    posKeyframe.m_data.z        = static_cast<float>( fbxPosition[2] );
                    jointAnimationCurve.m_positionKeyframeList.push_back( posKeyframe );
                }
            }
            if ( rotationAnimCurve )
            {
                numRotationKeys           = rotationAnimCurve->KeyGetCount();
				jointAnimationCurve.m_rotationKeyframeList.reserve( numRotationKeys );
                for ( int rotKeyIndex = 0; rotKeyIndex < numRotationKeys; rotKeyIndex++ )
                {
                    // Get time based on index
                    FbxTime       fbxTime        = rotationAnimCurve->KeyGetTime( rotKeyIndex );
                    FbxAMatrix    fbxTransform   = fbxJointNode->EvaluateLocalTransform( fbxTime );
                    FbxQuaternion fbxQuaternion  = fbxTransform.GetQ();

                    // Pushback rotation (Quats) for curJoint at time from rotKeyIndex
                    QuaternionKeyframe quatKeyframe;
					float      timeMiliSeconds   = float( fbxTime.GetMilliSeconds() );
                    quatKeyframe.m_keyframeTime  = timeMiliSeconds;
                    quatKeyframe.m_data.x        = static_cast<float>( fbxQuaternion[0] );
                    quatKeyframe.m_data.y        = static_cast<float>( fbxQuaternion[1] );
                    quatKeyframe.m_data.z        = static_cast<float>( fbxQuaternion[2] );
                    quatKeyframe.m_data.w        = static_cast<float>( fbxQuaternion[3] );
                    jointAnimationCurve.m_rotationKeyframeList.push_back( quatKeyframe );
                }
            }
            if ( scaleAnimCurve )
            {
                numScaleKeys        = scaleAnimCurve->KeyGetCount();
                jointAnimationCurve.m_scaleKeyframeList.reserve( numScaleKeys );
                for ( int scaleKeyIndex = 0; scaleKeyIndex < numScaleKeys; scaleKeyIndex++ )
                {
                    // Get time based on index
                    FbxTime       fbxTime        = scaleAnimCurve->KeyGetTime( scaleKeyIndex );
                    FbxAMatrix    fbxTransform   = fbxJointNode->EvaluateLocalTransform( fbxTime );
                    FbxVector4    fbxScale       = fbxTransform.GetS();

                    // Pushback scale for curJoint at time from scaleKeyIndex
                    Vec3Keyframe scaleKeyframe;
					float      timeMiliSeconds    = float( fbxTime.GetMilliSeconds() );
                    scaleKeyframe.m_keyframeTime  = timeMiliSeconds;
                    scaleKeyframe.m_data.x        = static_cast<float>( fbxScale[0] );
                    scaleKeyframe.m_data.y        = static_cast<float>( fbxScale[1] );
                    scaleKeyframe.m_data.z        = static_cast<float>( fbxScale[2] );
                    jointAnimationCurve.m_scaleKeyframeList.push_back( scaleKeyframe );
                }
            }
            animationClip.m_jointCurveList.push_back( jointAnimationCurve );
        }
    }
    animationClip.ComputeStartEndTime();

    DestroyFBX( &fbxScene, &fbxManager );
}


//----------------------------------------------------------------------------------------------------------------------
// Functions below are declared locally
//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------
void AddJointNodeToAnimationPose( FbxNode* fbxNode, int parentIndex, AnimationPose& pose )
{
    FbxAMatrix fbxMatrix        = fbxNode->EvaluateLocalTransform( 5 );
    FbxVector4 fbxPosition      = fbxMatrix.GetT();
    FbxQuaternion fbxRotation   = fbxMatrix.GetQ();
    FbxVector4 fbxScale         = fbxMatrix.GetS();

    // position
    Vec3 position;
    position.x = static_cast<float>( fbxPosition[0] );
    position.y = static_cast<float>( fbxPosition[1] );
    position.z = static_cast<float>( fbxPosition[2] );

    // rotation as Euler angles X, Y, Z
    Quaternion orientation;
    orientation.x = static_cast<float>( fbxRotation[0] );
    orientation.y = static_cast<float>( fbxRotation[1] );
    orientation.z = static_cast<float>( fbxRotation[2] );
    orientation.w = static_cast<float>( fbxRotation[3] );

    // scale
    Vec3 scale;
    scale.x = static_cast<float>( fbxScale[0] );
    scale.y = static_cast<float>( fbxScale[1] );
    scale.z = static_cast<float>( fbxScale[2] );

    Transform transform;
    transform.m_position = position;
    transform.m_rotation = orientation;
    transform.m_scale    = scale;

    std::string jointName = fbxNode->GetName();

    AnimationJoint joint;
    joint.m_jointName               = jointName;
    joint.m_localToParentTransform  = transform;
    joint.m_parentJointIndex        = parentIndex;

    pose.m_jointList.push_back(joint);

//     FbxAMatrix fbxMatrix    = fbxNode->EvaluateLocalTransform();
//     FbxVector4 fbxPosition  = fbxMatrix.GetT();
//     FbxQuaternion fbxRotation  = fbxMatrix.GetQ();
//     FbxVector4 fbxScale     = fbxMatrix.GetS();
// 
//     // position
//     Vec3 position;
//     position.x = static_cast<float>( fbxPosition[0] );
//     position.y = static_cast<float>( fbxPosition[1] );
//     position.z = static_cast<float>( fbxPosition[2] );
// 
//     // rotation as Euler angles X, Y, Z
//     EulerAngles orientation;
//     orientation.m_rollDegrees   = static_cast<float>( fbxRotation[0] );
//     orientation.m_pitchDegrees  = static_cast<float>( fbxRotation[1] );
//     orientation.m_yawDegrees    = static_cast<float>( fbxRotation[2] );
// 
//     // scale
//     Vec3 scale;
//     scale.x = static_cast<float>( fbxScale[0] );
//     scale.y = static_cast<float>( fbxScale[1] );
//     scale.z = static_cast<float>( fbxScale[2] );
// 
//     Mat44 matrixTransform = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
//     matrixTransform.SetTranslation3D(position);
//     matrixTransform.AppendScaleNonUniform3D(scale);
// 
//     std::string jointName = fbxNode->GetName();
// 
//     AnimationJoint joint;
//     joint.m_jointName = jointName;
//     joint.m_localToParentTransform = matrixTransform;
//     joint.m_parentJointIndex = parentIndex;
// 
//     pose.m_jointList.push_back(joint);
}


//----------------------------------------------------------------------------------------------------------------------
void InitializeFBX( std::string const& fileName, FbxScene** fbxScene, FbxManager** fbxManager )
{
    // Depth first search approach
    // Init FBX
    *fbxManager = FbxManager::Create();

    // define what to load from the fbx file, default is all the data
    FbxIOSettings* ioSettings = FbxIOSettings::Create( *fbxManager, IOSROOT );
    (*fbxManager)->SetIOSettings( ioSettings );

    // load file
    FbxImporter* importer	  = FbxImporter::Create( *fbxManager, "Importer" );
    bool		 importStatus = importer->Initialize( fileName.c_str(), -1, (*fbxManager)->GetIOSettings() );
    *fbxScene	  = nullptr;
    if ( importStatus )
    {
        *fbxScene = FbxScene::Create( *fbxManager, "FbxScene" );
        importer->Import( *fbxScene );
        importer->Destroy();
    }
    else   // error occurred
    {
//		ERROR_RECOVERABLE( "Error returned: %s\n\n", importer->GetStatus().GetErrorString() );
        DebuggerPrintf( "Call to FbxImporter::Initialize() Failed. \n" );
        DebuggerPrintf( "Error returned: %s\n\n", importer->GetStatus().GetErrorString() );
        return;
    }

    // Change the axis (basis) system in fbx scene
    FbxAxisSystem defaultAxisSystem = (*fbxScene)->GetGlobalSettings().GetAxisSystem();
    FbxAxisSystem sdAxisSystem		= FbxAxisSystem( FbxAxisSystem::EUpVector::eZAxis, FbxAxisSystem::eParityEven, FbxAxisSystem::eRightHanded );
    if ( defaultAxisSystem != sdAxisSystem )
    {
        sdAxisSystem.DeepConvertScene( *fbxScene );
    }

    // Change the scale of the fbx scene
    FbxSystemUnit defaultUnit = (*fbxScene)->GetGlobalSettings().GetSystemUnit();
    FbxSystemUnit sdUnit	  = FbxSystemUnit( SCALE_OF_THE_FBX_SCENE );
    sdUnit.ConvertScene( *fbxScene );
}


//----------------------------------------------------------------------------------------------------------------------
void DestroyFBX( FbxScene** fbxScene, FbxManager** fbxManager )
{
	(*fbxScene)  ->Destroy();
	(*fbxManager)->Destroy();
}

#endif      // ENGINE_ENABLE_FBXSDK