#include "ZombieArena.h"
#include "Zombie.h"

// create a group of Zombies

Zombie* createHorde(int numZombies, IntRect arena)
{
    Zombie* zombies = new Zombie[numZombies];
    int maxY = arena.height - 20;
    int minY = arena.top    + 20;
    int maxX = arena.width  - 20;
    int minX = arena.left   + 20;

    for(int i = 0; i < numZombies; i++)
    {
        srand((int)time(0) * i);
        int side = (rand() % 4);
        float x, y;


        // side
        switch(side)
        {
            case 0: //left
                x = minX;
                y = (rand() % maxY ) + minY;
                break;
            case 1: //right
                x = maxX;
                y = (rand() % maxY) + minY;
                break;
            case 2: //top
                x = (rand() % maxX) + minX;
                y = minY;
                break;
            case 3: //bottom
                x = (rand() % maxX) + minX;
                y = minY;
                break;
        }

        // type
        srand((int)time(0) * i * 2);
        int type = (rand() % 3);

        zombies[i].spawn(x, y, type, i);
    }
    return zombies;
}