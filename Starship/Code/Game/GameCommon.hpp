#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"

class App;
class Renderer;
class RandomNumberGenerator;
class InputSystem;
class AudioSystem;
		
extern App* g_theApp;
extern Renderer* g_theRenderer;
extern RandomNumberGenerator* g_theRNG;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;

constexpr float CLIENT_ASPECT = 2.0f; // We are requesting a 2:1 aspect (square) window area

constexpr int Num_Starting_Asteriods = 6;
constexpr int Num_Max_Asteriods = 12;
constexpr int Max_Bullets = 20;

constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;	

constexpr float SCREEN_SIZE_X = 1600.f;
constexpr float SCREEN_SIZE_Y =  800.f;
constexpr float SCREEN_CENTER_X = SCREEN_SIZE_X / 2.f;
constexpr float SCREEN_CENTER_Y = SCREEN_SIZE_Y / 2.f;

constexpr float ASTERIOD_SPEED = 10.f;
constexpr float ASTERIOD_PHYSICS_RADIUS = 1.6f;
constexpr float ASTERIOD_COSMETIC_RADIUS = 2.f;
constexpr float BULLET_LIFETIME_SECONDS = 2.f;
constexpr float BULLET_SPEED = 50.f;
constexpr float BULLET_PHYSICS_RADIUS = 0.5f;
constexpr float BULLET_COSMETIC_RADIUS = 2.f;
constexpr float PLAYER_SHIP_ACCERLERATION = 30.f;
constexpr float PLAYER_SHIP_TURN_SPEED = 300.f;
constexpr float PLAYER_SHIP_PHYSICS_RADIUS= 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS= 2.25f;

//----------------------------------------------------------------------------------------------------------------------
// Globally accessible objects
//
extern App*							g_theApp;
extern Renderer*					g_theRenderer;
extern InputSystem*					g_theInput;
extern AudioSystem*					g_theAudio;
extern RandomNumberGenerator*		g_theRNG;
// extern SoundID*						g_soundsIDs[NUM_GAME_SOUNDS];
//extern SoundPlaybackID				g_music;

//----------------------------------------------------------------------------------------------------------------------
// Game utility functions
void DebugDrawRing( Vec2 const& center, float radius, float thickness, Rgba8 const& color );
void DebugDrawLine( Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color ); // Question, should this function be declared here if its temp game-code?
