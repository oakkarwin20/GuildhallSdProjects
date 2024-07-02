#pragma once

#include "Game/GameModeBase.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"


//----------------------------------------------------------------------------------------------------------------------
class Map_Animations
{
public:
	Map_Animations();
	~Map_Animations();
	void Startup();
	void Shutdown();
	void Update( float deltaSeconds );
	void Render( std::vector<Vertex_PCU>& verts ) const;

public:
	GameMode_Animations* m_gameModeAnimations = nullptr;

	AABB3			m_floorBounds = AABB3();
	VertexBuffer*	m_vbo   = nullptr;
};
