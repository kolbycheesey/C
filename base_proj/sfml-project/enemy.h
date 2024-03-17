#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

#ifndef ENEMY_H
#define ENEMY_H

class Enemy
{
    sf::Texture enemyTexture;
    sf::Sprite enemySprite;
    sf::SoundBuffer attackSoundBuffer;
    sf::Sound attackSound;
public:
    int energy;
    Enemy(int);
    bool performSetup();
    bool checkIfHit(sf::Vector2i);
    bool takeDamage(int);
    void draw(sf::RenderWindow *);
};

#endif