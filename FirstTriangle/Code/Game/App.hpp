#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/VertexUtils.hpp"

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

private:
	void BeginFrame();
	void Update(float deltaSeconds);	// updates entities' position
	void Render() const;				// draws entities' every frame
	void EndFrame();

private:
	bool m_isQuitting = false;

	std::vector<uint8_t> m_vertexShaderByteCode;
	std::vector<uint8_t> m_pixelShaderByteCode;
};