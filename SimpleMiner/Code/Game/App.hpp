#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"

//------------------------------------------------------------------------------------------------------------------------
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

	void LoadGameConfig( std::string filepath );

	static bool Quit( EventArgs& args );

	Clock	m_gameClock;
private:
	void BeginFrame();
	void Update();				// updates entities' position
	void Render() const;		// draws entities' every frame
	void EndFrame();

private:
	bool	m_isQuitting		= false;
	Camera	m_devConsoleCamera;
};