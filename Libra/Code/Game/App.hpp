#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/NamedStrings.hpp"

//------------------------------------------------------------------------------------------------------------------------
class Game;

typedef NamedStrings EventArgs;

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
	void HardRestartDevCheat();

	static bool Quit( EventArgs& args );
	static bool TestFuncForEventSystem( EventArgs& eventArgs );

private:
	void BeginFrame();
	void Update(float deltaSeconds);	// updates entities' position
	void Render() const;				// draws entities' every frame
	void EndFrame();

private:
	bool	m_isQuitting	= false;
	Game*	m_theGame		= nullptr;
	Camera	m_devConsoleCamera;
};