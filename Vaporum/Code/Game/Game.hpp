#pragma once

#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/Player.hpp"
#include "Game/Tile.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/EventSystem.hpp"

#include <queue>


//----------------------------------------------------------------------------------------------------------------------
enum class GameState
{
	NONE,
	ATTRACT,
	PLAYING,
	LOBBY,
	PAUSED,
	COUNT
};


//----------------------------------------------------------------------------------------------------------------------
enum GameOverState
{
	NONE,
	PLAYER1_WINS,
	PLAYER2_WINS,
	MATCH_DRAW,
};


//----------------------------------------------------------------------------------------------------------------------
class Map;
class Button;


//----------------------------------------------------------------------------------------------------------------------
class queueCompare
{
public:
	int operator() ( Tile p1, Tile p2 )
	{
		return p1.m_hexDistToSelectedUnit > p2.m_hexDistToSelectedUnit;
	}
};


//----------------------------------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void StartUp();
	void Shutdown();
	void Update();
	void Render() const;

	// Console commands
	static bool Command_LoadModel(	EventArgs& args );
	static bool Command_LoadMap(	EventArgs& args );
	// Game Console Commands 
	static bool Command_SelectLastUnit					( EventArgs& args );
	static bool Command_SelectFirstUnit					( EventArgs& args );
	static bool Command_SelectPreviousUnit				( EventArgs& args );
	static bool Command_SelectNextUnit					( EventArgs& args );
	static bool Command_MoveToHoveredTile				( EventArgs& args );
	static bool Command_MoveSelectedUnit				( EventArgs& args );
	static bool Command_SelectHoveredUnit				( EventArgs& args );
	static bool Command_RecvCursorPos					( EventArgs& args );
	static bool Command_RecvHexPos						( EventArgs& args );
	static bool Command_MoveSelectedUnitToPrevTile		( EventArgs& args );
	static bool Command_TryAttack						( EventArgs& args );
	static bool Command_TrySelectNewUnit			 	( EventArgs& args );
	static bool Command_ConfirmMove						( EventArgs& args );
	static bool Command_EscButton_Logic					( EventArgs& args );
	static bool Command_ConfirmAttack					( EventArgs& args );
	static bool Command_EndTurn_Logic					( EventArgs& args );
	static bool Command_ConfirmEndTurn_Logic			( EventArgs& args );
	static bool Command_LocalPlayerReady				( EventArgs& args );
	static bool Command_Popup_Logic						( EventArgs& args );
	static bool Command_GenerateDistPath				( EventArgs& args );
	static bool Command_MatchDraw				 		( EventArgs& args );
	static bool Command_ResetRemoteGameOverStates		( EventArgs& args );


	// Player Functions
	Player* GetPointerToEnemyPlayer();
	void	InitLocalRemotePlayerRefs();
	bool	ArePlayersReady() const;

	// Local Cursor Pos
	void UpdatePrevCursorPosToCurrent();
	void UpdateCursorPos();

private:
	void UpdatePauseQuitAndSlowMo();
	void UpdateEntities( float deltaSeconds );
	void InitializeCameras();
	void RenderEntities() const;
	void RenderUI() const;

	// DebugRenderSystem aka DRS
	void Render_DRS_WorldBasisText();
	void Render_DRS_WorldBasis();
	void Render_DRS_UI_Text() const;
	void RenderPopUpUI() const;

	// AttractMode
	void UpdateGameStatesInput();
	void RenderAttractScreen() const;
	void UpdateInput_Enter();

	// Model Viewer
	void LoadModel();

	// Player Functions
	void InitializePlayers();
	void ToggleCurrentPlayerPointer();
	void InitializeLocalPlayerReady();

	// Player State Functions
	void UpdatePlayerStatesInput();
	

	// Distance Field
	void				GenerateDistanceField();
	void				GenerateDistancePath();
	std::vector<Tile>	GetFreshValidNeighborTileList( Tile currentTile );
	std::vector<Tile>	GetVisitedValidNeighborTileList( Tile currentTile );
	bool				IsWithinTaxiCabDistance( IntVec2 tileCoordsA, IntVec2 tileCoordB );
	void				GetRidOfDistanceFieldRendering();
public:
	void				RemoveDistFieldAndPathRendering();

private:
	// Unit Functions
	void	RenderEnemiesInAttackRange() const;
	Unit*	GetCurrentHoveredUnit();
	bool	IsUnitMovable( Unit const* currentHoveredUnit );
	bool	DoesUnitBelongToCurrentPlayer( Unit const* currentHoveredUnit );
	int		GetUnitIndexFromTileCoord( IntVec2 const& m_currentTileCoord );
	void	SetEnemiesUnitsInAttackRange_WithinAttackRangeUnitState();
	void	ResetAllEnemyUnitStates();
	void	SelectPreviousUnit_Left_UnitStateSelected();
	void	SelectNextUnit_RightUnitStateSelected();
	void	UpdateHoveredUnitStats();
	void	UpdatAllUnitsDeath();


	// Game state Functions
	void EnterState( GameState state );
	void ExitState( GameState state );
	void MatchDraw();
	void CheckGameOver();
	bool IsMyTurn();

	// Mouse input
	void UpdateInputUI();

	// Button Util Functions
	void InitializeButtons();
	void ToggleHighlightsLobbyButtons();
	void ToggleHighlightsPausedMenuButtons();

	// Networking State util functions
	void RecvUpdatedCursorPos( Vec2 const& newCursorPos );
	void SendCursorPos();
	void RecvUpdatedHexPos( IntVec2 const& newHexPos );
	void SendHexPos();
	void SelectHoveredUnit();
	void ResetMouseClick_UI();
	void UpdateMouseClickToCursor_UI();
	bool SelectLastUnit();
	bool SelectFirstUnit();
	void SelectPreviousUnit();
	void SelectNextUnit();
	void TrySelectNewUnit();
	bool AreAllUnitsUnavailable();
	void TryMoveOrSelectNewUnit();
	void MoveToHoveredTile();
	void UpdateHoveredAndPrevHex();
	void MoveSelectedUnitToPrevTile();
	// Core State Functions
	void MoveSelectedUnit();
	void ConfirmMove();
	void TryAttack();
	void ConfirmAttack();
	// Game States
	void Attract_Logic();
	void Popup_Logic();
	void EndTurn_Logic();
	void ConfirmEndTurn_Logic();
	void EscButton_Logic();
	void ResetRemoteGameOverStates();


private:
	bool m_isPaused			= false;
	bool m_isSlowMo			= false;

	Camera						m_attractCamera;
	Camera						m_screenCamera;
	std::vector<Entity*>		m_entityList;

	// Game state Variables
	GameState				m_requestedGameState	= GameState::ATTRACT;
	GameState				m_currentGameState		= GameState::ATTRACT;
	GameState				m_previousGameState		= GameState::ATTRACT;

public:
	Player*					m_currentPlayer			= nullptr;
	Player*					m_player1				= nullptr;
	Player*					m_player2				= nullptr;
	Player*					m_enemyPlayer			= nullptr;
	Texture*				m_logoTexture			= nullptr;
	Clock					m_clock;
	std::string				m_playerPosText; 
	std::string				m_timeText; 
	Map*					m_currentMap			= nullptr;
	Vec2					m_clientDimensions		= Vec2::ZERO;
	BitmapFont*				m_mapTexture			= nullptr;
	IntVec2					m_hoveredHex			= IntVec2( -1, -1 );			// Hex coord in Hex Space
	IntVec2					m_prevHoveredHex		= IntVec2( -1, -1 );
	Player*					m_localPlayer			= m_player1;
	Player*					m_remotePlayer			= m_player2;
	bool					m_localPlayerIsReady	= false;
	bool					m_remotePlayerIsReady	= false;

	//----------------------------------------------------------------------------------------------------------------------
	// Cursor
	//----------------------------------------------------------------------------------------------------------------------
	Vec2 m_currentCursorPos		= Vec2::ZERO;
	Vec2 m_previousCursorPos	= Vec2::ZERO;

	//----------------------------------------------------------------------------------------------------------------------
	// Buttons
	//----------------------------------------------------------------------------------------------------------------------
	// Lobby
	Button*					m_button_localGame		= nullptr;
	Button*					m_button_quit			= nullptr;
	// Pause Menu
	Button*					m_button_resume			= nullptr;
	Button*					m_button_mainMenu		= nullptr;
	// Game
	Button*					m_button_playerTurn		= nullptr;
	Button*					m_button_mouseSelect	= nullptr;
	Button*					m_button_previousUnit	= nullptr;
	Button*					m_button_nextUnit		= nullptr;
	Button*					m_button_escape			= nullptr;
	Button*					m_button_enter			= nullptr;
	Button*					m_button_health			= nullptr;
	Button*					m_button_move			= nullptr;
	Button*					m_button_range			= nullptr;
	Button*					m_button_defense		= nullptr;
	Button*					m_button_attack			= nullptr;
	Button*					m_button_name			= nullptr;
	std::vector<Button*>	m_lobbyButtonList;
	std::vector<Button*>	m_pausedButtonList;

	//----------------------------------------------------------------------------------------------------------------------
	// Mouse Clicks for UI
	//----------------------------------------------------------------------------------------------------------------------
	bool m_wasMouseClickedHovered_Previous	= false;
	bool m_wasMouseClickedHovered_Next		= false;


	//----------------------------------------------------------------------------------------------------------------------
	// Rendering stats for hovered unit
	//----------------------------------------------------------------------------------------------------------------------
	UnitDefinition const* m_HoveredUnitDef = nullptr;
	// Unit variables
	Unit*	m_currentSelectedUnit	= nullptr;
	Unit*	m_targetedEnemyUnit		= nullptr;
	Unit*	m_currentHoveredUnit	= nullptr;

	//----------------------------------------------------------------------------------------------------------------------
	// Distance Field
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<Tile>													m_distPathTileList;
	std::vector<Tile>													m_distanceFieldTileList;
	std::priority_queue< Tile, std::vector<Tile>, queueCompare >		m_pQueue;

	// Game over state
	bool			m_isGameOver	= false;
	GameOverState	m_gameOverState = GameOverState::NONE;
};
