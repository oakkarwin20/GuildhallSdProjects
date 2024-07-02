#include "Game/App.hpp"
#include "Game/EngineBuildPreferences.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Window/Window.hpp"

#define  WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

#if defined( ENGINE_DEBUG_RENDER )
	#include <dxgidebug.h>
	#pragma comment( lib, "dxguid.lib" )
#endif

//----------------------------------------------------------------------------------------------------------------------
ID3D11Device*				m_device					= nullptr;
ID3D11DeviceContext*		m_deviceContext				= nullptr;
IDXGISwapChain*				m_swapChain					= nullptr;
ID3D11RenderTargetView*		m_renderTargetView			= nullptr;

ID3D11VertexShader*			m_vertexShader				= nullptr;
ID3D11PixelShader*			m_pixelShader				= nullptr;
ID3D11InputLayout*			m_inputLayout				= nullptr;
ID3D11Buffer*				m_vertexBuffer				= nullptr;
ID3D11RasterizerState*		m_rasterizerState			= nullptr;

void* m_dxgiDebugModule;
void* m_dxgiDebug;

ID3DBlob*					m_pp_vertexByteCode			= nullptr;
ID3DBlob*					m_pp_vertexErrorMessage		= nullptr;
ID3DBlob*					m_pp_pixelByteCode			= nullptr;
ID3DBlob*					m_pp_pixelErrorMessage		= nullptr;

//----------------------------------------------------------------------------------------------------------------------
InputSystem*				g_theInput					= nullptr;
Window*						g_theWindow					= nullptr;
RandomNumberGenerator*		g_theRNG					= nullptr;
AudioSystem*				g_theAudio					= nullptr;
App*						g_theApp					= nullptr;	//Created and owned by Main_Windows.cpp

//----------------------------------------------------------------------------------------------------------------------
App::App()
{
}

//----------------------------------------------------------------------------------------------------------------------
App::~App()
{
}

//----------------------------------------------------------------------------------------------------------------------
Vertex_PCU vertices[] =
{
	Vertex_PCU( Vec3( -0.5f, -0.5f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
	Vertex_PCU( Vec3( 0.0f,  0.5f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
	Vertex_PCU( Vec3( 0.5f, -0.5f, 0.0f ), Rgba8( 255, 255, 255, 255 ), Vec2( 0.0f, 0.0f ) ),
};

const char* shaderSource = R"(
 struct vs_input_t
 {
 	float3 localPosition : POSITION;
 	float4 color : COLOR;
 	float2 uv : TEXCOORD;
 };
 
 struct v2p_t
 {
 	float4 position : SV_Position; 
 	float4 color : COLOR;
 	float2 uv : TEXCOORD;
 };
 
 v2p_t VertexMain(vs_input_t input)
 {
 	v2p_t v2p;
 	v2p.position = float4(input.localPosition, 1);
 	v2p.color = input.color;
 	v2p.uv = input.uv;
 	return v2p;
}
 
 float4 PixelMain(v2p_t input) : SV_Target0
 {
 	return float4(input.color);
 }
 )";
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
void App::Startup()
{
	// Create engine subsystems and game
	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem( inputSystemConfig );

	WindowConfig windowConfig;
	windowConfig.m_windowTitle = "FirstTriangle";
	windowConfig.m_clientAspect = 2.0f;
	windowConfig.m_inputSystem = g_theInput;
	g_theWindow = new Window ( windowConfig );

	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );
	
	// Start up engine subsystems and game
	g_theInput->StartUp();
	g_theWindow->Startup();
	g_theAudio->Startup();

//----------------------------------------------------------------------------------------------------------------------
#if defined( ENGINE_DEBUG_RENDER )
	m_dxgiDebugModule = ( void* ) ::LoadLibraryA( "dxgidebug.dll" );
	if ( m_dxgiDebugModule == nullptr )
	{
		ERROR_AND_DIE( "Could not load dxgidebug.dll" );
	}
	
	typedef HRESULT( WINAPI* GetDebugModuleCB )( REFIID, void** );
	( (GetDebugModuleCB) ::GetProcAddress( (HMODULE) m_dxgiDebugModule, "DXGIGetDebugInterface" ) )( __uuidof( IDXGIDebug ), &m_dxgiDebug);
	
	if ( m_dxgiDebug == nullptr )
	{
		ERROR_AND_DIE( "Could not load debug module" );
	}
#endif 

//----------------------------------------------------------------------------------------------------------------------
	DXGI_SWAP_CHAIN_DESC swapChainDesc	= { 0 };
	swapChainDesc.BufferDesc.Width		= g_theWindow->GetClientDimensions().x;
	swapChainDesc.BufferDesc.Height		= g_theWindow->GetClientDimensions().y;
	swapChainDesc.BufferDesc.Format		= DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count		= 1;
	swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount			= 2;
	swapChainDesc.OutputWindow			= static_cast<HWND>( g_theWindow->GetHwnd() );							// #Fixme // Not quite sure why this has to be cast to HWND?
	swapChainDesc.Windowed				= true;
	swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;

	//----------------------------------------------------------------------------------------------------------------------
	// #ToDo // check if this is in the correct place?
	unsigned int deviceFlags = 0;
#if defined( ENGINE_DEBUG_RENDER )
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif	
	//----------------------------------------------------------------------------------------------------------------------

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain
	( 
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE, 
		NULL, 
		deviceFlags,									// #ToDo // Question, what are "deviceFlags"? 
		nullptr, 
		0, 
		D3D11_SDK_VERSION, 
		&swapChainDesc, 
		&m_swapChain, 
		&m_device, 
		nullptr, 
		&m_deviceContext 
	);
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create D3D 11 device and swap chain." );
	}

	// Get back buffer texture
	ID3D11Texture2D* backBuffer;

	hr = m_swapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**) &backBuffer );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not get swap chain buffer." );
	}

	hr = m_device->CreateRenderTargetView( backBuffer, NULL, &m_renderTargetView );
	if ( !SUCCEEDED(hr) )
	{
		ERROR_AND_DIE( "Could create render target view for swap chain buffer." );
	}

	backBuffer->Release();
	  
//----------------------------------------------------------------------------------------------------------------------
	DWORD flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined( ENGINE_DEBUG_RENDER )
	flags = D3DCOMPILE_DEBUG;
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// Compile and create vertexShader
	hr = D3DCompile( shaderSource, strlen( shaderSource ), nullptr, nullptr, nullptr, "VertexMain", "vs_5_0", flags, 0, &m_pp_vertexByteCode, &m_pp_vertexErrorMessage );
	if ( !SUCCEEDED( hr ) )
	{
		DebuggerPrintf( static_cast<const char*>( m_pp_vertexErrorMessage->GetBufferPointer() ) );
		ERROR_AND_DIE( "Could not compile vertexShader." );
	}
	hr = m_device->CreateVertexShader( m_pp_vertexByteCode->GetBufferPointer(), m_pp_vertexByteCode->GetBufferSize(), nullptr, &m_vertexShader );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create vertexShader." );
	}

	// Compile and create pixelShader
	hr = D3DCompile( shaderSource, strlen( shaderSource ), nullptr, nullptr, nullptr, "PixelMain", "ps_5_0", flags, 0, &m_pp_pixelByteCode, &m_pp_pixelErrorMessage );
	if ( !SUCCEEDED( hr ) )
	{
		DebuggerPrintf( static_cast<const char*>( m_pp_pixelErrorMessage->GetBufferPointer() ) );
		ERROR_AND_DIE( "Could not create pixelShader." );
	}
	hr = m_device->CreatePixelShader( m_pp_pixelByteCode->GetBufferPointer(), m_pp_pixelByteCode->GetBufferSize(), nullptr, &m_pixelShader );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create pixelShader." );
	}

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = 
	{
		{  "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,							 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{	  "COLOR", 0,  DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{  "TEXCOORD", 0,    DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	
	// To get size of array, divide sizeof entire array by sizeof each element
	int numElement = sizeof( inputElementDesc ) / sizeof( inputElementDesc[0] );	
	hr = m_device->CreateInputLayout( inputElementDesc, numElement, m_pp_vertexByteCode->GetBufferPointer(), m_pp_vertexByteCode->GetBufferSize(), &m_inputLayout );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create inputLayout." );
	}

	//----------------------------------------------------------------------------------------------------------------------
	// Creating Vertex Buffer
	D3D11_BUFFER_DESC m_bufferDesc	= { 0 };
	m_bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	m_bufferDesc.ByteWidth			= sizeof( vertices );
	m_bufferDesc.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
	m_bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;

	hr = m_device->CreateBuffer( &m_bufferDesc, nullptr, &m_vertexBuffer );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create buffer." );
	}

	D3D11_MAPPED_SUBRESOURCE m_mappedSubResource; 
	hr = m_deviceContext->Map( m_vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &m_mappedSubResource );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not map resource." );
	}
	memcpy( m_mappedSubResource.pData, vertices, sizeof( vertices ) );
	m_deviceContext->Unmap( m_vertexBuffer, NULL );

	//----------------------------------------------------------------------------------------------------------------------
	// Set the Viewport
	D3D11_VIEWPORT viewport = { 0 };
	viewport.TopLeftX	= 0;
	viewport.TopLeftY	= 0 ;
	viewport.Width		= static_cast<float>( g_theWindow->GetClientDimensions().x ); 
	viewport.Height		= static_cast<float>( g_theWindow->GetClientDimensions().y );
	viewport.MinDepth	= 0;
	viewport.MaxDepth	= 1;

	m_deviceContext->RSSetViewports( 1, &viewport );

	//----------------------------------------------------------------------------------------------------------------------
	// Set the Rasterizer State
	D3D11_RASTERIZER_DESC rasterizerDesc = { };
	rasterizerDesc.FillMode					= D3D11_FILL_SOLID;
	rasterizerDesc.CullMode					= D3D11_CULL_NONE;
	rasterizerDesc.DepthClipEnable			= true;
	rasterizerDesc.AntialiasedLineEnable	= true;

	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerState );
	if ( !SUCCEEDED( hr ) )
	{
		ERROR_AND_DIE( "Could not create rasterizer state." );
	}
	m_deviceContext->RSSetState( m_rasterizerState );

	//----------------------------------------------------------------------------------------------------------------------
	// Set the Pipeline State
	UINT strides = sizeof( Vertex_PCU );
	UINT offset  = 0;
	
	m_deviceContext->IASetVertexBuffers( 0, 1, &m_vertexBuffer, &strides, &offset );
	m_deviceContext->IASetInputLayout( m_inputLayout );
	m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_deviceContext->VSSetShader( m_vertexShader, 0, 0 );
	m_deviceContext->PSSetShader( m_pixelShader, 0, 0 );
}

//----------------------------------------------------------------------------------------------------------------------
void App::Shutdown()
{
	g_theWindow->Shutdown();
	g_theInput->ShutDown();

	delete g_theWindow;
	g_theWindow = nullptr;

	delete g_theInput;
	g_theInput = nullptr;

	m_rasterizerState->Release();
	m_vertexBuffer->Release();
	m_inputLayout->Release();
	m_pixelShader->Release();
	m_vertexShader->Release();
	m_renderTargetView->Release();
	m_swapChain->Release();
	m_deviceContext->Release();
	m_device->Release();

#if defined( ENGINE_DEBUG_RENDER )
	( ( IDXGIDebug*) m_dxgiDebug )->ReportLiveObjects( DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS) (DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL ) );

	( ( IDXGIDebug* ) m_dxgiDebug )->Release();
	m_dxgiDebug = nullptr;

	::FreeLibrary( (HMODULE) m_dxgiDebugModule );
	m_dxgiDebugModule = nullptr;
#endif
}
 
//-----------------------------------------------------------------------------------------------
// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
void App::RunFrame()
{
	float deltaSeconds = 1.0f/60.0f;

	BeginFrame();	
	Update(deltaSeconds);
	Render();	
	EndFrame();	
}
 
//----------------------------------------------------------------------------------------------------------------------
bool App::HandleKeyPressed(unsigned char keyCode)
{
	g_theInput->HandleKeyPressed( keyCode );
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool App::HandleKeyReleased(unsigned char keyCode)
{
	g_theInput->HandleKeyReleased( keyCode );
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
void App::Run()
{
	// Program main loop; keep running frames until it's time to quit
	while (!IsQuitting())
	{
		g_theApp->RunFrame();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void App::BeginFrame()
{
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theAudio->BeginFrame();
}
 
//----------------------------------------------------------------------------------------------------------------------
void App::Update(float deltaSeconds) 
{ 
	UNUSED( deltaSeconds );

	if ( g_theInput->WasKeyJustPressed( KEYCODE_ESC ) )
	{
		HandleQuitRequested();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void App::Render() const
{
	// #Fixme // Not sure if the last parameter is correct? should it be null or nullptr?
	m_deviceContext->OMSetRenderTargets( 1, &m_renderTargetView, nullptr );

	Rgba8	color = Rgba8::GRAY;
	float	arrayRgba8AsFloat[4];
	color.GetAsFloat( arrayRgba8AsFloat );

	// #ToDo // Clarify how this works, Initially wasn't sure how to pass in the parameter for color.GetAsFloat()
	m_deviceContext->ClearRenderTargetView( m_renderTargetView, arrayRgba8AsFloat );
	
	// #ToDo // Question, why specify 3 instead of sizeof(m_vertexBuffer) 
//	UINT m_startVertexLocation = 0;
//	m_deviceContext->Draw( sizeof(m_vertexBuffer), m_startVertexLocation );
	m_deviceContext->Draw( 3, 0 );

	// #Fixme // Question. Is it correct to declare HRESULT multiple times? Since it was also declared in App::Startup
	HRESULT hr;
	hr = m_swapChain->Present( 0, 0 );
	if ( hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET )
	{
		ERROR_AND_DIE( "Device has been lost, application will now terminate." );
	}
} 

//----------------------------------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_theInput->EndFrame();
	g_theAudio->EndFrame();
}