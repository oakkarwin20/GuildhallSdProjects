#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"

//------------------------------------------------------------------------------------------------------------------------
class PlayerShip;
class Game;

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

	static bool SetGameTimeScale( EventArgs& args );
	static bool Quit( EventArgs& args );

	void BeginFrame();
	void Update();	//changes ship's position
	void Render() const;				// draws ship every frame
	void EndFrame();

	void hardRestartDevCheat();

	bool m_isQuitting = false;

	Game*	m_theGame			= nullptr;
	Camera	m_devConsoleCamera;
};