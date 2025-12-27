#include <SFML/Graphics.hpp>
#include "ZombieArena.h"



/*
The background is not drawn as a single sprite, instead, it is built from my tiles.
A VertexArray allows batching those tiles into one single drawable.
So the SFML sends one big mesh to the GPU instead of thousands of individual sprites. 
*/

/*. .texCoords
1. VertexArray has position, color and texCoords
2. texCoords: where to sample FROM a texture; there is no texture in VertexArray itself.
3. rVA[i].texCoords = Vector2f(0, 50);  it doesn't know image, texture or whether the coordinate makes sense,
   It just stores the numbers
4. in main.cpp, window.draw(background, &textureBackground); SFML interprets each vertex and sampel the pixel form textureBackground.
5. createBackground() focuses on describe the geometry and texture mapping, don't care which texture is used.
*/

int createBackground(VertexArray& rVA, IntRect arena)
{
    const int TILE_SIZE = 50;
    const int TILE_TYPES = 3;
    const int VERTS_IN_QUAD = 4;

    int worldWidth  = arena.width / TILE_SIZE;  // 500 / 50 = 10
    int worldHeight = arena.height / TILE_SIZE;// 500 / 50 = 10

    rVA.setPrimitiveType(Quads);  // 4 vertices per tile
    rVA.resize(worldWidth * worldHeight * VERTS_IN_QUAD);   

    int currentVertex = 0;

    for(int w = 0; w < worldWidth; w++)
    {
        for(int h = 0; h < worldHeight; h++)
        {
            // set the four vertex positions of certain tile
            // the position is in the world space(view)
            rVA[currentVertex + 0].position = Vector2f( w * TILE_SIZE,             h * TILE_SIZE);
            rVA[currentVertex + 1].position = Vector2f( w * TILE_SIZE + TILE_SIZE, h * TILE_SIZE);
            rVA[currentVertex + 2].position = Vector2f( w * TILE_SIZE + TILE_SIZE, h * TILE_SIZE + TILE_SIZE);
            rVA[currentVertex + 3].position = Vector2f( w * TILE_SIZE,             h * TILE_SIZE + TILE_SIZE);

            if( h == 0 || h == worldHeight -1 || w == 0 || w == worldWidth - 1)
            {
                // wall 
                //.texCoords: which part of the texture to use (0,150) (50, 150, (50, 200), (0, 200))
                // the texture has 4 (50 * 50) tiles, from top to bottom;  uses the last tile to build the wall
                rVA[currentVertex + 0].texCoords = Vector2f(0,                0 + TILE_TYPES * TILE_SIZE);
                rVA[currentVertex + 1].texCoords = Vector2f(TILE_SIZE,        0 + TILE_TYPES * TILE_SIZE);
                rVA[currentVertex + 2].texCoords = Vector2f(TILE_SIZE, TILE_SIZE + TILE_TYPES * TILE_SIZE);
                rVA[currentVertex + 3].texCoords = Vector2f(0,         TILE_SIZE + TILE_TYPES * TILE_SIZE);
            }
            else
            {
                // floor
                // There are 3 tiles can be used to build the floor;  randomly choose one each time
                // The three tiles: (0, n * 50), (50, n * 50), (50, 50 + n * 50), (0, 50 + n * 50) 
                // n: 0, 1, 2
                srand((int)time(0) + h * w - h);   
                int mOrG = (rand() % TILE_TYPES); 
                int verticalOffset = mOrG * TILE_SIZE; // n * 50

                rVA[currentVertex + 0].texCoords = Vector2f(0,                 0 + verticalOffset);
                rVA[currentVertex + 1].texCoords = Vector2f(TILE_SIZE,         0 + verticalOffset);
                rVA[currentVertex + 2].texCoords = Vector2f(TILE_SIZE, TILE_SIZE + verticalOffset);
                rVA[currentVertex + 3].texCoords = Vector2f(0,         TILE_SIZE + verticalOffset);
            }

            currentVertex = currentVertex + VERTS_IN_QUAD; // + 4

        }
    }

    return TILE_SIZE;

}