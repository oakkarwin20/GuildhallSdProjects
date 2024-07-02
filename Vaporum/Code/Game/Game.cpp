#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"
#include "Game/Model.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/UnitDefinition.hpp"
#include "Game/Map.hpp"

#include "Engine/Networking/NetSystem.hpp"
#include "Engine/UI/Button.hpp"
#include "Engine/Core/DevConsole.hpp"
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


//----------------------------------------------------------------------------------------------------------------------
Model* g_theModel	= nullptr;
Game*  g_theGame	= nullptr;


//----------------------------------------------------------------------------------------------------------------------
Game::Game()
{
	g_theGame	= this;
	g_theModel	= new Model();


	// Initializing textures;
	m_logoTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Logo.png" );
	m_mapTexture  = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Images/MoonSurface" );

	g_theEventSystem->SubscribeToEvent(		   		     "LoadModel", Command_LoadModel					 );
	g_theEventSystem->SubscribeToEvent(		   		       "LoadMap", Command_LoadMap					 );
	g_theEventSystem->SubscribeToEvent(		   	    "SelectLastUnit", Command_SelectLastUnit			 );
	g_theEventSystem->SubscribeToEvent(		       "SelectFirstUnit", Command_SelectFirstUnit			 );
	g_theEventSystem->SubscribeToEvent(		    "SelectPreviousUnit", Command_SelectPreviousUnit		 );
	g_theEventSystem->SubscribeToEvent(			    "SelectNextUnit", Command_SelectNextUnit			 );
	g_theEventSystem->SubscribeToEvent(		     "MoveToHoveredTile", Command_MoveToHoveredTile			 );
	g_theEventSystem->SubscribeToEvent(		      "MoveSelectedUnit", Command_MoveSelectedUnit			 );
	g_theEventSystem->SubscribeToEvent(		     "SelectHoveredUnit", Command_SelectHoveredUnit			 );
	g_theEventSystem->SubscribeToEvent(			     "RecvCursorPos", Command_RecvCursorPos				 );
	g_theEventSystem->SubscribeToEvent(				    "RecvHexPos", Command_RecvHexPos				 );
	g_theEventSystem->SubscribeToEvent(	"MoveSelectedUnitToPrevTile", Command_MoveSelectedUnitToPrevTile );
	g_theEventSystem->SubscribeToEvent(					 "TryAttack", Command_TryAttack					 );
	g_theEventSystem->SubscribeToEvent(			  "TrySelectNewUnit", Command_TrySelectNewUnit			 );
	g_theEventSystem->SubscribeToEvent(				   "ConfirmMove", Command_ConfirmMove				 );
	g_theEventSystem->SubscribeToEvent(			   "EscButton_Logic", Command_EscButton_Logic			 );
	g_theEventSystem->SubscribeToEvent(				 "ConfirmAttack", Command_ConfirmAttack				 );
	g_theEventSystem->SubscribeToEvent(				 "EndTurn_Logic", Command_EndTurn_Logic				 );
	g_theEventSystem->SubscribeToEvent(		  "ConfirmEndTurn_Logic", Command_ConfirmEndTurn_Logic		 );
	g_theEventSystem->SubscribeToEvent(			  "LocalPlayerReady", Command_LocalPlayerReady			 );
	g_theEventSystem->SubscribeToEvent(				   "Popup_Logic", Command_Popup_Logic				 );
	g_theEventSystem->SubscribeToEvent(			  "GenerateDistPath", Command_GenerateDistPath			 );
	g_theEventSystem->SubscribeToEvent(					 "MatchDraw", Command_MatchDraw					 );
	g_theEventSystem->SubscribeToEvent(	 "ResetRemoteGameOverStates", Command_ResetRemoteGameOverStates	 );


	//----------------------------------------------------------------------------------------------------------------------
	// Initialize definitions parsed from XML data
	//----------------------------------------------------------------------------------------------------------------------
	TileDefinition::InitializeTileDef( "Data/Definitions/TileDefinitions.xml" );
	 MapDefinition::InitializeMapDef(  "Data/Definitions/MapDefinitions.xml"  );
	UnitDefinition::InitializeUnitDef( "Data/Definitions/UnitDefinitions.xml" );

	m_clientDimensions = Vec2( g_theWindow->GetClientDimensions() );
}


//----------------------------------------------------------------------------------------------------------------------
Game::~Game()
{
	delete g_theModel;
	g_theModel = nullptr;

	UnitDefinition::DeleteDefinitions();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::StartUp()
{
	if ( m_currentGameState == GameState::ATTRACT )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Setting ortho views
		//----------------------------------------------------------------------------------------------------------------------
		m_attractCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( m_clientDimensions.x, m_clientDimensions.y ) );
	}
	else if ( m_currentGameState == GameState::PLAYING )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Initialize Player and Camera
		//----------------------------------------------------------------------------------------------------------------------
		InitializePlayers();
		InitializeCameras();

		//----------------------------------------------------------------------------------------------------------------------
		// Load model based on gameConfig
		//----------------------------------------------------------------------------------------------------------------------
		std::string modelToLoad = g_gameConfigBlackboard.GetValue( "modelToLoad", std::string( "Invalid Model" ) );
		g_theModel->ParseXmlData( "Data/Models/" + modelToLoad );

		//----------------------------------------------------------------------------------------------------------------------
		// Create new map
		//----------------------------------------------------------------------------------------------------------------------
		MapDefinition const* currentMapDef = MapDefinition::GetMapDefByName( g_theApp->m_defaultMap );
		m_currentMap = new Map( currentMapDef, this );

		//----------------------------------------------------------------------------------------------------------------------
		// Check both players are ready to start the game IF connected via network
		//----------------------------------------------------------------------------------------------------------------------
		if ( g_theNetSystem->IsNetworking() )
		{
			InitializeLocalPlayerReady();
			// Tell the remotePlayer THIS local player is ready
			g_theNetSystem->SendMessage( "LocalPlayerReady" );
		}

		InitializeButtons();
	}
	else if ( m_currentGameState == GameState::LOBBY )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Create Buttons
		//----------------------------------------------------------------------------------------------------------------------
		m_button_localGame	= new Button( AABB2( 0.0f, 0.0f, m_clientDimensions.x * 0.3f, m_clientDimensions.y * 0.05f ), "New Game" );
		m_button_quit		= new Button( AABB2( 0.0f, 0.0f, m_clientDimensions.x * 0.3f, m_clientDimensions.y * 0.05f ),	  "Quit" );
		m_button_localGame->m_bounds.SetBottomLeft( Vec2( m_clientDimensions.x * 0.24f, m_clientDimensions.y * 0.5f ) );
			 m_button_quit->m_bounds.SetBottomLeft( Vec2( m_clientDimensions.x * 0.24f, m_clientDimensions.y * 0.4f ) );
		m_button_localGame->SetIsHighlighted( true );
		m_lobbyButtonList.emplace_back( m_button_localGame	);
		m_lobbyButtonList.emplace_back( m_button_quit		);
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	delete m_currentMap;
	m_currentMap = nullptr;

	for ( int i = 0; i < m_entityList.size(); i++ )
	{
		delete m_entityList[i];
		m_entityList[i] = nullptr;
	}
	m_entityList.clear();

//	delete m_enemyPlayer;
	m_enemyPlayer = nullptr;
//	delete m_currentPlayer;
	m_currentPlayer = nullptr;
//	delete m_player1;
	m_player1 = nullptr;
//	delete m_player2;
	m_player2 = nullptr;

	// Delete definitions here
//	UnitDefinition::DeleteDefinitions();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::Update()
{
	float deltaSeconds = m_clock.GetDeltaSeconds();
	
	UpdateCursorPos();
	UpdateHoveredAndPrevHex();

	//----------------------------------------------------------------------------------------------------------------------
	// Check if currentGameState needs to be changed
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_requestedGameState != m_currentGameState )
	{
		m_previousGameState = m_currentGameState;
		ExitState( m_currentGameState );
		m_currentGameState = m_requestedGameState;
		EnterState( m_currentGameState );
		// Handle paused state specific logic
		if ( m_previousGameState != GameState::PAUSED )
		{
			StartUp();
		}
		else if ( m_previousGameState == GameState::PAUSED )
		{
			// Going from paused to lobby
			if ( m_currentGameState == GameState::LOBBY )
			{
				m_currentSelectedUnit = nullptr;
				m_targetedEnemyUnit   = nullptr;
				m_button_localGame->SetIsHighlighted( true );
				m_button_quit->SetIsHighlighted();
				m_enemyPlayer		  = nullptr;
				m_isGameOver		  = false;
			}
			// Going from paused to playing
			if ( m_currentGameState == GameState::PLAYING )
			{
				m_button_resume->SetIsHighlighted( true );
				m_button_mainMenu->SetIsHighlighted();
				m_clock.Unpause();
			}
		}
	}

	if ( m_currentGameState == GameState::ATTRACT )
	{
		// Take input from attractModeInput()
		UpdateGameStatesInput();
		UpdateInputUI();
	}
	else if ( m_currentGameState == GameState::PLAYING )
	{
		LoadModel();
		UpdatePauseQuitAndSlowMo();
		UpdateEntities( deltaSeconds );
		UpdateGameStatesInput();
		m_currentPlayer->ChangePlayerStates();
		m_currentPlayer->ChangePlayerTurnStates();
		m_currentMap->Update( deltaSeconds );
		UpdateHoveredUnitStats();
		UpdatAllUnitsDeath();
		SetEnemiesUnitsInAttackRange_WithinAttackRangeUnitState();
		CheckGameOver();
		if ( !IsMyTurn() )
		{
			return;
		}
		UpdatePlayerStatesInput();
	}
	else if ( m_currentGameState == GameState::PAUSED )
	{
		UpdateGameStatesInput();
		UpdateInputUI();
	}
	else if ( m_currentGameState == GameState::LOBBY )
	{
		UpdateGameStatesInput();
		UpdateInputUI();
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Draw attract mode
	if ( m_currentGameState == GameState::ATTRACT )
	{
		g_theRenderer->ClearScreen( Rgba8::DARK_RED );			// Clear AttractMode screen
		RenderAttractScreen();									// Render AttractMode 
	}
	else if ( m_currentGameState == GameState::PLAYING )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Render World
		//----------------------------------------------------------------------------------------------------------------------
		g_theRenderer->ClearScreen( Rgba8::LIGHTBLACK );
		g_theRenderer->BeginCamera( m_currentPlayer->m_worldCamera );
		if ( ArePlayersReady() )
		{
			DebugRenderWorld( m_currentPlayer->m_worldCamera );
	//		RenderEntities();
			m_currentMap->Render();
			RenderEnemiesInAttackRange();
		}
		RenderUI();
		g_theRenderer->EndCamera( m_currentPlayer->m_worldCamera );
	}
	else if ( m_currentGameState == GameState::PAUSED )
	{
		g_theRenderer->ClearScreen( Rgba8::DARKER_GREEN );	
		RenderAttractScreen();
	}
	else if ( m_currentGameState == GameState::LOBBY )
	{
		g_theRenderer->ClearScreen( Rgba8::CYAN );			
		RenderAttractScreen();				
	}
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
bool Game::Command_LoadMap( EventArgs& args )
{
 	std::string mapName = args.GetValue( "Name", "INVALID NAME" );
	MapDefinition const* currentMapDef = MapDefinition::GetMapDefByName( mapName );
	if ( currentMapDef == nullptr || mapName == "INVALID NAME" )
	{
		g_theDevConsole->AddLine( Rgba8::RED, "INVALID map name!" );	
		return false;
	}
	delete g_theGame->m_currentMap;
	g_theGame->m_currentMap = new Map( currentMapDef, g_theGame );
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_SelectLastUnit( EventArgs& args )
{
	UNUSED( args );
	g_theGame->SelectLastUnit();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_SelectFirstUnit( EventArgs& args )
{
	UNUSED( args );
	g_theGame->SelectFirstUnit();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_SelectPreviousUnit( EventArgs& args )
{
	UNUSED( args );
	g_theGame->SelectPreviousUnit();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_SelectNextUnit( EventArgs& args )
{
	UNUSED( args );
	g_theGame->SelectNextUnit();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_MoveToHoveredTile( EventArgs& args )
{
	UNUSED( args );
	g_theGame->MoveToHoveredTile();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_MoveSelectedUnit( EventArgs& args )
{
	UNUSED( args );
	g_theGame->MoveSelectedUnit();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_SelectHoveredUnit( EventArgs& args )
{
	UNUSED( args );
	g_theGame->SelectHoveredUnit();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_RecvCursorPos( EventArgs& args )
{
	Vec2 newCursorPos = args.GetValue( "CursorPos", Vec2( -1.0f, -1.0f ) );			// "RecvCursorPos=X,Y"
	// Update the local cursorPos with newCursorPos recv from net
	g_theGame->RecvUpdatedCursorPos( newCursorPos );
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_RecvHexPos( EventArgs& args )
{
	IntVec2 newHexPos = args.GetValue( "HexPos", IntVec2( -1, -1 ) );			// "RecvHexPos=X,Y"
	// Update the local cursorPos with newCursorPos recv from net
	g_theGame->RecvUpdatedHexPos( newHexPos );
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_MoveSelectedUnitToPrevTile( EventArgs& args )
{
	UNUSED( args );
	g_theGame->MoveSelectedUnitToPrevTile();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_TryAttack( EventArgs& args )
{
	UNUSED( args );
	g_theGame->TryAttack();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_TrySelectNewUnit( EventArgs& args )
{
	UNUSED( args );
	g_theGame->TrySelectNewUnit();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_ConfirmMove( EventArgs& args )
{
	UNUSED( args );
	g_theGame->ConfirmMove();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_EscButton_Logic( EventArgs& args )
{
	UNUSED( args );
	g_theGame->EscButton_Logic();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_ConfirmAttack( EventArgs& args )
{
	UNUSED( args );
	g_theGame->ConfirmAttack();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_EndTurn_Logic( EventArgs& args )
{
	UNUSED( args );
	g_theGame->EndTurn_Logic();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_ConfirmEndTurn_Logic( EventArgs& args )
{
	UNUSED( args );
	g_theGame->ConfirmEndTurn_Logic();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_LocalPlayerReady( EventArgs& args )
{
	UNUSED( args );
//	g_theGame->m_remotePlayer->m_isPlayerReady = true;
	g_theGame->m_remotePlayerIsReady = true;
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_Popup_Logic( EventArgs& args )
{
	UNUSED( args );
	g_theGame->Popup_Logic();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_GenerateDistPath( EventArgs& args )
{
	UNUSED( args );
	g_theGame->GenerateDistancePath();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_MatchDraw( EventArgs& args )
{
	UNUSED( args );
	g_theGame->MatchDraw();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::Command_ResetRemoteGameOverStates( EventArgs& args )
{
	UNUSED( args );
	g_theGame->ResetRemoteGameOverStates();
	return true;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::UpdatePauseQuitAndSlowMo()
{
	// Pause functionality
	if (g_theInput->WasKeyJustPressed('P') || g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::BUTTON_START))
	{
		m_clock.TogglePause();
		m_requestedGameState = GameState::PAUSED;
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
void Game::UpdateGameStatesInput()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Handle logic when 'ESC' is pressed
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->WasKeyJustPressed( KEYCODE_ESC ) )
	{
		if ( m_currentGameState == GameState::ATTRACT )
		{
			g_theApp->HandleQuitRequested();
		}
		else if ( m_currentGameState == GameState::PLAYING )
		{
/*
			// Handle esc pressed when confirmed end turn
			// move selected unit back to prev coord
			// tell net to do the same
			if ( m_currentPlayer->m_currentPlayerTurnState == PlayerTurnState::CONFIRM_END_TURN )
			{
				m_currentPlayer->m_requestedPlayerTurnState = PlayerTurnState::END_TURN;
				MoveSelectedUnitToPrevTile();
				g_theNetSystem->SendMessage( "MoveSelectedUnitToPrevTile" ); 
				return;
			}
*/

			if ( ( m_currentSelectedUnit == nullptr ) && ( m_currentPlayer->m_requestedPlayerTurnState == PlayerTurnState::END_TURN ) )
			{
				// If no unit is currently selected, pressing esc should take us to the pause menu
				m_requestedGameState = GameState::PAUSED;
			}
			else 
			{
				EscButton_Logic();
				g_theNetSystem->SendMessage( "EscButton_Logic" );
			}
		}
		else if ( m_currentGameState == GameState::PAUSED )
		{
			m_requestedGameState = GameState::LOBBY;
		}
		else if ( m_currentGameState == GameState::LOBBY )
		{
			g_theApp->HandleQuitRequested();
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Handle logic when 'Mouse Left Click' is pressed
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFT_MOUSE ) )
	{
		if ( m_currentGameState == GameState::ATTRACT )
		{
			Attract_Logic();
		}
		else if ( m_currentGameState == GameState::PLAYING )
		{
//			if ( m_localPlayerIsReady && m_remotePlayerIsReady )
			if ( IsMyTurn() )
			{
//				// Check if my turn, if currentPlayer netState == g_net.mode
//				std::string currentPlayerNetState	= m_currentPlayer->GetNetStateEnumAsString();
//				std::string localNetState			=  g_theNetSystem->GetModeEnumAsString();
//				if ( currentPlayerNetState != localNetState )
//				{
//					return;
//				}

				if ( m_currentPlayer->m_currentPlayerTurnState == PlayerTurnState::POP_UP )
				{
					Popup_Logic();
					g_theNetSystem->SendMessage( "Popup_Logic" );
				}

				//----------------------------------------------------------------------------------------------------------------------
				// Handle logic for clicking on uiButton_enter
				//----------------------------------------------------------------------------------------------------------------------
				// Check if button was hovered and clicked
				if ( m_button_enter->IsMouseHovered( m_currentCursorPos ) )
				{
					if ( m_currentPlayer->m_currentPlayerTurnState == PlayerTurnState::END_TURN )
					{
						EndTurn_Logic();
						g_theNetSystem->SendMessage( "EndTurn_Logic" );
					}
					else if ( m_currentPlayer->m_currentPlayerTurnState == PlayerTurnState::CONFIRM_END_TURN )
					{
						ConfirmEndTurn_Logic();
						g_theNetSystem->SendMessage( "ConfirmEndTurn_Logic" );
					}
				}
				//----------------------------------------------------------------------------------------------------------------------
				// Handle logic for clicking on uiButton_esc
				//----------------------------------------------------------------------------------------------------------------------
				if ( m_button_escape->IsMouseHovered( m_currentCursorPos ) )
				{
					if ( ( m_currentSelectedUnit == nullptr ) && ( m_currentPlayer->m_requestedPlayerTurnState == PlayerTurnState::END_TURN ) )
					{
						// If no unit is currently selected, pressing esc should take us to the pause menu
						m_requestedGameState = GameState::PAUSED;
					}
					else
					{
						EscButton_Logic();
						g_theNetSystem->SendMessage( "EscButton_Logic" );
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Handle logic when 'Enter' is pressed
	//----------------------------------------------------------------------------------------------------------------------
	if ( g_theInput->WasKeyJustPressed( KEYCODE_ENTER ) )
	{
		if ( m_currentGameState == GameState::ATTRACT )
		{
			Attract_Logic();  
		}
		else if ( m_currentGameState == GameState::PLAYING )
		{
			if ( m_localPlayerIsReady && m_remotePlayerIsReady )
			{
				// Check if my turn, if currentPlayer netState == g_net.mode
				std::string currentPlayerNetState	= m_currentPlayer->GetNetStateEnumAsString();
				std::string localNetState			=  g_theNetSystem->GetModeEnumAsString();
				if ( currentPlayerNetState != localNetState )
				{
					return;
				}

				//----------------------------------------------------------------------------------------------------------------------
				// Game state input logic
				//----------------------------------------------------------------------------------------------------------------------
				if ( m_currentPlayer->m_currentPlayerTurnState == PlayerTurnState::POP_UP )
				{
					Popup_Logic();
					g_theNetSystem->SendMessage( "Popup_Logic" );
				}
				else if ( m_currentPlayer->m_currentPlayerTurnState == PlayerTurnState::END_TURN )
				{
					EndTurn_Logic();
					g_theNetSystem->SendMessage( "EndTurn_Logic" );
				}
				else if ( m_currentPlayer->m_currentPlayerTurnState == PlayerTurnState::CONFIRM_END_TURN )
				{
					ConfirmEndTurn_Logic();
					g_theNetSystem->SendMessage( "ConfirmEndTurn_Logic" );
				}
				else if ( m_currentPlayer->m_currentPlayerTurnState == PlayerTurnState::WAITING_FOR_TURN )
				{
				}
			}
		}
		else if ( m_currentGameState == GameState::PAUSED )
		{
		}
		else if ( m_currentGameState == GameState::LOBBY )
		{
		}
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
	RenderPopUpUI();
	// End UI Camera
	g_theRenderer->EndCamera( m_screenCamera );
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
void Game::InitializeCameras()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Setting perspective view
	//----------------------------------------------------------------------------------------------------------------------
	m_player1->m_worldCamera.SetPerspectiveView( g_theApp->m_windowAspect, g_theApp->m_cameraFOVDegrees, g_theApp->m_cameraNearClip, g_theApp->m_cameraFarClip );
	m_player1->m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f ), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) );
	m_player2->m_worldCamera.SetPerspectiveView( g_theApp->m_windowAspect, g_theApp->m_cameraFOVDegrees, g_theApp->m_cameraNearClip, g_theApp->m_cameraFarClip );
	m_player2->m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f ), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) );

/*
	m_player1->m_worldCamera.SetPerspectiveView( g_theApp->m_windowAspect, g_theApp->m_cameraFOVDegrees, g_theApp->m_cameraNearClip, g_theApp->m_cameraFarClip );
	m_player1->m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f ), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) );
	m_player2->m_worldCamera.SetPerspectiveView( g_theApp->m_windowAspect, g_theApp->m_cameraFOVDegrees, g_theApp->m_cameraNearClip, g_theApp->m_cameraFarClip );
	m_player2->m_worldCamera.SetRenderBasis( Vec3( 0.0f, 0.0f, 1.0f ), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, 1.0f, 0.0f ) );
*/


	//----------------------------------------------------------------------------------------------------------------------
	// Setting ortho views
	//----------------------------------------------------------------------------------------------------------------------
	m_attractCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( m_clientDimensions.x, m_clientDimensions.y ) );
	 m_screenCamera.SetOrthoView( Vec2( 0.0f, 0.0f ), Vec2( m_clientDimensions.x, m_clientDimensions.y ) );
}


//----------------------------------------------------------------------------------------------------------------------
void Game::RenderAttractScreen() const
{
	// Draw everything in screen space
	g_theRenderer->BeginCamera( m_attractCamera );

	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> logoVerts;
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> mainMenuTextVerts;

	// Render Logo
	float smallerClientDim = m_clientDimensions.x < m_clientDimensions.y ? m_clientDimensions.x : m_clientDimensions.y;
	Vec2  mins		 = Vec2( 0.0f, 0.0f );
	Vec2  maxs		 = Vec2( smallerClientDim * 0.5f, smallerClientDim * 0.5f );
	AABB2 logoBounds = AABB2( mins, maxs );
	logoBounds.SetCenter( Vec2( m_clientDimensions.x * 0.5f, m_clientDimensions.y * 0.5f ) );
	AddVertsForAABB2D( logoVerts, logoBounds );

	//----------------------------------------------------------------------------------------------------------------------
	// Filter render based on game states
	//----------------------------------------------------------------------------------------------------------------------
	float titleCellHeight				= m_clientDimensions.y * 0.1f;
	float buttonCellHeight				= m_clientDimensions.y * 0.02f;
	float borderThickness				= m_clientDimensions.y * 0.01f;
	float cellAspect = 0.7f;
	if ( m_currentGameState == GameState::ATTRACT )
	{
		// Vaporum Title
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, AABB2( 0.0f, 0.0f, m_clientDimensions.x, m_clientDimensions.y ), 
														titleCellHeight, "VAPORUM", Rgba8::WHITE, cellAspect, Vec2( 0.5f, 0.95f ) );

		// Instructions to start game 
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, AABB2( 0.0f, 0.0f, m_clientDimensions.x, m_clientDimensions.y ), 
														buttonCellHeight, "Press Enter or Click to Start", Rgba8::WHITE, cellAspect, Vec2( 0.5f, 0.15f ) );
	}
	if ( m_currentGameState == GameState::LOBBY )
	{
		Vec2  buttonDimensions_localGame	= m_button_localGame->m_bounds.GetDimensions();
		float buttonCellHeight_localGame	= buttonDimensions_localGame.y * 0.7f;
		Vec2  buttonDimensions_quit			= m_button_quit->m_bounds.GetDimensions();
		float buttonCellHeight_quit			= buttonDimensions_quit.y * 0.7f;

		// Vertical Bar
		AddVertsForAABB2D( verts, AABB2( m_clientDimensions.x * 0.23f, m_clientDimensions.y * 0.0f, m_clientDimensions.x * 0.24f, m_clientDimensions.y * 1.0f ) );
		// Main Menu text
		g_theApp->m_textFont->AddVertsForTextInBox2D( mainMenuTextVerts, AABB2( 0.0f, 0.0f, m_clientDimensions.x, m_clientDimensions.y ),
													titleCellHeight * 0.5f, "Main Menu", Rgba8::WHITE, cellAspect, Vec2( 0.0f, 0.0f ) );

		//----------------------------------------------------------------------------------------------------------------------
		// 'Local Game' Button
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_button_localGame->GetIsHighlighted() )
		{
			AddVertsForAABB2D( verts, m_button_localGame->m_bounds );
			AddVertsForBordersAABB2D( verts, m_button_localGame->m_bounds, borderThickness, Rgba8::BLACK );
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_localGame->m_bounds, buttonCellHeight_localGame, m_button_localGame->m_text1, Rgba8::BLACK, cellAspect, Vec2( 0.1f, 0.5f ) );
		}
		else
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_localGame->m_bounds, buttonCellHeight_localGame, m_button_localGame->m_text1, Rgba8::WHITE, cellAspect, Vec2( 0.1f, 0.5f ) );
		}
		//----------------------------------------------------------------------------------------------------------------------
		// 'Quit' Button
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_button_quit->GetIsHighlighted() )
		{
			AddVertsForAABB2D( verts, m_button_quit->m_bounds );
			AddVertsForBordersAABB2D( verts, m_button_quit->m_bounds, borderThickness, Rgba8::BLACK );
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_quit->m_bounds, buttonCellHeight_quit, m_button_quit->m_text1, Rgba8::BLACK, cellAspect, Vec2( 0.05f, 0.5f ) );
		}
		else
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_quit->m_bounds, buttonCellHeight_quit, m_button_quit->m_text1, Rgba8::WHITE, cellAspect, Vec2( 0.05f, 0.5f ) );
		}
	}
	if ( m_currentGameState == GameState::PAUSED )
	{
		Vec2  buttonDimensions_localGame	= m_button_localGame->m_bounds.GetDimensions();
		float buttonCellHeight_localGame	= buttonDimensions_localGame.y * 0.7f;
		Vec2  buttonDimensions_quit			= m_button_quit->m_bounds.GetDimensions();
		float buttonCellHeight_quit			= buttonDimensions_quit.y * 0.7f;


		// Vertical Bar
		AddVertsForAABB2D( verts, AABB2( m_clientDimensions.x * 0.23f, m_clientDimensions.y * 0.0f, m_clientDimensions.x * 0.24f, m_clientDimensions.y * 1.0f ) );

		// Paused Text
		g_theApp->m_textFont->AddVertsForTextInBox2D( mainMenuTextVerts, AABB2( 0.0f, 0.0f, m_clientDimensions.x, m_clientDimensions.y ),
			titleCellHeight * 0.5f, "Main Menu", Rgba8::WHITE, cellAspect, Vec2( 0.0f, 0.0f ) );

		//----------------------------------------------------------------------------------------------------------------------
		// 'Resume' Button
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_button_resume->GetIsHighlighted() )
		{
			AddVertsForAABB2D( verts, m_button_resume->m_bounds );
			AddVertsForBordersAABB2D( verts, m_button_resume->m_bounds, borderThickness, Rgba8::BLACK );
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_resume->m_bounds, buttonCellHeight_localGame, m_button_resume->m_text1, Rgba8::BLACK, cellAspect, Vec2( 0.1f, 0.5f ) );
		}
		else
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_resume->m_bounds, buttonCellHeight_localGame, m_button_resume->m_text1, Rgba8::WHITE, cellAspect, Vec2( 0.1f, 0.5f ) );
		}
		
		//----------------------------------------------------------------------------------------------------------------------
		// 'Main Menu' Button 
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_button_mainMenu->GetIsHighlighted() )
		{
			AddVertsForAABB2D( verts, m_button_mainMenu->m_bounds );
			AddVertsForBordersAABB2D( verts, m_button_mainMenu->m_bounds, borderThickness, Rgba8::BLACK );
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_mainMenu->m_bounds, buttonCellHeight_quit, m_button_mainMenu->m_text1, Rgba8::BLACK, cellAspect, Vec2( 0.05f, 0.5f ) );
		}
		else
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_mainMenu->m_bounds, buttonCellHeight_quit, m_button_mainMenu->m_text1, Rgba8::WHITE, cellAspect, Vec2( 0.05f, 0.5f ) );
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Debug code for "isPointInside" button
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_button_localGame != nullptr )
	{
		Vec2 nearestPoint_localGame = m_button_localGame->m_bounds.GetNearestPoint( m_currentCursorPos );
		Vec2 nearestPoint_quit		= m_button_quit->m_bounds.GetNearestPoint( m_currentCursorPos );
		AddVertsForDisc2D( verts, nearestPoint_localGame, 10.0f, Rgba8::YELLOW	);
		AddVertsForDisc2D( verts,	   nearestPoint_quit, 10.0f, Rgba8::MAGENTA	);
	}

	// logoVerts Draw Call
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( m_logoTexture );
	g_theRenderer->DrawVertexArray( int( logoVerts.size() ), logoVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	// verts Draw Call
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( int( verts.size() ), verts.data() );
	g_theRenderer->BindTexture( nullptr );

	// mainMenuTextVerts Draw Call
	Mat44 mat = Mat44();
	mat.AppendZRotation( 90.0f );
	mat.SetTranslation2D( Vec2( m_clientDimensions.x * 0.23f, m_clientDimensions.y * 0.25f ) );
	g_theRenderer->SetModelConstants( mat );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( int( mainMenuTextVerts.size() ), mainMenuTextVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	// generalTextVerts Draw Call
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->DrawVertexArray( int( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );

	// End attractMode Camera
	g_theRenderer->EndCamera( m_attractCamera );
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
void Game::InitializePlayers()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Adding player1 to scene
	//----------------------------------------------------------------------------------------------------------------------
	m_player1													= new Player(this);
	m_player1->m_worldCamera.m_position							= g_theApp->m_cameraStartPosition;
	m_player1->m_worldCamera.m_orientation.m_yawDegrees			= g_theApp->m_cameraFixedAngle.x;
	m_player1->m_worldCamera.m_orientation.m_pitchDegrees		= g_theApp->m_cameraFixedAngle.y;
	m_player1->m_worldCamera.m_orientation.m_rollDegrees		= g_theApp->m_cameraFixedAngle.z;
	m_player1->m_playerID										= 1;
//	m_player1->SetNetState();
	m_entityList.push_back( m_player1 );

	//----------------------------------------------------------------------------------------------------------------------
	// Adding player2 to scene
	//----------------------------------------------------------------------------------------------------------------------
	m_player2													= new Player(this);
	m_player2->m_worldCamera.m_position							= g_theApp->m_cameraStartPosition;
	m_player2->m_worldCamera.m_orientation.m_yawDegrees			= g_theApp->m_cameraFixedAngle.x;
	m_player2->m_worldCamera.m_orientation.m_pitchDegrees		= g_theApp->m_cameraFixedAngle.y;
	m_player2->m_worldCamera.m_orientation.m_rollDegrees		= g_theApp->m_cameraFixedAngle.z;
	m_player2->m_playerID										= 2;
	m_player2->m_currentPlayerTurnState							= PlayerTurnState::WAITING_FOR_TURN;
	m_player2->m_requestedPlayerTurnState						= PlayerTurnState::WAITING_FOR_TURN;
//	m_player2->SetNetState();
	m_entityList.push_back( m_player2 );

	//----------------------------------------------------------------------------------------------------------------------
	// Set currentPlayer as player1 to start
	//----------------------------------------------------------------------------------------------------------------------
	m_currentPlayer = m_player1;

	//----------------------------------------------------------------------------------------------------------------------
	// Set local and remote playerRefs
	//----------------------------------------------------------------------------------------------------------------------
	InitLocalRemotePlayerRefs();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::ToggleCurrentPlayerPointer()
{
	if ( m_currentPlayer == m_player1 )
	{
		m_currentPlayer = m_player2;
	}
	else if ( m_currentPlayer == m_player2 )
	{
		m_currentPlayer = m_player1;
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::InitializeLocalPlayerReady()
{
	m_localPlayerIsReady = true;
}


//----------------------------------------------------------------------------------------------------------------------
Player* Game::GetPointerToEnemyPlayer()
{
	// Get a pointer to the "other player"
	Player* enemyPlayer = nullptr;
	if ( m_currentPlayer == m_player1 )
	{
		enemyPlayer = m_player2;
		return enemyPlayer;
	}
	else if ( m_currentPlayer == m_player2 )
	{
		enemyPlayer = m_player1;
		return enemyPlayer;
	}
	return nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::InitLocalRemotePlayerRefs()
{
	if ( g_theNetSystem->IsNetworking() )
	{
		m_player1->m_netState = NetState::SERVER;
		m_player2->m_netState = NetState::CLIENT;

		std::string player1NetState	= m_player1->GetNetStateEnumAsString();
		std::string localNetState	= g_theNetSystem->GetModeEnumAsString();
		if ( player1NetState == localNetState )
		{
			m_localPlayer	= m_player1;
			m_remotePlayer  = m_player2;
		}
		else
		{
			m_localPlayer	= m_player2;
			m_remotePlayer  = m_player1;
		}
	}
	else
	{
		// Default initialize local to P1 and remote to P2 just to be safe
		m_localPlayer			= m_player1;
		m_remotePlayer			= m_player2;
		m_localPlayerIsReady	= true;
		m_remotePlayerIsReady	= true;
	}
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::ArePlayersReady() const
{
	if ( m_localPlayerIsReady && m_remotePlayerIsReady )
//	if ( m_localPlayer->m_isPlayerReady && m_remotePlayer->m_isPlayerReady )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::UpdatePrevCursorPosToCurrent()
{
	m_previousCursorPos = m_currentCursorPos;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateCursorPos()
{
	UpdatePrevCursorPosToCurrent();
	m_currentCursorPos		= g_theInput->GetCursorClientPosition();
	m_currentCursorPos.y	= m_clientDimensions.y - m_currentCursorPos.y;		// Negate or Flips cursor Y 

/*
	// Update cursor normally if not in "Playing state"
	if ( m_currentGameState != GameState::PLAYING )
	{
		UpdateCursorPos();
		return;
	}

	// If my turn, update cursorPos from g_input AND send cursorPos to net when hoveredHex changes
	if ( IsMyTurn() )
	{
		// Get tile coords from prev cursor and current cursor, check if not same, then update
		Vec2 currentCursorPos	= g_theInput->GetCursorClientPosition();
		currentCursorPos.y		= m_clientDimensions.y - currentCursorPos.y;		// Negate or Flips cursor Y 
		if ( m_currentMap != nullptr )
		{
			IntVec2 prevHexCoord	= m_currentMap->GetHexCoordsFromCursorPos( m_previousCursorPos );
			IntVec2 currentHexCoord = m_currentMap->GetHexCoordsFromCursorPos( currentCursorPos	   );
			if ( prevHexCoord != currentHexCoord )
			{		
//				if ( g_theNetSystem->m_mode == NetSystem::Mode::SERVER )
//				{
					m_currentCursorPos = currentCursorPos;
					SendCursorPos();
//				}
			}
		} 
	}
	// If waiting, update cursor from network
	else	
	{
		// If waiting for my turn
		// Update cursor from Network
		UpdatePrevCursorPosToCurrent();
	}
*/
}


//----------------------------------------------------------------------------------------------------------------------
void Game::UpdatePlayerStatesInput()
{
	if ( m_currentPlayer->m_currentPlayerTurnState == PlayerTurnState::CONFIRM_END_TURN )
	{
		return;
	}

	if (	  m_currentPlayer->m_currentPlayerState == PlayerState::SELECTING )
	{
		ResetMouseClick_UI();

		//----------------------------------------------------------------------------------------------------------------------
		// Logic for moving a tank unit
		// If the leftMouse was just clicked, AND 
		// the cursor is hovering over a unit that's movable AND
		// that unit belongs to the currentPlayer THEN
		// update the player and unit states accordingly
		//----------------------------------------------------------------------------------------------------------------------
		if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFT_MOUSE ) )
		{			
			SelectHoveredUnit();
			g_theNetSystem->SendMessage( "SelectHoveredUnit" );
		}

		//----------------------------------------------------------------------------------------------------------------------
		// Cycle through units ( previous / next ) 
		//----------------------------------------------------------------------------------------------------------------------
		// Previous
		if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFTARROW ) || m_wasMouseClickedHovered_Previous )
		{
			SelectPreviousUnit();
			g_theNetSystem->SendMessage( "SelectPreviousUnit" );
		}
		// Next 
		if ( g_theInput->WasKeyJustPressed( KEYCODE_RIGHTARROW ) || m_wasMouseClickedHovered_Next )
		{
			SelectNextUnit();
			g_theNetSystem->SendMessage( "SelectNextUnit" );
		}
	}
	else if ( m_currentPlayer->m_currentPlayerState == PlayerState::SELECTED )
	{
		ResetMouseClick_UI();

		//----------------------------------------------------------------------------------------------------------------------
		// Handle Left mouse clicked logic
		//----------------------------------------------------------------------------------------------------------------------
		if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFT_MOUSE ) )
		{
			TryMoveOrSelectNewUnit();
		}


		//----------------------------------------------------------------------------------------------------------------------
		// Generate DistancePath while hovering over tiles AND a unit is currently selected
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_currentSelectedUnit != nullptr )
		{
			GenerateDistancePath();
			g_theNetSystem->SendMessage( "GenerateDistPath" );
		}


		//----------------------------------------------------------------------------------------------------------------------
		// Cycle through units ( previous / next ) 
		//----------------------------------------------------------------------------------------------------------------------
		// Previous
		if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFTARROW ) || m_wasMouseClickedHovered_Previous )
		{
			SelectPreviousUnit();
			g_theNetSystem->SendMessage( "SelectPreviousUnit" );
		}
		// Next
		if ( g_theInput->WasKeyJustPressed( KEYCODE_RIGHTARROW ) || m_wasMouseClickedHovered_Next )
		{
			SelectNextUnit();
			g_theNetSystem->SendMessage( "SelectNextUnit" );
		}
	}
	else if ( m_currentPlayer->m_currentPlayerState == PlayerState::MOVING )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// If leftMouseClicked
		// If cursor matches currentHoveredUnit tileCoord
		//		True:  Confirm move
		//		False: Do nothing
		//----------------------------------------------------------------------------------------------------------------------
		if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFT_MOUSE ) )
		{
			MoveSelectedUnit();
			g_theNetSystem->SendMessage( "MoveSelectedUnit" );
		}
		if ( g_theInput->WasKeyJustPressed( KEYCODE_ESC ) )
		{
			MoveSelectedUnitToPrevTile();
			g_theNetSystem->SendMessage( "MoveSelectedUnitToPrevTile" );
		}
	}
	else if ( m_currentPlayer->m_currentPlayerState == PlayerState::CONFIRM_MOVE )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Update unitState of enemy units within attack range to render dark red hex border
		//----------------------------------------------------------------------------------------------------------------------
		// Loop through all enemy units and check which units are within attack range

		//----------------------------------------------------------------------------------------------------------------------
		// If leftMouseClicked
		// GetUnitOnHoveredHex
		// If no unit exists, end turn
		//----------------------------------------------------------------------------------------------------------------------
		if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFT_MOUSE ) )
		{
			ConfirmMove();
			g_theNetSystem->SendMessage( "ConfirmMove" );
		}
	}
	else if ( m_currentPlayer->m_currentPlayerState == PlayerState::TRY_ATTACK )
	{
		if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFT_MOUSE ) )
		{
			TryAttack();
			g_theNetSystem->SendMessage( "TryAttack" );
		}
	}
	else if ( m_currentPlayer->m_currentPlayerState == PlayerState::CONFIRM_ATTACK )
	{	

		ConfirmAttack();
		g_theNetSystem->SendMessage( "ConfirmAttack" );
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::MoveToHoveredTile()
{
//	IntVec2 currentHoveredTile  = m_currentMap->GetTileHoveredByCursor();
	IntVec2 currentHoveredTile  = m_hoveredHex;
	Vec2	worldPos			= m_currentMap->GetWorldPosFromTileCoords( currentHoveredTile );
	if ( m_currentMap->IsHexCenterWithinMapBounds( worldPos ) )
	{ 
		if ( !m_currentMap->IsTileBlocked( currentHoveredTile ) )
		{
			// Get Tile from tileCoord
			// Check if that tile.dist is NOT -1 AND dist is within maxMoveRange					
			Tile& hoveredTile = m_currentMap->GetTileFromTileCoord( currentHoveredTile );
			if ( ( hoveredTile.m_hexDistToSelectedUnit > -1 ) && ( hoveredTile.m_hexDistToSelectedUnit <= m_currentSelectedUnit->m_unitDef->m_movementRange ) )
			{
				m_currentSelectedUnit->m_previousTileCoord	= m_currentSelectedUnit->m_currentTileCoord;
				m_currentSelectedUnit->m_currentTileCoord	= currentHoveredTile;
				m_currentSelectedUnit->m_currentUnitState	= UnitState::MOVED;
				m_currentPlayer->m_requestedPlayerState		= PlayerState::MOVING;
			}
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateHoveredAndPrevHex()
{
	//----------------------------------------------------------------------------------------------------------------------
	// 1. Get hexCoordFromCursorPos
	//		1a. Save currentHexCoord
	// 2. Replace all locations where I getHexFromCursor with currentHexCoord
	//----------------------------------------------------------------------------------------------------------------------
	IntVec2 currentHoveredHex = IntVec2( -1, -1 );
	if ( m_currentMap != nullptr )
	{
		// Make sure to convert to worldSpace to render hex correctly
		m_prevHoveredHex	= m_hoveredHex;
		currentHoveredHex	= m_currentMap->GetHexCoordsFromCursorPos();
	}

	// Send hoveredHex to network party if hexCoord has changed 
//	if ( g_theNetSystem->m_mode == NetSystem::Mode::CLIENT )
	if ( IsMyTurn() )
	{
		if ( m_prevHoveredHex != currentHoveredHex )
		{
			m_hoveredHex = currentHoveredHex;
			SendHexPos();
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::MoveSelectedUnitToPrevTile()
{
	if ( m_currentSelectedUnit == nullptr )
	{
//		int x = 0;
		return;
	}

	m_currentPlayer->m_requestedPlayerTurnState = PlayerTurnState::END_TURN;
	m_currentSelectedUnit->m_currentTileCoord	= m_currentSelectedUnit->m_previousTileCoord;
	if ( m_targetedEnemyUnit != nullptr )
	{
		m_targetedEnemyUnit->m_currentUnitState = READY;
	}
	m_targetedEnemyUnit = nullptr;
	m_currentMap->m_shouldRenderHighlightedHex_Red = false;
	ResetAllEnemyUnitStates();
	RemoveDistFieldAndPathRendering();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::GenerateDistanceField()
{
	// 0. Reset all Tiles dist and bool
	for ( int i = 0; i < m_currentMap->m_tileList.size(); i++ )
	{
		Tile& currentTile					= m_currentMap->m_tileList[i];
		currentTile.m_hexDistToSelectedUnit = -1;
		currentTile.m_isAlreadyVisted		= false;
	}
	while ( !m_pQueue.empty() )
	{
		m_pQueue.pop();
	} 
	

	// 1. Set currentHexTile variables and add to m_pQueue
	//		1a. Set currentHexTile.dist to 0
	//		1b. Set currentHexTile.isAlreadyVisted to true
	//		1c. PushBack into pQueue
	int tileIndex						= m_currentMap->GetTileIndexForTileCoords( m_currentSelectedUnit->m_currentTileCoord.x, m_currentSelectedUnit->m_currentTileCoord.y );
	Tile& currentTile					= m_currentMap->m_tileList[tileIndex];
	currentTile.m_hexDistToSelectedUnit = 0;
	currentTile.m_isAlreadyVisted		= true;
	m_pQueue.push( currentTile );
	// 2. While the pQueue is not empty, loop through the list
	//		2a. Get the topTile 
	//		2b. If topTile.dist is greater than maxRange, break out of the loop
	int maxRangeCurrentUnit = m_currentSelectedUnit->m_unitDef->m_movementRange;
	while ( !m_pQueue.empty() )
	{
		Tile const& topTile = m_pQueue.top();
		if ( topTile.m_hexDistToSelectedUnit >= maxRangeCurrentUnit )
		{
			break;
		}
		// 3. GetFreshValidNeighbors of topTile ( aka, currentHexTile )
		//		3a. Set neighbor.dist to ( topTile.dist + 1 )
		//		3b. Set neighbor.bool to true
		std::vector<Tile> neighborTileList;
		neighborTileList = GetFreshValidNeighborTileList( topTile );
		// 4. Push neighbor into queue list
		for ( int i = 0; i < neighborTileList.size(); i++ )
		{
			m_pQueue.push( neighborTileList[i] );
		}
		// 5. Pop the queue top
		m_pQueue.pop();
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::GenerateDistancePath()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Generate distancePath from currentUnitPos to cursorHighlightedHex
	//----------------------------------------------------------------------------------------------------------------------
	// Get TileHoveredByCursor
	m_distPathTileList.clear();
	if ( m_currentMap == nullptr )
	{
		return;
	}

//	IntVec2 currentTileCoords = m_currentMap->GetTileHoveredByCursor();
	IntVec2 currentTileCoords = m_hoveredHex;
	if ( m_currentMap->IsTileValid( currentTileCoords ) )
	{
		if ( m_currentSelectedUnit != nullptr )
		{
			int tileCoordsIndex			= m_currentMap->GetTileIndexForTileCoords( currentTileCoords.x, currentTileCoords.y );
			Tile currentTile			= m_currentMap->m_tileList[tileCoordsIndex];
			bool isWithinTaxiCabDist	= IsWithinTaxiCabDistance( currentTileCoords, m_currentSelectedUnit->m_currentTileCoord );
			if ( isWithinTaxiCabDist )
			{
				m_distPathTileList.push_back( currentTile );
			}

			int currentTileDistValue = currentTile.m_hexDistToSelectedUnit;
			while ( currentTileDistValue >= 0 )
			{
				std::vector<Tile> vistedValidNeighborTileList = GetVisitedValidNeighborTileList( currentTile );
				int distOneLower = currentTileDistValue - 1;
				for ( int i = 0; i < vistedValidNeighborTileList.size(); i++ )
				{
					if ( vistedValidNeighborTileList[i].m_hexDistToSelectedUnit == distOneLower )
					{
						m_distPathTileList.push_back( vistedValidNeighborTileList[i] );
						currentTile = vistedValidNeighborTileList[i];
						i			= int( vistedValidNeighborTileList.size() - 1 );
					}
				}
				currentTileDistValue -= 1;
			}
			// Get Tile at current highlighted hex, 
			// Get neighborTile dist - 1
			// Save that tile
			// Repeat the loop until dist == 0
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
std::vector<Tile> Game::GetFreshValidNeighborTileList( Tile currentTile )
{
	std::vector<Tile>		freshValidNeighborTileList;
	std::vector<IntVec2>	neighborPosList;
	// Get NeighborTiles
	// North	 Neighbor (+Y)
	IntVec2 northNeighbor		= IntVec2( currentTile.m_hexCoord.x,	 currentTile.m_hexCoord.y + 1	);
	// South	 Neighbor (-Y)									   
	IntVec2 southNeighbor		= IntVec2( currentTile.m_hexCoord.x,	 currentTile.m_hexCoord.y - 1	);
	// East		 Neighbor (+X)
	IntVec2 eastNeighbor		= IntVec2( currentTile.m_hexCoord.x + 1, currentTile.m_hexCoord.y		);
	// West		 Neighbor (-X)
	IntVec2 westNeighbor		= IntVec2( currentTile.m_hexCoord.x - 1, currentTile.m_hexCoord.y		);
	// SouthEast Neighbor (+X,-Y)
	IntVec2 southEastNeighbor	= IntVec2( currentTile.m_hexCoord.x + 1, currentTile.m_hexCoord.y - 1	);
	// NorthWest Neighbor (-X,+Y)
	IntVec2 northWestNeighbor	= IntVec2( currentTile.m_hexCoord.x - 1, currentTile.m_hexCoord.y  + 1  );
	neighborPosList.push_back( northNeighbor	  );
	neighborPosList.push_back( southNeighbor	  );
	neighborPosList.push_back( eastNeighbor		  );
	neighborPosList.push_back( westNeighbor		  );
	neighborPosList.push_back( southEastNeighbor  );
	neighborPosList.push_back( northWestNeighbor  );

	// Check if neighborTile is fresh AND valid
	for ( int i = 0; i < neighborPosList.size(); i++ )
	{
		IntVec2 currentNeighborTileCoords	= neighborPosList[i]; 
		bool isValid						= m_currentMap->IsTileValid( currentNeighborTileCoords );
		// Get index from tile coords
		// Get tile from index
		if ( isValid )
		{
			int tileIndex									= m_currentMap->GetTileIndexForTileCoords( currentNeighborTileCoords.x, currentNeighborTileCoords.y );
			Tile& currentNeighborTile						= m_currentMap->m_tileList[tileIndex];
			bool isAlreadyVisited							= currentNeighborTile.m_isAlreadyVisted;
			if ( !isAlreadyVisited )
			{
				currentNeighborTile.m_isAlreadyVisted		= true;
				currentNeighborTile.m_hexDistToSelectedUnit	= currentTile.m_hexDistToSelectedUnit + 1;
				freshValidNeighborTileList.push_back( currentNeighborTile );
			}
		}
	}
	return freshValidNeighborTileList;
}


//----------------------------------------------------------------------------------------------------------------------
std::vector<Tile> Game::GetVisitedValidNeighborTileList( Tile currentTile )
{
	std::vector<Tile>		visitedValidNeighborTileList;
	std::vector<IntVec2>	neighborPosList;
	// Get NeighborTiles
	// North	 Neighbor (+Y)
	IntVec2 northNeighbor		= IntVec2( currentTile.m_hexCoord.x,	 currentTile.m_hexCoord.y + 1	);
	// South	 Neighbor (-Y)									   
	IntVec2 southNeighbor		= IntVec2( currentTile.m_hexCoord.x,	 currentTile.m_hexCoord.y - 1	);
	// East		 Neighbor (+X)
	IntVec2 eastNeighbor		= IntVec2( currentTile.m_hexCoord.x + 1, currentTile.m_hexCoord.y		);
	// West		 Neighbor (-X)
	IntVec2 westNeighbor		= IntVec2( currentTile.m_hexCoord.x - 1, currentTile.m_hexCoord.y		);
	// SouthEast Neighbor (+X,-Y)
	IntVec2 southEastNeighbor	= IntVec2( currentTile.m_hexCoord.x + 1, currentTile.m_hexCoord.y - 1	);
	// NorthWest Neighbor (-X,+Y)
	IntVec2 northWestNeighbor	= IntVec2( currentTile.m_hexCoord.x - 1, currentTile.m_hexCoord.y  + 1  );
	neighborPosList.push_back( northNeighbor	  );
	neighborPosList.push_back( southNeighbor	  );
	neighborPosList.push_back( eastNeighbor		  );
	neighborPosList.push_back( westNeighbor		  );
	neighborPosList.push_back( southEastNeighbor  );
	neighborPosList.push_back( northWestNeighbor  );

	// Check if neighborTile is visited AND valid
	for ( int i = 0; i < neighborPosList.size(); i++ )
	{
		IntVec2 currentNeighborTileCoords	= neighborPosList[i]; 
		bool isValid						= m_currentMap->IsTileValid( currentNeighborTileCoords );
		// Get index from tile coords
		// Get tile from index
		if ( isValid )
		{
			int tileIndex					= m_currentMap->GetTileIndexForTileCoords( currentNeighborTileCoords.x, currentNeighborTileCoords.y );
			Tile& currentNeighborTile		= m_currentMap->m_tileList[tileIndex];
			bool isAlreadyVisited			= currentNeighborTile.m_isAlreadyVisted;
			if ( isAlreadyVisited )
			{
				visitedValidNeighborTileList.push_back( currentNeighborTile );
			}
		}
	}
	return visitedValidNeighborTileList;
}
 

//----------------------------------------------------------------------------------------------------------------------
bool Game::IsWithinTaxiCabDistance( IntVec2 tileCoordA, IntVec2 tileCoordB )
{
	// Get taxiCabDist between two coords
	int a				= abs( tileCoordA.x - tileCoordB.x );
	int b				= abs( tileCoordA.x + tileCoordA.y - tileCoordB.x - tileCoordB.y );
	int c				= abs( tileCoordA.y - tileCoordB.y );
	int taxiCabDist		= ( a + b + c ) / 2;
	int moveRange		= m_currentSelectedUnit->m_unitDef->m_movementRange;
	if ( taxiCabDist <= moveRange )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::GetRidOfDistanceFieldRendering()
{
	// Get rid of distance field rendering
	for ( int i = 0; i < m_currentMap->m_tileList.size(); i++ )
	{
		Tile& currentTile = m_currentMap->m_tileList[ i ];
		if ( currentTile.m_hexDistToSelectedUnit >= 0 )
		{
			currentTile.m_hexDistToSelectedUnit = -1;
			currentTile.m_isAlreadyVisted = false;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::RemoveDistFieldAndPathRendering()
{
	m_distPathTileList.clear();
	GetRidOfDistanceFieldRendering();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::RenderEnemiesInAttackRange() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render red borders when enemy is within currentPlayer's attack range 
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_enemyPlayer != nullptr && m_enemyPlayer->m_playerID <=2 )
	{
		std::vector<Vertex_PCU> verts;
		for ( int i = 0; i < m_enemyPlayer->m_unitList.size(); i++ )
		{
			Unit& currentEnemyUnit = m_enemyPlayer->m_unitList[ i ];
			if ( currentEnemyUnit.m_currentUnitState != UnitState::IS_DEAD )
			{
				if ( m_currentSelectedUnit != nullptr )
				{
					if ( m_currentSelectedUnit->m_currentUnitState != FINISHED_MOVING_THIS_TURN )
					{
						if ( currentEnemyUnit.m_currentUnitState == WITHIN_ENEMY_ATTACK_RANGE )
						{
							Vec2 centerPos_worldSpace = m_currentMap->GetWorldPosFromTileCoords( currentEnemyUnit.m_currentTileCoord );
							AddVertsForHexagonBorders2D( verts, centerPos_worldSpace, CIRCUMRADIUS * 0.75f, Rgba8::DARK_RED );
						}
					}
				}
			}
		}
		// Draw call
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->BindTexture( nullptr );
		Mat44 mat;
		mat.SetTranslation3D( Vec3( 0.0f, 0.0f, 0.0001f ) );
		g_theRenderer->SetModelConstants( mat );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->DrawVertexArray( int( verts.size() ), verts.data() );

	}
	if ( m_targetedEnemyUnit != nullptr )
	{
		if ( m_targetedEnemyUnit->m_currentUnitState != UnitState::IS_DEAD )
		{			
			std::vector<Vertex_PCU> verts;
			Vec2 centerPos_worldSpace = m_currentMap->GetWorldPosFromTileCoords( m_targetedEnemyUnit->m_currentTileCoord );
			AddVertsForHexagonBorders2D( verts, centerPos_worldSpace, CIRCUMRADIUS * 0.75f, Rgba8::RED );

			// Draw call
			g_theRenderer->SetBlendMode( BlendMode::ALPHA );
			g_theRenderer->BindTexture( nullptr );
			Mat44 mat;
			mat.SetTranslation3D( Vec3( 0.0f, 0.0f, 0.0001f ) );
			g_theRenderer->SetModelConstants( mat );
			g_theRenderer->BindShader( nullptr );
			g_theRenderer->SetDepthMode( DepthMode::DISABLED );
			g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			g_theRenderer->DrawVertexArray( int( verts.size() ), verts.data() );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
Unit* Game::GetCurrentHoveredUnit()
{
//	UpdateHoveredAndPrevHex();
	Unit* currentHoveredUnit = nullptr;
//	IntVec2 cursorHoveredHex = m_currentMap->GetHexCoordsFromCursorPos( m_currentCursorPos );
	IntVec2 cursorHoveredHex = m_hoveredHex;
	if ( ( cursorHoveredHex == IntVec2(-1, -1) ) || ( m_currentPlayer == nullptr ) )
	{
		return nullptr;
	}
	// Loop through all current player's units and check if the unitCoords matches the cursorCoords
	for ( int i = 0; i < m_currentPlayer->m_unitList.size(); i++ )
	{
		Unit& currentUnit = m_currentPlayer->m_unitList[i];
		if ( currentUnit.m_currentTileCoord == cursorHoveredHex )
		{
			if ( currentUnit.m_currentUnitState == UnitState::IS_DEAD )
			{
				continue;
			}
			currentHoveredUnit = &currentUnit;
			return currentHoveredUnit;
		}
	}
	
	// If no "friendly" unit was found, check the enemy player's unit list
	Player* enemyPlayer = GetPointerToEnemyPlayer();
	// Loop through all enemy player's units and check if the unitCoords matches the cursorCoords
	for ( int i = 0; i < enemyPlayer->m_unitList.size(); i++ )
	{
		Unit& enemyUnit = enemyPlayer->m_unitList[i];
		if ( enemyUnit.m_currentTileCoord == cursorHoveredHex )
		{
			if ( enemyUnit.m_currentUnitState == UnitState::IS_DEAD )
			{
				continue;
			}
			currentHoveredUnit = &enemyUnit;
			return currentHoveredUnit;
		}
	}
	return nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::IsUnitMovable( Unit const* currentHoveredUnit )
{
	// Check if turn is already finished
	if ( currentHoveredUnit->m_currentUnitState == UnitState::IS_DEAD )
	{
		return false;
	}
	if ( currentHoveredUnit->m_currentUnitState != UnitState::FINISHED_MOVING_THIS_TURN )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::DoesUnitBelongToCurrentPlayer( Unit const* currentHoveredUnit )
{
	// Check if this unit belongs to current player by matching currentHoveredUnit's tileCoords with the currentPlayer's units' tileCoords
	for ( int i = 0; i < m_currentPlayer->m_unitList.size(); i++ )
	{
		Unit& currentUnit = m_currentPlayer->m_unitList[i];
		if ( currentUnit.m_currentTileCoord == currentHoveredUnit->m_currentTileCoord )
		{
			if ( !currentUnit.IsDead() )
			{
				return true;
			}
		}
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
int Game::GetUnitIndexFromTileCoord( IntVec2 const& currentTileCoord )
{
	int unitIndex = -1;
	for ( int i = 0; i < m_currentPlayer->m_unitList.size(); i++ )
	{
		Unit& currentUnit = m_currentPlayer->m_unitList[i];
		if ( currentUnit.m_currentTileCoord == currentTileCoord )
		{
			unitIndex = i;
		}
	}
	return unitIndex;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::SetEnemiesUnitsInAttackRange_WithinAttackRangeUnitState()
{
	if ( m_currentSelectedUnit == nullptr )
	{
		return;
	}
	m_enemyPlayer = GetPointerToEnemyPlayer();
	for ( int i = 0; i < m_enemyPlayer->m_unitList.size(); i++ )
	{
		// Get taxiCabDist from current unit to enemy 
		Unit& currentEnemyUnit = m_enemyPlayer->m_unitList[ i ];
		if ( currentEnemyUnit.m_currentUnitState == UnitState::IS_DEAD )
		{
			continue;
		}
		bool isWithinAttackRange = currentEnemyUnit.IsWithinAttackRangeOfRefUnit( m_currentSelectedUnit );
		if ( isWithinAttackRange )
		{
			currentEnemyUnit.m_currentUnitState = WITHIN_ENEMY_ATTACK_RANGE;
		}
		else
		{
			currentEnemyUnit.m_currentUnitState = READY;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::ResetAllEnemyUnitStates()
{
	// Loop through all enemy units and reset their unit states back to ready
	m_enemyPlayer = GetPointerToEnemyPlayer();
	for ( int i = 0; i < m_enemyPlayer->m_unitList.size(); i++ )
	{
		Unit& currentEnemyUnit = m_enemyPlayer->m_unitList[i];
		if ( currentEnemyUnit.m_currentUnitState != UnitState::IS_DEAD )
		{
			currentEnemyUnit.m_currentUnitState = UnitState::READY;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::SelectPreviousUnit_Left_UnitStateSelected()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_LEFTARROW ) )
	{
		// Get index of currentSelectedUnit
		// decrement the index
		int unitIndex = GetUnitIndexFromTileCoord( m_currentSelectedUnit->m_currentTileCoord );
		unitIndex--;
		//			while loop if current Unit is not finished turn or not dead
		while (1)
		{
			if ( unitIndex < 0 )
			{
				unitIndex = ( int( m_currentPlayer->m_unitList.size() ) - 1 ) ;
			}
			if ( ( m_currentPlayer->m_unitList[unitIndex].m_currentUnitState != UnitState::FINISHED_MOVING_THIS_TURN ) && 
				( m_currentPlayer->m_unitList[unitIndex].m_currentUnitState != UnitState::IS_DEAD ) )
			{
				break;
			}
			// If the new unit has already finished moving this turn, decrement again!
			unitIndex--;
		}
		if ( unitIndex >= 0 )
		{
			m_currentSelectedUnit->m_currentUnitState	= UnitState::READY;
			m_currentSelectedUnit						= &m_currentPlayer->m_unitList[unitIndex];
			m_currentSelectedUnit->m_currentUnitState	= UnitState::SELECTED;
		}
		else
		{
			// If the decremented index is negative, then loop back to the size of the list
			unitIndex									= int( ( m_currentPlayer->m_unitList.size() - 1 ) );
			m_currentSelectedUnit->m_currentUnitState	= UnitState::READY;
			m_currentSelectedUnit						= &m_currentPlayer->m_unitList[unitIndex];
			m_currentSelectedUnit->m_currentUnitState	= UnitState::SELECTED;
		}
		// Trigger distance field generation
		GenerateDistanceField();
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::SelectNextUnit_RightUnitStateSelected()
{
	if ( g_theInput->WasKeyJustPressed( KEYCODE_RIGHTARROW ) )
	{
		// Get index of currentSelectedUnit
		// increment the index
		int unitIndex = GetUnitIndexFromTileCoord( m_currentSelectedUnit->m_currentTileCoord );
		unitIndex++;
		while (1)
		{
			if ( unitIndex >= m_currentPlayer->m_unitList.size() )
			{
				unitIndex = 0;
			}
			if ( ( m_currentPlayer->m_unitList[ unitIndex ].m_currentUnitState != UnitState::FINISHED_MOVING_THIS_TURN ) && 
				( m_currentPlayer->m_unitList[ unitIndex ].m_currentUnitState != UnitState::IS_DEAD ) )
			{
				break;
			}
			// If the new unit has already finished moving this turn, decrement again!
			unitIndex++;
		}
		if ( unitIndex <= (m_currentPlayer->m_unitList.size() - 1) )
		{
			m_currentSelectedUnit->m_currentUnitState	= UnitState::READY;
			m_currentSelectedUnit						= &m_currentPlayer->m_unitList[unitIndex];
			m_currentSelectedUnit->m_currentUnitState	= UnitState::SELECTED;
		}
		else
		{
			// If the decremented index is negative, then loop back to the size of the list
			m_currentSelectedUnit->m_currentUnitState	= UnitState::READY;
			unitIndex									= 0;
			m_currentSelectedUnit						= &m_currentPlayer->m_unitList[unitIndex];
			m_currentSelectedUnit->m_currentUnitState	= UnitState::SELECTED;
		}
		// Trigger distance field generation
		GenerateDistanceField();
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateHoveredUnitStats()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Get unitDef of highlighted Unit to display stats 
	//----------------------------------------------------------------------------------------------------------------------
	// Get m_highlightedHexCenterPos
	// Compare if this position matches with positions of units
	//		True:  Get the information of 'that' unit
	//		False: Set information to default
	// Render the information
	IntVec2 highlightedHex = m_hoveredHex;
	for ( int i = 0; i < m_player1->m_unitList.size(); i++ )
	{
		Unit& currentUnit = m_player1->m_unitList[i];
		if ( currentUnit.m_currentUnitState == UnitState::IS_DEAD )
		{
			m_currentHoveredUnit = nullptr;
			m_HoveredUnitDef	 = nullptr;
		}
		else
		{
			if ( highlightedHex == currentUnit.m_currentTileCoord )
			{
				m_currentHoveredUnit = &currentUnit;
				m_HoveredUnitDef = currentUnit.m_unitDef;
				return;
			}
			else
			{
				m_currentHoveredUnit = nullptr;
				m_HoveredUnitDef	 = nullptr;
			}
		}
	}
	for ( int i = 0; i < m_player2->m_unitList.size(); i++ )
	{
		Unit& currentUnit = m_player2->m_unitList[i];
		if ( currentUnit.m_currentUnitState == UnitState::IS_DEAD )
		{
			m_currentHoveredUnit = nullptr;
			m_HoveredUnitDef	 = nullptr;
		}
		else
		{
			if ( highlightedHex == currentUnit.m_currentTileCoord )
			{
				m_currentHoveredUnit = &currentUnit;
				m_HoveredUnitDef	 = currentUnit.m_unitDef;
				return;
			}
			else
			{
				m_currentHoveredUnit = nullptr;
				m_HoveredUnitDef	 = nullptr;
			}
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::UpdatAllUnitsDeath()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Update Units to check for death
	//----------------------------------------------------------------------------------------------------------------------
	for ( int i = 0; i < m_player1->m_unitList.size(); i++ )
	{
		Unit& currentUnit = m_player1->m_unitList[ i ];
		currentUnit.Update();
	}
	for ( int i = 0; i < m_player2->m_unitList.size(); i++ )
	{
		Unit& currentUnit = m_player2->m_unitList[ i ];
		currentUnit.Update();
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::EnterState( GameState state )
{
	if ( state == GameState::ATTRACT )
	{
	}
	else if ( state == GameState::PLAYING )
	{	
	}
	else if ( state == GameState::LOBBY )
	{
		if ( m_previousGameState == GameState::PAUSED )
		{
			g_theNetSystem->SendMessage( "MatchDraw" );
		}
		else if ( m_previousGameState == GameState::PLAYING )
		{
			if ( g_theNetSystem->IsNetworking() )
			{
				g_theNetSystem->SendMessage( "ResetRemoteGameOverStates" );
			}
		}
		Shutdown();
		ResetRemoteGameOverStates();
	}
	else if ( state == GameState::PAUSED )
	{
	}
}
 

//----------------------------------------------------------------------------------------------------------------------
void Game::ExitState( GameState state )
{
	if ( state == GameState::ATTRACT )
	{
	}
	else if ( state == GameState::PLAYING )
	{
	}
	else if ( state == GameState::LOBBY )
	{
		for ( int i = 0; i < m_lobbyButtonList.size(); i++ )
		{
			m_lobbyButtonList[i]->SetIsHighlighted( false );
		}
	}
	else if ( state == GameState::PAUSED )
	{
		for ( int i = 0; i < m_pausedButtonList.size(); i++ )
		{
			m_pausedButtonList[i]->SetIsHighlighted( false );
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::MatchDraw()
{
	// Match is tied
	m_gameOverState = GameOverState::MATCH_DRAW;
	m_isGameOver	= true;
	m_button_playerTurn->SetIsHighlighted( true );
	m_currentPlayer->m_requestedPlayerTurnState = PlayerTurnState::POP_UP;
	m_remotePlayerIsReady = false;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::CheckGameOver()
{
	if ( m_gameOverState != NONE  )
	{
		return;
	}

	// 1. Loop through player1 units
	// If all player1's units are dead, player2 wins
	bool isPlayer1Dead = true;
	for ( int i = 0; i < m_player1->m_unitList.size(); i++ )
	{
		Unit& currentUnit = m_player1->m_unitList[i];
		if ( currentUnit.m_currentUnitState != UnitState::IS_DEAD )
		{
			isPlayer1Dead = false;
		}
	}
	// 2. Loop through player2 units
	// If all player2's units are dead, player1 wins
	bool isPlayer2Dead = true;
	for ( int j = 0; j < m_player2->m_unitList.size(); j++ )
	{
		Unit& currentUnit = m_player2->m_unitList[j];
		if ( currentUnit.m_currentUnitState != UnitState::IS_DEAD )
		{
			isPlayer2Dead = false;
		}
	}
	// 3. If all units from both teams are dead, the match is tied
	if ( isPlayer1Dead && !isPlayer2Dead )
	{
		// Player 2 wins
		m_gameOverState = GameOverState::PLAYER2_WINS;
		m_isGameOver = true;
	}
	else if ( isPlayer2Dead && !isPlayer1Dead )
	{
		// Player 1 wins
		m_gameOverState = GameOverState::PLAYER1_WINS;
		m_isGameOver = true;
	}
	else if ( isPlayer1Dead && isPlayer2Dead )
	{
		MatchDraw();
	}
	else
	{
		m_gameOverState = GameOverState::NONE;
		m_isGameOver = false;
	}
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::IsMyTurn()
{
	// Only check for turns if we are playing online
	if ( ( m_currentPlayer == nullptr ) || ( !g_theNetSystem->IsNetworking() ) )
	{
		return true;
	}
	if ( m_isGameOver )
	{
		return true;
	}
	if ( g_theNetSystem->IsConnected() )
	{
		if ( !m_localPlayerIsReady || !m_remotePlayerIsReady )
		{
			return false;
		}
	}
	else if ( !g_theNetSystem->IsConnected() )
	{
		return false;
	}

	bool isMyTurn = false;
	// Is it my turn ( Check if currentPlayer's "netState" is the same as local netSystem's "mode" )
	// If enum is equal, it's the localPlayer's turn
	std::string currentPlayerNetState	= m_currentPlayer->GetNetStateEnumAsString();
	std::string localNetState			=  g_theNetSystem->GetModeEnumAsString();
	if ( currentPlayerNetState == localNetState )
	{
		// and I'm not waiting
		if ( m_currentPlayer->m_currentPlayerTurnState != PlayerTurnState::WAITING_FOR_TURN )
		{
			isMyTurn = true;
		}
	}
	return isMyTurn;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateInputUI()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Get mouse position
	// Check if mousePos isInsideAABB2
	//		True:  wasLeftMouseJustPressed
	//		False: Do nothing
	//----------------------------------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------------------------------------
	// 'Local Game' Button
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_currentGameState == GameState::ATTRACT )
	{
	}
	if ( m_currentGameState == GameState::LOBBY )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Arrow keys to toggle button highlights
		//----------------------------------------------------------------------------------------------------------------------
		if ( g_theInput->WasKeyJustPressed( KEYCODE_UPARROW ) || g_theInput->WasKeyJustPressed( KEYCODE_DOWNARROW ) )
		{
			ToggleHighlightsLobbyButtons();
		}

		//----------------------------------------------------------------------------------------------------------------------
		// 'Local Game' Button
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_button_localGame->IsMouseHovered( m_currentCursorPos ) )
		{
			m_button_localGame->SetIsHighlighted( true );
			m_button_quit->SetIsHighlighted();
			if ( ( g_theInput->WasKeyJustPressed(KEYCODE_ENTER) ) || ( g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE) ) )
			{
				m_button_quit->SetIsHighlighted();
				m_requestedGameState = GameState::PLAYING;
			}
		}
		if ( m_button_localGame->GetIsHighlighted() && g_theInput->WasKeyJustPressed( KEYCODE_ENTER ) )
		{
			m_button_quit->SetIsHighlighted();
			m_requestedGameState = GameState::PLAYING;
		}

		//----------------------------------------------------------------------------------------------------------------------
		// 'Quit' Button
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_button_quit->IsMouseHovered( m_currentCursorPos ) )
		{
			m_button_quit->SetIsHighlighted( true );
			m_button_localGame->SetIsHighlighted();
			if ( ( g_theInput->WasKeyJustPressed( KEYCODE_ENTER ) ) || ( g_theInput->WasKeyJustPressed( KEYCODE_LEFT_MOUSE ) ) )
			{
				m_button_localGame->SetIsHighlighted();
				g_theApp->HandleQuitRequested();
			}
		}
		if ( m_button_quit->GetIsHighlighted() && g_theInput->WasKeyJustPressed( KEYCODE_ENTER ) )
		{
			m_button_localGame->SetIsHighlighted();
			g_theApp->HandleQuitRequested();
		}
	}
	if ( m_currentGameState == GameState::PAUSED )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Arrow keys to toggle button highlights
		//----------------------------------------------------------------------------------------------------------------------
		if ( g_theInput->WasKeyJustPressed( KEYCODE_UPARROW ) || g_theInput->WasKeyJustPressed( KEYCODE_DOWNARROW ) )
		{
			ToggleHighlightsPausedMenuButtons();
		}

		//----------------------------------------------------------------------------------------------------------------------
		// 'Resume' Button
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_button_resume->IsMouseHovered( m_currentCursorPos ) )
		{
			m_button_mainMenu->SetIsHighlighted();
			m_clock.TogglePause();
			if ( ( g_theInput->WasKeyJustPressed( KEYCODE_ENTER ) ) || ( g_theInput->WasKeyJustPressed( KEYCODE_LEFT_MOUSE ) ) )
			{
				m_button_mainMenu->SetIsHighlighted();
				m_requestedGameState = GameState::PLAYING;
				m_clock.TogglePause();
			}
		}
		if ( m_button_resume->GetIsHighlighted() && g_theInput->WasKeyJustPressed(KEYCODE_ENTER) )
		{
			m_button_mainMenu->SetIsHighlighted();
			m_requestedGameState = GameState::PLAYING;
			m_clock.TogglePause();
		}

		//----------------------------------------------------------------------------------------------------------------------
		// 'Main Menu' Button
		//----------------------------------------------------------------------------------------------------------------------
		if ( m_button_mainMenu->IsMouseHovered( m_currentCursorPos ) )
		{
			m_button_resume->SetIsHighlighted();
			if ( ( g_theInput->WasKeyJustPressed( KEYCODE_ENTER ) ) || ( g_theInput->WasKeyJustPressed( KEYCODE_LEFT_MOUSE ) ) )
			{
				m_button_resume->SetIsHighlighted();
				m_requestedGameState = GameState::LOBBY;
			}
		}
		if ( m_button_mainMenu->GetIsHighlighted() && g_theInput->WasKeyJustPressed( KEYCODE_ENTER ) )
		{
			m_button_resume->SetIsHighlighted();
			m_button_resume->SetIsHighlighted();
			m_requestedGameState = GameState::LOBBY;
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::InitializeButtons()
{
	//----------------------------------------------------------------------------------------------------------------------
	// Initialize Buttons
	//----------------------------------------------------------------------------------------------------------------------
	// Pop up Buttons
	m_button_playerTurn				= new Button( AABB2( 0.0f, 0.0f, m_clientDimensions.x * 0.33f, m_clientDimensions.y * 0.33f ), "Player 1's Turn" );
	m_button_playerTurn->m_text2	= "Player 2's Turn" ;
	m_button_playerTurn->m_text3	= "Player 1 Wins!" ;
	m_button_playerTurn->m_text4	= "Player 2 Wins!" ;
	m_button_playerTurn->m_text5	= "Match Tied!" ;
	m_button_playerTurn->m_text6	= "Waiting for players ..." ;
	m_button_playerTurn->m_bounds.SetCenter( Vec2( m_clientDimensions.x * 0.5f, m_clientDimensions.y * 0.5f ) );
	m_button_playerTurn->SetIsHighlighted( true );
	// Game Buttons
	float width_x			= m_clientDimensions.x / 6.0f;
	float height_y			= m_clientDimensions.y / 20.0f;
	// Mouse
	m_button_mouseSelect			= new Button( AABB2( 0.0f, 0.0f, width_x, m_clientDimensions.y * 0.05f ), "[Mouse]" );
	m_button_mouseSelect->m_bounds.SetBottomLeft( Vec2( width_x * 0.0f, 0.0f ) );
	m_button_mouseSelect->m_text2	= "Select";
	m_button_mouseSelect->m_text3	= "Move";
	m_button_mouseSelect->m_text4	= "Confirm Move";
	m_button_mouseSelect->m_text5	= "Attack / Done";
	m_button_mouseSelect->m_text6	= "Confirm Attack / Done";
	// Previous
	m_button_previousUnit	= new Button( AABB2( 0.0f, 0.0f, width_x, m_clientDimensions.y * 0.05f ), "[Left] Previous Unit" );
	m_button_previousUnit->m_bounds.SetBottomLeft( Vec2( width_x * 1.0f, 0.0f ) );
	// Next
	m_button_nextUnit		= new Button( AABB2( 0.0f, 0.0f, width_x, m_clientDimensions.y * 0.05f ), "[Right] Next Unit" );
	m_button_nextUnit->m_bounds.SetBottomLeft( Vec2( width_x * 2.0f, 0.0f ) );
	// Escape
	m_button_escape			= new Button( AABB2( 0.0f, 0.0f, width_x, m_clientDimensions.y * 0.05f ), "[Escape] Pause" );
	m_button_escape->m_bounds.SetBottomLeft( Vec2( width_x * 3.0f, 0.0f ) );
	m_button_escape->m_text2 = "[Escape] Cancel";
	// Enter
	m_button_enter			= new Button( AABB2( 0.0f, 0.0f, width_x, m_clientDimensions.y * 0.05f ), "[Enter] End Turn" );
	m_button_enter->m_bounds.SetBottomLeft( Vec2( width_x * 4.0f, 0.0f ) );
	m_button_enter->m_text2 = "[Enter] Confirm End Turn";
	// Health
	m_button_health			= new Button( AABB2( 0.0f, 0.0f, width_x, m_clientDimensions.y * 0.05f ), "Health" );
	m_button_health->m_bounds.SetBottomLeft( Vec2( width_x * 5.0f, 0.0f ) );
	// Move	
	m_button_move			= new Button( AABB2( 0.0f, 0.0f, width_x, m_clientDimensions.y * 0.05f ), "Move" );
	m_button_move->m_bounds.SetBottomLeft( Vec2( width_x * 5.0f, height_y * 1.0f ) );
	// Range
	m_button_range			= new Button( AABB2( 0.0f, 0.0f, width_x, m_clientDimensions.y * 0.05f ), "Range" );
	m_button_range->m_bounds.SetBottomLeft( Vec2( width_x * 5.0f, height_y * 2.0f ) );
	// Defense		
	m_button_defense		= new Button( AABB2( 0.0f, 0.0f, width_x, m_clientDimensions.y * 0.05f ), "Defense" );
	m_button_defense->m_bounds.SetBottomLeft( Vec2( width_x * 5.0f, height_y * 3.0f ) );
	// Attack			
	m_button_attack			= new Button( AABB2( 0.0f, 0.0f, width_x, m_clientDimensions.y * 0.05f ), "Attack" );
	m_button_attack->m_bounds.SetBottomLeft( Vec2( width_x * 5.0f, height_y * 4.0f ) );
	// Name
	m_button_name			= new Button( AABB2( 0.0f, 0.0f, width_x, m_clientDimensions.y * 0.05f ), "Name" );
	m_button_name->m_bounds.SetBottomLeft( Vec2( width_x * 5.0f, height_y * 5.0f ) );

	// Pause Menu Buttons
	m_button_resume			= new Button( AABB2( 0.0f, 0.0f, m_clientDimensions.x * 0.3f, m_clientDimensions.y * 0.05f ), "Resume Game" );
	m_button_mainMenu		= new Button( AABB2( 0.0f, 0.0f, m_clientDimensions.x * 0.3f, m_clientDimensions.y * 0.05f ), "Main Menu"	);
	m_button_resume->m_bounds.SetBottomLeft( Vec2( m_clientDimensions.x * 0.24f, m_clientDimensions.y * 0.5f ) );
	m_button_mainMenu->m_bounds.SetBottomLeft( Vec2( m_clientDimensions.x * 0.24f, m_clientDimensions.y * 0.4f ) );
	m_button_resume->SetIsHighlighted( true );
	m_pausedButtonList.emplace_back( m_button_resume   );
	m_pausedButtonList.emplace_back( m_button_mainMenu );
}


//----------------------------------------------------------------------------------------------------------------------
void Game::ToggleHighlightsLobbyButtons()
{
	m_button_localGame->ToggleIsHighlighted();
		 m_button_quit->ToggleIsHighlighted();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::ToggleHighlightsPausedMenuButtons()
{
	  m_button_resume->ToggleIsHighlighted();
	m_button_mainMenu->ToggleIsHighlighted();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::RecvUpdatedCursorPos( Vec2 const& newCursorPos )
{
	g_theGame->m_currentCursorPos = newCursorPos;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::SendCursorPos()
{
	std::string cursorPos = Stringf( "RecvCursorPos CursorPos=%0.2f,%0.2f", m_currentCursorPos.x, m_currentCursorPos.y );
	g_theNetSystem->SendMessage( cursorPos );
}


//----------------------------------------------------------------------------------------------------------------------
void Game::RecvUpdatedHexPos( IntVec2 const& newHexPos )
{
	g_theGame->m_hoveredHex = newHexPos;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::SendHexPos()
{
	std::string hexPos = Stringf( "RecvHexPos HexPos=%0.2d,%0.2d", m_hoveredHex.x, m_hoveredHex.y );
	g_theNetSystem->SendMessage( hexPos );
}


//----------------------------------------------------------------------------------------------------------------------
void Game::SelectHoveredUnit()
{
	Unit* currentHoveredUnit = GetCurrentHoveredUnit();
	if ( currentHoveredUnit != nullptr )
	{
		if ( IsUnitMovable( currentHoveredUnit ) )
		{
			if ( DoesUnitBelongToCurrentPlayer( currentHoveredUnit ) )
			{
				m_currentSelectedUnit						= currentHoveredUnit;
				m_currentPlayer->m_requestedPlayerState		= PlayerState::SELECTED;
				m_currentSelectedUnit->m_currentUnitState	= UnitState::SELECTED;

				// Trigger distance field generation
				GenerateDistanceField();
			}
		}
	}
	UpdateMouseClickToCursor_UI();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::ResetMouseClick_UI()
{
	m_wasMouseClickedHovered_Previous	= false;
	m_wasMouseClickedHovered_Next		= false;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::UpdateMouseClickToCursor_UI()
{
	m_wasMouseClickedHovered_Previous	 = m_button_previousUnit->IsMouseHovered( m_currentCursorPos );
	m_wasMouseClickedHovered_Next		 =     m_button_nextUnit->IsMouseHovered( m_currentCursorPos );
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::SelectLastUnit()
{
	if ( m_currentSelectedUnit == nullptr )
	{
		int maxUnitIndex							= int( m_currentPlayer->m_unitList.size() - 1 );
		m_currentSelectedUnit						= &m_currentPlayer->m_unitList[ maxUnitIndex ];
		if ( m_currentSelectedUnit->IsDead() )
		{
			m_currentSelectedUnit->m_currentUnitState = UnitState::IS_DEAD;
			m_currentSelectedUnit = nullptr;
			return false;
		}
		if ( m_currentSelectedUnit->FinishedMovingThisTurn() )
		{
			m_currentSelectedUnit = nullptr;
			return false;
		}
		m_currentSelectedUnit->m_currentUnitState	= UnitState::SELECTED;
		m_currentPlayer->m_requestedPlayerState		= PlayerState::SELECTED;
		GenerateDistanceField();
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::SelectFirstUnit()
{
	if ( m_currentSelectedUnit == nullptr )
	{
		m_currentSelectedUnit = &m_currentPlayer->m_unitList[0];
		if ( m_currentSelectedUnit->IsDead() )
		{
			m_currentSelectedUnit->m_currentUnitState = UnitState::IS_DEAD;
			m_currentSelectedUnit = nullptr;
			return false;
		}
		if ( m_currentSelectedUnit->FinishedMovingThisTurn() )
		{
			m_currentSelectedUnit = nullptr;
			return false;
		}
		m_currentSelectedUnit->m_currentUnitState	= UnitState::SELECTED;
		m_currentPlayer->m_requestedPlayerState		= PlayerState::SELECTED;
		GenerateDistanceField();
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::SelectPreviousUnit()
{
	if ( AreAllUnitsUnavailable() )
	{
		return;
	}
	bool wasDecremented = SelectLastUnit();
	int unitIndex		= 0;
	if ( m_currentSelectedUnit == nullptr )
	{
		unitIndex = int( m_currentPlayer->m_unitList.size() );
//		int x = 0;
	}
	else
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Get index of currentSelectedUnit
		//----------------------------------------------------------------------------------------------------------------------
		unitIndex = GetUnitIndexFromTileCoord( m_currentSelectedUnit->m_currentTileCoord );
	}
	if ( !wasDecremented  )
	{
		// Decrement the index only if did NOT get a unit
		unitIndex--;
	}

	while ( true )
	{
		if ( unitIndex < 0 )
		{
			unitIndex = ( int( m_currentPlayer->m_unitList.size() ) - 1 ) ;
		}
		Unit& thisUnit = m_currentPlayer->m_unitList[ unitIndex ];
		if ( ( thisUnit.m_currentUnitState != UnitState::FINISHED_MOVING_THIS_TURN ) &&
			( thisUnit.m_currentUnitState != UnitState::IS_DEAD ) )
		{
			break;
		}
		// If the new unit has already finished moving this turn, decrement again!
		unitIndex--;
	}
	Unit& thisUnit = m_currentPlayer->m_unitList[ unitIndex ];
	if ( unitIndex >= 0 )
	{
		m_currentSelectedUnit = &thisUnit;
		if ( !thisUnit.IsDead() && !thisUnit.FinishedMovingThisTurn() )
		{
			m_currentSelectedUnit->m_currentUnitState = UnitState::READY;
		}
		m_currentSelectedUnit->m_currentUnitState	= UnitState::SELECTED;
		m_currentPlayer->m_requestedPlayerState = PlayerState::SELECTED;
	}
	else
	{
		// If the decremented index is negative, then loop back to the size of the list
		unitIndex									= int( ( m_currentPlayer->m_unitList.size() - 1 ) );
		if ( !thisUnit.IsDead() && !thisUnit.FinishedMovingThisTurn() )
		{
			m_currentSelectedUnit->m_currentUnitState = UnitState::READY;
		}
		m_currentSelectedUnit->m_currentUnitState	= UnitState::SELECTED;
		m_currentPlayer->m_requestedPlayerState		= PlayerState::SELECTED;
	}
	// Trigger distance field generation
	GenerateDistanceField();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::SelectNextUnit()
{
	if ( AreAllUnitsUnavailable() )
	{
		return;
	}
	bool wasIncremented = SelectFirstUnit();
	int unitIndex = 0;
	if ( m_currentSelectedUnit == nullptr )
	{
		unitIndex = 0;
//		int x = 0;
	}
	else
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Get index of currentSelectedUnit
		//----------------------------------------------------------------------------------------------------------------------
		unitIndex = GetUnitIndexFromTileCoord( m_currentSelectedUnit->m_currentTileCoord );
	}
	if ( !wasIncremented )
	{
		// Decrement the index only if did NOT get a unit
		unitIndex++;
	}

	while ( true )
	{
		if ( unitIndex >= m_currentPlayer->m_unitList.size() )
		{
			unitIndex = 0;
		}
		Unit& thisUnit = m_currentPlayer->m_unitList[ unitIndex ];
		if ( ( thisUnit.m_currentUnitState != UnitState::FINISHED_MOVING_THIS_TURN ) && 
			 ( thisUnit.m_currentUnitState != UnitState::IS_DEAD ) )
		{
			break;
		}
		// If the new unit has already finished moving this turn, decrement again!
		unitIndex++;
	}
	Unit& thisUnit = m_currentPlayer->m_unitList[ unitIndex ];
	if ( unitIndex <= (m_currentPlayer->m_unitList.size() - 1) )
	{
		m_currentSelectedUnit						= &thisUnit;
		if ( !thisUnit.IsDead() && !thisUnit.FinishedMovingThisTurn() )
		{
			m_currentSelectedUnit->m_currentUnitState = UnitState::READY;
		}
		m_currentSelectedUnit->m_currentUnitState	= UnitState::SELECTED;
		m_currentPlayer->m_requestedPlayerState		= PlayerState::SELECTED;
	}
	else
	{
		// If the decremented index is negative, then loop back to the size of the list
		unitIndex									= 0;
		m_currentSelectedUnit						= &thisUnit;
		if ( !thisUnit.IsDead() && !thisUnit.FinishedMovingThisTurn() )
		{
			m_currentSelectedUnit->m_currentUnitState = UnitState::READY;
		}
		m_currentSelectedUnit->m_currentUnitState	= UnitState::SELECTED;
		m_currentPlayer->m_requestedPlayerState		= PlayerState::SELECTED;
	}
	
	// Trigger distance field generation
	GenerateDistanceField();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::TrySelectNewUnit()
{
	Unit* currentHoveredUnit = GetCurrentHoveredUnit();
	if ( DoesUnitBelongToCurrentPlayer( m_currentSelectedUnit ) )
	{
		if ( ( currentHoveredUnit->m_currentUnitState != UnitState::FINISHED_MOVING_THIS_TURN ) &&
			( currentHoveredUnit->m_currentUnitState != UnitState::IS_DEAD ) )
		{
			if ( currentHoveredUnit != m_currentSelectedUnit )
			{
				if ( DoesUnitBelongToCurrentPlayer( currentHoveredUnit ) )
				{
					m_currentSelectedUnit->m_currentUnitState	= UnitState::READY;
					currentHoveredUnit->m_currentUnitState		= UnitState::SELECTED;
					m_currentSelectedUnit						= currentHoveredUnit;
					// Trigger distance field generation
					GenerateDistanceField();
				}
				else
				{
					// If an enemy unit is in this tile, check if it's dead, if true, we can move to this tile
					// If NOT dead, attack the enemy
					if ( currentHoveredUnit->m_currentUnitState != UnitState::IS_DEAD )
					{
						if ( currentHoveredUnit->IsWithinAttackRangeOfRefUnit( m_currentSelectedUnit ) )
						{
						//	TryAttack();
						//	g_theNetSystem->SendMessage( "TryAttack" );
							ConfirmMove();
							g_theNetSystem->SendMessage( "ConfirmMove" );
						}
					}
				}
			}

			if ( m_currentSelectedUnit->m_unitDef->m_type == "Artillery" )
			{
				// Change enemyUnit state to "targeted"
				bool isAlly = DoesUnitBelongToCurrentPlayer( currentHoveredUnit );
				if ( !isAlly )
				{
					bool isWithinAttackRange = currentHoveredUnit->IsWithinAttackRangeOfRefUnit( m_currentSelectedUnit );
					if ( isWithinAttackRange )
					{
						ResetAllEnemyUnitStates();
						m_currentPlayer->m_requestedPlayerState = PlayerState::TRY_ATTACK;
						currentHoveredUnit->m_currentUnitState	= UnitState::TARGETED_BY_ENEMY;
						m_targetedEnemyUnit						= currentHoveredUnit;
					}
				}
			}
		}
		{
			if ( currentHoveredUnit->m_currentUnitState == UnitState::IS_DEAD )
			{
				MoveToHoveredTile();
				g_theNetSystem->SendMessage( "MoveToHoveredTile" );
			}
		}
	}
}


//----------------------------------------------------------------------------------------------------------------------
bool Game::AreAllUnitsUnavailable()
{
	//----------------------------------------------------------------------------------------------------------------------
	// If all units finished moving, break out of inf loop
	//----------------------------------------------------------------------------------------------------------------------
	int numUnits = int( m_currentPlayer->m_unitList.size() );
	int unitCounter = 0;
	for ( int i = 0; i < m_currentPlayer->m_unitList.size(); i++ )
	{
		Unit& currentUnit = m_currentPlayer->m_unitList[ i ];
		if ( currentUnit.FinishedMovingThisTurn() || currentUnit.IsDead() || currentUnit.m_currentUnitState == IS_DEAD )
		{
			unitCounter++;
		}
	}
	if ( numUnits == unitCounter )
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::TryMoveOrSelectNewUnit()
{
	Unit* currentHoveredUnit = GetCurrentHoveredUnit();
	if ( currentHoveredUnit == nullptr )
	{
		MoveToHoveredTile();
		g_theNetSystem->SendMessage( "MoveToHoveredTile" );
	}
	else
	{
		TrySelectNewUnit();
		g_theNetSystem->SendMessage( "TrySelectNewUnit" );
	}
	UpdateMouseClickToCursor_UI();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::MoveSelectedUnit()
{
	if ( ( m_currentSelectedUnit == nullptr ) || ( m_hoveredHex == IntVec2( -1, -1 ) ) )
	{
//		int x = 0;
		return;
	}

	IntVec2 currentHoveredTile = m_hoveredHex;
	if ( currentHoveredTile == m_currentSelectedUnit->m_currentTileCoord )
	{
		if ( m_currentSelectedUnit->m_unitDef->m_type == "Tank" )
		{
			// Logic to check if enemies are within range,
			// True:  Go into try attack state
			// False: End turn
			bool enemiesAreInAttackRange = false;
			for ( int i = 0; i < m_enemyPlayer->m_unitList.size(); i++ )
			{
				Unit& currentEnemyUnit = m_enemyPlayer->m_unitList[ i ];
				if ( currentEnemyUnit.m_currentUnitState != UnitState::IS_DEAD )
				{
					if ( currentEnemyUnit.IsWithinAttackRangeOfRefUnit( m_currentSelectedUnit ) )
					{
						enemiesAreInAttackRange = true;
					}
				}
			}
			if ( enemiesAreInAttackRange )
			{
				// We tried to attack ourselves or our teammate so do nothing and end our turn
				m_currentPlayer->m_requestedPlayerState = PlayerState::CONFIRM_MOVE;
			}
			else
			{
				m_currentPlayer->m_requestedPlayerState   = PlayerState::CONFIRM_ATTACK;
				m_currentSelectedUnit->m_currentUnitState = UnitState::FINISHED_MOVING_THIS_TURN;
			}
		}
		else if ( m_currentSelectedUnit->m_unitDef->m_type == "Artillery" )
		{
			m_currentPlayer->m_requestedPlayerState   = PlayerState::CONFIRM_ATTACK;
			m_currentSelectedUnit->m_currentUnitState = UnitState::FINISHED_MOVING_THIS_TURN;
		}
		RemoveDistFieldAndPathRendering();
	}
	else
	{
		// Do nothing
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::ConfirmMove()
{
	if ( m_currentSelectedUnit == nullptr )
	{
//		int x = 0;
		return;
	}

	IntVec2 currentHoveredTile = m_hoveredHex;
	Unit* currentHoveredUnit   = GetCurrentHoveredUnit();
	if ( currentHoveredUnit != nullptr )
	{
		// Tank moves AND attacks
		if ( m_currentSelectedUnit->m_unitDef->m_type == "Tank" )
		{
			// If we clicked on a tile with a unit, try to attack this unit
			// if the hoveredUnit is an enemy, and the hoveredUnit is within attack range 
			bool isAlly = DoesUnitBelongToCurrentPlayer( currentHoveredUnit );
			if ( !isAlly )
			{
				// Check if currentEnemy is within attack range
				bool isWithinAttackRange = currentHoveredUnit->IsWithinAttackRangeOfRefUnit( m_currentSelectedUnit );
				if ( isWithinAttackRange )
				{
					// We found an enemy, lets move to 'Confirm attack' state
					m_currentPlayer->m_requestedPlayerState = PlayerState::TRY_ATTACK;
					// Render enemy unit bright red if clicked on 
					currentHoveredUnit->m_currentUnitState	= TARGETED_BY_ENEMY;
					m_targetedEnemyUnit						= currentHoveredUnit;
					ResetAllEnemyUnitStates();
				}
			}
			else
			{
				// We tried to attack ourselves or our teammate so do nothing and end our turn
				m_currentPlayer->m_requestedPlayerState = PlayerState::TRY_ATTACK;
			}
		}
	}
	else	// If the currentHoveredUnit is nullptr, a unit does NOT exist
	{
		// Tank moves AND attacks
		if ( m_currentSelectedUnit->m_unitDef->m_type == "Tank" )
		{
			// We tried to attack ourselves or our teammate so do nothing and end our turn
			m_currentPlayer->m_requestedPlayerState   = PlayerState::CONFIRM_ATTACK;
			m_currentSelectedUnit->m_currentUnitState = UnitState::FINISHED_MOVING_THIS_TURN;
		}
	}

	RemoveDistFieldAndPathRendering();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::TryAttack()
{
	if ( m_currentSelectedUnit == nullptr )
	{
//		int x = 0;
		return;
	}

	Unit* currentHoveredUnit = GetCurrentHoveredUnit();
	if ( currentHoveredUnit != nullptr )
	{
		bool isAlly = DoesUnitBelongToCurrentPlayer( currentHoveredUnit );
		if ( isAlly )
		{
			// We tried to attack ourselves or our teammate so do nothing and end our turn
			m_currentPlayer->m_requestedPlayerState	  = PlayerState::CONFIRM_ATTACK;
			m_currentSelectedUnit->m_currentUnitState = UnitState::FINISHED_MOVING_THIS_TURN;
		}
		else
		{
			// Write logic to deal damage
			// Highlight current hovered unit in red hex border
			m_currentPlayer->m_requestedPlayerState	  = PlayerState::CONFIRM_ATTACK;
			m_currentSelectedUnit->m_currentUnitState = UnitState::FINISHED_MOVING_THIS_TURN;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Game::ConfirmAttack()
{
	if ( m_currentSelectedUnit == nullptr )
	{
//		int x = 0;
		return;
	}

	m_currentSelectedUnit->m_previousTileCoord	= m_currentSelectedUnit->m_currentTileCoord;
	m_currentPlayer->m_requestedPlayerState		= PlayerState::SELECTING;
	if ( m_targetedEnemyUnit != nullptr )
	{
		m_targetedEnemyUnit->TakeDamage( m_currentSelectedUnit->m_unitDef->m_groundAttackDamage );
		if ( m_currentSelectedUnit->IsWithinAttackRangeOfRefUnit( m_targetedEnemyUnit ) )
		{
			m_currentSelectedUnit->TakeDamage( m_targetedEnemyUnit->m_unitDef->m_groundAttackDamage );
		}
		if ( m_targetedEnemyUnit->m_currentUnitState != UnitState::IS_DEAD )
		{
			m_targetedEnemyUnit->m_currentUnitState = UnitState::READY;
		}
		m_targetedEnemyUnit	= nullptr;
	}
	m_currentSelectedUnit = nullptr;
	RemoveDistFieldAndPathRendering();
	//----------------------------------------------------------------------------------------------------------------------
	// Loop through all enemy tank units and reset their states back to ready
	//----------------------------------------------------------------------------------------------------------------------
	ResetAllEnemyUnitStates();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::Attract_Logic()
{
	m_requestedGameState = GameState::LOBBY;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::Popup_Logic()
{
	if ( m_currentPlayer == nullptr )
	{
//		int x = 0;
		return;
	}

	m_currentPlayer->m_requestedPlayerTurnState = PlayerTurnState::END_TURN;
	m_button_playerTurn->SetIsHighlighted( false );
	if ( m_gameOverState )
	{
		m_currentPlayer->m_requestedPlayerTurnState = PlayerTurnState::POP_UP;
		m_requestedGameState	= GameState::LOBBY;
		m_currentSelectedUnit	= nullptr;
		ResetAllEnemyUnitStates();
		RemoveDistFieldAndPathRendering();
	}
}


//----------------------------------------------------------------------------------------------------------------------
void Game::EndTurn_Logic()
{
	if ( m_currentPlayer == nullptr )
	{
//		int x = 0;
		return;
	}

	m_currentPlayer->m_requestedPlayerTurnState = PlayerTurnState::CONFIRM_END_TURN;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::ConfirmEndTurn_Logic()
{
	if ( m_currentPlayer == nullptr )
	{
//		int x = 0;
		return;
	}

	m_currentPlayer->m_requestedPlayerTurnState = PlayerTurnState::WAITING_FOR_TURN;
	ToggleCurrentPlayerPointer();
	m_currentPlayer->m_requestedPlayerTurnState = PlayerTurnState::POP_UP;
	m_button_playerTurn->SetIsHighlighted( true );
	m_currentSelectedUnit = nullptr;
	RemoveDistFieldAndPathRendering();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::EscButton_Logic()
{
	if ( m_currentPlayer == nullptr )
	{
//		int x = 0;
		return;
	}

	if ( m_currentPlayer->m_currentPlayerTurnState == PlayerTurnState::CONFIRM_END_TURN )
	{
		m_currentPlayer->m_requestedPlayerTurnState = PlayerTurnState::END_TURN;
		MoveSelectedUnitToPrevTile();
		g_theNetSystem->SendMessage( "MoveSelectedUnitToPrevTile" ); 
		return;
	}

	if ( m_currentSelectedUnit != nullptr )
	{
		m_currentPlayer->m_requestedPlayerState		= PlayerState::SELECTING;
		m_currentSelectedUnit->m_currentUnitState	= UnitState::READY;
		if ( m_currentPlayer->m_currentPlayerState == PlayerState::SELECTED		|| 
			 m_currentPlayer->m_currentPlayerState == PlayerState::MOVING		||
			 m_currentPlayer->m_currentPlayerState == PlayerState::CONFIRM_MOVE	||
			 m_currentPlayer->m_currentPlayerState == PlayerState::TRY_ATTACK	)
		{
			MoveSelectedUnitToPrevTile();
		}
		m_currentSelectedUnit = nullptr;
	}
	ResetAllEnemyUnitStates();
	RemoveDistFieldAndPathRendering();
}


//----------------------------------------------------------------------------------------------------------------------
void Game::ResetRemoteGameOverStates()
{
	m_isGameOver	= false;
	m_gameOverState = NONE;
	InitializeLocalPlayerReady();
//	m_localPlayerIsReady  = false;
//	m_remotePlayerIsReady = false;
}


//----------------------------------------------------------------------------------------------------------------------
void Game::Render_DRS_UI_Text() const
{
	// Initialize and set UI variables
	float cellHeight = m_clientDimensions.x * 0.0115f;
	float duration							= 0.0f;
	Vec2 camPos								= Vec2( 0.0f, m_clientDimensions.y );
	Vec2 camYPR								= Vec2( 0.0f, m_clientDimensions.y - ( cellHeight * 1.1f ) );
	Vec2 playerPos							= Vec2( 0.0f, m_clientDimensions.y - ( cellHeight * 2.1f ) );
	Vec2 hexCenterPos						= Vec2( 0.0f, m_clientDimensions.y - ( cellHeight * 3.1f ) );
	Vec2 currentPlayerStatePos				= Vec2( 0.0f, m_clientDimensions.y - ( cellHeight * 4.1f ) );
	Vec2 HexGridCoordPos					= Vec2( 0.0f, m_clientDimensions.y - ( cellHeight * 5.1f ) );
	Vec2 currentPlayerTurnStatePos			= Vec2( 0.0f, m_clientDimensions.y - ( cellHeight * 6.1f ) );
	Vec2 cursorTextPos						= Vec2( 0.0f, m_clientDimensions.y - ( cellHeight * 7.1f ) );
	Vec2 playerAlignment					= Vec2( 0.0f, 1.0f );
	Vec2 timePosition						= Vec2( m_clientDimensions.x, ( m_clientDimensions.y ) );
	Vec2 timeAlignment						= Vec2( 1.0f, 1.0f );
	float fps								= 1.0f / m_clock.GetDeltaSeconds();
	std::string camPosText					= Stringf(	  "Camera position:        %0.2f, %0.2f, %0.2f",				  m_currentPlayer->m_worldCamera.m_position.x,				   m_currentPlayer->m_worldCamera.m_position.y,				   m_currentPlayer->m_worldCamera.m_position.z );
	std::string playerYPRText				= Stringf(	  "Camera YPR:             %0.2f, %0.2f, %0.2f",	m_currentPlayer->m_worldCamera.m_orientation.m_yawDegrees, m_currentPlayer->m_worldCamera.m_orientation.m_pitchDegrees, m_currentPlayer->m_worldCamera.m_orientation.m_rollDegrees );
	std::string screenCenterPosText			= Stringf(	  "ScreenCenter pos:       %0.2f, %0.2f, %0.2f",				  m_currentPlayer->m_worldCenterPosOnScreen.x,				   m_currentPlayer->m_worldCenterPosOnScreen.y,				   m_currentPlayer->m_worldCenterPosOnScreen.z );
	std::string hexCenterPosText			= Stringf(	  "HexCenterPos:           %0.2f, %0.2f, %0.2f",					m_currentMap->m_highlightedHexCenterPos.x,					 m_currentMap->m_highlightedHexCenterPos.y,					 m_currentMap->m_highlightedHexCenterPos.z );
	std::string currentPlayerStateText		= Stringf(	  "currentPlayerState:     %s",								 m_currentPlayer->GetPlayerStateAsString().c_str() );
	IntVec2		highlightedHex				= m_currentMap->GetHexCoordsFromCursorPos( m_currentCursorPos );
//	std::string hexGridCoordText			= Stringf(	  "HexGridCoord:           %0.2d, %0.2d", highlightedHex.x, highlightedHex.y );
	std::string hexGridCoordText			= Stringf(	  "HexGridCoord:           %0.2d, %0.2d", m_hoveredHex.x, m_hoveredHex.y );
	std::string currentPlayerTurnStateText	= Stringf(	  "currentPlayerTurnState: %s", m_currentPlayer->GetPlayerTurnStateAsString().c_str() );
	
	Vec2 cursorPos  = g_theInput->GetCursorClientPosition();
	cursorPos.y		= m_clientDimensions.y - cursorPos.y;		// Negate or Flips cursor Y 

//	std::string cursorPosText				= Stringf(	  "CursorPos:              %0.2f, %0.2f", cursorPos.x, cursorPos.y );
	std::string cursorPosText				= Stringf(	  "CursorPos:              %0.2f, %0.2f", m_currentCursorPos.x, m_currentCursorPos.y );
//	std::string timeText					= Stringf( "Time: %0.2f. FPS: %0.2f", m_clock.GetTotalSeconds(), fps );
	std::string timeText					= Stringf( "FPS: %0.2f", fps );
	
	// Render DRS UI text
	DebugAddScreenText(		   			camPosText,				       camPos, cellHeight, playerAlignment, duration );
	DebugAddScreenText(		   		 playerYPRText,				       camYPR, cellHeight, playerAlignment, duration );
	DebugAddScreenText(		   screenCenterPosText,				    playerPos, cellHeight, playerAlignment, duration );
	DebugAddScreenText(		      hexCenterPosText,				 hexCenterPos, cellHeight, playerAlignment, duration );
	DebugAddScreenText(		currentPlayerStateText,		currentPlayerStatePos, cellHeight, playerAlignment, duration );
	DebugAddScreenText(			  hexGridCoordText,			  HexGridCoordPos, cellHeight, playerAlignment, duration );
	DebugAddScreenText( currentPlayerTurnStateText,	currentPlayerTurnStatePos, cellHeight, playerAlignment, duration );
	DebugAddScreenText(				 cursorPosText,				cursorTextPos, cellHeight, playerAlignment, duration );
	DebugAddScreenText(					  timeText,			     timePosition, cellHeight,   timeAlignment, duration );

	//----------------------------------------------------------------------------------------------------------------------
	// ScreenCenter reticle
	//----------------------------------------------------------------------------------------------------------------------
	Vec2 screenCenter  = Vec2( m_clientDimensions.x * 0.5f, m_clientDimensions.y * 0.5f );
	float cellHeight2  = m_clientDimensions.y * 0.005f;
	DebugAddScreenText(						   "0",				 screenCenter, cellHeight2, timeAlignment, duration, Rgba8::MAGENTA, Rgba8::MAGENTA );

/*
	// Hack for debugging "convertCursorPosToHexCoords"
	highlightedHex			= m_currentMap->GetTileCoordsForWorldPos( screenCenter );
	hexGridCoordText		= Stringf( "HexGridCoord:           %0.2d, %0.2d", highlightedHex.x, highlightedHex.y );
	DebugAddScreenText(	hexGridCoordText, HexGridCoordPos, cellHeight, playerAlignment, duration );
*/
}


//----------------------------------------------------------------------------------------------------------------------
void Game::RenderPopUpUI() const
{
	//----------------------------------------------------------------------------------------------------------------------
	// Render pop up UI
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> vertsUI;
	float cellAspect = 0.5f;
	if ( m_button_playerTurn->GetIsHighlighted() )
	{
		// Instruction Pop up
		float cellHeight = m_button_playerTurn->m_bounds.GetDimensions().x * 0.07f;
		AddVertsForAABB2D( vertsUI, m_button_playerTurn->m_bounds, Rgba8::MAGENTA );
		AddVertsForBordersAABB2D( vertsUI, m_button_playerTurn->m_bounds, 10.0f );
		//----------------------------------------------------------------------------------------------------------------------
		// Render game over state
		//----------------------------------------------------------------------------------------------------------------------
		bool arePlayersReady	= ArePlayersReady();
		bool isNetworking		= g_theNetSystem->IsNetworking();
		if ( m_isGameOver )
		{
			if ( m_gameOverState == GameOverState::PLAYER1_WINS )
			{
				// Player 1 wins
				g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_playerTurn->m_bounds, cellHeight, m_button_playerTurn->m_text3, Rgba8::WHITE, cellAspect, Vec2( 0.5f, 0.8f ) );
			}
			if ( m_gameOverState == GameOverState::PLAYER2_WINS )
			{
				// Player 2 wins
				g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_playerTurn->m_bounds, cellHeight, m_button_playerTurn->m_text4, Rgba8::WHITE, cellAspect, Vec2( 0.5f, 0.8f ) );
			}
			if ( m_gameOverState == GameOverState::MATCH_DRAW )
			{
				// Match Tied
				g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_playerTurn->m_bounds, cellHeight, m_button_playerTurn->m_text5, Rgba8::WHITE, cellAspect, Vec2( 0.5f, 0.8f ) );
			}
		}
		else if ( isNetworking && !arePlayersReady )
		{
			g_theRenderer->ClearScreen( Rgba8::RED );
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_playerTurn->m_bounds, cellHeight, m_button_playerTurn->m_text6, Rgba8::WHITE, cellAspect, Vec2( 0.5f, 0.8f ) );
		}
		else if ( m_currentPlayer == m_player1 )
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_playerTurn->m_bounds, cellHeight,	m_button_playerTurn->m_text1, Rgba8::WHITE, cellAspect, Vec2( 0.5f, 0.8f ) );
		}
		else // if currentPlayer is player2
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_playerTurn->m_bounds, cellHeight, m_button_playerTurn->m_text2, Rgba8::WHITE, cellAspect, Vec2( 0.5f, 0.8f ) );
		}
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_playerTurn->m_bounds, cellHeight * 0.49f, "Press ENTER or click to continue", Rgba8::WHITE, cellAspect, Vec2( 0.5f, 0.3f ) );
	}
	else
	{
		//----------------------------------------------------------------------------------------------------------------------
		// UI buttons at the bottom of the game screen
		//----------------------------------------------------------------------------------------------------------------------
		float cellHeight		= m_button_mouseSelect->m_bounds.GetDimensions().x * 0.05f;
		float borderThickness	= cellHeight * 0.5f;
		// [Mouse] Select
		AddVertsForAABB2D( vertsUI, m_button_mouseSelect->m_bounds, Rgba8::DARK_BLUE );
		AddVertsForBordersAABB2D( vertsUI, m_button_mouseSelect->m_bounds, borderThickness );
		// The text previews the "next move"
		std::string spaceText = " ";
		if ( m_currentPlayer->m_currentPlayerState == PlayerState::SELECTING )
		{
			// "Select"
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_mouseSelect->m_bounds, cellHeight, m_button_mouseSelect->m_text1  + spaceText + m_button_mouseSelect->m_text2, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );	
		}
		else if ( m_currentPlayer->m_currentPlayerState == PlayerState::SELECTED )
		{
			// "Move"
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_mouseSelect->m_bounds, cellHeight, m_button_mouseSelect->m_text1 + spaceText + m_button_mouseSelect->m_text3, Rgba8::WHITE, 1.0f, Vec2(0.1f, 0.5f));
		}
		else if ( m_currentPlayer->m_currentPlayerState == PlayerState::MOVING )
		{
			// "Confirm Move"
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_mouseSelect->m_bounds, cellHeight,  m_button_mouseSelect->m_text1 + spaceText +  m_button_mouseSelect->m_text4, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		}
		else if ( m_currentPlayer->m_currentPlayerState == PlayerState::CONFIRM_MOVE )
		{
			// "Attack / Done"
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_mouseSelect->m_bounds, cellHeight,  m_button_mouseSelect->m_text1 + spaceText + m_button_mouseSelect->m_text5, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		}
		else if ( m_currentPlayer->m_currentPlayerState == PlayerState::TRY_ATTACK )
		{
			// "Confirm Attack / Done"
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_mouseSelect->m_bounds, cellHeight,  m_button_mouseSelect->m_text1 + spaceText + m_button_mouseSelect->m_text6, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		}
		// [Left] previous unit
		AddVertsForAABB2D( vertsUI, m_button_previousUnit->m_bounds, Rgba8::DARK_BLUE );
		AddVertsForBordersAABB2D( vertsUI, m_button_previousUnit->m_bounds, borderThickness );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_previousUnit->m_bounds, cellHeight, m_button_previousUnit->m_text1, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		// [Right] next unit
		AddVertsForAABB2D( vertsUI, m_button_nextUnit->m_bounds, Rgba8::DARK_BLUE );
		AddVertsForBordersAABB2D( vertsUI, m_button_nextUnit->m_bounds, borderThickness );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_nextUnit->m_bounds, cellHeight, m_button_nextUnit->m_text1, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		// [Escape] Cancel
		AddVertsForAABB2D( vertsUI, m_button_escape->m_bounds, Rgba8::DARK_BLUE );
		AddVertsForBordersAABB2D( vertsUI, m_button_escape->m_bounds, borderThickness );
		if ( m_currentPlayer->m_currentPlayerState == PlayerState::SELECTING )
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_escape->m_bounds, cellHeight, m_button_escape->m_text1, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		}
		else
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_escape->m_bounds, cellHeight, m_button_escape->m_text2, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		}
		// [Enter] End turn
		AddVertsForAABB2D( vertsUI, m_button_enter->m_bounds, Rgba8::DARK_BLUE );
		AddVertsForBordersAABB2D( vertsUI, m_button_enter->m_bounds, borderThickness );
		if ( m_currentPlayer->m_currentPlayerTurnState == PlayerTurnState::CONFIRM_END_TURN )
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_enter->m_bounds, cellHeight, m_button_enter->m_text2, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		}
		else
		{
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_enter->m_bounds, cellHeight, m_button_enter->m_text1, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		}
		//----------------------------------------------------------------------------------------------------------------------
		// Tank Unit Stat buttons
		//----------------------------------------------------------------------------------------------------------------------
		// Health
		AddVertsForAABB2D( vertsUI, m_button_health->m_bounds, Rgba8::DARK_BLUE );
		AddVertsForBordersAABB2D( vertsUI, m_button_health->m_bounds, borderThickness );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts,  m_button_health->m_bounds, cellHeight,  m_button_health->m_text1, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		// Move
		AddVertsForAABB2D( vertsUI, m_button_move->m_bounds, Rgba8::DARK_BLUE );
		AddVertsForBordersAABB2D( vertsUI, m_button_move->m_bounds, borderThickness );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts,    m_button_move->m_bounds, cellHeight,    m_button_move->m_text1, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		// Range
		AddVertsForAABB2D( vertsUI, m_button_range->m_bounds, Rgba8::DARK_BLUE );
		AddVertsForBordersAABB2D( vertsUI, m_button_range->m_bounds, borderThickness );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts,   m_button_range->m_bounds, cellHeight,   m_button_range->m_text1, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		// Defense
		AddVertsForAABB2D( vertsUI, m_button_defense->m_bounds, Rgba8::DARK_BLUE );
		AddVertsForBordersAABB2D( vertsUI, m_button_defense->m_bounds, borderThickness );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_defense->m_bounds, cellHeight, m_button_defense->m_text1, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		// Attack
		AddVertsForAABB2D( vertsUI, m_button_attack->m_bounds, Rgba8::DARK_BLUE );
		AddVertsForBordersAABB2D( vertsUI, m_button_attack->m_bounds, borderThickness );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts,  m_button_attack->m_bounds, cellHeight,  m_button_attack->m_text1, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		// Name
		AddVertsForAABB2D( vertsUI, m_button_name->m_bounds, Rgba8::DARK_BLUE );
		AddVertsForBordersAABB2D( vertsUI, m_button_name->m_bounds, borderThickness );
		g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts,    m_button_name->m_bounds, cellHeight,    m_button_name->m_text1, Rgba8::WHITE, 1.0f, Vec2( 0.1f, 0.5f ) );
		if ( ( m_HoveredUnitDef != nullptr ) && ( m_currentHoveredUnit != nullptr ) && ( m_currentHoveredUnit->m_currentUnitState != UnitState::IS_DEAD ) )
		{
			// Health
			m_button_health->m_text2 = std::to_string( m_currentHoveredUnit->m_currentHealth );
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts,  m_button_health->m_bounds, cellHeight,   m_button_health->m_text2, Rgba8::WHITE, 1.0f, Vec2( 0.9f, 0.5f ) );
			// Move																							  
			m_button_move->m_text2 = std::to_string( m_HoveredUnitDef->m_movementRange );
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts,    m_button_move->m_bounds, cellHeight,     m_button_move->m_text2, Rgba8::WHITE, 1.0f, Vec2( 0.9f, 0.5f ) );
			// Range	
			m_button_range->m_text2 = std::to_string( m_HoveredUnitDef->m_groundAttackRangeMin ) + "  -";
			m_button_range->m_text3 = std::to_string( m_HoveredUnitDef->m_groundAttackRangeMax );
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts,   m_button_range->m_bounds, cellHeight,    m_button_range->m_text2, Rgba8::WHITE, 1.0f, Vec2( 0.7f, 0.5f ) );
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts,   m_button_range->m_bounds, cellHeight,    m_button_range->m_text3, Rgba8::WHITE, 1.0f, Vec2( 0.9f, 0.5f ) );
			// Defense																						 
			m_button_defense->m_text2 = std::to_string( m_HoveredUnitDef->m_defense );
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts, m_button_defense->m_bounds, cellHeight,  m_button_defense->m_text2, Rgba8::WHITE, 1.0f, Vec2( 0.9f, 0.5f ) );
			// Attack
			m_button_attack->m_text2 = std::to_string( m_HoveredUnitDef->m_groundAttackDamage );
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts,  m_button_attack->m_bounds, cellHeight,   m_button_attack->m_text2, Rgba8::WHITE, 1.0f, Vec2( 0.9f, 0.5f ) );
			// Name																							
			m_button_name->m_text2 = m_HoveredUnitDef->m_name;
			g_theApp->m_textFont->AddVertsForTextInBox2D( textVerts,    m_button_name->m_bounds, cellHeight,     m_button_name->m_text2, Rgba8::WHITE, 1.0f, Vec2( 0.9f, 0.5f ) );
		}
		else
		{
			// Clear strings when not hovering over units
			m_button_health->m_text2	= "";
			m_button_move->m_text2		= "";
			m_button_range->m_text2		= "";
			m_button_range->m_text3		= "";
			m_button_defense->m_text2	= "";
			m_button_attack->m_text2	= "";
			m_button_name->m_text2		= "";
		}
	}


	//----------------------------------------------------------------------------------------------------------------------
	// Render nearestPoint to Hud UI buttons for debugging
	//----------------------------------------------------------------------------------------------------------------------
	if ( m_button_previousUnit != nullptr && m_button_nextUnit != nullptr )
	{
		Vec2 nearestPoint_previous	= m_button_previousUnit->m_bounds.GetNearestPoint( m_currentCursorPos );
		Vec2 nearestPoint_next		= m_button_nextUnit->m_bounds.GetNearestPoint( m_currentCursorPos );
		AddVertsForDisc2D( vertsUI, nearestPoint_previous, 10.0f, Rgba8::YELLOW		);
		AddVertsForDisc2D( vertsUI, nearestPoint_next	 , 10.0f, Rgba8::MAGENTA	);
	}
	if ( m_button_escape != nullptr && m_button_enter != nullptr )
	{
		Vec2 nearestPoint_esc		= m_button_escape->m_bounds.GetNearestPoint( m_currentCursorPos );
		Vec2 nearestPoint_enter		= m_button_enter->m_bounds.GetNearestPoint( m_currentCursorPos );
		AddVertsForDisc2D( vertsUI, nearestPoint_esc,   10.0f, Rgba8::YELLOW );
		AddVertsForDisc2D( vertsUI, nearestPoint_enter, 10.0f, Rgba8::MAGENTA );
	}


	// vertsUI Draw Call
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->DrawVertexArray( int( vertsUI.size() ), vertsUI.data() );

	// Text Draw Call
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture( &g_theApp->m_textFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->DrawVertexArray( int( textVerts.size() ), textVerts.data() );
	g_theRenderer->BindTexture( nullptr );
}
