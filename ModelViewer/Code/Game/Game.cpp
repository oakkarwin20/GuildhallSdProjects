#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"
#include "Game/Model.hpp"

#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

Model* g_theModel = nullptr;

//----------------------------------------------------------------------------------------------------------------------
Game::Game()
{
	// Initializing textures;
	m_testTexture	= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/TestUV.png" );
	m_grassTexture	= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Grass_Diffuse.png" );
	m_brickTexture	= g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Brick_Diffuse.png" );

	g_theEventSystem->SubscribeToEvent( "LoadModel", Command_LoadModel );
}

//----------------------------------------------------------------------------------------------------------------------
Game::~Game()
{
	delete g_theModel;
	g_theModel = nullptr;
	
	for ( int i = 0; i < m_entityList.size(); i++ )
	{
		delete m_entityList[ i ];
		m_entityList[ i ] = nullptr;
	}

	m_player = nullptr;

	delete m_vboArrow;
	m_vboArrow = nullptr;

	delete m_grassMaterial;
	delete m_brickMaterial;
	delete m_tutorialBoxMaterial;
	m_grassMaterial			= nullptr;
	m_brickMaterial			= nullptr;
	m_tutorialBoxMaterial	= nullptr;

	m_prop_Sphere	= nullptr;
	m_prop_Cube		= nullptr;

	m_testTexture		= nullptr;
	m_grassTexture		= nullptr;
	m_brickTexture		= nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void Game::StartUp()
{
	if (m_AttractModeIsOn)
	{
		// Get main menu visuals verts
		InitializeAttractModeVerts();
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Adding player to scene
	m_player									  = new Player(this);
	m_player->m_position						  = Vec3( 3.0f, 0.0f, 3.0f );
	m_player->m_orientationDegrees.m_yawDegrees	  = 180.0f;
	m_player->m_orientationDegrees.m_pitchDegrees =  45.0f;
	m_entityList.push_back(m_player);

	// Adding Prop(s) and Sphere to scene
	Prop* m_propGridLines	 = new Prop( this );
	m_entityList.push_back( m_propGridLines );

	//----------------------------------------------------------------------------------------------------------------------
	// Camera
	UpdateCameras();

/*
	//----------------------------------------------------------------------------------------------------------------------
	// Set Cube position
	m_propRotatingCube->m_position = Vec3(  2.0f,  2.0f, 0.0f );
	m_propBlinkingCube->m_position = Vec3( -2.0f, -2.0f, 0.0f );

	// Add Cubes to scene
	m_propRotatingCube->AddCubeToScene( 0.5f, 0.5f, 0.5f );
	m_propBlinkingCube->AddCubeToScene( 0.5f, 0.5f, 0.5f );
	m_propBlinkingCube->m_isBlinking = true;

	// Set Cube pitch and roll degrees
	m_propRotatingCube->m_angularVelocity.m_pitchDegrees = 30.0f;
	m_propRotatingCube->m_angularVelocity.m_rollDegrees  = 30.0f;

	//----------------------------------------------------------------------------------------------------------------------
	// Set Sphere position
	m_propSphere->m_position = Vec3( 10.0f, -5.0f, 1.0f );

	// Add Sphere to scene
	m_propSphere->AddSphereToScene( Vec3(0.0f, 0.0f, 0.0f), 1.0f, 32.0f, 16.0f, m_testTexture );

	// Rotate Sphere
	m_propSphere->m_angularVelocity.m_yawDegrees = 45.0f;		
*/
	//----------------------------------------------------------------------------------------------------------------------
	// Set Grid Lines in scene
	m_propGridLines->AddGridLinesToScene();
	
	// Render DRS world Basis and text
	Render_DRS_WorldBasisText();
	Render_DRS_WorldBasis();

	//----------------------------------------------------------------------------------------------------------------------
	g_theModel = new Model( this );
//	g_theModel->ParseXmlData( "Data/Models/Cube_v.xml" );
// 	g_theModel->ParseXmlData( "Data/Models/Cube_vf.xml" );
// 	g_theModel->ParseXmlData( "Data/Models/Cube_vfn.xml" );
// 	g_theModel->ParseXmlData( "Data/Models/Cube_vfnt.xml" );
// 	g_theModel->ParseXmlData( "Data/Models/Cow.xml" );
// 	g_theModel->ParseXmlData( "Data/Models/Skyscraper.xml" );
// 	g_theModel->ParseXmlData( "Data/Models/SovietMainBattleTank.xml" );
//	g_theModel->ParseXmlData( "Data/Models/Tank1.xml" );
//	g_theModel->ParseXmlData( "Data/Models/Tank2.xml" );
//	g_theModel->ParseXmlData( "Data/Models/Tank3.xml" );
//	g_theModel->ParseXmlData( "Data/Models/Tank4.xml" );
//	g_theModel->ParseXmlData( "Data/Models/Teapot.xml" );
//	g_theModel->ParseXmlData( "Data/Models/Teddy.xml" );

	// Load model based on gameConfig
	std::string modelToLoad = g_gameConfigBlackboard.GetValue( "modelToLoad", std::string( "Invalid Model" ) );
	g_theModel->ParseXmlData( "Data/Models/" + modelToLoad );

	//	FileReadToString( outString, "Data/Models/Cube_v.xml" );
	//	FileReadToString( outString, "Data/Models/Cube_vf.xml" );
	//	FileReadToString( outString, "Data/Models/Cube_vfn.xml" );
	//	FileReadToString( outString, "Data/Models/Cube_vfnt.xml" );
	//	FileReadToString( outString, "Data/Models/Cow.xml" );
	//	FileReadToString( outString, "Data/Models/Skyscraper.xml" );
	//	FileReadToString( outString, "Data/Models/SovietMainBattleTank.xml" );
	//	FileReadToString( outString, "Data/Models/Tank1.xml" );
	//	FileReadToString( outString, "Data/Models/Tank2.xml" );
	//	FileReadToString( outString, "Data/Models/Tank3.xml" );
	//	FileReadToString( outString, "Data/Models/Tank4.xml" );
	//	FileReadToString( outString, "Data/Models/Teapot.xml" );
	//	FileReadToString( outString, "Data/Models/Teddy.xml" );

	// Load Materials
	m_grassMaterial			= new Material( "Data/Materials/Grass.xml"		  );
	m_brickMaterial			= new Material( "Data/Materials/Brick.xml"		  );
	m_tutorialBoxMaterial	= new Material( "Data/Materials/Tutorial_Box.xml" );

	//----------------------------------------------------------------------------------------------------------------------
	// Init index sphere (normal, biNormal, tangent)
	//----------------------------------------------------------------------------------------------------------------------
	m_prop_Sphere = new Prop( this );
	AddVertsForSphere3D( m_prop_Sphere->m_vertList_PCUTBN, m_prop_Sphere->m_indexList, Vec3::ZERO, 1.0f, 16.0f, 16.0f );
	// Set Cube pitch and roll degrees
	m_prop_Sphere->m_angularVelocity.m_yawDegrees = 45.0f;
	CalculateTangents  ( m_prop_Sphere->m_vertList_PCUTBN, m_prop_Sphere->m_indexList );
	m_prop_Sphere->m_vbo = g_theRenderer->CreateVertexBuffer( m_prop_Sphere->m_vertList_PCUTBN.size(), sizeof( Vertex_PCUTBN ) );
	m_prop_Sphere->m_ibo = g_theRenderer->CreateIndexBuffer ( m_prop_Sphere->m_indexList.size() );
	g_theRenderer->Copy_CPU_To_GPU( m_prop_Sphere->m_vertList_PCUTBN.data(), sizeof( Vertex_PCUTBN ) *  m_prop_Sphere->m_vertList_PCUTBN.size(), m_prop_Sphere->m_vbo, sizeof( Vertex_PCUTBN ) );
	g_theRenderer->Copy_CPU_To_GPU(  m_prop_Sphere->m_indexList.data(), sizeof( unsigned int )  *   m_prop_Sphere->m_indexList.size(), m_prop_Sphere->m_ibo );
	m_entityList.push_back( m_prop_Sphere );

	//----------------------------------------------------------------------------------------------------------------------
	// Init index cube (normal, biNormal, tangent)
	//----------------------------------------------------------------------------------------------------------------------
	m_prop_Cube = new Prop( this );
	m_prop_Cube->m_angularVelocity.m_yawDegrees = 45.0f;
	AABB3 bounds = AABB3( Vec3( -1.0f, -1.0f, -1.0f ), Vec3( 1.0f, 1.0f, 1.0f ) );
	AddvertsforCube3D( m_prop_Cube->m_vertList_PCUTBN, m_prop_Cube->m_indexList, bounds );
	CalculateTangents( m_prop_Cube->m_vertList_PCUTBN, m_prop_Cube->m_indexList );
	m_prop_Cube->m_vbo = g_theRenderer->CreateVertexBuffer( m_prop_Cube->m_vertList_PCUTBN.size(), sizeof( Vertex_PCUTBN ) );
	m_prop_Cube->m_ibo = g_theRenderer->CreateIndexBuffer( m_prop_Cube->m_indexList.size() );
	g_theRenderer->Copy_CPU_To_GPU( m_prop_Cube->m_vertList_PCUTBN.data(), sizeof( Vertex_PCUTBN ) * m_prop_Cube->m_vertList_PCUTBN.size(), m_prop_Cube->m_vbo, sizeof( Vertex_PCUTBN ) );
	g_theRenderer->Copy_CPU_To_GPU(		  m_prop_Cube->m_indexList.data(), sizeof( unsigned int )  *	   m_prop_Cube->m_indexList.size(), m_prop_Cube->m_ibo );
	m_entityList.push_back( m_prop_Cube );

	//----------------------------------------------------------------------------------------------------------------------
	// Light dir arrow
	//----------------------------------------------------------------------------------------------------------------------
	AddVertsForArrow3D( m_arrowVerts, Vec3::ZERO, Vec3( 0.5f, 0.0f, 0.0f ), 0.05f );
	m_vboArrow					= g_theRenderer->CreateVertexBuffer( m_arrowVerts.size(), sizeof( Vertex_PCU ) );
	g_theRenderer->Copy_CPU_To_GPU( m_arrowVerts.data(), sizeof( Vertex_PCU ) * m_arrowVerts.size(), m_vboArrow, sizeof( Vertex_PCU ) );
	m_arrowYPR.m_pitchDegrees	= 90.0f;
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Shutdown()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Update()
{
	float deltaSeconds = m_clock.GetDeltaSeconds();

	if (m_AttractModeIsOn)
	{
		// Take input from attractModeInput()
		AttractModeInput();
		return;
	}

//	LoadModel();
//	Update_DRS_Input();
	UpdatePauseQuitAndSlowMo();
	UpdateReturnToAttractMode();

/*
	if ( m_prop_Sphere->m_angularVelocity.m_yawDegrees != 45.0f )
	{
		m_prop_Sphere->m_angularVelocity.m_yawDegrees = 45.0f;
	}
	if ( m_prop_Cube->m_angularVelocity.m_yawDegrees != 45.0f )
	{
		m_prop_Cube->m_angularVelocity.m_yawDegrees = 45.0f;
	}
*/

	//----------------------------------------------------------------------------------------------------------------------
	// Render input
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->WasKeyJustPressed( '1' ) )
	{
		m_prop_Cube->m_orientationDegrees.m_yawDegrees = 0.0f;
		m_objectToRender = CUBE_GRASS;
	}
	if ( g_theInput->WasKeyJustPressed( '2' ) )
	{
		m_prop_Sphere->m_orientationDegrees.m_yawDegrees = 0.0f;
		m_prop_Sphere->m_angularVelocity.m_yawDegrees	 = 0.0f;
		m_objectToRender = SPHERE_GRASS;
	}
	if ( g_theInput->WasKeyJustPressed( '3' ) )
	{
		m_prop_Cube->m_orientationDegrees.m_yawDegrees = 0.0f;
		m_objectToRender = CUBE_BRICKS;
	}
	if ( g_theInput->WasKeyJustPressed( '4' ) )
	{
		m_prop_Sphere->m_orientationDegrees.m_yawDegrees = 0.0f;
		m_prop_Sphere->m_angularVelocity.m_yawDegrees	 = 0.0f;
		m_objectToRender = SPHERE_BRICKS;
	}
	if ( g_theInput->WasKeyJustPressed( '5' ) )
	{
		g_theModel->m_orientation.m_yawDegrees = 0.0f;
		m_objectToRender = MODEL;
	}
	if ( g_theInput->WasKeyJustPressed( 'B' ) )
	{
		m_renderTBN = !m_renderTBN;
	}
	g_theModel->m_orientation.m_yawDegrees += ( 45.0f * deltaSeconds );

	//----------------------------------------------------------------------------------------------------------------------
	// Update sun dir arrow input
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->IsKeyDown( KEYCODE_DOWNARROW ) )
	{
		m_arrowYPR.m_pitchDegrees -= 45.0f * deltaSeconds;	
	}
	if ( g_theInput->IsKeyDown( KEYCODE_UPARROW ) )
	{
		m_arrowYPR.m_pitchDegrees += 45.0f * deltaSeconds;
	}
	if ( g_theInput->IsKeyDown( KEYCODE_LEFTARROW ) )
	{
		m_arrowYPR.m_yawDegrees += 45.0f * deltaSeconds;
	}
	if ( g_theInput->IsKeyDown( KEYCODE_RIGHTARROW ) )
	{
		m_arrowYPR.m_yawDegrees -= 45.0f * deltaSeconds;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_COMMA ) )
	{
		m_sunIntensity		+= 0.1f;
		m_ambientIntensity  -= 0.1f;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_PERIOD ) )
	{
		m_sunIntensity		-= 0.1f;
		m_ambientIntensity  += 0.1f;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_SEMICOLON ) )
	{
		m_specularIntensity += 0.1f;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_SINGLEQUOTE ) )
	{
		m_specularIntensity -= 0.1f;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFTBRACKET ) )
	{
		m_specularPower += 0.1f;
	}
	if ( g_theInput->WasKeyJustPressed( KEYCODE_RIGHTBRACKET ) )
	{
		m_specularPower -= 0.1f;
	}
	m_sunIntensity		= GetClamped(      m_sunIntensity, 0.0f, 1.0f );
	m_ambientIntensity	= GetClamped(  m_ambientIntensity, 0.0f, 1.0f );
	m_specularIntensity = GetClamped( m_specularIntensity, 0.0f, 1.0f );
	if ( g_theInput->WasKeyJustPressed( 'N' ) )
	{
		m_isNormalsTexture = !m_isNormalsTexture;
	}
	if ( g_theInput->WasKeyJustPressed( 'M' ) )
	{
		m_isSpecTexture = !m_isSpecTexture;
	}
//	DebuggerPrintf( Stringf( "sphereYPR %0.2f\n", m_prop_Sphere->m_orientationDegrees.m_yawDegrees ).c_str() );
	m_prop_Sphere->m_orientationDegrees.m_yawDegrees = 0.0f;
	
	UpdateEntities( deltaSeconds );
	UpdateCameras();
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Clear screen
	g_theRenderer->ClearScreen( Rgba8::LIGHTBLACK );

	// Draw attract mode
	if (m_AttractModeIsOn)
	{
		// Clear AttractMode screen
		g_theRenderer->ClearScreen( Rgba8::DARK_RED );
		// uses attractModeCam and draws relevant stuff
		RenderAttractMode();
		return;
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Begin WorldCamera
	g_theRenderer->BeginCamera(m_player->m_worldCamera);

	DebugRenderWorld( m_player->m_worldCamera );
	RenderEntities();

	//----------------------------------------------------------------------------------------------------------------------
	// #ToDo delete
	// Test debugRenderSystem Billboard Text
//if ( g_theInput->IsKeyDown( '4' ) )
//{
//	float duration = 10.0f;
//	std::string text = "Camera Orientation: ";
//	float textHeight = 2.0f;
//	Vec3 alignment = Vec3( 0.0, 0.0f, 0.0f );
//	Vec3 origin = Vec3( 0.0, 0.0f, 0.0f );
//	//		DebugAddWorldBillboardText( text, origin, textHeight, alignment, duration, Rgba8::WHITE, Rgba8::RED, DebugRenderMode::USE_DEPTH );

	//----------------------------------------------------------------------------------------------------------------------
	// Render Model Viewer
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCUTBN> vertexes;
	std::vector<Vertex_PCU>		verts;
	std::vector<unsigned int>   indexes;

	Vec3  sunDir				= m_arrowYPR.GetForwardDir_XFwd_YLeft_ZUp();
	int   normalMode			= 0;
	int   specularMode			= 0;
	if ( !m_isNormalsTexture )
	{
		normalMode = 1;
	}
	if ( !m_isSpecTexture )
	{
		specularMode = 1;
	}
	g_theRenderer->SetLightingConstants( sunDir, m_sunIntensity, m_ambientIntensity, m_player->m_position, normalMode, specularMode, m_specularIntensity, m_specularPower );

	if ( m_objectToRender == NONE )
	{
		// Do nothing
	}
	else if ( m_objectToRender == SPHERE_BRICKS )
	{
		g_theRenderer->BindShader( m_brickMaterial->m_shader );
		g_theRenderer->BindTexture( m_brickMaterial->m_diffuseTexture, m_brickMaterial->m_normalTexture, m_brickMaterial->m_specGlossEmitTexture );
		g_theRenderer->DrawVertexAndIndexBuffer( m_prop_Sphere->m_vbo, m_prop_Sphere->m_ibo, int( m_prop_Sphere->m_indexList.size() ) );
		if ( m_renderTBN )
		{
			RenderNormalLines( m_prop_Sphere->m_vertList_PCUTBN );
		}
	}
	else if ( m_objectToRender == SPHERE_GRASS )
	{
		g_theRenderer->BindShader( m_grassMaterial->m_shader );
		g_theRenderer->BindTexture( m_grassMaterial->m_diffuseTexture, m_grassMaterial->m_normalTexture, m_grassMaterial->m_specGlossEmitTexture );
		g_theRenderer->DrawVertexAndIndexBuffer( m_prop_Sphere->m_vbo, m_prop_Sphere->m_ibo, int( m_prop_Sphere->m_indexList.size() ) );
		if ( m_renderTBN )
		{
			RenderNormalLines( m_prop_Sphere->m_vertList_PCUTBN );
		}
	}
	if ( m_objectToRender == CUBE_BRICKS )
	{
		g_theRenderer->BindShader( m_brickMaterial->m_shader );
		g_theRenderer->BindTexture( m_brickMaterial->m_diffuseTexture, m_brickMaterial->m_normalTexture, m_brickMaterial->m_specGlossEmitTexture );
		g_theRenderer->DrawVertexAndIndexBuffer( m_prop_Cube->m_vbo, m_prop_Cube->m_ibo, int( m_prop_Cube->m_indexList.size() ) );
		if ( m_renderTBN )
		{
			RenderNormalLines( m_prop_Cube->m_vertList_PCUTBN );
		}
	}
	if ( m_objectToRender == CUBE_GRASS )
	{
		g_theRenderer->BindShader( m_grassMaterial->m_shader );
		g_theRenderer->BindTexture( m_grassMaterial->m_diffuseTexture, m_grassMaterial->m_normalTexture, m_grassMaterial->m_specGlossEmitTexture );
		g_theRenderer->DrawVertexAndIndexBuffer( m_prop_Cube->m_vbo, m_prop_Cube->m_ibo, int( m_prop_Cube->m_indexList.size() ) );
		if ( m_renderTBN )
		{
			RenderNormalLines( m_prop_Cube->m_vertList_PCUTBN );
		}
	}
	if ( m_objectToRender == MODEL )
	{
		g_theRenderer->BindShader( m_tutorialBoxMaterial->m_shader );
		g_theModel->Render();
		if ( m_renderTBN )
		{
			RenderNormalLines( g_theModel->m_cpuMesh->m_vertexes );
		}
	}
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( nullptr );

	EulerAngles orientation = EulerAngles::GetAsEulerAnglesFromFwdAndLeftBasis_XFwd_YLeft_ZUp( sunDir, Vec3::ZERO );
	Mat44 arrowMat			= orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	arrowMat.SetTranslation3D( Vec3( 0.0f, 0.0f, 2.0f ) );
	g_theRenderer->SetModelConstants( arrowMat, Rgba8::YELLOW );
	g_theRenderer->DrawVertexBuffer( m_vboArrow,int( m_arrowVerts.size() ) );

	// End m_worldCam
	g_theRenderer->EndCamera(m_player->m_worldCamera);

	// RenderUI()
	RenderUI();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::RenderNormalLines( std::vector<Vertex_PCUTBN> const& verts ) const
{
	std::vector<Vertex_PCU>	vertsToRender;
	vertsToRender.reserve( 170'000 );
	float scalar = 0.2f;
	// Line render for Normals
	for ( int i = 0; i < verts.size(); i++ )
	{
		Vertex_PCUTBN const& currentVert = verts[ i ];
		// Normals
		Vec3 start = currentVert.m_position;
		Vec3 end = start + ( currentVert.m_normal * scalar );
		AddVertsForLine3D( vertsToRender, start, end, Rgba8::BLUE );
		// Tangent
		if ( currentVert.m_tangent != Vec3::ZERO )
		{
			end = start + ( currentVert.m_tangent * scalar );
			AddVertsForLine3D( vertsToRender, start, end, Rgba8::RED );
		}
		if ( currentVert.m_biNormal != Vec3::ZERO )
		{
			// BiNormal
			end = start + ( currentVert.m_biNormal * scalar );
			AddVertsForLine3D( vertsToRender, start, end, Rgba8::GREEN );
		}
	}
	g_theRenderer->BindShader ( nullptr );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( int( vertsToRender.size() ), vertsToRender.data(), PrimitiveTopology::D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_LoadModel( EventArgs& args )
{

	std::string filePath = args.GetValue( "File", "INVALID_FILE" );
	if ( filePath == "INVALID_FILE" )
	{
		return false;
	}
	g_theModel->ParseXmlData( filePath );

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if (g_theInput->WasKeyJustPressed('P') || g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::BUTTON_START))
	{
		SoundID testSound = g_theAudio->CreateOrGetSound("Data/Audio/TestSound.mp3");
		UNUSED( testSound );
//		g_theAudio->StartSound(testSound);			// Comment out this line of code to remove pause sound playing

		m_clock.TogglePause();
	}

	// Slow-Mo functionality
	m_isSlowMo = g_theInput->IsKeyDown('T');
	if (m_isSlowMo)
	{
		m_clock.SetTimeScale( 0.1f );
	}
	if ( g_theInput->WasKeyJustReleased( 'T' ) )
	{
		m_clock.SetTimeScale( 1.0f );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateReturnToAttractMode()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) && m_AttractModeIsOn == false)
	{
		g_theApp->HandleQuitRequested();
//		m_AttractModeIsOn = true;
	}
}
  
//----------------------------------------------------------------------------------------------------------------------
void Game::AttractModeInput()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		g_theApp->HandleQuitRequested();
	}
	
	XboxController const& controller = g_theInput->GetController(0);
	
	if ( g_theInput->WasKeyJustPressed(' ') || g_theInput->WasKeyJustPressed('N') || controller.WasButtonJustPressed(BUTTON_START) || controller.WasButtonJustPressed(BUTTON_A))
	{
		m_AttractModeIsOn = false;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateEntities(float deltaSeconds)
{	
	//----------------------------------------------------------------------------------------------------------------------
	// Call Update() for each entity inside entityList
	for ( int i = 0; i < m_entityList.size(); i++ )
	{
		m_entityList[i]->Update( deltaSeconds );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderEntities() const
{
	for ( int j = 0; j < m_entityList.size(); j++)
	{
		m_entityList[j]->Render();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	// Begin UI Camera
	g_theRenderer->BeginCamera( m_screenCamera );

	// Pass m_screenCamera for DebugRenderScreen to use
	DebugRenderScreen( m_screenCamera );
	Render_DRS_UI_Text();

	// End UI Camera
	g_theRenderer->EndCamera( m_screenCamera );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Update_DRS_Input()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Test debugRenderSystem variables
	Vec3 center		= Vec3( 0.0f, 0.0f, 0.0f );
	float radius	= 1.0f;
	float duration	= 10.0f;

	// Test debugRenderSystem Sphere
	if ( g_theInput->IsKeyDown('1') )
	{
		duration			= 5.0f;
		Mat44 matrix		= m_player->GetModelMatrix( m_player->m_position, m_player->m_orientationDegrees );
		Vec3 iBasis			= matrix.GetIBasis3D();
		Vec3 stepsForward	= (iBasis * 2.0f);
		Vec3 posInFront		= m_player->m_position + stepsForward;
		DebugAddWorldWireSphere( posInFront, radius, duration, Rgba8::GREEN, Rgba8::RED, DebugRenderMode::USE_DEPTH );
	}

	// Test debugRenderSystem Line 
	if ( g_theInput->IsKeyDown( '2' ) )
	{
		duration			= 10.0f;
		Mat44 matrix		= m_player->GetModelMatrix( m_player->m_position, m_player->m_orientationDegrees );
		Vec3 iBasis			= matrix.GetIBasis3D();
		Vec3 stepsForward	= (iBasis * 20.0f);
		Vec3 stepsBehind	= (iBasis * 0.2f);
		Vec3 end			= m_player->m_position + stepsForward;
		Vec3 posBehind		= m_player->m_position - stepsBehind;
		float lineRadius	= 0.1f;
		DebugAddWorldLine( posBehind, end, lineRadius, duration, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::X_RAY );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Test debugRenderSystem Arrow IJK WorldBasis
	if ( g_theInput->IsKeyDown( '3' ) )
	{
		duration			= 20.0f;
		Mat44 matrix		= m_player->GetModelMatrix( m_player->m_position, m_player->m_orientationDegrees );
		Vec3 iBasis			= matrix.GetIBasis3D();
		Vec3 jBasis			= matrix.GetJBasis3D();
		Vec3 kBasis			= matrix.GetKBasis3D();
		Vec3 stepsForward	= (iBasis * 1.5f);
		Vec3 stepsLeft		= (jBasis * 1.5f);
		Vec3 stepsSkyward	= (kBasis * 1.5f);
		Vec3 iEnd			= m_player->m_position + stepsForward;
		Vec3 jEnd			= m_player->m_position + stepsLeft;
		Vec3 kEnd			= m_player->m_position + stepsSkyward;
		radius				= 0.2f;

		// X-axis Red
		DebugAddWorldArrow( m_player->m_position, iEnd, radius, duration, Rgba8::RED, Rgba8::RED );
		// Y-axis Green
		DebugAddWorldArrow( m_player->m_position, jEnd, radius, duration, Rgba8::GREEN, Rgba8::GREEN );
		// Z-axis Blue																					  
		DebugAddWorldArrow( m_player->m_position, kEnd, radius, duration, Rgba8::BLUE, Rgba8::BLUE );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Test debugRenderSystem Billboard Text
	if ( g_theInput->IsKeyDown( '4' ) )
	{
		duration			= 10.0f;
		std::string text	= "Camera Orientation: ";
		float textHeight	= 20.0f;
		Vec2 alignment		= Vec2( 0.5, 0.5f );
//		Vec3 origin			= Vec3( 0.0, 0.0f, 0.0f );
		Vec3 origin			= m_player->m_position;
//		FULL_CAMERA_FACING
		DebugAddWorldBillboardText( text, origin, textHeight, alignment, duration, Rgba8::WHITE, Rgba8::RED, DebugRenderMode::USE_DEPTH );

		//----------------------------------------------------------------------------------------------------------------------
		// Test code
//		std::vector<Vertex_PCU> verts;
//		BitmapFont* g_testFont = nullptr;
//		g_testFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Images/SquirrelFixedFont" );
//		g_testFont->AddVertsForText3D( verts, Vec2(0.0f, 0.0f), 1.0f, text );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Test debugRenderSystem transformable cylinder
	if ( g_theInput->IsKeyDown( '5' ) )
	{
		duration			= 10.0f;
		Mat44 matrix		= m_player->GetModelMatrix( m_player->m_position, m_player->m_orientationDegrees );
		Vec3 iBasis			= matrix.GetIBasis3D();
		Vec3 stepsForward	= (iBasis * 2.0f);
		Vec3 top			= m_player->m_position + stepsForward;

		DebugAddWorldWireCylinder( m_player->m_position + Vec3( 0.0f, 0.0f, 0.0f), m_player->m_position + Vec3( 0.0f, 0.0f, 1.0f ), radius, duration, Rgba8::WHITE, Rgba8::RED, DebugRenderMode::USE_DEPTH );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Test debugRenderSystem World Point
	if ( g_theInput->IsKeyDown( '6' ) )
	{
		duration = 60.0f;
		DebugAddWorldPoint( m_player->m_position, radius, duration, Rgba8::BROWN, Rgba8::BROWN, DebugRenderMode::USE_DEPTH );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Test debugRenderSystem UI Text
	if ( g_theInput->IsKeyDown( '7' ) )
	{
		duration			= 5.0f;
		std::string text	= "Camera Orientation: ";
		Vec2 position		= Vec2 ( SCREEN_SIZE_X * 0.1f, SCREEN_SIZE_Y * 0.9f );
		float size			= 2.0f;
		Vec2 alignment		= Vec2( 0.0, 0.0f );
		DebugAddScreenText( text, position, size, alignment, duration );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Render_DRS_WorldBasisText()
{
	// DebugRenderSystem world Text	// World basis text
	float duration				= 10.0f;
	std::string xForwardText	= "X - Forward";
	std::string yLeftText		= "Y - Left";
	std::string zUpText			= "Z - Up";

	float textHeight			= 0.5f;
	Vec2 alignment				= Vec2( 0.0, 0.0f );

	Mat44 xTransform;
	Mat44 yTransform;
	Mat44 zTransform;
	xTransform.AppendZRotation( -90.0f );
	yTransform.AppendZRotation( 180.0f );
	zTransform.AppendYRotation( 180.0f );
	zTransform.AppendXRotation( -90.0f );

	xTransform.SetTranslation3D( Vec3( 0.2f,  0.0f, 0.2f ) );
	yTransform.SetTranslation3D( Vec3( 0.0f,  4.2f, 0.2f ) );
	zTransform.SetTranslation3D( Vec3( 0.0f, -0.8f, 0.2f ) );

	DebugAddWorldText( xForwardText, xTransform, textHeight, alignment, duration,   Rgba8::RED,   Rgba8::RED, DebugRenderMode::ALWAYS );		// X - Forward
	DebugAddWorldText(	  yLeftText, yTransform, textHeight, alignment, duration, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::ALWAYS );		// Y - Left
	DebugAddWorldText(		zUpText, zTransform, textHeight, alignment, duration,  Rgba8::BLUE,  Rgba8::BLUE, DebugRenderMode::ALWAYS );		// Z - Up
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Render_DRS_WorldBasis()
{
	// DebugRenderSystem World Basis
	float radius	= 0.1f;
	float duration = -1.0f;
	DebugAddWorldArrow( Vec3::ZERO, Vec3( 1.0f, 0.0f, 0.0f ), radius, duration,   Rgba8::RED, Rgba8::RED   );		// iBasisArrow
	DebugAddWorldArrow( Vec3::ZERO, Vec3( 0.0f, 1.0f, 0.0f ), radius, duration, Rgba8::GREEN, Rgba8::GREEN );		// jBasisArrow
	DebugAddWorldArrow( Vec3::ZERO, Vec3( 0.0f, 0.0f, 1.0f ), radius, duration,  Rgba8::BLUE, Rgba8::BLUE  );		// kBasisArrow
}

//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Setting perspective view
	m_player->m_worldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 100.0f );
	m_player->m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f ), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) );

//	m_worldCamera.SetPerspectiveView( 2.0f, 60.0f, 0.1f, 100.0f );
//	m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f), Vec3( -1.0f, 0.0f, 0.0f), Vec3( 0.0f, 1.0f, 0.0f) );

	//----------------------------------------------------------------------------------------------------------------------
	// Setting ortho views
//	  m_worldCamera.SetOrthoView( Vec2(-1.0f, -1.0f), Vec2(1.0f, 1.0f) );
	m_attractCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2(1600.0f, 800.0f) );
	m_screenCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y) );
}

//----------------------------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const
{
	// Draw everything in screen space
	g_theRenderer->BeginCamera(m_attractCamera);

	g_theRenderer->SetModelConstants();

//	//----------------------------------------------------------------------------------------------------------------------
//	// Drawing triangle in attractMode
//	Vertex_PCU tempAttractModeVerts[NUM_SINGLE_TRI_VERTS];
//	for (int i = 0; i < NUM_SINGLE_TRI_VERTS; i++)
//	{
//		tempAttractModeVerts[i] = m_localVerts[i];
//	}
//
//	TransformVertexArrayXY3D(NUM_SINGLE_TRI_VERTS, tempAttractModeVerts, 80.0f, 0.0f, Vec2(ATTRACT_MODE_CENTER_X, ATTRACT_MODE_CENTER_Y));
//	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
//	g_theRenderer->BindTexture( nullptr );
//	g_theRenderer->SetModelConstants();
//	g_theRenderer->DrawVertexArray(NUM_SINGLE_TRI_VERTS, tempAttractModeVerts);

	//----------------------------------------------------------------------------------------------------------------------
	// Drawing "First Triangle"
	// #Question // How are we setting the worldPos of the triangle?
	Vertex_PCU vertices[] =
	{
		Vertex_PCU( Vec3( 1200.0f, 200.0f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
		Vertex_PCU( Vec3(  800.0f, 600.0f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
		Vertex_PCU( Vec3(  400.0f, 200.0f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
	};

	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( 3, vertices );

	// End attractMode Camera
	g_theRenderer->EndCamera(m_attractCamera);
} 

//----------------------------------------------------------------------------------------------------------------------
void Game::InitializeAttractModeVerts()
{
	m_localVerts[0].m_position = Vec3( 0.0f,  2.0f, 0.0f);		// Triangle A, position of vert A
	m_localVerts[1].m_position = Vec3( 4.0f, -3.0f, 0.0f);		// Triangle A, position of vert B
	m_localVerts[2].m_position = Vec3(-4.0f, -3.0f, 0.0f);		// Triangle A, position of vert C

	m_localVerts[0].m_color = Rgba8::WHITE;						// Triangle A, color of vert A
	m_localVerts[1].m_color = Rgba8::WHITE;						// Triangle A, color of vert B
	m_localVerts[2].m_color = Rgba8::WHITE;						// Triangle A, color of vert C
}

//----------------------------------------------------------------------------------------------------------------------
void Game::LoadModel()
{
	if ( g_theInput->WasKeyJustPressed( 'L' ) )
	{
		g_theInput->SetCursorMode( false, false );
		g_theInput->BeginFrame();
		std::string filePathToLoad = g_theWindow->GetXMLFileName( "Data" );
		g_theModel->ParseXmlData( "Data/Models/" + filePathToLoad );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::Render_DRS_UI_Text() const
{
	// Initialize and set UI variables
	float cellHeight			= 2.0f;
	float duration				= 0.0f;
	Vec2 playerPos				= Vec2( 0.0f, 100.0f );
	Vec2 playerYPR				= Vec2( 0.0f,  97.0f );
	Vec2 playerAlignment		= Vec2( 0.0f, 1.0f );
	Vec2 playerYPRAlignment		= Vec2( 0.0f, 1.0f );
	Vec2 timePosition			= Vec2( SCREEN_SIZE_X, ( SCREEN_SIZE_Y ) );
	Vec2 timeAlignment			= Vec2( 1.0f, 1.0f );
	float fps					= 1.0f / m_clock.GetDeltaSeconds();
	float scale					= m_clock.GetTimeScale();
	std::string playerPosText	= Stringf( "Player position:   %0.2f, %0.2f, %0.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z );
	std::string playerYPRText	= Stringf( "Player YPR:        %0.2f, %0.2f, %0.2f", m_player->m_orientationDegrees.m_yawDegrees, m_player->m_orientationDegrees.m_pitchDegrees, m_player->m_orientationDegrees.m_rollDegrees );
	std::string timeText		= Stringf( "Time: %0.2f. FPS: %0.2f, Scale %0.2f.", m_clock.GetTotalSeconds(), fps, scale );
	// Light info
	std::string sunYPR				= Stringf( "Sun YPR:   ( %0.2f, %0.2f, %0.2f )", m_arrowYPR.m_yawDegrees, m_arrowYPR.m_pitchDegrees, m_arrowYPR.m_rollDegrees );
	Vec3 sunDirVec						= m_arrowYPR.GetForwardDir_XFwd_YLeft_ZUp();
	std::string sunDir				= Stringf( "Sun Dir: ( %0.2f, %0.2f, %0.2f )", sunDirVec.x, sunDirVec.y, sunDirVec.z );
	std::string sunIntensity		= Stringf( "Sun Intensity:   %0.2f", m_sunIntensity );
	std::string ambientIntensity 	= Stringf( "Ambient Intensity:    %0.2f", m_ambientIntensity  );
	std::string normals				= Stringf( "Normals: Texture" );
	if ( !m_isNormalsTexture )
	{
		normals = Stringf( "Normals: Vertex" );
	}
	std::string specular			= Stringf( "Specular: Texture" );
	if ( !m_isSpecTexture )
	{
		specular = Stringf( "Specular: Constant" );
	}
	std::string specularIntensity   = Stringf( "Specular Intensity:    %0.2f", m_specularIntensity );
	std::string specularPower		= Stringf( "Specular Power:   %0.2f", m_specularPower );


	// Render DRS UI text
	DebugAddScreenText( playerPosText, playerPos, cellHeight, playerAlignment, duration );
	DebugAddScreenText( playerYPRText, playerYPR, cellHeight, playerAlignment, duration );
	DebugAddScreenText( timeText, timePosition, cellHeight, timeAlignment, duration );
	// Light
	DebugAddScreenText( sunYPR				, Vec2( timePosition.x, timePosition.y - ( cellHeight * 1.0f ) ), cellHeight, timeAlignment, duration );
	DebugAddScreenText( sunDir				, Vec2( timePosition.x, timePosition.y - ( cellHeight * 2.0f ) ), cellHeight, timeAlignment, duration );
	DebugAddScreenText( sunIntensity		, Vec2( timePosition.x, timePosition.y - ( cellHeight * 3.0f ) ), cellHeight, timeAlignment, duration );
	DebugAddScreenText( ambientIntensity 	, Vec2( timePosition.x, timePosition.y - ( cellHeight * 4.0f ) ), cellHeight, timeAlignment, duration );
	DebugAddScreenText( normals				, Vec2( timePosition.x, timePosition.y - ( cellHeight * 5.0f ) ), cellHeight, timeAlignment, duration );
	DebugAddScreenText( specular			, Vec2( timePosition.x, timePosition.y - ( cellHeight * 6.0f ) ), cellHeight, timeAlignment, duration );
	DebugAddScreenText( specularIntensity   , Vec2( timePosition.x, timePosition.y - ( cellHeight * 7.0f ) ), cellHeight, timeAlignment, duration );
	DebugAddScreenText( specularPower		, Vec2( timePosition.x, timePosition.y - ( cellHeight * 8.0f ) ), cellHeight, timeAlignment, duration );
}
