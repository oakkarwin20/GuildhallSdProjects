#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

#include <string>
#include <vector>

//----------------------------------------------------------------------------------------------------------------------
class SpriteAnimationGroupDefinition
{
public:
	SpriteAnimationGroupDefinition( XmlElement* spriteAnimGroupDef, SpriteSheet& spriteSheet ); /* takes XML element of one AnimationGroup */ /* also the sprite sheet*/ 
	~SpriteAnimationGroupDefinition();

public:
	std::string		 m_name;
	bool			 m_scaleBySpeed = false;
	std::string		 m_playbackMode;

	struct DirectionalAnimation
	{
		Vec3					m_direction;
		SpriteAnimDefinition*	m_spriteAnimDef = nullptr;
	};
	std::vector<DirectionalAnimation> m_directionalAnimList;
};