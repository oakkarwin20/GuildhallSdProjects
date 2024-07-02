#pragma once

#include <string>
#include <vector>

#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

//----------------------------------------------------------------------------------------------------------------------
struct WeaponAnimGroupDef
{
public:
	// Base variables
	std::string				m_name				= "WeaponAnimGroupDef name INVALID";
	SpriteAnimDefinition*	m_spriteAnimDef		= nullptr;
	
	// Shaders variables
	Shader*					m_shader			= nullptr;
	std::string				m_shaderName		= "invalidShaderName";

	// Textures variables
	Texture*				m_texture			= nullptr;
	std::string				m_textureName		= "invalidTextureName";

	// Cell count
	IntVec2					m_cellCount			= IntVec2::ZERO;

	float					m_secondsPerFrame	= 0.0f;
	float					m_framesPerSecond	= 0.0f;
	int						m_startFrame		= 0;
	int						m_endFrame			= 0;
};

//----------------------------------------------------------------------------------------------------------------------
class WeaponDefinition
{
public:
	WeaponDefinition( XmlElement const& weaponDefElement );
	~WeaponDefinition();

	static void InitializeWeaponDef( char const* path );
	static WeaponDefinition const* GetWeaponDefByName( std::string const& name );
	static std::vector<WeaponDefinition*> s_weaponDefinitionsList;

public:
	// Core attributes
	std::string		m_name				= "UNNAMED WEAPON TYPE";
	float			m_refireTime		= 0.0f;

	// Raycast aka Pistol attributes 
	int				m_rayCount			= 0;
	float			m_rayCone			= 0.0f;
	float			m_rayRange			= 0.0f;
	FloatRange		m_rayDamage			= FloatRange( 0.0f, 0.0f );
	float			m_rayImpulse		= 0.0f;

	// Projectile actors aka Plasma Rifle attributes 
	int				m_projectileCount			= 0;
	float			m_projectileCone			= 0.0f;
	float			m_projectileSpeed			= 0.0f;
	float			m_projectileAttackRange		= 0.0f;
	std::string		m_projectileActor			= "UNNAMED PROJECTILE ACTOR";

	// Demon melee attributes
	int				m_meleeCount		= 0;
	float			m_meleeRange		= 0.0f;
	int				m_meleeArc			= 0;
	FloatRange		m_meleeDamage		= FloatRange( 0.0f, 0.0f );
	float			m_meleeImpulse		= 0.0f;

	// Hud / Reticle / Animation / Shader variables 
	Texture*		m_baseTexture			= nullptr;
	Texture*		m_reticleTexture		= nullptr;
	SpriteSheet*	m_spriteSheet			= nullptr;
	std::string		m_spriteSheetName		= "invalidSpriteSheet";
	std::string		m_baseTextureName		= "invalidBaseTextureName";
	std::string		m_reticleTextureName	= "invalidReticleTextureName";
	Vec2			m_reticleSize			= Vec2::ZERO;
	std::string		m_shaderName			= "invalidShaderName";
	Shader*			m_shader				= nullptr;
	Vec2			m_spriteSize			= Vec2::ZERO;
	Vec2			m_spritePivot			= Vec2::ZERO;

	std::vector<WeaponAnimGroupDef> m_weaponAnimGroupDefList;

	// Sound variables
	std::string		m_sound		= "invalid Sound";
	std::string		m_soundName = "invalid SoundName";		// "sound name" is "file path"
};