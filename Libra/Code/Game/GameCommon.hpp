#pragma once
#include "Game/Map.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
class App;
class Renderer;
class RandomNumberGenerator;
class InputSystem;
class AudioSystem;
class Map;
class Game;
		
//----------------------------------------------------------------------------------------------------------------------
// Globally accessible objects
extern App*						g_theApp;
extern Renderer*				g_theRenderer;
extern RandomNumberGenerator*	g_theRNG;
extern InputSystem*				g_theInput;
extern AudioSystem*				g_theAudio;

//extern Game*					g_theGame;
// extern SoundID*						g_soundsIDs[NUM_GAME_SOUNDS];
// extern SoundPlaybackID				g_music;

constexpr float CLIENT_ASPECT = 2.0f; // We are requesting a 2:1 aspect (square) window area

//constexpr float ORTHO_SIZE_X = 16.0f;
//constexpr float ORTHO_SIZE_Y =  8.0f;
//constexpr float ORTHO_CENTER_X = ORTHO_SIZE_X / 2.f;
//constexpr float ORTHO_CENTER_Y = ORTHO_SIZE_Y / 2.f;

constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;

constexpr float SCREEN_SIZE_X = 1600.f;
constexpr float SCREEN_SIZE_Y = 800.f;
constexpr float SCREEN_CENTER_X = SCREEN_SIZE_X / 2.f;
constexpr float SCREEN_CENTER_Y = SCREEN_SIZE_Y / 2.f;

constexpr float ATTRACT_MODE_SIZE_X = 200.f;
constexpr float ATTRACT_MODE_SIZE_Y = 100.f;
constexpr float ATTRACT_MODE_CENTER_X = ATTRACT_MODE_SIZE_X / 2.f;
constexpr float ATTRACT_MODE_CENTER_Y = ATTRACT_MODE_SIZE_Y / 2.f;

constexpr float PLAYER_TANK_MAX_SPEED = 1.0f;			// tiles per second
constexpr float PLAYER_TANK_TURN_SPEED = 180.0f;		// degrees per second

constexpr float BULLET_MAX_SPEED = 50.0f;			// tiles per second

//constexpr float WORLD_SIZE_X = 20.0f;
//constexpr float WORLD_SIZE_Y = 30.0f;

constexpr float EAST	  = 0.0f;
constexpr float NORTH	  = 90.0f;
constexpr float WEST	  = 180.0f;
constexpr float SOUTH	  = 270.0f;
constexpr float NORTHEAST = 45.0f;
constexpr float NORTHWEST = 135.0f;
constexpr float SOUTHEAST = 315.0f;
constexpr float SOUTHWEST = 225.0f;

//----------------------------------------------------------------------------------------------------------------------
// Game utility functions
void DebugDrawRing( Vec2 const& center, float radius, float thickness, Rgba8 const& color );
void DebugDrawLine( Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color );
