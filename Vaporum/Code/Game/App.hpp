#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/IntVec2.hpp"

//------------------------------------------------------------------------------------------------------------------------
class Game; 
class BitmapFont;

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
	void LoadNetworkingConfig( std::string configFilePath );
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
	BitmapFont* m_textFont			= nullptr;

	std::string m_commandLineArg;

	// Game config data
	float		m_windowAspect			= 0.0f;
	bool		m_windowFullscreen		= false;
	Vec3		m_cameraStartPosition	= Vec3::ZERO;
	Vec3		m_cameraFixedAngle		= Vec3::ZERO;
	float		m_cameraPanSpeed		= 0.0f;
	float		m_cameraElevateSpeed	= 0.0f;
	float		m_cameraMinHeight		= 0.0f;
	float		m_cameraFOVDegrees		= 0.0f;
	float		m_cameraNearClip		= 0.0f;
	float		m_cameraFarClip			= 0.0f;
	std::string m_defaultMap			= "Un-initialized String";

	// Client / Server Config data
	std::string		m_netMode			= "Invalid Netmode string";
	IntVec2			m_windowSize		= IntVec2( -1, -1 );
	IntVec2			m_windowPosition	= IntVec2( -1, -1 );
	std::string		m_windowTitle		= "Invalid title string";
	std::string		m_netHostAddress	= "Invalid netHostAddress";
	int				m_recvBufferSize	= -1;
	int				m_sendBufferSize	= -1;
};