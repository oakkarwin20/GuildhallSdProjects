#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"
//----------------------------------------------------------------------------------------------------------------------
// Declaring instead of including classes
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
// extern SoundID*				g_soundsIDs[NUM_GAME_SOUNDS];
// extern SoundPlaybackID		g_music;

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
// FIFA TEST Variables				// Units described in Meters
//----------------------------------------------------------------------------------------------------------------------

// Pitch Variables
constexpr float PITCH_DIMENSION_X	= 105.0f;
constexpr float PITCH_DIMENSION_Y	=  68.0f;
constexpr float PITCH_SIZE_X		= PITCH_DIMENSION_X;
constexpr float PITCH_SIZE_Y		= PITCH_DIMENSION_Y;
constexpr float PITCH_SIZE_Z		= 50.0f;
constexpr float PITCH_CENTER_X		= PITCH_SIZE_X * 0.5f;
constexpr float PITCH_CENTER_Y		= PITCH_SIZE_Y * 0.5f;
constexpr float MAX_PITCH_HEIGHT	= 10'000.0f;

// World Physics constants Variables
constexpr float AIR_DRAG			= 1.0f - 0.00025f;		// #ToDo: calculate ACTUAL air drag. (0.9f is a made up BS value)
//constexpr float AIR_DRAG			= 0.9998f;		// #ToDo: calculate ACTUAL air drag. (0.9f is a made up BS value)
constexpr float GRAVITY				= 9.81f;
constexpr float FLOOR_ELASTICITY	= 0.6f;			// Also known as "Restitution"
constexpr float FLOOR_FRICTION		= 0.99f;

// Football Variables
constexpr float FOOTBALL_RADIUS		= 0.22f;			// 22cm == 0.22 meters
constexpr float GARWIN_ALPHA		= 2.0f / 5.0f;		// (2.0f / 5.0f) = 0.4f
constexpr float FOOTBALL_MASS		= 0.45f;
// constexpr float FOOTBALL_DENSITY	= 0.021;			// 0
constexpr float DAMPENING			= 0.8f;
constexpr float ELASTICITY_X		= 0.65f;
constexpr float ELASTICITY_y		= 0.65f;

//----------------------------------------------------------------------------------------------------------------------
// Debug Globals
//----------------------------------------------------------------------------------------------------------------------
extern bool g_debugBallCanMoveIndependently;

//----------------------------------------------------------------------------------------------------------------------
// Game utility functions
void DebugDrawRing(); // Question, should this function be declared here if its temp game-code?
void DebugDrawLine( Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color ); // Question, should this function be declared here if its temp game-code?
