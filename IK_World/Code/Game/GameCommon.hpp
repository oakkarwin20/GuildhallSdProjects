#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
// Declaring instead of including classes
//----------------------------------------------------------------------------------------------------------------------
class App;
class Renderer;
class RandomNumberGenerator;
class InputSystem; 
class AudioSystem;
class Window;
class Game;
class BitmapFont;
class ConstantBuffer;
		
//----------------------------------------------------------------------------------------------------------------------
// Globally accessible objects
//----------------------------------------------------------------------------------------------------------------------
extern App*						g_theApp;
extern Renderer*				g_theRenderer;
extern RandomNumberGenerator*	g_theRNG;
extern InputSystem*				g_theInput;
extern AudioSystem*				g_theAudio;
extern Window*					g_theWindow;
extern Game*					g_theGame;
extern BitmapFont*				g_font;
// extern SoundID*				g_soundsIDs[NUM_GAME_SOUNDS];
// extern SoundPlaybackID		g_music;

//----------------------------------------------------------------------------------------------------------------------
// Debug globals
//----------------------------------------------------------------------------------------------------------------------
extern bool g_debugUseWhiteTexture;
extern bool g_debugDrawChunkBoundaries;
extern bool g_debugDrawChunkStates;
extern bool g_debugDrawLightValues;
extern bool g_debugDrawCurrentBlockIter;
extern bool g_debugDrawCaves;
extern bool g_debugUseWorldShader;
extern bool g_debugDrawRaycast;

//----------------------------------------------------------------------------------------------------------------------
// Common rendering variables
//----------------------------------------------------------------------------------------------------------------------
constexpr float CLIENT_ASPECT = 2.0f; // We are requesting a 2:1 aspect (square) window area

constexpr float WORLD_SIZE_X			= 200.f;
constexpr float WORLD_SIZE_Y			= 100.f;
constexpr float WORLD_CENTER_X			= WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y			= WORLD_SIZE_Y / 2.f;	

constexpr float SCREEN_SIZE_X			= 200.f;
constexpr float SCREEN_SIZE_Y			= 100.f;
constexpr float SCREEN_CENTER_X			= SCREEN_SIZE_X / 2.f;
constexpr float SCREEN_CENTER_Y			= SCREEN_SIZE_Y / 2.f;

constexpr float ATTRACT_MODE_SIZE_X		= 1600.f;
constexpr float ATTRACT_MODE_SIZE_Y		= 800.f;
constexpr float ATTRACT_MODE_CENTER_X	= ATTRACT_MODE_SIZE_X / 2.f;
constexpr float ATTRACT_MODE_CENTER_Y	= ATTRACT_MODE_SIZE_Y / 2.f;

//----------------------------------------------------------------------------------------------------------------------
// SIMPLE_MINER Variables
//----------------------------------------------------------------------------------------------------------------------
constexpr int SPRITESHEET_GRID_LAYOUT_X = 64;
constexpr int SPRITESHEET_GRID_LAYOUT_Y = 64;
constexpr int GROUND_HEIGHT_Z			= 64;

//----------------------------------------------------------------------------------------------------------------------
//struct SimpleMinerGPUData : register( b8 )
struct SimpleMinerGPUData
{
	Vec4	m_indoorLightColor;        // gameCamera-to-clip ( includes gameCamera->screenCamera axis swaps )
//    Vec4	c_outdoorLightColor;       // gameCamera-to-clip ( includes gameCamera->screenCamera axis swaps )
//    Vec4	c_fogColor;                // Fog color to blend in
//    Vec4	c_fogStartDistance;        // World units away where fog begins    (0%)
//    Vec4	c_fogEndDistance;          // World units away where fog maxes out (100%)
//    Vec4	c_dummyPadding1;
//    Vec4	c_dummyPadding2;
};

extern ConstantBuffer* g_simpleMinerCBO;

//----------------------------------------------------------------------------------------------------------------------
// Game utility functions
//----------------------------------------------------------------------------------------------------------------------
void DebugDrawRing(); 
void DebugDrawLine( Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color ); 