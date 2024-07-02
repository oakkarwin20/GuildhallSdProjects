#include "Game/Football2D.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Core/VertexUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
Football2D::Football2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
Football2D::~Football2D()
{
}

//----------------------------------------------------------------------------------------------------------------------
void Football2D::Update( float deltaSeconds )
{
	UpdatePhysics( deltaSeconds );
	UpdateClampToWorldBounds();
}

//----------------------------------------------------------------------------------------------------------------------
void Football2D::Render( std::vector<Vertex_PCU>& outVerts ) const
{
	AddVertsForDisc2D( outVerts, m_footballPosition, m_footballRadius, m_footballColor );
}

//----------------------------------------------------------------------------------------------------------------------
void Football2D::UpdatePhysics( float deltaSeconds )
{
	// Decelerate every frame with drag
	Vec2 decelerationForce = -m_footballDrag * m_footballVelocity;
	AddForce( decelerationForce );

	// Apply physics movement
	m_footballVelocity += m_footballAcceleration * deltaSeconds;
	m_footballPosition += m_footballVelocity * deltaSeconds;

	// Reset acceleration
	m_footballAcceleration = Vec2::ZERO;
}

//----------------------------------------------------------------------------------------------------------------------
void Football2D::MoveInDirection( Vec2 directionToMove, float speed )
{
	// Add Force in direction of target
	Vec2 forceAmount = directionToMove * m_footballDrag * speed;			// Direction should be normalized
	AddForce( forceAmount );
//	DebuggerPrintf( "forceAmount X = %0.2f, Y = %0.2f, m_playerAcceleration X = %0.2f, Y = %0.2f \n", forceAmount.x, forceAmount.y, m_playerAcceleration.x, m_playerAcceleration.y );
}

//----------------------------------------------------------------------------------------------------------------------
void Football2D::AddForce( Vec2 forceAmount )
{
	m_footballAcceleration = m_footballAcceleration + forceAmount;
}

//----------------------------------------------------------------------------------------------------------------------
void Football2D::UpdateClampToWorldBounds()
{
	if ( 
		 ( ( m_footballPosition.x - m_footballRadius ) < 0.0f			)  ||
		 ( ( m_footballPosition.x + m_footballRadius ) > WORLD_SIZE_X	)  ||
		 ( ( m_footballPosition.y - m_footballRadius ) < 0.0f			)  ||
		 ( ( m_footballPosition.y + m_footballRadius ) > WORLD_SIZE_Y	) 
	   )
	{
		m_footballPosition = Vec2( WORLD_CENTER_X, WORLD_CENTER_Y );
		m_footballVelocity = Vec2::ZERO;
	}

//	if ( (m_footballPosition.x - m_footballRadius) < 0.0f )
//	{
//		m_footballPosition.x *= -1.0f;
//	}
//	else if ( (m_footballPosition.x + m_footballRadius) > WORLD_SIZE_X )
//	{
//		m_footballPosition.x *= -1.0f; 
//	}
//	else if ( (m_footballPosition.y - m_footballRadius) < 0.0f )
//	{
//		m_footballPosition.y *= -1.0f;
//	}
//	else if ( (m_footballPosition.y + m_footballRadius) > WORLD_SIZE_Y )
//	{
//		m_footballPosition.y *= -1.0f;
//	}
}