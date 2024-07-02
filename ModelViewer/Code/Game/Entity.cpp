#include "Game/Entity.hpp"
#include "Game/Game.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Mat44.hpp"

//----------------------------------------------------------------------------------------------------------------------
Entity::Entity(Game* game)
{
	m_game = game;
}

//----------------------------------------------------------------------------------------------------------------------
Entity::~Entity()
{
	delete m_vbo;
	delete m_ibo;
	m_vbo = nullptr;
	m_ibo = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 Entity::GetModelMatrix( Vec3 position, EulerAngles orientation ) const
{
	Mat44 modelMatrix;
	modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.SetTranslation3D( position );
	return modelMatrix;
}
