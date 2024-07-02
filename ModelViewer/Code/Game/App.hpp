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

	static bool Quit( EventArgs& args );

	void LoadGameConfig( std::string configFilePath );
	static bool Command_LoadGameConfig( EventArgs& args );
	void LoadAdditionalGameConfig( std::string const& commandLineArg );


	Clock	m_gameClock;
private:
	void BeginFrame();
	void Update();				// updates entities' position
	void Render() const;		// draws entities' every frame
	void EndFrame();

private:
	bool		m_isQuitting		= false;
	Game*		m_theGame			= nullptr;
	Camera		m_devConsoleCamera;

public:
	std::string m_commandLineArg;
};