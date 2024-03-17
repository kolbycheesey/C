#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "enemy.h"
#include "texts.h"

#ifndef GAMEWORLD_H
#define GAMEWORLD_H

class GameWorld
{
    bool isGameOver;
    int damage;
    sf::Texture backgroundTexture;
    sf::Sprite background;
    sf::Time time;
    Enemy enemy;
    Texts texts;
    bool loadBackground();
public:
    GameWorld();
    bool performSetup();
    bool runGame();
};

#endif