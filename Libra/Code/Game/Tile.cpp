#include "Game/Tile.hpp"

//----------------------------------------------------------------------------------------------------------------------
AABB2 Tile::GetBounds() const
{
    AABB2 bounds;
    bounds.m_mins.x = static_cast<float>( m_tileCoords.x );
    bounds.m_mins.y = static_cast<float>( m_tileCoords.y );
    bounds.m_maxs.x = bounds.m_mins.x + 1.0f;
    bounds.m_maxs.y = bounds.m_mins.y + 1.0f;
    return bounds;
}

//----------------------------------------------------------------------------------------------------------------------
Rgba8 Tile::GetColor() const
{
    Rgba8 color( 255, 0, 255 );
    if ( m_tileDefIndex == TILE_TYPE_GRASS )
    {
        color = Rgba8( 20, 60, 20 );
    } 
    else if ( m_tileDefIndex == TILE_TYPE_STONE )
    {
		color = Rgba8( 130 , 130 , 130 );
    }
    else if ( m_tileDefIndex == TILE_TYPE_TEST_RED )
    {
        color = Rgba8( 100, 0, 0, 200 );
    }
    else if ( m_tileDefIndex == TILE_TYPE_TEST_BLUE )
    {
        color = Rgba8( 0, 0, 255, 200 );
    }
    return color;
}
