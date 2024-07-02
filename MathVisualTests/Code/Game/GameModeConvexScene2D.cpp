#include "Game/GameModeConvexScene2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/TreeNode_AABB2_BVH.hpp"

#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/OBB2D.hpp"
#include "Engine/Math/OBB3D.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/BufferWriter.hpp"
#include "Engine/Core/BufferParser.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/BufferParser.hpp"
#include "Engine/Core/BufferWriter.hpp"


//----------------------------------------------------------------------------------------------------------------------
GameModeConvexScene2D::GameModeConvexScene2D()
{
	// Buffer writer test case
// 	std::vector<unsigned char> buffer;
// 	BufferWriter bufferWriter = BufferWriter( buffer );
// 	bool			boolean1	= false;
// 	bool			boolean2	= true;
//  	float			f			= 2.0f;
//  	unsigned char	b			= 'a';
// 	unsigned char   uChar		= 'b';
// 	double			d			= 55;
//  	short			s			= -5;
// 	unsigned short	us			= 25;
// 	int				i			= 13;
// 	int64_t			i64			= -105;
// 	unsigned int	ui			= 255;
// 	uint64_t		ui64		= 45;
// 	Vec2			vec2		= Vec2(	  20.0f, 2.0f );
// 	Vec3			vec3		= Vec3(	  20.0f, 2.0f, 2000.0f );
// 	Vec4			vec4		= Vec4( 2000.0f, 2.0f, 20.0f, 0.0f );
// 	IntVec2			intVec2		= IntVec2( 20, 2 );
// 	IntVec3			intVec3		= IntVec3( 20, 2, 2000 );
// 	Rgba8			color1		= Rgba8( 127, 217, 200, 222 );
// 	Rgba8			color2		= Rgba8( 127, 217, 200, 0 );
// 	AABB2			box2D		= AABB2( vec2, Vec2( 2.0f, 80.0f ) ); 
// 	AABB3			box3D		= AABB3( vec3, Vec3( 2.0f, 80.0f, 200.0f ) );
// 	Plane2D			plane2D		= Plane2D( Vec2( 0.6f, 0.8f ), -5.0f );
// 	Vertex_PCU		vertex_PCU	= Vertex_PCU( Vec3( 2.0f, 1.0f, -1.0f ), color1, Vec2( -5.0f, -2.0f ) );
// 	OBB2D			obb2		= OBB2D( vec2, Vec2(-1.0f, -2.0f), Vec2( -40.0f, 80.0f ) );
// 	OBB3D			obb3		= OBB3D( vec3, Vec3(-1.0f, -2.0f, -3.0f), Vec3( -40.0f, 80.0f, 2.4f ), Vec3( 1.1f, 2.2f, 3.3f), Vec3(4.4f, 5.5f, 6.6f ) );
// 	bufferWriter.AppendBool( boolean1 );
// 	bufferWriter.AppendBool( boolean2 );
// 	bufferWriter.AppendFloat( f );
//  	bufferWriter.AppendByte(b);
//  	bufferWriter.AppendChar(uChar);
// 	bufferWriter.AppendDouble(d);
// 	bufferWriter.AppendShort(s);
// 	bufferWriter.AppendUnsignedShort( us );
// 	bufferWriter.AppendInt(i);
// 	bufferWriter.AppendInt64( i64 );
// 	bufferWriter.AppendUnsignedInt( ui );
// 	bufferWriter.AppendUnsignedInt64( ui64 );
// 	bufferWriter.AppendVec2( vec2 );
// 	bufferWriter.AppendVec3( vec3 );
// 	bufferWriter.AppendVec4( vec4 );
// 	bufferWriter.AppendIntVec2( intVec2 );
// 	bufferWriter.AppendIntVec3( intVec3 );
// 	bufferWriter.AppendRgba8( color1 );
// 	bufferWriter.AppendRgb( color2 );
// 	bufferWriter.AppendAABB2( box2D );
// 	bufferWriter.AppendAABB3( box3D );
// 	bufferWriter.AppendPlane2D( plane2D );
// 	bufferWriter.AppendVertex_PCU( vertex_PCU );
// 	bufferWriter.AppendObb2D( obb2 );
// 	bufferWriter.AppendObb3D( obb3 );
	// Parsing test case
// 	BufferParser bufferParser	= BufferParser( bufferWriter.m_buffer.data(), unsigned char( bufferWriter.m_buffer.size() ) );
// 	boolean1					= bufferParser.ParseBool();
// 	boolean2					= bufferParser.ParseBool();
//  	f							= bufferParser.ParseFloat();
// 	b							= bufferParser.ParseByte();
// 	uChar						= bufferParser.ParseChar();
// 	d							= bufferParser.ParseDouble();
// 	s							= bufferParser.ParseShort();
// 	us							= bufferParser.ParseUnsignedShort();
// 	i							= bufferParser.ParseInt32();
//  	i64							= bufferParser.ParseInt64();
// 	ui							= bufferParser.ParseUnsignedInt32();
//  	ui64						= bufferParser.ParseUnsignedInt64();
// 	vec2						= bufferParser.ParseVec2();
// 	vec3						= bufferParser.ParseVec3();
// 	vec4						= bufferParser.ParseVec4();
// 	intVec2						= bufferParser.ParseIntVec2();
// 	intVec3						= bufferParser.ParseIntVec3();
// 	color1						= bufferParser.ParseRgba8();
// 	color2						= bufferParser.ParseRgb();
// 	box2D						= bufferParser.ParseAABB2();
// 	box3D						= bufferParser.ParseAABB3();
// 	plane2D						= bufferParser.ParsePlane2D();
// 	vertex_PCU					= bufferParser.ParseVertex_PCU();
// 	obb2						= bufferParser.ParseOBB2D();
// 	obb3						= bufferParser.ParseOBB3D();
	std::vector<unsigned char> buffer;
	BufferEndian endianMode   = LITTLE;
	BufferWriter bufferWriter = BufferWriter( buffer, endianMode );
	AppendTestFileBufferData( bufferWriter, endianMode );
	BufferParser bufferParser = BufferParser( buffer.data(), unsigned char( buffer.size() ), endianMode );
	ParseTestFileBufferData( bufferParser, endianMode );
}


//----------------------------------------------------------------------------------------------------------------------
GameModeConvexScene2D::~GameModeConvexScene2D()
{
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::Startup()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Init convex objects
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < 2; i++ )
	{
		// Create new gameConvex
		GameConvexObject* newGameConvexObject = new GameConvexObject();
		newGameConvexObject->RandomizeConvexPosAndShape();
		Vec2 randDiscCenter		= Vec2::ZERO;
		randDiscCenter.x		= g_theRNG->RollRandomFloatInRange( 10.0f, 190.0f );
		randDiscCenter.y		= g_theRNG->RollRandomFloatInRange(  5.0f,  95.0f );
		float randRadius		= g_theRNG->RollRandomFloatInRange( 11.0f,  35.0f );
		newGameConvexObject->RandomizeConvexPolyShape( randDiscCenter, randRadius );
		m_gameConvexObjectList.push_back( newGameConvexObject );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Loading test
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < m_gameConvexObjectList.size(); i++ )
 	{
 		delete m_gameConvexObjectList[ i ];
 	}
 	m_gameConvexObjectList.clear();
 	std::vector<uint8_t> squirrelBuffer;
	FileReadToBuffer( squirrelBuffer, "Data/ConvexScenes/pikachu.ghcs" );
// 	FileReadToBuffer( squirrelBuffer, "Data/ConvexScenes/TwoObjects.ghcs" );
//	FileReadToBuffer( squirrelBuffer, "Data/ConvexScenes/OakFile.ghcs" );
 	ParseConvexSceneFromFile( squirrelBuffer );

 //	FileReadToBuffer( squirrelBuffer, "Data/ConvexScenes/pikachu2.ghcs" );
 //	FileReadToBuffer( squirrelBuffer, "Data/ConvexScenes/Peter8.ghcs" );
//  FileReadToBuffer( squirrelBuffer, "Data/ConvexScenes/JamieFile.ghcs" );

	//----------------------------------------------------------------------------------------------------------------------
	// Init treeNodes
	//----------------------------------------------------------------------------------------------------------------------
	m_rootTreeNode = new TreeNode_AABB2_BVH( m_gameConvexObjectList );
	m_rootTreeNode->SplitAndCreateChildNodes( 7 );

	//----------------------------------------------------------------------------------------------------------------------
	// Test file saving
	//----------------------------------------------------------------------------------------------------------------------
//    	std::vector<unsigned char> bufferToWrite;
//    	BufferWriter bufferWriter = BufferWriter( bufferToWrite );
//    	bufferWriter.SetEndianMode( LITTLE );
//    	AppendHeader( bufferWriter );
//    	AppendChunkData( bufferWriter, SCENE_INFO );
//    	AppendChunkData( bufferWriter, CONVEX_POLY_2D );
//    	AppendChunkData( bufferWriter, BOUNDING_DISCS_2D );
//    	AppendTOC( bufferWriter );
//    	std::string newFileName = "Data/ConvexScenes/OakFile.GHCS";
//    	WriteBinaryBufferToFile( bufferToWrite, newFileName );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::Update( float deltaSeconds )
{
	deltaSeconds = m_gameClock.GetDeltaSeconds();
	UpdateGameCamera_ConvexScene2D();
	UpdateGeneralInput( deltaSeconds );
	UpdateHotkeysInput( deltaSeconds );
	RegenerateBVH();
	Update1000Rays();
	UpdateRaycastResult();
	UpdatePauseQuitAndSlowMo();
	// Updating ray results
	m_rayFwd	= ( m_rayEndPos - m_rayStartPos ).GetNormalized();
	m_rayLength	= ( m_rayEndPos - m_rayStartPos ).GetLength();
	// Scaling data if world bounds have changed
	float height;
	if ( m_shouldChangeWorldBounds )
	{
		height				= g_parsedWorldBounds.GetDimensions().y;
	}
	else
	{
		height				= WORLD_SIZE_Y;
	}
	m_arrowSize				= height * 0.01f;
	m_arrowThickness		= height * 0.005f;
	m_rayImpactDiscRadius	= height * 0.01f;
	m_rayImpactNormalScalar = height * 0.05f;
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::Render() const
{
	RenderWorldObjects();
	RenderUIObjects();
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::Reshuffle()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::UpdateGameCamera_ConvexScene2D()
{
	if ( m_shouldChangeWorldBounds )
	{
		m_worldCamera_ConvexScene2D.SetOrthoView( g_parsedWorldBounds.m_mins, g_parsedWorldBounds.m_maxs );
	}
	else
	{
		m_worldCamera_ConvexScene2D.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
	}
	m_UiCamera_ConvexScene2D.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::RenderWorldObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin World Camera
	g_theRenderer->BeginCamera( m_worldCamera_ConvexScene2D );

	std::vector<Vertex_PCU> verts;
	verts.reserve( 3'000'000 );

	//----------------------------------------------------------------------------------------------------------------------
	// Render "hard coded" convex poly with borders
	//----------------------------------------------------------------------------------------------------------------------
// 	std::vector<Vec2> ccwOrderedPoints;
// 	ccwOrderedPoints.push_back( Vec2( 50.0f, 25.0f ) );
// 	ccwOrderedPoints.push_back( Vec2( 75.0f, 25.0f ) );
// 	ccwOrderedPoints.push_back( Vec2( 80.0f, 45.0f ) );
// 	ccwOrderedPoints.push_back( Vec2( 80.0f, 65.0f ) );
// 	ccwOrderedPoints.push_back( Vec2( 60.0f, 65.0f ) );
// 	ccwOrderedPoints.push_back( Vec2( 40.0f, 45.0f ) );
// 	ConvexPoly2D convexPoly2D = ConvexPoly2D( ccwOrderedPoints );
// 	AddVertsForConvexPoly2D_Border( verts, convexPoly2D );

	//----------------------------------------------------------------------------------------------------------------------
	// Toggle convex alpha value
	//----------------------------------------------------------------------------------------------------------------------
	unsigned char alpha = 255;
	Rgba8 color			= Rgba8::MAGENTA;
	Rgba8 borderColor	= Rgba8::GRAY;
	if ( !m_renderConvexOpaque_F2 )
	{
		alpha	= 127;
		color.a = alpha;
	}
// 	bool isPointInsideConvexPoly = IsPointInsideConvexPoly2D( m_rayStartPos, convexPoly2D );
//  	if ( isPointInsideConvexPoly )
//  	{
// 		color	= Rgba8::YELLOW;
// 		color.a	= alpha;
// 		AddVertsForConvexPoly2D( verts, convexPoly2D, color );
//  	}
//  	else
//  	{
//  		AddVertsForConvexPoly2D( verts, convexPoly2D, color );
//  	}

	//----------------------------------------------------------------------------------------------------------------------
	// Rendering all poly(s) borders first
	for ( int i = 0; i < m_gameConvexObjectList.size(); i++ )
	{
		GameConvexObject const* curConvex	  = m_gameConvexObjectList[ i ];
		ConvexPoly2D	 const& curConvexPoly = curConvex->GetConvexPoly();
		AddVertsForConvexPoly2D_Border( verts, curConvexPoly, borderColor );
	}
	// Rendering all poly(s) fill second
	for ( int i = 0; i < m_gameConvexObjectList.size(); i++ )
	{
		GameConvexObject const* curConvex	  = m_gameConvexObjectList[ i ];
		ConvexPoly2D	 const& curConvexPoly = curConvex->GetConvexPoly();
		AddVertsForConvexPoly2D( verts, curConvexPoly, color );
	}


// 	//----------------------------------------------------------------------------------------------------------------------
// 	// Make convexHull from convexPoly and render planes
// 	//----------------------------------------------------------------------------------------------------------------------
// 	GameConvexObject gameConvexObject	= GameConvexObject( convexPoly2D, Vec2::ZERO, 1.0f );
// 	ConvexHull2D const& convexHull		= gameConvexObject.GetConvexHull();
// 	AddVertsForConvexHull2D_Planes( verts, convexHull, 1000.0f, Rgba8::CYAN, 0.2f, true, 5.0f );
	

	//----------------------------------------------------------------------------------------------------------------------
	// Render hull planes if only 1 poly exists
	//----------------------------------------------------------------------------------------------------------------------
	int numPoly = int( m_gameConvexObjectList.size() );
	if ( numPoly == 1 )
	{
		GameConvexObject const* gameConvexObject	= m_gameConvexObjectList[0];
		ConvexHull2D const& convexHull				= gameConvexObject->GetConvexHull();
		AddVertsForConvexHull2D_Planes( verts, convexHull, 1000.0f, Rgba8::CYAN, 0.2f, true, 5.0f );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Is poly "hovered" Test (by cursor)
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_hoveredConvexObject != nullptr )
	{
		color = Rgba8::RED;
		color.a = alpha;
		AddVertsForConvexPoly2D_Border( verts, m_hoveredConvexObject->GetConvexPoly(), borderColor );
		AddVertsForConvexPoly2D( verts, m_hoveredConvexObject->GetConvexPoly(), color );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Is gameObject "selected" Test 
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_selectedConvexObject != nullptr )
	{
		color = Rgba8::YELLOW;
		color.a = alpha;
		AddVertsForConvexPoly2D_Border( verts, m_selectedConvexObject->GetConvexPoly(), borderColor );
		AddVertsForConvexPoly2D( verts, m_selectedConvexObject->GetConvexPoly(), color );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// F1 debugRenderBoundingDisc
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_debugRenderBoundingDisc_F1 )
	{
		for ( int i = 0; i < m_gameConvexObjectList.size(); i++ )
		{
			GameConvexObject const* curConvex = m_gameConvexObjectList[ i ];
			curConvex->DebugRenderBoundingDisc( verts, m_arrowThickness, Rgba8::CYAN );
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Broad phase optimizations using Bounding Volume Hierarchies (BVH)
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_debugRenderBoundsBVH_F4 )
	{
		std::queue<TreeNode_AABB2_BVH*> treeList;
		treeList.push( m_rootTreeNode );
		bool isParent = true;
		while ( !treeList.empty() )
		{
			TreeNode_AABB2_BVH* curTreeNode = treeList.front();
			treeList.pop();
			Rgba8 colorABB2Tree = Rgba8::YELLOW;
			if ( !isParent )
			{
				if ( curTreeNode->m_leftChild == nullptr && curTreeNode->m_rightChild == nullptr )
				{
					// Node with no children
					colorABB2Tree = Rgba8::RED;
				}
				else if ( curTreeNode->m_leftChild == nullptr || curTreeNode->m_rightChild == nullptr )
				{
					// Node with one missing child
					colorABB2Tree = Rgba8::GREEN;
				}
				else if ( curTreeNode->m_leftChild != nullptr && curTreeNode->m_rightChild != nullptr )
				{
					// Last node with both children
					colorABB2Tree = Rgba8::LIGHTBLUE;
				}
			}
			AddVertsForBordersAABB2D( verts, curTreeNode->m_bounds, m_arrowThickness, colorABB2Tree );
			isParent = false;
			// Push child into queue
			if ( curTreeNode->m_leftChild )
			{
				treeList.push( curTreeNode->m_leftChild );
			}
			if ( curTreeNode->m_rightChild )
			{
				treeList.push( curTreeNode->m_rightChild );
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render debug rays
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < m_debugRayResultsList.size(); i++ )
	{
		RaycastResult2D const& currentRay = m_debugRayResultsList[i];
		// Render actual rays
		AddVertsForArrow2D( verts, currentRay.m_rayStartPos, currentRay.m_rayEndPos, m_arrowSize, m_arrowThickness, Rgba8::LIGHTBLUE );
		// Render impact normal if hit
		if ( currentRay.m_didImpact )
		{
			Vec2 impactNormalEnd = currentRay.m_impactPos + ( currentRay.m_impactNormal * m_rayImpactNormalScalar );
			AddVertsForArrow2D( verts, currentRay.m_impactPos, impactNormalEnd, m_arrowSize, m_arrowThickness, Rgba8::INDIGO );
			AddVertsForDisc2D( verts, currentRay.m_impactPos, m_rayImpactDiscRadius, Rgba8::BROWN );
		}
	}


	//----------------------------------------------------------------------------------------------------------------------
	// RayVsConvexHull2D test
	//----------------------------------------------------------------------------------------------------------------------
	// 	RaycastResult2D rayResult = RaycastVsConvexHull2D( m_rayStartPos, m_rayFwd, m_rayLength, convexHull );
	// 	if ( rayResult.m_didImpact )
	// 	{
	// 		AddVertsForDisc2D( verts, rayResult.m_impactPos, 1.0f, Rgba8::WHITE );
	// 		Vec2 impactNormalEnd = rayResult.m_impactPos + ( rayResult.m_impactNormal * 5.0f );
	// 		AddVertsForArrow2D( verts, rayResult.m_impactPos, impactNormalEnd, 1.0f, 0.5f, Rgba8::DARK_BLUE );
	// 	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render Raycast arrow
	// Ray start to end with no impact
	AddVertsForArrow2D( verts, m_rayStartPos, m_rayEndPos, m_arrowSize, m_arrowThickness, m_rayDefaultColor );

	//----------------------------------------------------------------------------------------------------------------------
	// RayVsConvexObjects (Get nearest poly hit by ray)
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_rayConvexResult2D.m_didImpact )
	{
		Vec2 impactNormalEnd = m_rayConvexResult2D.m_impactPos + ( m_rayConvexResult2D.m_impactNormal * m_rayImpactNormalScalar );
		AddVertsForArrow2D( verts, m_rayConvexResult2D.m_impactPos, impactNormalEnd, m_arrowSize, m_arrowThickness, Rgba8::DARK_BLUE );
		AddVertsForDisc2D( verts, m_rayConvexResult2D.m_impactPos, m_rayImpactDiscRadius, Rgba8::WHITE );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Render ring at Raycast arrow startPos
	AddVertsForRing2D( verts, m_rayStartPos, m_arrowSize, m_arrowThickness, Rgba8::LIGHTBLUE );


	// Render world center for reference
	if ( m_shouldChangeWorldBounds )
	{
		Vec2 worldCenter = g_parsedWorldBounds.GetCenter();
		AddVertsForDisc2D( verts, worldCenter, m_rayImpactNormalScalar, Rgba8::WHITE );
	}
	else
	{
		AddVertsForDisc2D( verts, Vec2( 100.0f, 50.0f ), m_rayImpactNormalScalar, Rgba8::WHITE );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Draw call for World camera
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts.size() ), verts.data() );

	// End World Camera
	g_theRenderer->EndCamera( m_worldCamera_ConvexScene2D );
}

//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::RenderUIObjects() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_UiCamera_ConvexScene2D );

	//----------------------------------------------------------------------------------------------------------------------
	// Get or Create font
	std::vector<Vertex_PCU> textVerts;
	float cellHeight	= 2.0f;
	AABB2 textbox1		= AABB2( Vec2( 0.0f, 0.0f ), Vec2( WORLD_SIZE_X, WORLD_SIZE_Y - 1.0f ) );
	AABB2 textbox2		= AABB2( textbox1.m_mins, Vec2( textbox1.m_maxs.x, textbox1.m_maxs.y - cellHeight ) );
	float deltaSeconds	= m_gameClock.GetDeltaSeconds();
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox1, cellHeight, Stringf( " Mode (F6/F7 for prev/next): Convex Scene (2D)                                                                                             ds = %.2fms", deltaSeconds * 1000.0f ), Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );

	std::string	UIString		= Stringf( "F1: Discs | F2: Alpha | F8: Randomize | S/E Move ray | W/R: Rotate | K/L: Scale | LMB: Translate | Comma/Period: Change num convex" );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, UIString, Rgba8::GREEN, 0.75f, Vec2( 0.0f, 1.0f ), TextDrawMode::SHRINK_TO_FIT );	
	int numConvexObjects		 = int( m_gameConvexObjectList.size() );
	std::string optimizationType = GetEnumAsString();
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, Stringf( " 1: Generate debugRays | 2: Remove rendering debugRays" ), Rgba8::GREEN, 0.75f, Vec2( 0.0f, 0.98f ), TextDrawMode::SHRINK_TO_FIT );
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, Stringf( " Num ConvexObjects: %d, OptimizationType %s", numConvexObjects, optimizationType.c_str() ), Rgba8::YELLOW, 0.75f, Vec2( 0.0f, 0.96f ), TextDrawMode::SHRINK_TO_FIT );
	float totalTimeDebugRays	= m_timeAfterDebugRays - m_timeBeforeDebugRays;
	g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, textbox2, cellHeight, Stringf( " DebugRays (%d) computationTime in sec(s): %0.2f, timeBefore: %0.2f, timeAfter: %0.2f", m_numRandRays, totalTimeDebugRays, m_timeBeforeDebugRays, m_timeAfterDebugRays ), Rgba8::CYAN, 0.75f, Vec2( 0.0f, 0.94f ), TextDrawMode::SHRINK_TO_FIT );


	//----------------------------------------------------------------------------------------------------------------------
	// End UI Camera
	g_theRenderer->EndCamera( m_UiCamera_ConvexScene2D );

	//----------------------------------------------------------------------------------------------------------------------
	// Draw for UI camera
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( static_cast<int>( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::UpdateGeneralInput( float deltaSeconds )
{
	// Pre-compute cursor pos since its commonly used
	Vec2 cursorPos		= g_theWindow->GetNormalizedCursorPos();
	float lerpedX		= Interpolate( 0.0f, WORLD_SIZE_X, cursorPos.x );
	float lerpedY		= Interpolate( 0.0f, WORLD_SIZE_Y, cursorPos.y );
	if ( m_shouldChangeWorldBounds )
	{
		lerpedX			= Interpolate( g_parsedWorldBounds.m_mins.x, g_parsedWorldBounds.m_maxs.x, cursorPos.x  );
		lerpedY			= Interpolate( g_parsedWorldBounds.m_mins.y, g_parsedWorldBounds.m_maxs.y, cursorPos.y  );
	}
	cursorPos			= Vec2( lerpedX, lerpedY );

	//----------------------------------------------------------------------------------------------------------------------
	// Double or half num gameConvexObjects
	//----------------------------------------------------------------------------------------------------------------------
	int numObjects = int( m_gameConvexObjectList.size() );
	// Double
	if ( g_theInput->WasKeyJustPressed( KEYCODE_PERIOD ) )
	{
		// Loop numObjects times and create new poly(s)
		for ( int i = 0; i < numObjects; i++ )
		{
			GameConvexObject* newGameObject = new GameConvexObject();
			newGameObject->RandomizeConvexPosAndShape();
			m_gameConvexObjectList.push_back( newGameObject );
		}
		ToggleRegenerateBVH();
	}
	// Half 
	if ( g_theInput->WasKeyJustPressed( KEYCODE_COMMA ) )
	{
		if ( numObjects == 1 )
		{
			// Early out check to keep at least one poly in the scene
			return;
		}
		// Resize the vector (gameObjectList) to half will remove all objects at the end
		int newNumObjects	   = numObjects / 2;
		m_gameConvexObjectList.resize( newNumObjects );
		m_selectedConvexObject = nullptr;
		m_hoveredConvexObject  = nullptr;
		ToggleRegenerateBVH();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Double or half num randRaycasts
	//----------------------------------------------------------------------------------------------------------------------
	// Double
	if ( g_theInput->WasKeyJustPressed( 'M' ) )
	{
		if ( m_numRandRays == 0 )
		{
			// Early out check to keep at least one poly in the scene
			m_numRandRays = 1;
			return;
		}
		m_numRandRays *= 2;
		ToggleRegenerateBVH();
	}
	// Half 
	if ( g_theInput->WasKeyJustPressed( 'N' ) )
	{
		if ( m_numRandRays == 0 )
		{
			// Early out check to keep at least one poly in the scene
			return;
		}
		if ( m_numRandRays == 1 )
		{
			// Early out check to keep at least one poly in the scene
			m_numRandRays = 0;
			return;
		}
		// Resize the vector (gameObjectList) to half will remove all objects at the end
		m_numRandRays /= 2;
		ToggleRegenerateBVH();
	}


	//----------------------------------------------------------------------------------------------------------------------
	// Select GameConvexObject
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_LEFT_MOUSE ) )
	{
		// Try select convex
		if ( m_selectedConvexObject == nullptr )
		{
			SelectConvexObject( m_selectedConvexObject, cursorPos );
		}
		// Translate convex IF selected
		if ( m_selectedConvexObject != nullptr )
		{
			m_selectedConvexObject->Translate( cursorPos, deltaSeconds );
			ToggleRegenerateBVH();
		}
	}
	//----------------------------------------------------------------------------------------------------------------------
	// De-Select GameConvexObject
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->WasKeyJustReleased( KEYCODE_LEFT_MOUSE ) )
	{
		m_selectedConvexObject = nullptr;
		ToggleRegenerateBVH();
	}
	//----------------------------------------------------------------------------------------------------------------------
	// Rotating gameConvexObject
	//----------------------------------------------------------------------------------------------------------------------
	float rotateDegrees = 75.0f;
	if ( g_theInput->IsKeyDown( 'W' ) )
	{
		SelectConvexObject( m_selectedConvexObject, cursorPos );
		// Rotate CCW IF selected
		if ( m_selectedConvexObject != nullptr )
		{
			m_selectedConvexObject->Rotate( cursorPos, rotateDegrees, deltaSeconds );
			ToggleRegenerateBVH();
		}
	}
	if ( g_theInput->IsKeyDown( 'R' ) )
	{
		// Rotate CW
		SelectConvexObject( m_selectedConvexObject, cursorPos );
		// Rotate CCW IF selected
		if ( m_selectedConvexObject != nullptr )
		{
			rotateDegrees = -75.0f;
			m_selectedConvexObject->Rotate( cursorPos, rotateDegrees, deltaSeconds );
			ToggleRegenerateBVH();
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Scaling gameConvexObject
	//----------------------------------------------------------------------------------------------------------------------
	float scaleAmount = -0.4f;
	if ( g_theInput->IsKeyDown( 'K' ) )
	{
		// Deflate IF selected
		SelectConvexObject( m_selectedConvexObject, cursorPos );
		if ( m_selectedConvexObject != nullptr )
		{
			m_selectedConvexObject->Scale( cursorPos, scaleAmount, deltaSeconds );
			ToggleRegenerateBVH();
		}
	}
	if ( g_theInput->IsKeyDown( 'L' ) )
	{
		scaleAmount = 0.4f;
		// Inflate IF selected
		SelectConvexObject( m_selectedConvexObject, cursorPos );
		if ( m_selectedConvexObject != nullptr )
		{
			m_selectedConvexObject->Scale( cursorPos, scaleAmount, deltaSeconds );
			ToggleRegenerateBVH();
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Raycasts
	//----------------------------------------------------------------------------------------------------------------------
	// Mouse input Raycast startPos
	if ( g_theInput->IsKeyDown( 'S' ) )
	{
		m_rayStartPos = cursorPos;
	}
	// Mouse input Raycast end
	else if ( g_theInput->IsKeyDown( 'E' ) )
	{
		m_rayEndPos = cursorPos;
	}

	// ONLY check if isCursorIsInPoly at the end of update since
	// the current hovered poly might be deleted
	SelectConvexObject( m_selectedConvexObject, cursorPos );
	SelectConvexObject(  m_hoveredConvexObject, cursorPos );
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::UpdateHotkeysInput( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	//----------------------------------------------------------------------------------------------------------------------
	// Hotkeys
	//----------------------------------------------------------------------------------------------------------------------
	// F1 (Debug draw bounding disc)
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F1 ) )
	{
		m_debugRenderBoundingDisc_F1 = !m_debugRenderBoundingDisc_F1;
	}
	// F2 (Toggle alpha value for convex rendering)
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F2 ) )
	{
		m_renderConvexOpaque_F2 = !m_renderConvexOpaque_F2;
	}
	// F4 (Toggle debug BVH rendering)
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F4 ) )
	{
		m_debugRenderBoundsBVH_F4 = !m_debugRenderBoundsBVH_F4;
	}
	// F8 (RandomizeConvexPolyShape)
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F8 ) )
	{
		for ( int i = 0; i < m_gameConvexObjectList.size(); i++ )
		{
			GameConvexObject* curGameConvex = m_gameConvexObjectList[i];
			curGameConvex->RandomizeConvexPosAndShape();
		}
		ToggleRegenerateBVH();
	}
	// F9 (Toggle optimization types)
	if ( g_theInput->WasKeyJustPressed( KEYCODE_F9 ) )
	{
		int typeNum = m_optimizationType;
		// Handle edge case wrapping back to start when incrementing
		typeNum++;
		if ( typeNum >= NUM_TYPES )
		{
			typeNum = NONE;
		}

		if ( typeNum == 0 )
		{
			m_optimizationType = NONE;
		}
		else if ( typeNum == 1 )
		{
			m_optimizationType = NARROW_DISC;
		}
		else if ( typeNum == 2 )
		{
			m_optimizationType = BROAD_BVH;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if ( g_theInput->WasKeyJustPressed( 'P' ) || g_theInput->GetController( 0 ).WasButtonJustPressed( XboxButtonID::BUTTON_START ) )
	{
		SoundID testSound = g_theAudio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
		g_theAudio->StartSound( testSound );			// Comment out this line of code to remove pause sound playing

		m_gameClock.TogglePause();
	}

	// Slow-Mo functionality
	m_isSlowMo = g_theInput->IsKeyDown( 'T' );
	if ( m_isSlowMo )
	{
		m_gameClock.SetTimeScale( 0.1f );
	}
	if ( g_theInput->WasKeyJustReleased( 'T' ) )
	{
		m_gameClock.SetTimeScale( 1.0f );
	}

	// Fast-Mo functionality
	m_isSlowMo = g_theInput->IsKeyDown( 'Y' );
	if ( m_isSlowMo )
	{
		m_gameClock.SetTimeScale( 2.0f );
	}
	if ( g_theInput->WasKeyJustReleased( 'Y' ) )
	{
		m_gameClock.SetTimeScale( 1.0f );
	}

	// Step one frame
	if ( g_theInput->WasKeyJustPressed( 'O' ) )
	{
		m_gameClock.StepSingleFrame();
	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::Update1000Rays()
{
	if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		// Remove old results
		m_debugRayResultsList.clear();
	}
	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		// Only execute this function if '1' was pressed this frame
		// Remove old results
		m_debugRayResultsList.clear();
	}
	else
	{
		return;
	}
	// Record time before
	m_timeBeforeDebugRays = float( GetCurrentTimeSeconds() );		

	if ( m_optimizationType == NONE )
	{
		// Loop 1024 times, create raycasts with rand start and end
		for ( int i = 0; i < m_numRandRays; i++ )
		{
			// Generate rand start and end
			Vec2 randStart			= Vec2::ZERO;
			Vec2 randEnd			= Vec2::ZERO;
			randStart.x				= g_theRNG->RollRandomFloatInRange( 5.0f, 195.0f );
			randStart.y				= g_theRNG->RollRandomFloatInRange( 5.0f,  95.0f );
			randEnd.x				= g_theRNG->RollRandomFloatInRange( 5.0f, 195.0f );
			randEnd.y				= g_theRNG->RollRandomFloatInRange( 5.0f,  95.0f );
			Vec2 randRayFwd			= ( randEnd - randStart ).GetNormalized();
			float randRayLength		= ( randEnd - randStart ).GetLength();

			//----------------------------------------------------------------------------------------------------------------------
			// RayVsConvexObjects (Get nearest poly hit by ray)
			//----------------------------------------------------------------------------------------------------------------------
			RaycastResult2D	nearestRayResult;
			float nearestDist = 999999.9f;
			for ( int j = 0; j < m_gameConvexObjectList.size(); j++ )
			{
				GameConvexObject const* curConvex  = m_gameConvexObjectList[ j ];
				ConvexHull2D	 const& convexHull = curConvex->GetConvexHull();
				RaycastResult2D			rayResult  = RaycastVsConvexHull2D( randStart, randRayFwd, randRayLength, convexHull );
				if ( rayResult.m_didImpact )
				{
					// Check for nearest convexHull
					if ( rayResult.m_impactDist < nearestDist )
					{
						nearestRayResult = rayResult;
						nearestDist = nearestRayResult.m_impactDist;
					}
				}
			}
			if ( !nearestRayResult.m_didImpact )
			{
				nearestRayResult.m_rayStartPos  = randStart;
				nearestRayResult.m_rayEndPos    = randEnd;
				nearestRayResult.m_rayMaxLength = randRayLength;
			}
			m_debugRayResultsList.push_back( nearestRayResult );
		}
	}
	else if ( m_optimizationType == NARROW_DISC )
	{
		// Loop 1024 times, create raycasts with rand start and end
		for ( int i = 0; i < m_numRandRays; i++ )
		{
			// Generate rand start and end
			Vec2 randStart			= Vec2::ZERO;
			Vec2 randEnd			= Vec2::ZERO;
			randStart.x				= g_theRNG->RollRandomFloatInRange( 5.0f, 195.0f );
			randStart.y				= g_theRNG->RollRandomFloatInRange( 5.0f,  95.0f );
			randEnd.x				= g_theRNG->RollRandomFloatInRange( 5.0f, 195.0f );
			randEnd.y				= g_theRNG->RollRandomFloatInRange( 5.0f,  95.0f );
			Vec2 randRayFwd			= ( randEnd - randStart ).GetNormalized();
			float randRayLength		= ( randEnd - randStart ).GetLength();

			//----------------------------------------------------------------------------------------------------------------------
			// RayVsConvexObjects (Get nearest poly hit by ray)
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Raycast against boundingDiscs to get the nearest gameObject
			RaycastResult2D	nearestRayResult;
			ConvexHull2D    nearestConvexHull;
			float nearestDist = FLT_MAX;
			for ( int j = 0; j < m_gameConvexObjectList.size(); j++ )
			{
				GameConvexObject const* curConvex   = m_gameConvexObjectList[ j ];
				ConvexHull2D    convexHull			= curConvex->GetConvexHull();
				Vec2			boundingDiscCenter	= curConvex->GetBoundingDiscCenter();
				float			boundingDiscRadius	= curConvex->GetBoundingDiscRadius();
				RaycastResult2D			 rayResult  = RaycastVsDisc2D( randStart, randRayFwd, randRayLength, boundingDiscCenter, boundingDiscRadius );
				if ( rayResult.m_didImpact )
				{
					// Check for nearest convexHull
					if ( rayResult.m_impactDist < nearestDist )
					{
						nearestRayResult	= rayResult;
						nearestDist			= nearestRayResult.m_impactDist;
						nearestConvexHull	= convexHull; 
					}
				}
			}
			// 2. Raycast against nearest poly to get actual rayResult if there was an impact
			if ( nearestRayResult.m_didImpact )
			{
				nearestRayResult = RaycastVsConvexHull2D( randStart, randRayFwd, randRayLength, nearestConvexHull );
			}
			if ( !nearestRayResult.m_didImpact )
			{
				nearestRayResult.m_rayStartPos  = randStart;
				nearestRayResult.m_rayEndPos    = randEnd;
				nearestRayResult.m_rayMaxLength = randRayLength;
			}
			m_debugRayResultsList.push_back( nearestRayResult );
		}
	}
	else if ( m_optimizationType == BROAD_BVH )
	{
		// Loop 1024 times, create raycasts with rand start and end
		for ( int i = 0; i < m_numRandRays; i++ )
		{
			// Generate rand start and end
			Vec2 randStart			= Vec2::ZERO;
			Vec2 randEnd			= Vec2::ZERO;
			randStart.x				= g_theRNG->RollRandomFloatInRange( 5.0f, 195.0f );
			randStart.y				= g_theRNG->RollRandomFloatInRange( 5.0f,  95.0f );
			randEnd.x				= g_theRNG->RollRandomFloatInRange( 5.0f, 195.0f );
			randEnd.y				= g_theRNG->RollRandomFloatInRange( 5.0f,  95.0f );
			Vec2 randRayFwd			= ( randEnd - randStart ).GetNormalized();
			float randRayLength		= ( randEnd - randStart ).GetLength();

			// 1. Init raycast
			RaycastResult2D	raycastResult;
			raycastResult.m_rayStartPos		 = m_rayStartPos;
			raycastResult.m_rayForwardNormal = m_rayFwd;
			raycastResult.m_rayMaxLength	 = m_rayLength;
			std::vector<GameConvexObject*> entitiesHere;
			m_rootTreeNode->RaycastNodes( raycastResult, entitiesHere );

			//----------------------------------------------------------------------------------------------------------------------
			// RayVsConvexObjects (Get nearest poly hit by ray)
			//----------------------------------------------------------------------------------------------------------------------
			// 1. Raycast against boundingDiscs to get the nearest gameObject
			RaycastResult2D	nearestRayResult;
			ConvexHull2D    nearestConvexHull;
			float nearestDist = FLT_MAX;
			for ( int j = 0; j < entitiesHere.size(); j++ )
			{
				GameConvexObject const* curConvex   = entitiesHere[ j ];
				ConvexHull2D    convexHull			= curConvex->GetConvexHull();
				Vec2			boundingDiscCenter	= curConvex->GetBoundingDiscCenter();
				float			boundingDiscRadius	= curConvex->GetBoundingDiscRadius();
				RaycastResult2D			 rayResult  = RaycastVsDisc2D( randStart, randRayFwd, randRayLength, boundingDiscCenter, boundingDiscRadius );
				if ( rayResult.m_didImpact )
				{
					// Check for nearest convexHull
					if ( rayResult.m_impactDist < nearestDist )
					{
						nearestRayResult	= rayResult;
						nearestDist			= nearestRayResult.m_impactDist;
						nearestConvexHull	= convexHull; 
					}
				}
			}
			// 2. Raycast against nearest poly to get actual rayResult if there was an impact
			if ( nearestRayResult.m_didImpact )
			{
				nearestRayResult = RaycastVsConvexHull2D( randStart, randRayFwd, randRayLength, nearestConvexHull );
			}
			if ( !nearestRayResult.m_didImpact )
			{
				nearestRayResult.m_rayStartPos  = randStart;
				nearestRayResult.m_rayEndPos    = randEnd;
				nearestRayResult.m_rayMaxLength = randRayLength;
			}
			m_debugRayResultsList.push_back( nearestRayResult );
		}
	}


	// Record time after
	m_timeAfterDebugRays = float( GetCurrentTimeSeconds() );
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::UpdateRaycastResult()
{
 	if ( m_optimizationType == NONE )
 	{
 		Raycast_Poly_NoOptimizations();
 	}
 	else if ( m_optimizationType == NARROW_DISC )
 	{
		Raycast_NarrowPhase_Disc();
 	}
 	else if ( m_optimizationType == BROAD_BVH )
 	{
 		Raycast_BroadPhase_BVH();
 	}
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::Raycast_Poly_NoOptimizations()
{
	//----------------------------------------------------------------------------------------------------------------------
	// RayVsConvexObjects (Get nearest poly hit by ray)
	//----------------------------------------------------------------------------------------------------------------------
	RaycastResult2D	nearestRayResult;
	float nearestDist = 999999.9f;
	for ( int i = 0; i < m_gameConvexObjectList.size(); i++ )
	{
		GameConvexObject const* curConvex  = m_gameConvexObjectList[ i ];
		ConvexHull2D	 const& convexHull = curConvex->GetConvexHull();
		RaycastResult2D			rayResult  = RaycastVsConvexHull2D( m_rayStartPos, m_rayFwd, m_rayLength, convexHull );
		if ( rayResult.m_didImpact )
		{
			// Check for nearest convexHull
			if ( rayResult.m_impactDist < nearestDist )
			{
				nearestRayResult = rayResult;
				nearestDist = nearestRayResult.m_impactDist;
			}
		}
	}
	m_rayConvexResult2D = nearestRayResult;
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::Raycast_NarrowPhase_Disc()
{
	//----------------------------------------------------------------------------------------------------------------------
	// RayVsConvexObjects (Get nearest poly hit by ray)
	//----------------------------------------------------------------------------------------------------------------------
	// 1. Raycast against boundingDiscs to get the nearest gameObject
	RaycastResult2D	nearestRayResult;
	ConvexHull2D    nearestConvexHull;
	float nearestDist = 999999.9f;
	for ( int i = 0; i < m_gameConvexObjectList.size(); i++ )
	{
		GameConvexObject const* curConvex   = m_gameConvexObjectList[ i ];
		ConvexHull2D    convexHull			= curConvex->GetConvexHull();
		Vec2			boundingDiscCenter	= curConvex->GetBoundingDiscCenter();
		float			boundingDiscRadius	= curConvex->GetBoundingDiscRadius();
		RaycastResult2D			 rayResult  = RaycastVsDisc2D( m_rayStartPos, m_rayFwd, m_rayLength, boundingDiscCenter, boundingDiscRadius );
		if ( rayResult.m_didImpact )
		{
			// Check for nearest convexHull
			if ( rayResult.m_impactDist < nearestDist )
			{
				nearestRayResult	= rayResult;
				nearestDist			= nearestRayResult.m_impactDist;
				nearestConvexHull	= convexHull; 
			}
		}
	}
	// 2. Raycast against nearest poly to get actual rayResult if there was an impact
	if ( nearestRayResult.m_didImpact )
	{
		nearestRayResult = RaycastVsConvexHull2D(  m_rayStartPos, m_rayFwd, m_rayLength, nearestConvexHull );
	}
	m_rayConvexResult2D = nearestRayResult;
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::Raycast_BroadPhase_BVH()
{
	// 1. Init raycast
	RaycastResult2D	raycastResult;
	raycastResult.m_rayStartPos		 = m_rayStartPos;
	raycastResult.m_rayForwardNormal = m_rayFwd;
	raycastResult.m_rayMaxLength	 = m_rayLength;
	std::vector<GameConvexObject*> entitiesHere;
	m_rootTreeNode->RaycastNodes( raycastResult, entitiesHere );

	//----------------------------------------------------------------------------------------------------------------------
	// RayVsConvexObjects (Get nearest poly hit by ray)
	//----------------------------------------------------------------------------------------------------------------------
	// 2. Loop through all entities inside rayHit treeNode_AABB and raycast against their bounding disc
	// Raycast against boundingDiscs to get the nearest gameObject
	RaycastResult2D	nearestRayResult;
	ConvexHull2D    nearestConvexHull;
	float nearestDist = 999999.9f;
	for ( int i = 0; i < entitiesHere.size(); i++ )
	{
		GameConvexObject const* curConvex   = entitiesHere[ i ];
		ConvexHull2D    convexHull			= curConvex->GetConvexHull();
		Vec2			boundingDiscCenter	= curConvex->GetBoundingDiscCenter();
		float			boundingDiscRadius	= curConvex->GetBoundingDiscRadius();
		RaycastResult2D			 rayResult  = RaycastVsDisc2D( m_rayStartPos, m_rayFwd, m_rayLength, boundingDiscCenter, boundingDiscRadius );
		if ( rayResult.m_didImpact )
		{
			nearestRayResult	= rayResult;
			nearestDist			= nearestRayResult.m_impactDist;
			nearestConvexHull	= convexHull; 
		}
	}
	// 2. Raycast against nearest poly to get actual rayResult if there was an impact
	if ( nearestRayResult.m_didImpact )
	{
		nearestRayResult = RaycastVsConvexHull2D(  m_rayStartPos, m_rayFwd, m_rayLength, nearestConvexHull );
	}
	m_rayConvexResult2D = nearestRayResult;
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::SelectConvexObject( GameConvexObject*& gameConvex, Vec2 const& testPoint )
{
	for ( int i = 0; i < m_gameConvexObjectList.size(); i++ )
	{
		GameConvexObject* curGameConvexObject	= m_gameConvexObjectList[i];
		ConvexHull2D const& convexHull			= curGameConvexObject->GetConvexHull();
		bool isPointInside = IsPointInsideConvexHull2D( testPoint, convexHull );
		if ( isPointInside )
		{
			curGameConvexObject->ComputeOffsetsToPoint( testPoint );
			gameConvex = curGameConvexObject;
			break;
		}
		else
		{
			gameConvex = nullptr;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
std::string const GameModeConvexScene2D::GetEnumAsString() const
{
	std::string name = "NONE";
	if ( m_optimizationType == NONE )
	{
		name = "NONE";
	}
	else if ( m_optimizationType == NARROW_DISC )
	{
		name = "NARROW_DISC";
	}
	else if ( m_optimizationType == BROAD_BVH )
	{
		name = "BROAD_BVH";
	}
	return name;
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::RegenerateBVH()
{
	if ( !m_regenerateBVH )
	{
	 	return;
	}
	delete m_rootTreeNode;
	m_rootTreeNode = new TreeNode_AABB2_BVH( m_gameConvexObjectList );
	m_rootTreeNode->SplitAndCreateChildNodes( 3 );
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::ToggleRegenerateBVH()
{
	// Toggle BVH regeneration
	m_regenerateBVH = true;
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::ParseConvexSceneFromFile( std::vector<unsigned char>& buffer )
{
	BufferParser bufferParser = BufferParser( buffer.data(), unsigned char( buffer.size() ) );

	//----------------------------------------------------------------------------------------------------------------------
	// 1. Parse file header
	// 2. Parse TOC
	// 3. Parse chunks based on info provided from TOC
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// File header
	// 1. Add GHCS
	// 2. Cohort ID
	// 3. Major file version
	// 4. Minor file version
	// 5. Endian
	// 6. Location of TOC as offset from header
	// 7. ENDH (End of file header)
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// 1. Add GHCS
	std::string stringGHCS;
	char curChar = bufferParser.ParseChar();	// G
	stringGHCS.push_back( curChar );
	curChar = bufferParser.ParseChar();			// H
	stringGHCS.push_back( curChar );
	curChar = bufferParser.ParseChar();			// C
	stringGHCS.push_back( curChar );
	curChar = bufferParser.ParseChar();			// S
	stringGHCS.push_back( curChar );
	if ( stringGHCS != "GHCS" )
	{
		DebuggerPrintf("Invalid header 4CC, convex scene not parsed from file");
		return;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 2. Cohort ID
	char cohortID = bufferParser.ParseChar();
	if ( cohortID != 32 )
	{
		DebuggerPrintf( "Invalid cohortID, convex scene not parsed from file" );
		return;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 3. Major file version
	char majorFileVersion = bufferParser.ParseChar();
	if ( majorFileVersion != 1 )
	{
		DebuggerPrintf( "Invalid majorFileVersion, convex scene not parsed from file" );
		return;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 4. Minor file version
	char minorFileVersion = bufferParser.ParseChar();
	if ( minorFileVersion != 1 )
	{
		DebuggerPrintf( "Invalid minorFileVersion, convex scene not parsed from file" );
		return;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 5. Endian
	char endianMode = bufferParser.ParseChar();
	if ( endianMode == 0 )
	{
		// #Todo: Make sure that 0 is unsupported and not NATIVE
		DebuggerPrintf( "Unsupported endian mode, convex scene not parsed from file" );
		return;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 6. Location of TOC as offset from header
	int offsetToTOC = bufferParser.ParseInt32();
	//----------------------------------------------------------------------------------------------------------------------
	// 7. ENDH (End of file header)
	std::string stringENDH;
	curChar = bufferParser.ParseChar();	// E
	stringENDH.push_back( curChar );
	curChar = bufferParser.ParseChar();	// N
	stringENDH.push_back( curChar );
	curChar = bufferParser.ParseChar();	// D
	stringENDH.push_back( curChar );
	curChar = bufferParser.ParseChar();	// H
	stringENDH.push_back( curChar );
	if ( stringENDH != "ENDH" )
	{
		DebuggerPrintf("Invalid End of file header, convex scene not parsed from file");
		return;
	}


	//----------------------------------------------------------------------------------------------------------------------
	// Table of contents (TOC)
	// 1.  GHTC (Guildhall Table of Contents)
	// 2.  Number of chunks in this file
	// 3.  Loop through numChunks and create chunks 
	// 3a. Chunk type
	// 3b. Chunk header start
	// 3c. Chunk total size
	// 4.  ENDT (End of table of contents)
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// 1.  GHTC (Guildhall Table of Contents)
	// 1a. Apply offset from buffer start to TOC
	bufferParser.JumpToPos( offsetToTOC );
	// 1b. Parse 4CC
	std::string stringGHTC;
	curChar = bufferParser.ParseChar();	// G
	stringGHTC.push_back( curChar );
	curChar = bufferParser.ParseChar();	// H
	stringGHTC.push_back( curChar );
	curChar = bufferParser.ParseChar();	// T
	stringGHTC.push_back( curChar );
	curChar = bufferParser.ParseChar();	// C
	stringGHTC.push_back( curChar );
	if ( stringGHTC != "GHTC" )
	{
		DebuggerPrintf( "Invalid GHTC, guildhall table of contents 4CC, convex scene not parsed from file" );
		return;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 2. Number of chunks in this file
	char numberOfChunks = bufferParser.ParseChar();
	//----------------------------------------------------------------------------------------------------------------------
	// 3.  Loop through numChunks and create chunk entries 
	std::vector<TocEntryGHCS*> tocEntryList;
	for ( int i = 0; i < numberOfChunks; i++ )
	{
		// 3a. Chunk type
		uint8_t chunkType				= bufferParser.ParseUnsignedInt8();
		// 3b. Chunk header start
		uint32_t chunkHeaderStart		= bufferParser.ParseInt32();
		// 3c. Chunk total size
		uint32_t chunkTotalSize			= bufferParser.ParseInt32();
		// 3d. Add tocEntries to tocEntryList
		TocEntryGHCS* curTocEntry		= new TocEntryGHCS( ChunkTypeGHCS( chunkType ), chunkHeaderStart, chunkTotalSize );
		tocEntryList.push_back( curTocEntry );
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 4. ENDT (End of table of contents)
	std::string stringENDT;
	curChar = bufferParser.ParseChar();	// E
	stringENDT.push_back( curChar );
	curChar = bufferParser.ParseChar();	// N
	stringENDT.push_back( curChar );
	curChar = bufferParser.ParseChar();	// D
	stringENDT.push_back( curChar );
	curChar = bufferParser.ParseChar();	// T
	stringENDT.push_back( curChar );
	if ( stringENDT != "ENDT" )
	{
		DebuggerPrintf( "Invalid ENDT, end of table of contents 4CC, convex scene not parsed from file" );
		return;
	}


	//----------------------------------------------------------------------------------------------------------------------
	// Create chunks based on tocEntries
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<ConvexPoly2D>			convexPolyList;
	std::vector<BoundingDiscObject2D>	boundingDiscObjectList;
//	TreeNode_AABB2_BVH*					rootTreeNode = nullptr;
	for ( int i = 0; i < tocEntryList.size(); i++ )
	{
		// 1. Call appropriate parse function based on specified chunk type
		TocEntryGHCS* curEntry = tocEntryList[i];
		if ( curEntry->m_chunkType == ChunkTypeGHCS::SCENE_INFO )
		{
			ParseConvexSceneInfo( bufferParser, curEntry->m_chunkOffset, curEntry->m_chunkTotalSize );
		}
		else if ( curEntry->m_chunkType == ChunkTypeGHCS::CONVEX_POLY_2D )
		{
			convexPolyList = ParseConvexPolys2D( bufferParser, curEntry->m_chunkOffset, curEntry->m_chunkTotalSize );
		}
		else if ( curEntry->m_chunkType == ChunkTypeGHCS::BOUNDING_DISCS_2D )
		{
			boundingDiscObjectList = ParseBoundingDisc2D( bufferParser, curEntry->m_chunkOffset, curEntry->m_chunkTotalSize );
		}
		else if ( curEntry->m_chunkType == ChunkTypeGHCS::AABB2_TREE )
		{
//			m_rootTreeNode = ParseAABBTree( bufferParser, curEntry.m_chunkOffset, curEntry.m_chunkTotalSize );
		}
		else
		{
			DebuggerPrintf( "Unknown chunk type" );
		}
	}
	// Create new game objects
	for ( int i = 0; i < convexPolyList.size(); i++ )
	{
		ConvexPoly2D			curConvexPoly   = convexPolyList[i];
		BoundingDiscObject2D	curBoundingDisc = boundingDiscObjectList[i];
		GameConvexObject*		gameObject		= new GameConvexObject( curConvexPoly, curBoundingDisc.m_discCenter, curBoundingDisc.m_radius );
		m_gameConvexObjectList.push_back( gameObject );
	}
	m_rootTreeNode = new TreeNode_AABB2_BVH( m_gameConvexObjectList );
	m_rootTreeNode->SplitAndCreateChildNodes( 3 );
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::AppendHeader( BufferWriter& bufferWriter )
{
	//----------------------------------------------------------------------------------------------------------------------
	// 1. Add GHCS
	bufferWriter.AppendChar( 'G' );	// G
	bufferWriter.AppendChar( 'H' );	// H
	bufferWriter.AppendChar( 'C' );	// C
	bufferWriter.AppendChar( 'S' );	// S
	//----------------------------------------------------------------------------------------------------------------------
	// 2. Cohort ID
	char cohortID = 32;
	bufferWriter.AppendChar( cohortID );
	//----------------------------------------------------------------------------------------------------------------------
	// 3. Major file version
	char majorFileVersion = 1;
	bufferWriter.AppendChar( majorFileVersion );
	//----------------------------------------------------------------------------------------------------------------------
	// 4. Minor file version
	char minorFileVersion = 1;
	bufferWriter.AppendChar( minorFileVersion );
	//----------------------------------------------------------------------------------------------------------------------
	// 5. Endian
	char endianNess = char( bufferWriter.GetEndianMode() );
	bufferWriter.AppendChar( endianNess );
	//----------------------------------------------------------------------------------------------------------------------
	// 6. Location of TOC as offset from header
	m_tocLocation = unsigned int( bufferWriter.m_buffer.size() );
	bufferWriter.AppendUnsignedInt32( 1 );		// This value will be changed later
	//----------------------------------------------------------------------------------------------------------------------
	// 7. ENDH (End of file header)
	std::string stringENDH;
	bufferWriter.AppendChar( 'E' );
	bufferWriter.AppendChar( 'N' );
	bufferWriter.AppendChar( 'D' );
	bufferWriter.AppendChar( 'H' );
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::AppendChunkData( BufferWriter& bufferWriter, ChunkTypeGHCS chunkType )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Chunk structure
	// 1. GHCK
	// 2. Chunk type
	// 3. Chunk data endian-ness
	// 4. Chunk data size (between header & footer)
	// 5. Parse world bounds
	// 6. Number of objects in the scene
	// 7. Chunk footer, ENDC (End of chunk)
	//----------------------------------------------------------------------------------------------------------------------

	uint32_t curDataStart = uint32_t( bufferWriter.m_buffer.size() );
	if ( chunkType == SCENE_INFO )
	{
		// 1. GHCK
		bufferWriter.AppendChar( 'G' );
		bufferWriter.AppendChar( 'H' );
		bufferWriter.AppendChar( 'C' );
		bufferWriter.AppendChar( 'K' );
		// 2. Chunk type
		bufferWriter.AppendChar( char(chunkType) );
		// 3. Chunk data endian-ness
		char endianness = char( bufferWriter.GetEndianMode() );
		bufferWriter.AppendChar( endianness );
		// 4. Chunk data size (between header & footer)
		bufferWriter.AppendUnsignedInt32( 20 );
		// 5. Parse world bounds
		if ( m_shouldChangeWorldBounds )
		{
			bufferWriter.AppendAABB2( g_parsedWorldBounds );
		}
		else
		{
			AABB2 worldBounds = AABB2( Vec2::ZERO, Vec2( WORLD_SIZE_X, WORLD_SIZE_Y ) );
			bufferWriter.AppendAABB2( worldBounds );
		}
		// 6. Number of objects in the scene
		uint32_t numObjectsInScene = uint32_t( m_gameConvexObjectList.size() );
		bufferWriter.AppendUnsignedInt32( numObjectsInScene );
		// 7. Chunk footer, ENDC (End of chunk)
		bufferWriter.AppendChar( 'E' );
		bufferWriter.AppendChar( 'N' );
		bufferWriter.AppendChar( 'D' );
		bufferWriter.AppendChar( 'C' );
	}
	if ( chunkType == CONVEX_POLY_2D )
	{
		// 1. GHCK
		bufferWriter.AppendChar( 'G' );
		bufferWriter.AppendChar( 'H' );
		bufferWriter.AppendChar( 'C' );
		bufferWriter.AppendChar( 'K' );
		// 2. Chunk type
		bufferWriter.AppendChar( char(chunkType) );
		// 3. Chunk data endian-ness
		char endianness = char( bufferWriter.GetEndianMode() );
		bufferWriter.AppendChar( endianness );
		// 4. Chunk data size (between header & footer)
		uint32_t dataSizeLocation	= uint32_t( bufferWriter.m_buffer.size() );
		uint32_t baseDataSize		= 14;
		uint32_t extraInfo			= 10;
		bufferWriter.AppendUnsignedInt32( baseDataSize + extraInfo );
		// 5. Append numObjects
		uint32_t numObjects	= uint32_t( m_gameConvexObjectList.size() );
		bufferWriter.AppendUnsignedInt32( numObjects );
		// 6. Append numPoints and positions of curPoint for each object
		uint16_t totalNumPoints = 0;
		for ( uint32_t i = 0; i < numObjects; i++ )
		{
			GameConvexObject*	 curConvexObject	= m_gameConvexObjectList[i];
			ConvexPoly2D const & curConvexPoly		= curConvexObject->GetConvexPoly();
			std::vector<Vec2>	 pointList			= curConvexPoly.GetCcwOrderedPoints();
			uint16_t			 numPointsPerObject	= uint16_t( pointList.size() );
			bufferWriter.AppendUnsignedInt16( numPointsPerObject );
			for ( int j = 0; j < numPointsPerObject; j++ )
			{
				Vec2 curPoint = pointList[j];
				bufferWriter.AppendVec2( curPoint);
			}
			totalNumPoints += numPointsPerObject;
		}
		// Overwrite data size info
		uint32_t newDataSize = 4 + ( numObjects * 2 ) + ( 8 * totalNumPoints );
		bufferWriter.OverwriteUint32( dataSizeLocation, newDataSize );
		// 7. Chunk footer, ENDC (End of chunk)
		bufferWriter.AppendChar( 'E' );
		bufferWriter.AppendChar( 'N' );
		bufferWriter.AppendChar( 'D' );
		bufferWriter.AppendChar( 'C' );
	}
	if ( chunkType == BOUNDING_DISCS_2D )
	{
		// 1. GHCK
		bufferWriter.AppendChar( 'G' );
		bufferWriter.AppendChar( 'H' );
		bufferWriter.AppendChar( 'C' );
		bufferWriter.AppendChar( 'K' );
		// 2. Chunk type
		bufferWriter.AppendChar( char(chunkType) );
		// 3. Chunk data endian-ness
		char endianness = char( bufferWriter.GetEndianMode() );
		bufferWriter.AppendChar( endianness );
		// 4. Chunk data size (between header & footer)
		uint32_t dataSizeLocation	= uint32_t( bufferWriter.m_buffer.size() );
		uint32_t baseDataSize		= 14;
		uint32_t extraInfo			= 16;
		bufferWriter.AppendUnsignedInt32( baseDataSize + extraInfo );
		// 5. Append numObjects
		uint32_t numObjects	= uint32_t( m_gameConvexObjectList.size() );
		bufferWriter.AppendUnsignedInt32( numObjects );
		// 6. Append numPoints and positions of curPoint for each object
		for ( uint32_t i = 0; i < numObjects; i++ )
		{
			GameConvexObject*	curConvexObject = m_gameConvexObjectList[i];
			Vec2				curBoundingDisc = curConvexObject->GetBoundingDiscCenter();
			float				curRadius		= curConvexObject->GetBoundingDiscRadius();
			bufferWriter.AppendVec2 ( curBoundingDisc );
			bufferWriter.AppendFloat( curRadius );
		}
		// Overwrite data size info
		uint32_t newDataSize = 4 + ( numObjects * ( 8 + 4 ) );
		bufferWriter.OverwriteUint32( dataSizeLocation, newDataSize );
		// 7. Chunk footer, ENDC (End of chunk)
		bufferWriter.AppendChar( 'E' );
		bufferWriter.AppendChar( 'N' );
		bufferWriter.AppendChar( 'D' );
		bufferWriter.AppendChar( 'C' );
	}

	// 3d. Add tocEntries to tocEntryList
	uint32_t chunkTotalSize			 = uint32_t( bufferWriter.m_buffer.size() );
	chunkTotalSize					-= curDataStart;
	TocEntryGHCS* curTocEntry		 = new TocEntryGHCS( chunkType, curDataStart, chunkTotalSize );
	m_tocEntryList.push_back( curTocEntry );
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::AppendTOC( BufferWriter& bufferWriter )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Table of contents (TOC)
	// 1.  GHTC (Guildhall Table of Contents)
	// 2.  Number of chunks in this file
	// 3.  Loop through numChunks and create chunks 
	// 3a. Chunk type
	// 3b. Chunk header start
	// 3c. Chunk total size
	// 4.  ENDT (End of table of contents)
	//----------------------------------------------------------------------------------------------------------------------

	// Update tocLocation in the header
	unsigned int newTocLocation = unsigned int( bufferWriter.m_buffer.size() );
	bufferWriter.OverwriteUint32( m_tocLocation, newTocLocation );

	//----------------------------------------------------------------------------------------------------------------------
	// 1.  GHTC (Guildhall Table of Contents)
	bufferWriter.AppendChar( 'G' );
	bufferWriter.AppendChar( 'H' );
	bufferWriter.AppendChar( 'T' );
	bufferWriter.AppendChar( 'C' );
	//----------------------------------------------------------------------------------------------------------------------
	// 2. Number of chunks in this file
	char numberOfChunks = char( m_tocEntryList.size() );
	bufferWriter.AppendChar( numberOfChunks );
	//----------------------------------------------------------------------------------------------------------------------
	// 3.  Loop through numChunks and create chunk entries 
	for ( int i = 0; i < int(numberOfChunks); i++ )
	{
		TocEntryGHCS* curTocEntry = m_tocEntryList[i];
 		// 3a. Chunk Type
 		bufferWriter.AppendChar( char(curTocEntry->m_chunkType) );
 		// 3b. Chunk header start
 		bufferWriter.AppendUnsignedInt32( curTocEntry->m_chunkOffset );
 		// 3c. Chunk total size
		bufferWriter.AppendUnsignedInt32( curTocEntry->m_chunkTotalSize );
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 4. ENDT (End of table of contents)
	bufferWriter.AppendChar( 'E' );
	bufferWriter.AppendChar( 'N' );
	bufferWriter.AppendChar( 'D' );
	bufferWriter.AppendChar( 'T' );
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::ParseConvexSceneInfo( BufferParser& bufferParser, uint32_t chunkLocation, uint32_t chunkTotalSize )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Chunk structure
	// 1. GHCK
	// 2. Chunk type
	// 3. Chunk data endian-ness
	// 4. Chunk data size (between header & footer)
	// 5. Parse world bounds
	// 6. Number of objects in the scene
	// 7. Chunk footer, ENDC (End of chunk)
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// 1. GHCK
	// 1a. Apply offset from buffer start to arrive at current data
	bufferParser.JumpToPos( chunkLocation );
	// 1b. Parse 4CC
	std::string stringGHCK;
	char curChar;
	curChar = bufferParser.ParseChar();	// G
	stringGHCK.push_back( curChar );
	curChar = bufferParser.ParseChar();	// H
	stringGHCK.push_back( curChar );
	curChar = bufferParser.ParseChar();	// C
	stringGHCK.push_back( curChar );
	curChar = bufferParser.ParseChar();	// K
	stringGHCK.push_back( curChar );
	if ( stringGHCK != "GHCK" )
	{
		DebuggerPrintf( "Invalid GHCK, chunk structure 4CC, chunk not parsed from file" );
		return;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 2. Chunk type
	char chunkType  = bufferParser.ParseChar();
	chunkType		= ChunkTypeGHCS( chunkType );
	if ( chunkType != ChunkTypeGHCS::SCENE_INFO )
	{
		DebuggerPrintf( "Invalid chunk type, expected SCENE INFO, chunk not parsed from file" );
		return;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 3. Chunk data endian-ness
	char chunkDataEndian = bufferParser.ParseChar();
	if ( chunkDataEndian == 0 )
	{
		DebuggerPrintf( "Invalid chunk data endian-ness, chunk not parsed from file" );
		return;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 4. Chunk data size (between header & footer)
	unsigned int chunkDataSize = bufferParser.ParseUnsignedInt32();
	unsigned int metaData = 14;
	if ( (chunkDataSize + metaData) != chunkTotalSize )
	{
 		DebuggerPrintf( "Invalid chunk data size, chunk not parsed from file" );
 		return;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 5. Parse world bounds
	g_parsedWorldBounds	= bufferParser.ParseAABB2();
	m_shouldChangeWorldBounds = true;
	// 6. Number of objects in the scene
	uint32_t numObjectsInScene  = bufferParser.ParseInt32();		// #Squirrel: How do I use this number?
	UNUSED( numObjectsInScene );
	//----------------------------------------------------------------------------------------------------------------------
	// 7. Chunk footer, ENDC (End of chunk)
	std::string stringENDC;
	curChar = bufferParser.ParseChar();	// E
	stringENDC.push_back( curChar );
	curChar = bufferParser.ParseChar();	// N
	stringENDC.push_back( curChar );
	curChar = bufferParser.ParseChar();	// D
	stringENDC.push_back( curChar );
	curChar = bufferParser.ParseChar();	// C
	stringENDC.push_back( curChar );
	if ( stringENDC != "ENDC" )
	{
		DebuggerPrintf( "Invalid ENDC, chunk footer 4CC, convex scene not parsed from file" );
		return;
	}
}


//----------------------------------------------------------------------------------------------------------------------
std::vector<ConvexPoly2D> GameModeConvexScene2D::ParseConvexPolys2D( BufferParser& bufferParser, uint32_t chunkLocation, uint32_t chunkTotalSize )
{
	std::vector<ConvexPoly2D> emptyList;
	//----------------------------------------------------------------------------------------------------------------------
	// Chunk structure
	// 1. GHCK
	// 2. Chunk type
	// 3. Chunk data endian-ness
	// 4. Chunk data size (between header & footer)
	// 5. Chunk private data
	// 6. Chunk footer, ENDC (End of chunk)
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// 1. GHCK
	// 1a. Jump bufferParser pointer to chunk location
	bufferParser.JumpToPos( chunkLocation );
	// 1b. Parse 4CC
	std::string stringGHCK;
	char curChar;
	curChar = bufferParser.ParseChar();	// G
	stringGHCK.push_back( curChar );
	curChar = bufferParser.ParseChar();	// H
	stringGHCK.push_back( curChar );
	curChar = bufferParser.ParseChar();	// C
	stringGHCK.push_back( curChar );
	curChar = bufferParser.ParseChar();	// K
	stringGHCK.push_back( curChar );
	if ( stringGHCK != "GHCK" )
	{
		DebuggerPrintf( "Invalid GHCK, chunk structure 4CC, chunk not parsed from file" );
		return emptyList;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 2. Chunk type
	char chunkType  = bufferParser.ParseChar();
	chunkType		= ChunkTypeGHCS( chunkType );
	if ( chunkType != ChunkTypeGHCS::CONVEX_POLY_2D )
	{
		DebuggerPrintf( "Invalid chunk type, expected CONVEX POLY2D, chunk not parsed from file" );
		return emptyList;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 3. Chunk data endian-ness
	char chunkDataEndian = bufferParser.ParseChar();
	if ( chunkDataEndian == 0 )
	{
		DebuggerPrintf( "Invalid chunk data endian-ness, chunk not parsed from file" );
		return emptyList;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 4. Chunk data size (between header & footer)
	unsigned int chunkDataSize  = bufferParser.ParseUnsignedInt32();
	unsigned int metaData		= 14;
	if ( (chunkDataSize + metaData) != chunkTotalSize )
	{
	  	DebuggerPrintf( "Invalid chunk data size, chunk not parsed from file" );
	  	return emptyList;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 5. Parse numObjects
	std::vector<ConvexPoly2D> convexPolyList;
	uint32_t numObjects = bufferParser.ParseUnsignedInt32();
	for ( uint32_t i = 0; i < numObjects; i++ )
	{
		std::vector<Vec2> orderedPoints;
		uint16_t numPoints = bufferParser.ParseShort();
		for ( int j = 0; j < numPoints; j++ )
		{
			Vec2 curPoint = bufferParser.ParseVec2();
			orderedPoints.push_back( curPoint );
		}
		ConvexPoly2D curConvexPoly2D = ConvexPoly2D( orderedPoints );
		convexPolyList.push_back( curConvexPoly2D );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// 6. Chunk footer, ENDC (End of chunk)
	std::string stringENDC;
	curChar = bufferParser.ParseChar();	// E
	stringENDC.push_back( curChar );
	curChar = bufferParser.ParseChar();	// N
	stringENDC.push_back( curChar );
	curChar = bufferParser.ParseChar();	// D
	stringENDC.push_back( curChar );
	curChar = bufferParser.ParseChar();	// C
	stringENDC.push_back( curChar );
	if ( stringENDC != "ENDC" )
	{
		DebuggerPrintf( "Invalid ENDC, chunk footer 4CC, chunk not parsed from file" );
		return emptyList;
	}

	return convexPolyList;
}


//----------------------------------------------------------------------------------------------------------------------
std::vector<BoundingDiscObject2D> GameModeConvexScene2D::ParseBoundingDisc2D( BufferParser& bufferParser, uint32_t chunkLocation, uint32_t chunkTotalSize )
{
	std::vector<BoundingDiscObject2D> emptyList;
	//----------------------------------------------------------------------------------------------------------------------
	// Chunk structure
	// 1. GHCK
	// 2. Chunk type
	// 3. Chunk data endian-ness
	// 4. Chunk data size (between header & footer)
	// 5. Chunk private data
	// 6. Chunk footer, ENDC (End of chunk)
	//----------------------------------------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------------------------------------
	// 1. GHCK
	// 1a. Jump bufferParser pointer to chunk location
	bufferParser.JumpToPos( chunkLocation );
	// 1b. Parse 4CC
	std::string stringGHCK;
	char curChar;
	curChar = bufferParser.ParseChar();	// G
	stringGHCK.push_back( curChar );
	curChar = bufferParser.ParseChar();	// H
	stringGHCK.push_back( curChar );
	curChar = bufferParser.ParseChar();	// C
	stringGHCK.push_back( curChar );
	curChar = bufferParser.ParseChar();	// K
	stringGHCK.push_back( curChar );
	if ( stringGHCK != "GHCK" )
	{
		DebuggerPrintf( "Invalid GHCK, chunk structure 4CC, chunk not parsed from file" );
		return emptyList;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 2. Chunk type
	uint8_t chunkType  = bufferParser.ParseUnsignedInt8();
	chunkType		= ChunkTypeGHCS( chunkType );
	if ( chunkType != ChunkTypeGHCS::BOUNDING_DISCS_2D )
	{
		DebuggerPrintf( "Invalid chunk type, expected CONVEX POLY2D, chunk not parsed from file" );
		return emptyList;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 3. Chunk data endian-ness
	char chunkDataEndian = bufferParser.ParseChar();
	if ( chunkDataEndian == 0 )
	{
		DebuggerPrintf( "Invalid chunk data endian-ness, chunk not parsed from file" );
		return emptyList;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 4. Chunk data size (between header & footer)
	unsigned int chunkDataSize = bufferParser.ParseUnsignedInt32();
	unsigned int metaData = 14;
	if ( (chunkDataSize + metaData) != chunkTotalSize )
	{
 		DebuggerPrintf( "Invalid chunk data size, chunk not parsed from file" );
 		return emptyList;
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 5. Parse numObjects
	std::vector<BoundingDiscObject2D> boundingDiscObjectList;
	uint32_t numObjects = bufferParser.ParseUnsignedInt32();
	for ( uint32_t i = 0; i < numObjects; i++ )
	{
		Vec2  discCenter	= bufferParser.ParseVec2();
		float discRadius	= bufferParser.ParseFloat();	
		BoundingDiscObject2D curBoundingDisc;
		curBoundingDisc.m_discCenter = discCenter;
		curBoundingDisc.m_radius	 = discRadius;
		boundingDiscObjectList.push_back( curBoundingDisc );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// 6. Chunk footer, ENDC (End of chunk)
	std::string stringENDC;
	curChar = bufferParser.ParseChar();	// E
	stringENDC.push_back( curChar );
	curChar = bufferParser.ParseChar();	// N
	stringENDC.push_back( curChar );
	curChar = bufferParser.ParseChar();	// D
	stringENDC.push_back( curChar );
	curChar = bufferParser.ParseChar();	// C
	stringENDC.push_back( curChar );
	if ( stringENDC != "ENDC" )
	{
		DebuggerPrintf( "Invalid ENDC, chunk footer 4CC, chunk not parsed from file" );
		return emptyList;
	}

	return boundingDiscObjectList;
}


//----------------------------------------------------------------------------------------------------------------------
// TreeNode_AABB2_BVH* GameModeConvexScene2D::ParseAABBTree( BufferParser& bufferParser, uint32_t chunkLocation, uint32_t chunkTotalSize )
// {
// 	TreeNode_AABB2_BVH* newAAbb2Tree = nullptr;
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// Chunk structure
// 	// 1. GHCK
// 	// 2. Chunk type
// 	// 3. Chunk data endian-ness
// 	// 4. Chunk data size (between header & footer)
// 	// 5. Chunk private data
// 	// 6. Chunk footer, ENDC (End of chunk)
// 	//----------------------------------------------------------------------------------------------------------------------
// 
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 1. GHCK
// 	// 1a. Jump bufferParser pointer to chunk location
// 	bufferParser.JumpToPos( chunkLocation );
// 	// 1b. Parse 4CC
// 	std::string stringGHCK;
// 	char curChar;
// 	curChar = bufferParser.ParseChar();	// G
// 	stringGHCK.push_back( curChar );
// 	curChar = bufferParser.ParseChar();	// H
// 	stringGHCK.push_back( curChar );
// 	curChar = bufferParser.ParseChar();	// C
// 	stringGHCK.push_back( curChar );
// 	curChar = bufferParser.ParseChar();	// K
// 	stringGHCK.push_back( curChar );
// 	if ( stringGHCK != "GHCK" )
// 	{
// 		DebuggerPrintf( "Invalid GHCK, chunk structure 4CC, chunk not parsed from file" );
// 		return nullptr;
// 	}
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 2. Chunk type
// 	uint8_t chunkType = bufferParser.ParseUnsignedInt8();
// 	chunkType = ChunkTypeGHCS( chunkType );
// 	if ( chunkType != ChunkTypeGHCS::AABB2_TREE )
// 	{
// 		DebuggerPrintf( "Invalid chunk type, expected AABB2 TREE, chunk not parsed from file" );
// 		return nullptr;
// 	}
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 3. Chunk data endian-ness
// 	char chunkDataEndian = bufferParser.ParseChar();
// 	if ( chunkDataEndian == 0 )
// 	{
// 		DebuggerPrintf( "Invalid chunk data endian-ness, chunk not parsed from file" );
// 		return nullptr;
// 	}
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 4. Chunk data size (between header & footer)
// 	unsigned int chunkDataSize = bufferParser.ParseUnsignedInt32();
// 	unsigned int metaData = 18;
// 	if ( ( chunkDataSize + metaData ) != chunkTotalSize )
// 	{
// 		DebuggerPrintf( "Invalid chunk data size, chunk not parsed from file" );
// 		return nullptr;
// 	}
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 5. Parse numObjects
// 	uint32_t numNodes = bufferParser.ParseUnsignedInt32();
// //	m_rootTreeNode = new TreeNode_AABB2_BVH( m_gameConvexObjectList );
// //	m_rootTreeNode->SplitAndCreateChildNodes( numNodes );
// 	for ( uint32_t i = 0; i < numNodes; i++ )
// 	{
// 		char     leftOrRight			= bufferParser.ParseChar();
// 		uint32_t discRadius				= bufferParser.ParseUnsignedInt32();
// 		AABB2	 curAabb2				= bufferParser.ParseAABB2();
// 		uint32_t numObjectsInCurNode	= bufferParser.ParseUnsignedInt32();
// 		m_rootTreeNode->m_bounds		= curAabb2;
// 		for ( uint32_t j = 0; j < numObjectsInCurNode; j++ )
// 		{
// 			uint32_t objectIndex = bufferParser.ParseUnsignedInt32();
// 			// Root
// 			if ( leftOrRight == 0 )
// 			{
// //				m_rootTreeNode->
// 			}
// 			// Left
// 			if ( leftOrRight == 1 )
// 			{
// // 				TreeNode_AABB2_BVH* childNode = TreeNode_AABB2_BVH( numObjectsInCurNode );
// // 				m_rootTreeNode->m_leftChild = objectIndex;
// 			}
// 			// Right
// 			if ( leftOrRight == 2 )
// 			{
// 
// 			}
// 		}
// 	}
// 
// 	//----------------------------------------------------------------------------------------------------------------------
// 	// 6. Chunk footer, ENDC (End of chunk)
// 	std::string stringENDC;
// 	curChar = bufferParser.ParseChar();	// E
// 	stringENDC.push_back( curChar );
// 	curChar = bufferParser.ParseChar();	// N
// 	stringENDC.push_back( curChar );
// 	curChar = bufferParser.ParseChar();	// D
// 	stringENDC.push_back( curChar );
// 	curChar = bufferParser.ParseChar();	// C
// 	stringENDC.push_back( curChar );
// 	if ( stringENDC != "ENDC" )
// 	{
// 		DebuggerPrintf( "Invalid ENDC, chunk footer 4CC, chunk not parsed from file" );
// 		return nullptr;
// 	}
// 
// 	return nullptr;
// }


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::AppendTestFileBufferData( BufferWriter& bufferWriter, BufferEndian endianMode )
{
	bufferWriter.SetEndianMode( endianMode );
	bufferWriter.AppendChar( 'T' );
	bufferWriter.AppendChar( 'E' );
	bufferWriter.AppendChar( 'S' );
	bufferWriter.AppendChar( 'T' );
	bufferWriter.AppendByte( 2 ); // Version 2
	bufferWriter.AppendByte( (unsigned char)bufferWriter.GetEndianMode() );
	bufferWriter.AppendBool( false );
	bufferWriter.AppendBool( true );
	bufferWriter.AppendUnsignedInt32( 0x12345678 );
	bufferWriter.AppendInt32( -7 ); // signed 32-bit int
	bufferWriter.AppendFloat( 1.f ); // in memory looks like hex: 00 00 80 3F (or 3F 80 00 00 in big endian)
	bufferWriter.AppendDouble( 3.1415926535897932384626433832795 ); // actually 3.1415926535897931 (best it can do)
	bufferWriter.AppendStringZeroTerminated("Hello"); // written with a trailing 0 ('\0') after (6 bytes total)
	bufferWriter.AppendStringAfter32BitLength("Is this thing on?"); // uint 17, then 17 chars (no zero-terminator after)
	bufferWriter.AppendRgba8( Rgba8( 200, 100, 50, 255 ) ); // four bytes in RGBA order (endian-independent)
	bufferWriter.AppendByte( 8 ); // 0x08 == 8 (byte)
	bufferWriter.AppendRgb( Rgba8( 238, 221, 204, 255 ) ); // written as 3 bytes (RGB) only; ignores Alpha
	bufferWriter.AppendByte( 9 ); // 0x09 == 9 (byte)
	bufferWriter.AppendIntVec2( IntVec2( 1920, 1080 ) );
	bufferWriter.AppendVec2( Vec2( -0.6f, 0.8f ) );
	bufferWriter.AppendVertexPCU( Vertex_PCU( Vec3( 3.f, 4.f, 5.f ), Rgba8( 100, 101, 102, 103 ), Vec2( 0.125f, 0.625f ) ) );
}


//----------------------------------------------------------------------------------------------------------------------
void GameModeConvexScene2D::ParseTestFileBufferData( BufferParser& bufferParser, BufferEndian endianMode )
{
	// Parse known test file elements
	bufferParser.SetEndianMode( endianMode );
	char fourCC0_T = bufferParser.ParseChar(); // 'T' == 0x54 hex == 84 decimal
	char fourCC1_E = bufferParser.ParseChar(); // 'E' == 0x45 hex == 84 decimal
	char fourCC2_S = bufferParser.ParseChar(); // 'S' == 0x53 hex == 69 decimal
	char fourCC3_T = bufferParser.ParseChar(); // 'T' == 0x54 hex == 84 decimal
	unsigned char version = bufferParser.ParseByte(); // version 2
	BufferEndian mode = (BufferEndian)bufferParser.ParseByte(); // 1 for little endian, or 2 for big endian
	bool shouldBeFalse = bufferParser.ParseBool(); // written in buffer as byte 0 or 1
	bool shouldBeTrue = bufferParser.ParseBool(); // written in buffer as byte 0 or 1
	unsigned int largeUint = bufferParser.ParseUnsignedInt32(); // 0x12345678
	int negativeSeven = bufferParser.ParseInt32(); // -7 (as signed 32-bit int)
	float oneF = bufferParser.ParseFloat(); // 1.0f
	double pi = bufferParser.ParseDouble(); // 3.1415926535897932384626433832795 (or as best it can)

	std::string helloString, isThisThingOnString;
	helloString			= bufferParser.ParseStringZeroTerminated(); // written with a trailing 0 ('\0') after (6 bytes total)
 	isThisThingOnString = bufferParser.ParseStringAfter32BitLength(); // written as uint 17, then 17 characters (no zero-terminator after)

	Rgba8 rustColor = bufferParser.ParseRgba8(); // Rgba8( 200, 100, 50, 255 )
	unsigned char eight = bufferParser.ParseByte(); // 0x08 == 8 (byte)
	Rgba8 seashellColor = bufferParser.ParseRgb(); // Rgba8( 238, 221, 204) written as 3 bytes (RGB) only; assume alpha is 255
	unsigned char nine = bufferParser.ParseByte(); // 0x09 == 9 (byte)
	IntVec2 highDefRes = bufferParser.ParseIntVec2(); // IntVector2( 1920, 1080 )
	Vec2 normal2D = bufferParser.ParseVec2(); // Vector2( -0.6f, 0.8f )
	Vertex_PCU vertex = bufferParser.ParseVertexPCU(); // VertexPCU( 3.f, 4.f, 5.f, Rgba(100,101,102,103), 0.125f, 0.625f ) );

	// Validate actual values parsed
	GUARANTEE_RECOVERABLE(fourCC0_T == 'T', "Invalid Parse Char");
	GUARANTEE_RECOVERABLE(fourCC1_E == 'E', "Invalid Parse Char");
	GUARANTEE_RECOVERABLE(fourCC2_S == 'S', "Invalid Parse Char");
	GUARANTEE_RECOVERABLE(fourCC3_T == 'T', "Invalid Parse Char");
	GUARANTEE_RECOVERABLE(version == 2, "Error Parsing ParseByte");
	GUARANTEE_RECOVERABLE(mode == endianMode, "Error Parsing Endian Parse BYte"); // verify that we're receiving things in the endianness we expect
	GUARANTEE_RECOVERABLE(shouldBeFalse == false, "Error Parsing boolean");
	GUARANTEE_RECOVERABLE(shouldBeTrue == true, "Error Parsing boolean");
	GUARANTEE_RECOVERABLE(largeUint == 0x12345678, "Error Parsing uint32");
	GUARANTEE_RECOVERABLE(negativeSeven == -7, "Error Parsing int32");
	GUARANTEE_RECOVERABLE(oneF == 1.f, "Error Parsing FLOAT");
	GUARANTEE_RECOVERABLE(pi == 3.1415926535897932384626433832795, "Error Parsing Double");
	GUARANTEE_RECOVERABLE(helloString == "Hello", "Error Parsing helloString");
	GUARANTEE_RECOVERABLE(isThisThingOnString == "Is this thing on?", "Error Parsing is this thing on string");
	GUARANTEE_RECOVERABLE(rustColor == Rgba8(200, 100, 50, 255), "Error Parsing RGBA");
	GUARANTEE_RECOVERABLE(eight == 8, "Error Parsing byte");
	GUARANTEE_RECOVERABLE(seashellColor == Rgba8(238, 221, 204), "Error Parsing RGB");
	GUARANTEE_RECOVERABLE(nine == 9, "Error Parsing byte");
	GUARANTEE_RECOVERABLE(highDefRes == IntVec2(1920, 1080), "Error Parsing Intvec2");
	GUARANTEE_RECOVERABLE(normal2D == Vec2(-0.6f, 0.8f), "Error Parsing Vec2");
	GUARANTEE_RECOVERABLE(vertex.m_position == Vec3(3.f, 4.f, 5.f), "Error Parsing vertex position");
	GUARANTEE_RECOVERABLE(vertex.m_color == Rgba8(100, 101, 102, 103), "Error Parsing vertex color");
	GUARANTEE_RECOVERABLE(vertex.m_uvTexCoords == Vec2(0.125f, 0.625f), "Error Parsing vertex tex coords");
}