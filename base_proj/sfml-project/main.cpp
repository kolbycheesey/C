#include "gameWorld.cpp"
#include <iostream>

int main()
{
    while(true) {
        GameWorld world = GameWorld();
        if (!world.performSetup()) {
            return 1;
        }
        if (!world.runGame()) {
            return 0;
        }
    }
    return 0;
}