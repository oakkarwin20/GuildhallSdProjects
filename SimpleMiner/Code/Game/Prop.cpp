#include "Game/Prop.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"

//----------------------------------------------------------------------------------------------------------------------
Prop::Prop( Game* game ) 
	: Entity( game )
{
}

//----------------------------------------------------------------------------------------------------------------------
void Prop::Update( float deltaSeconds )
{	
	m_orientationDegrees.m_yawDegrees	+= m_angularVelocity.m_yawDegrees   * deltaSeconds;
	m_orientationDegrees.m_pitchDegrees += m_angularVelocity.m_pitchDegrees * deltaSeconds;
	m_orientationDegrees.m_rollDegrees	+= m_angularVelocity.m_rollDegrees  * deltaSeconds;

	if ( m_isBlinking )
	{
		//----------------------------------------------------------------------------------------------------------------------
		// Blink prop black and white
		float totalSeconds		= m_game->m_clock.GetTotalSeconds();
		float someNumber		= CosDegrees( totalSeconds * 90.0f );
		unsigned char newColor	= static_cast<unsigned char>( RangeMap( someNumber, -1.0f, 1.0f, 0.0f, 255.0f ) );
		m_color					= Rgba8( newColor, newColor, newColor, 255 );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void Prop::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->BindTexture( m_texture );
	g_theRenderer->SetModelConstants( GetModelMatrix( m_position, m_orientationDegrees ), m_color );				// How to set model color?
	g_theRenderer->DrawVertexArray( static_cast<int>( m_vertexes.size() ), m_vertexes.data() );
	g_theRenderer->BindTexture( m_texture );
}

//----------------------------------------------------------------------------------------------------------------------
void Prop::AddCubeToScene( float x, float y, float z )
{
	//----------------------------------------------------------------------------------------------------------------------
	// Add cube to scene
	Vec3 bottomLeft;
	Vec3 bottomRight;
	Vec3 topLeft;
	Vec3 topRight;

	// +X face (East)
	bottomLeft	= Vec3(  x, -y, -z );
	bottomRight = Vec3(  x,  y, -z );
	topRight	= Vec3(  x,  y,  z );
	topLeft		= Vec3(  x, -y,  z );
	AddVertsForQuad3D( m_vertexes, bottomLeft, bottomRight, topRight, topLeft, Rgba8::RED );

	// -X face (West)
	bottomLeft	= Vec3( -x,  y, -z );
	bottomRight = Vec3( -x, -y, -z );
	topRight	= Vec3( -x, -y,  z );
	topLeft		= Vec3( -x,  y,  z );
	AddVertsForQuad3D( m_vertexes, bottomLeft, bottomRight, topRight, topLeft, Rgba8::CYAN );

	// +Y face (North)
	bottomLeft  = Vec3(  x, y, -z );
	bottomRight = Vec3( -x, y, -z );
	topRight	= Vec3( -x, y,  z );
	topLeft		= Vec3(  x, y,  z );
	AddVertsForQuad3D( m_vertexes, bottomLeft, bottomRight, topRight, topLeft, Rgba8::GREEN );

	// -Y face (South)
	bottomLeft	= Vec3( -x, -y, -z );
	bottomRight = Vec3(  x, -y, -z );
	topRight	= Vec3(  x, -y,  z );
	topLeft		= Vec3( -x, -y,  z );
	AddVertsForQuad3D( m_vertexes, bottomLeft, bottomRight, topRight, topLeft, Rgba8::MAGENTA );

	// +Z face (Skyward)
	bottomLeft	= Vec3( -x, -y, z );
	bottomRight = Vec3(  x, -y, z );
	topRight	= Vec3(  x,  y, z );
	topLeft		= Vec3( -x,  y, z );
	AddVertsForQuad3D( m_vertexes, bottomLeft, bottomRight, topRight, topLeft, Rgba8::BLUE );

	// -Z face (GroundWard)
	bottomLeft	= Vec3(  x, -y, -z );
	bottomRight = Vec3( -x, -y, -z );
	topRight	= Vec3( -x,  y, -z );
	topLeft		= Vec3(  x,  y, -z );
	AddVertsForQuad3D( m_vertexes, bottomLeft, bottomRight, topRight, topLeft, Rgba8::YELLOW );
}

//----------------------------------------------------------------------------------------------------------------------
void Prop::AddSphereToScene( Vec3 sphereCenter, float sphereRadius, float sphereNumSlices, float sphereNumStacks, Texture* texture )
{
	m_texture = texture;
	AddVertsForSphere3D( m_vertexes, sphereCenter, sphereRadius, sphereNumSlices, sphereNumStacks );
}

//----------------------------------------------------------------------------------------------------------------------
void Prop::AddGridLinesToScene()
{
	// Render X axis as Red Color
	for ( int i = -50; i < 50; i++ )
	{
		Rgba8 x_axisColor = Rgba8::GRAY;
		AABB3 x_bounds = AABB3( -50.0f, -0.02f, -0.02f, 50.0f, 0.02f, 0.02f );
		x_bounds.m_mins.y += i;
		x_bounds.m_maxs.y += i;

		// X Line is at origin, set color = brightest Red
		if ( i == 0 )
		{
			x_axisColor = Rgba8::RED;
			x_bounds.m_maxs.y += 0.04f;
			x_bounds.m_mins.y -= 0.04f;
			x_bounds.m_mins.z -= 0.04f;
			x_bounds.m_maxs.z += 0.04f;
		}

		// X Line is divisible by 5, set color Gray && increase line thickness
		else if ( i % 5 == 0 )
		{
			x_axisColor = Rgba8::DARK_RED;
			x_bounds.m_mins.y -= 0.02f;
			x_bounds.m_maxs.y += 0.02f;
			x_bounds.m_mins.z -= 0.02f;
			x_bounds.m_maxs.z += 0.02f;
		}

		AddVertsForAABB3D( m_vertexes, x_bounds, x_axisColor );
	}

	// Render X axis as Green Color	
	for ( int j = -50; j < 50; j++ )
	{
		Rgba8 y_axisColor = Rgba8::GRAY;
		AABB3 y_bounds = AABB3( -0.02f, -50.0f, -0.02f, 0.02f, 50.0f, 0.02f );
		y_bounds.m_mins.x += j;
		y_bounds.m_maxs.x += j;

		// X Line is at origin, set color = brightest Red
		if ( j == 0 )
		{
			y_axisColor = Rgba8::GREEN;
			y_bounds.m_mins.x -= 0.04f;
			y_bounds.m_maxs.x += 0.04f;
			y_bounds.m_mins.z -= 0.04f;
			y_bounds.m_maxs.z += 0.04f;
		}

		// X Line is divisible by 5, set color Gray && increase line thickness
		else if ( j % 5 == 0 )
		{
			y_axisColor = Rgba8::DARK_GREEN;
			y_bounds.m_mins.x -= 0.02f;
			y_bounds.m_maxs.x += 0.02f;
			y_bounds.m_mins.z -= 0.02f;
			y_bounds.m_maxs.z += 0.02f;
		}

		AddVertsForAABB3D( m_vertexes, y_bounds, y_axisColor );
	}
}