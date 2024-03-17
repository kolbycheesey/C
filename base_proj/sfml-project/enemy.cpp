#include "enemy.h"
#include <iostream>

Enemy::Enemy(int e) {
    energy = e;
}

bool Enemy::performSetup() {
    if (!enemyTexture.loadFromFile("assets/enemy.png")) {
        std::cout << "Could not load enemy image" << std::endl;
        return false;
    }
    enemySprite.setTexture(enemyTexture);
    enemySprite.setPosition(sf::Vector2f(225,400));
    enemySprite.scale(sf::Vector2f(2,2));

    if (!attackSoundBuffer.loadFromFile("assets/damage.ogg")) {
        std::cout << "Could not load enemy audio" << std::endl;
        return false;
    }
    attackSound.setBuffer(attackSoundBuffer);

    return true;
}

void Enemy::draw(sf::RenderWindow * window) {
    window->draw(enemySprite);
}

bool Enemy::checkIfHit(sf::Vector2i mousePos) {
    float enemyMinX = enemySprite.getGlobalBounds().left;
    float enemyMaxX = enemySprite.getGlobalBounds().width + enemyMinX;
    float enemyMinY = enemySprite.getGlobalBounds().top;
    float enemyMaxY = enemySprite.getGlobalBounds().height + enemyMinY;

    float mouseX = mousePos.x;
    float mouseY = mousePos.y;

    return mouseX >= enemyMinX && mouseX <= enemyMaxX && mouseY >= enemyMinY && mouseY <= enemyMaxY;
}

bool Enemy::takeDamage(int damage) {
    energy -= damage;
    attackSound.play();
    return energy <= 0;
}
