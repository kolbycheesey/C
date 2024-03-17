#include "gameWorld.h"
#include "enemy.cpp"
#include "texts.cpp"

GameWorld::GameWorld(): enemy(100), texts() {
    damage = 10;
}

bool GameWorld::loadBackground() {
    if (!backgroundTexture.loadFromFile("assets/background.png")) {
        std::cout << "Could not load background image" << std::endl;
        return false;
    }
    background.setTexture(backgroundTexture);
    background.scale(sf::Vector2f(1.6,2.25));
    return true;
}

bool GameWorld::performSetup() {
    isGameOver = false;
    enemy = Enemy(100);
    texts = Texts();
    return loadBackground() && enemy.performSetup() && texts.performSetup();
}

bool GameWorld::runGame() {
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Point and Click Game");
    sf::Clock clock;

    while (window.isOpen())
    {
        if (!isGameOver) {
            time = clock.getElapsedTime();
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
                return false;
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (!isGameOver) {
                    if (enemy.checkIfHit(sf::Mouse::getPosition(window))) {
                        isGameOver = enemy.takeDamage(damage);
                        std::cout << "Clicked on enemy" << std::endl;
                    }
                }
            } else if (event.type == sf::Event::KeyPressed) {
                if (isGameOver) {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                        return true;
                    }
                }
            }
        }
        window.clear();
        window.draw(background);
        if (isGameOver) {
            texts.drawEndGameText(&window, time);
        } else {
            enemy.draw(&window);
            texts.drawInGameText(&window, time, enemy.energy);
        }
        window.display();
    }
    return false;
}