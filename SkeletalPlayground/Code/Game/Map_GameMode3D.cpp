#include "Game/Map_GameMode3D.hpp"
#include "Game/GameMode3D.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/FoodManager.hpp"

#include "Engine/ThirdParty/Squirrel/Noise/SmoothNoise.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


//----------------------------------------------------------------------------------------------------------------------
Map_GameMode3D::Map_GameMode3D( GameMode3D* game )
{
	m_gameMode3D = game;
	InitializeBlocks();

	InitDynamicTerrain();

	//----------------------------------------------------------------------------------------------------------------------
	// Initialize food orbs
	//----------------------------------------------------------------------------------------------------------------------
//	m_foodManager = new FoodManager( 1, m_gameMode3D );
}


//----------------------------------------------------------------------------------------------------------------------
Map_GameMode3D::~Map_GameMode3D()
{
	m_gameMode3D = nullptr;
	delete m_vbo;
	m_vbo = nullptr;
	delete m_ibo;
	m_ibo = nullptr;
	delete m_foodManager;
	m_foodManager = nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
void Map_GameMode3D::Update( float deltaSeconds )
{
	UpdateDynamicTerrain( deltaSeconds );
	//----------------------------------------------------------------------------------------------------------------------
	// Update Dynamic Terrain and "elevators"
	//----------------------------------------------------------------------------------------------------------------------
	float time	= float( g_theApp->m_gameClock.GetTotalSeconds() );
	float sine	= SinDegrees( time * 75.0f ) * deltaSeconds * 4.0f;
	float cos	= CosDegrees( time * 75.0f ) * deltaSeconds * 4.0f;
	for ( int i = 1; i < m_blockList.size(); i++ )
	{
		int   mod			 = i % 2;
		float mathWaveToUse	 = sine;
		if ( mod == 0 )
		{
			mathWaveToUse	 = cos;
		}
		Block* curBlock  = m_blockList[i];
		Vec3 blockCenter = curBlock->m_aabb3.GetCenter();
		curBlock->m_aabb3.SetCenterXYZ( Vec3( blockCenter.x, blockCenter.y, blockCenter.z + mathWaveToUse ) );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Raycast test logic
	//----------------------------------------------------------------------------------------------------------------------
	float rayMaxLength = 100.0f;
	RaycastResult3D tempRayResult;
	tempRayResult.m_rayMaxLength				= rayMaxLength;
	m_gameMode3D->m_rayVsTri					= tempRayResult;
	m_gameMode3D->m_rayVsTri.m_rayStartPosition	= m_gameMode3D->m_gameMode3DWorldCamera.m_position;
	m_gameMode3D->m_rayVsTri.m_rayFwdNormal		= m_gameMode3D->m_gameMode3DWorldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp();

	float t, u, v = 0.0f;
	for ( int i = i = 0; i < m_indexList.size(); i += 3 )
	{
		int const& currentIndex_A = m_indexList[ i + 0];
		int const& currentIndex_B = m_indexList[ i + 1];
		int const& currentIndex_C = m_indexList[ i + 2];

		//  m_vbo->m_buffer[ currentIndex ];
		Vec3 vert0 = m_planeVerts[ currentIndex_A ].m_position;
		Vec3 vert1 = m_planeVerts[ currentIndex_B ].m_position;
		Vec3 vert2 = m_planeVerts[ currentIndex_C ].m_position;

		tempRayResult = RaycastVsTriangle( m_gameMode3D->m_gameMode3DWorldCamera.m_position, m_gameMode3D->m_gameMode3DWorldCamera.m_orientation.GetForwardDir_XFwd_YLeft_ZUp(), 
																	m_gameMode3D->m_rayVsTri.m_rayMaxLength, vert0, vert1, vert2, t, u, v );
		if ( tempRayResult.m_didImpact )
		{
			// If ray hit AND is closer
			if ( tempRayResult.m_impactDist < m_gameMode3D->m_rayVsTri.m_impactDist )
			{
				m_gameMode3D->m_rayVsTri = tempRayResult;
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Lerp foodBalls to goal positions
	//----------------------------------------------------------------------------------------------------------------------
	m_foodManager->Update( deltaSeconds );

	//----------------------------------------------------------------------------------------------------------------------
	// Move "elevator" using sine
	//----------------------------------------------------------------------------------------------------------------------
//	sine				= SinDegrees( time * 100.0f );
//	Vec3 elevatorCenter = m_elevator_1->m_aabb3.GetCenter();
//	m_elevator_1->m_aabb3.SetCenterXYZ( Vec3( elevatorCenter.x, elevatorCenter.y, elevatorCenter.z + sine ) );
}


//----------------------------------------------------------------------------------------------------------------------
void Map_GameMode3D::Render() const
{
	std::vector<Vertex_PCU> skyVerts;
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> verts_Blocks;
	verts_Blocks.reserve( 90'000 );
	verts.reserve( 441 );
	if ( g_debugRenderRaycast_F2 )
	{
// 		for ( int i = 0; i < m_planeVerts.size(); i++ )
// 		{
// 			Vertex_PCU const& currentVert = m_planeVerts[ i ];
// 			AddVertsForSphere3D( verts, currentVert.m_position, 1.0f, 4.0f, 4.0f, Rgba8::MAGENTA );
// 		}
	}
	// Skybox (old)
//	AddVertsForSphere3D( skyVerts, Vec3::ZERO, 800.0f, 32.0f, 32.0f );
	//----------------------------------------------------------------------------------------------------------------------
	// Render skybox
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> verts_skybox;
	AddVertsForSphere3D( verts_skybox, Vec3( 0.0f, 0.0f, 100.0f ), 1000.0f, 32.0f, 32.0f );
	//	AddVertsForQuad3D( verts_skybox, Vec3(1000.0f, -1000.0f, -1000.0f), Vec3(-1000.0f, -1000.0f, -1000.0f), Vec3(-1000.0f, -1000.0f, 1000.0f), Vec3(1000.0f, -1000.0f, 1000.0f), Rgba8::WHITE, AABB2::ZERO_TO_ONE );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theApp->m_texture_Skybox );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_skybox.size() ), verts_skybox.data() );
	// Reset bindings
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
 
 
	// Blocks
	RenderBlocks( verts_Blocks );

	//----------------------------------------------------------------------------------------------------------------------
	// Render food orbs
	//----------------------------------------------------------------------------------------------------------------------
//	m_foodManager->Render( verts );

	// Render plane
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), Rgba8::LIGHTBLUE );
	g_theRenderer->BindTexture( g_theApp->m_texture_MoonSurface );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->DrawVertexAndIndexBuffer( m_vbo, m_ibo, int( m_indexList.size() ) );
	// Reset bindings
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );

	// Render blocks
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( Mat44(), Rgba8(80, 109, 80, 255) );
	g_theRenderer->BindTexture( g_theApp->m_texture_MoonSurface );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( static_cast<int>( verts_Blocks.size() ), verts_Blocks.data() );
	g_theRenderer->BindTexture( nullptr );

	// Debug Render vert positions
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( g_theApp->m_texture_Galaxy );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( int( skyVerts.size() ), skyVerts.data() );
	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );

	// Debug Render vert positions
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->DrawVertexArray( int( verts.size() ), verts.data() );
}


//----------------------------------------------------------------------------------------------------------------------
void Map_GameMode3D::RenderBlocks( std::vector<Vertex_PCU>& verts ) const
{
	//----------------------------------------------------------------------------------------------------------------------
	// New version without colors
	//----------------------------------------------------------------------------------------------------------------------
	AddVertsForAABB3D( verts, m_floor_NW->m_aabb3 ); 
	AddVertsForAABB3D( verts, m_floor_SE->m_aabb3 ); 
	AddVertsForAABB3D( verts, m_floor_SW->m_aabb3 ); 

	//----------------------------------------------------------------------------------------------------------------------
	// OG Version with colors
	//----------------------------------------------------------------------------------------------------------------------
	// Floors
// 	AddVertsForAABB3D( verts, m_floor_NE->m_aabb3, Rgba8::DARK_YELLOW	); 
// 	AddVertsForAABB3D( verts, m_floor_NW->m_aabb3, Rgba8::DARKER_GRAY	); 
// 	AddVertsForAABB3D( verts, m_floor_SE->m_aabb3, Rgba8::DARKER_GRAY	); 
// 	AddVertsForAABB3D( verts, m_floor_SW->m_aabb3, Rgba8::INDIGO		); 
// 	// Boxes
// 	AddVertsForAABB3D( verts,   m_box1_E->m_aabb3, Rgba8::DARK_GREEN	); 
// 	AddVertsForAABB3D( verts,  m_box2_NE->m_aabb3, Rgba8::DARK_GREEN	);
// 	AddVertsForAABB3D( verts,  m_box3_SE->m_aabb3, Rgba8::DARK_GREEN	);
// 	AddVertsForAABB3D( verts,  m_box4_SE->m_aabb3, Rgba8::DARKER_RED	);
// 	// Stairs
// 	AddVertsForAABB3D( verts,  m_stairs1->m_aabb3, Rgba8::DARK_YELLOW ); 
// 	AddVertsForAABB3D( verts,  m_stairs2->m_aabb3, Rgba8::DARKER_GRAY );
// 	AddVertsForAABB3D( verts,  m_stairs3->m_aabb3, Rgba8::DARK_YELLOW );
// 	AddVertsForAABB3D( verts,  m_stairs4->m_aabb3, Rgba8::DARKER_GRAY );
// 	AddVertsForAABB3D( verts,  m_stairs5->m_aabb3, Rgba8::DARK_YELLOW );
// 	AddVertsForAABB3D( verts,  m_stairs6->m_aabb3, Rgba8::DARKER_GRAY );
// 	AddVertsForAABB3D( verts,  m_stairs7->m_aabb3, Rgba8::DARK_YELLOW );
// 	AddVertsForAABB3D( verts,  m_stairs8->m_aabb3, Rgba8::DARKER_GRAY );
// 	AddVertsForAABB3D( verts,  m_stairs9->m_aabb3, Rgba8::DARK_YELLOW );
// 	AddVertsForAABB3D( verts, m_stairs10->m_aabb3, Rgba8::DARKER_GRAY );
// 	// Slopes							
// 	AddVertsForAABB3D( verts, m_slope_SE->m_aabb3, Rgba8::PINK			);
// 	// Elevator
// 	AddVertsForAABB3D( verts, m_elevator_1->m_aabb3, Rgba8::PINK		);
// 	AddVertsForAABB3D( verts, m_elevator_2->m_aabb3, Rgba8::PINK		);
// 	// Cliff
// 	AddVertsForAABB3D( verts,	   m_cliff->m_aabb3, Rgba8::PINK		);
}


//----------------------------------------------------------------------------------------------------------------------
void Map_GameMode3D::InitializeBlocks()
{
	//----------------------------------------------------------------------------------------------------------------------
	// WalkableObjects (Floors, environmental proxies)
	//----------------------------------------------------------------------------------------------------------------------
	// Floors
//	m_blockList.emplace_back( m_floor_NE ); 
	m_blockList.emplace_back( m_floor_NW ); 
	m_blockList.emplace_back( m_floor_SE ); 
	m_blockList.emplace_back( m_floor_SW ); 
	// Boxes
//	m_box1_E->m_aabb3.SetCenterXYZ( Vec3( 300.0f,  -80.0f,  0.0f ) );
//	m_box2_NE->m_aabb3.SetCenterXYZ( Vec3( 250.0f,  100.0f,  0.0f ) );
//	m_box3_SE->m_aabb3.SetCenterXYZ( Vec3( 250.0f, -150.0f,  0.0f ) );
//	m_box4_SE->m_aabb3.SetCenterXYZ( Vec3( 250.0f,  -80.0f, 10.0f ) );
//	m_blockList.emplace_back( m_box1_E  ); 
//	m_blockList.emplace_back( m_box2_NE );
//	m_blockList.emplace_back( m_box3_SE );
//	m_blockList.emplace_back( m_box4_SE );
//	// Stairs
//	float startHeight	   = -5.0f;
//	float startDecrement   =  5.0f;
//	float xOffset		   = 205.0f;
//	float xOffsetIncrement =  15.0f;
//	m_stairs1->m_aabb3. SetCenterXYZ( Vec3( xOffset, -20.0f, startHeight ) );
//	m_stairs2->m_aabb3. SetCenterXYZ( Vec3( xOffset += xOffsetIncrement, -20.0f, startHeight -= startDecrement ) ); 
//	m_stairs3->m_aabb3. SetCenterXYZ( Vec3( xOffset += xOffsetIncrement, -20.0f, startHeight -= startDecrement ) ); 
//	m_stairs4->m_aabb3. SetCenterXYZ( Vec3( xOffset += xOffsetIncrement, -20.0f, startHeight -= startDecrement ) ); 
//	m_stairs5->m_aabb3. SetCenterXYZ( Vec3( xOffset += xOffsetIncrement, -20.0f, startHeight -= startDecrement ) ); 
//	m_stairs6->m_aabb3. SetCenterXYZ( Vec3( xOffset += xOffsetIncrement, -20.0f, startHeight -= startDecrement ) ); 
//	m_stairs7->m_aabb3. SetCenterXYZ( Vec3( xOffset += xOffsetIncrement, -20.0f, startHeight -= startDecrement ) ); 
//	m_stairs8->m_aabb3. SetCenterXYZ( Vec3( xOffset += xOffsetIncrement, -20.0f, startHeight -= startDecrement ) ); 
//	m_stairs9->m_aabb3. SetCenterXYZ( Vec3( xOffset += xOffsetIncrement, -20.0f, startHeight -= startDecrement ) ); 
//	m_stairs10->m_aabb3.SetCenterXYZ( Vec3( xOffset += xOffsetIncrement, -20.0f, startHeight -= startDecrement ) ); 
//	m_blockList.emplace_back( m_stairs1  );
//	m_blockList.emplace_back( m_stairs2  );
//	m_blockList.emplace_back( m_stairs3  );
//	m_blockList.emplace_back( m_stairs4  );
//	m_blockList.emplace_back( m_stairs5  );
//	m_blockList.emplace_back( m_stairs6  );
//	m_blockList.emplace_back( m_stairs7  );
//	m_blockList.emplace_back( m_stairs8  );
//	m_blockList.emplace_back( m_stairs9  );
//	m_blockList.emplace_back( m_stairs10 );
//	// Slopes
//	m_slope_SE->m_aabb3.SetCenterXYZ( Vec3( 210.0f, -80.0f, 0.0f ) ); 
//	m_blockList.emplace_back( m_slope_SE );
//	// Elevator
//	m_elevator_1->m_aabb3.SetCenterXYZ( Vec3(  50.0f, -270.0f, 0.0f ) ); 
//	m_elevator_2->m_aabb3.SetCenterXYZ( Vec3( 200.0f, -270.0f, 0.0f ) ); 
//	m_blockList.emplace_back( m_elevator_1 );
//	m_blockList.emplace_back( m_elevator_2 );
	// Cliff
//	m_cliff->m_aabb3.SetCenterXY( Vec3( 50.0f, 500.0f, 0.0f ) );
//	m_cliff->m_aabb3.SetNewZ( 0.0f );
//	m_blockList.emplace_back( m_cliff );
}


//----------------------------------------------------------------------------------------------------------------------
void Map_GameMode3D::InitDynamicTerrain()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Initialize randomized Z plane
	//----------------------------------------------------------------------------------------------------------------------
	// Randomize floor height
	float height = 22.0f;
//	AddVertsForPlane( m_planeVerts, m_indexList, Vec3(5.0f, 5.0f, -5.0f), 10, 20, 20 );
	AddVertsForPlane( m_planeVerts, m_indexList, Vec3(0.0f, 0.0f, -5.0f), 15, 13, 13 );
	for ( int i = 0; i < m_planeVerts.size(); i++ )
	{
		Vertex_PCU& currentVert		= m_planeVerts[ i ];
		currentVert.m_position.z	= height;
		float rand					= g_theRNG->RollRandomFloatInRange( m_minFloorHeight, m_maxFloorHeight );
		height						+= rand;
	}
	m_vbo = g_theRenderer->CreateVertexBuffer( m_planeVerts.size(), sizeof(Vertex_PCU) );
	m_ibo = g_theRenderer->CreateIndexBuffer (  m_indexList.size() );
	g_theRenderer->Copy_CPU_To_GPU( m_planeVerts.data(), sizeof( Vertex_PCU )   * m_planeVerts.size(), m_vbo, sizeof( Vertex_PCU ) );
	g_theRenderer->Copy_CPU_To_GPU(  m_indexList.data(), sizeof( unsigned int ) *  m_indexList.size(), m_ibo );
}


//----------------------------------------------------------------------------------------------------------------------
void Map_GameMode3D::UpdateDynamicTerrain( float deltaSeconds )
{
	float noiseTimeScale = 20.0f;
	m_noiseTime += deltaSeconds * noiseTimeScale;
	float baseHeight = 10.0f;
	for ( int i = 0; i < m_planeVerts.size(); i++ )
	{
		Vertex_PCU& currentVert		= m_planeVerts[ i ];
		float rand					= Compute3dPerlinNoise( currentVert.m_position.x, currentVert.m_position.y, m_noiseTime, 50.0f );
		float offset				= rand * 10.0f;
		float height				= baseHeight + offset;
		currentVert.m_position.z	= height;
	}
	g_theRenderer->Copy_CPU_To_GPU( m_planeVerts.data(), sizeof( Vertex_PCU )   * m_planeVerts.size(), m_vbo, sizeof( Vertex_PCU ) );
	g_theRenderer->Copy_CPU_To_GPU(  m_indexList.data(), sizeof( unsigned int ) *  m_indexList.size(), m_ibo );
}
