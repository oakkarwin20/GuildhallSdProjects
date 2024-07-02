#pragma once

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/Entity.hpp"

//----------------------------------------------------------------------------------------------------------------------
class Prop : public Entity
{
public:
	Prop( Game* game );
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

	void AddCubeToScene( float x, float y, float z );
	void AddSphereToScene( Vec3 sphereCenter, float sphereRadius, float sphereNumSlices, float sphereNumStacks, Texture* texture );
	void AddGridLinesToScene();

public:
	Texture*	m_texture	 = nullptr;
	bool		m_isBlinking = false;
};