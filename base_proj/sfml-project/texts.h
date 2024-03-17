#include <SFML/Graphics.hpp>

#ifndef TEXTS_H
#define TEXTS_H

class Texts
{
    sf::Font font;
    void setUpText(sf::Text *, sf::Vector2f);
public:
    sf::Text energyText;
    sf::Text timeText;
    sf::Text endGameWonText;
    sf::Text endGameTimeText;
    sf::Text endGameSpaceText;

    Texts();
    bool performSetup();
    void drawInGameText(sf::RenderWindow *, sf::Time, int);
    void drawEndGameText(sf::RenderWindow *, sf::Time);
};

#endif