#include "Game/SpriteAnimationGroupDefinition.hpp"

//----------------------------------------------------------------------------------------------------------------------
SpriteAnimationGroupDefinition::SpriteAnimationGroupDefinition( XmlElement* spriteAnimGroupDef, SpriteSheet& spriteSheet )
{
	// Parse XML data and set member variable data
	m_name					= ParseXmlAttribute( *spriteAnimGroupDef,	 		 "name", m_name				);
	m_scaleBySpeed			= ParseXmlAttribute( *spriteAnimGroupDef,	 "scaleBySpeed", m_scaleBySpeed		);
	m_playbackMode			= ParseXmlAttribute( *spriteAnimGroupDef,	 "playbackMode", m_playbackMode		);

	// Set playbackMode based on parsed XML data
	SpriteAnimPlaybackType playbackMode = SpriteAnimPlaybackType::ONCE;
	if ( m_playbackMode == "Once" )
	{
		playbackMode = SpriteAnimPlaybackType::ONCE;
	}
	if ( m_playbackMode == "Loop" )
	{
		playbackMode = SpriteAnimPlaybackType::LOOP;
	}
	if ( m_playbackMode == "PingPong" )
	{
		playbackMode = SpriteAnimPlaybackType::PING_PONG;
	}

	// Parse XML data for secondsPerFrame
	float secondsPerFrame	= -1;
	secondsPerFrame			= ParseXmlAttribute( *spriteAnimGroupDef, "secondsPerFrame", secondsPerFrame );

	// Get child element of "Direction" and loop through 
	XmlElement* childDirectionElement = spriteAnimGroupDef->FirstChildElement( "Direction" );
	while ( childDirectionElement )
	{
		DirectionalAnimation dirAnim;
		dirAnim.m_direction					= ParseXmlAttribute( *childDirectionElement, "vector", Vec3::ZERO );
		dirAnim.m_direction					= dirAnim.m_direction.GetNormalized();
		int startFrame						= -1;
		int endFrame						= -1;
		XmlElement* childAnimationElement   = childDirectionElement->FirstChildElement( "Animation" );
		startFrame							= ParseXmlAttribute( *childAnimationElement, "startFrame", startFrame );
		endFrame							= ParseXmlAttribute( *childAnimationElement,   "endFrame",	 endFrame );
		float framesPerSecond				= (1.f / secondsPerFrame);
		SpriteAnimDefinition* spriteAnimDef = new SpriteAnimDefinition( spriteSheet, startFrame, endFrame, framesPerSecond, playbackMode );
		dirAnim.m_spriteAnimDef				= spriteAnimDef;
		m_directionalAnimList.push_back( dirAnim );
		childDirectionElement				= childDirectionElement->NextSiblingElement();
	}
}

//----------------------------------------------------------------------------------------------------------------------
SpriteAnimationGroupDefinition::~SpriteAnimationGroupDefinition()
{
}
