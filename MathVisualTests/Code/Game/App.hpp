#pragma once

#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EventSystem.hpp"

//------------------------------------------------------------------------------------------------------------------------
class GameModeBase;

//----------------------------------------------------------------------------------------------------------------------
constexpr int NUM_SINGLE_TRI_VERTS = 3;

//----------------------------------------------------------------------------------------------------------------------
class App
{
public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void Run();
	void RunFrame();
	 
	bool IsQuitting() const { return m_isQuitting; }
	bool HandleKeyPressed(unsigned char keyCode);	
	bool HandleKeyReleased(unsigned char keyCode);
	bool HandleQuitRequested();

	//----------------------------------------------------------------------------------------------------------------------
	// AttractMode functions
	void UpdateAttractModeCam();
	void UpdateReturnToAttractMode();
	void AttractModeInput();
	void RenderAttractMode() const;
	void RenderGameModeMenu() const;

	static bool Quit( EventArgs& args );

public:
	Camera			m_attractCamera;
	BitmapFont*		m_textFont			= nullptr;

private:
	void BeginFrame();
	void Update(float deltaSeconds);	// updates entities' position
	void Render() const;				// draws entities' every frame
	void EndFrame();

private:
	bool			m_attractModeIsOn	= true;
	bool			m_isQuitting		= false;
	GameModeBase*	m_theGameMode		= nullptr;
	Camera			m_devConsoleCamera;
};