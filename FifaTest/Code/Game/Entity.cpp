#include "Game/Entity.hpp"
//#include "Game/GameModeProtogame3D.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Mat44.hpp"

//----------------------------------------------------------------------------------------------------------------------
Entity::Entity(GameModeProtogame3D* game)
{
	m_game = game;
}

//----------------------------------------------------------------------------------------------------------------------
Entity::~Entity()
{
}

//----------------------------------------------------------------------------------------------------------------------
Mat44 Entity::GetModelMatrix( Vec3 position, EulerAngles orientation ) const
{
	Mat44 modelMatrix;
	modelMatrix = orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.SetTranslation3D( position );
	return modelMatrix;
}
