#include "Game/App.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <Windows.h>			// #include this (massive, platform-specific) header in very few places
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include "GameCommon.hpp"
#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Move each of these items to its proper place, once that place is established
// 
// HWND g_hWnd = nullptr;								// ...becomes Window::m_windowHandle
// HDC g_displayDeviceContext = nullptr;				// ...becomes Window::m_displayContext
// HGLRC g_openGLRenderingContext = nullptr;			// ...becomes Renderer::m_apiRenderingContext
// const char* APP_NAME = "SD1-A2: Starship Prototype";	// ...becomes ??? (Change this per project!)
extern App* g_theApp;
   
//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED(applicationInstanceHandle);

	UNUSED(commandLineString);

	g_theApp = new App();
	g_theApp->Startup();
	g_theApp->Run();
	g_theApp->Shutdown();
	delete g_theApp;
	g_theApp = nullptr;

	return 0;
}


