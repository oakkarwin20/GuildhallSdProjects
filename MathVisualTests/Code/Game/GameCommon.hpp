#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

//----------------------------------------------------------------------------------------------------------------------
// Declaring instead of including classes
//----------------------------------------------------------------------------------------------------------------------
class App;
class Renderer;
class RandomNumberGenerator;
class InputSystem;
class AudioSystem;
class Window;

//----------------------------------------------------------------------------------------------------------------------
// Globally accessible objects
//----------------------------------------------------------------------------------------------------------------------
extern App*						g_theApp;
extern Renderer*				g_theRenderer;
extern RandomNumberGenerator*	g_theRNG;
extern InputSystem*				g_theInput;
extern AudioSystem*				g_theAudio;
extern Window*					g_theWindow;

//----------------------------------------------------------------------------------------------------------------------
// Common rendering variables
//----------------------------------------------------------------------------------------------------------------------
constexpr float CLIENT_ASPECT = 2.0f; // We are requesting a 2:1 aspect (square) window area

constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;

constexpr float SCREEN_SIZE_X = 200.f;
constexpr float SCREEN_SIZE_Y = 100.f;
constexpr float SCREEN_CENTER_X = SCREEN_SIZE_X / 2.f;
constexpr float SCREEN_CENTER_Y = SCREEN_SIZE_Y / 2.f;

constexpr float ATTRACT_MODE_SIZE_X = 1600.f;
constexpr float ATTRACT_MODE_SIZE_Y = 800.f;
constexpr float ATTRACT_MODE_CENTER_X = ATTRACT_MODE_SIZE_X / 2.f;
constexpr float ATTRACT_MODE_CENTER_Y = ATTRACT_MODE_SIZE_Y / 2.f;

//----------------------------------------------------------------------------------------------------------------------
// FIFA_TEST Variables
//----------------------------------------------------------------------------------------------------------------------
constexpr float FOOTBALL_RADIUS	= 5.0f;
constexpr float GARWIN_ALPHA	= 2.0f/5.0f; 
constexpr float BALL_MASS		= 5.0f;
constexpr float DAMPENING		= 0.8f;
constexpr float ELASTICITY_X	= 0.65f;
constexpr float ELASTICITY_y	= 0.65f;

//----------------------------------------------------------------------------------------------------------------------
// Convex Scene
//----------------------------------------------------------------------------------------------------------------------
extern AABB2 g_parsedWorldBounds;
extern bool  m_shouldChangeWorldBounds;

//----------------------------------------------------------------------------------------------------------------------
// Game utility functions
void DebugDrawRing(); // Question, should this function be declared here if its temp game-code?
void DebugDrawLine( Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color ); // Question, should this function be declared here if its temp game-code?
