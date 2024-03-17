// Installing and Running Template Code
// #include <SFML/Graphics.hpp>

// int main()
// {
//     sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
//     sf::CircleShape shape(100.f);
//     shape.setFillColor(sf::Color::Green);
//     while (window.isOpen())
//     {
//         sf::Event event;
//         while (window.pollEvent(event))
//         {
//             if (event.type == sf::Event::Closed)
//                 window.close();
//         }
//         window.clear();
//         window.draw(shape);
//         window.display();
//     }
//     return 0;
// }

// How SFML Runs
// #include <SFML/Graphics.hpp>
// int main()
// {
//     sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
//     while (window.isOpen())
//     {
//         sf::Event event;
//         while (window.pollEvent(event))
//         {
//             if (event.type == sf::Event::Closed)
//                 window.close();
//         }
//         window.clear();
//         window.display();
//     }
//     return 0;
// }

// Drawing Sprites
// #include <SFML/Graphics.hpp>
// #include <iostream>
// int main()
// {
//     sf::RenderWindow window(sf::VideoMode(500, 500), "SFML works!");
//     sf::Texture texture;
//     if (!texture.loadFromFile("assets/enemy.png")) {
//         std::cout << "Could not load enemy texture";
//         return 0;
//     }
//     sf::Sprite enemySprite;
//     enemySprite.setTexture(texture);
//     enemySprite.setPosition(sf::Vector2f(100,100));
//     enemySprite.scale(sf::Vector2f(1,1.5));
//     while (window.isOpen())
//     {
//         sf::Event event;
//         while (window.pollEvent(event))
//         {
//             if (event.type == sf::Event::Closed)
//                 window.close();
//         }
//         window.clear();
//         window.draw(enemySprite);
//         window.display();
//     }
//     return 0;
// }

// Drawing Text
// #include <SFML/Graphics.hpp>
// #include <iostream>
// int main()
// {
//     sf::RenderWindow window(sf::VideoMode(500, 500), "SFML works!");
//     sf::Font font;
//     if (!font.loadFromFile("assets/Arial.ttf")) {
//         std::cout << "Could not load font file";
//         return 0;
//     }
//     sf::Text text;
//     text.setFont(font);
//     text.setCharacterSize(30);
//     text.setFillColor(sf::Color::White);
//     text.setStyle(sf::Text::Bold);
//     text.setPosition(sf::Vector2f(100,100));
//     text.setString("Here is some text");
//     while (window.isOpen())
//     {
//         sf::Event event;
//         while (window.pollEvent(event))
//         {
//             if (event.type == sf::Event::Closed)
//                 window.close();
//         }
//         window.clear();
//         window.draw(text);
//         window.display();
//     }
//     return 0;
// }

// Playing Audio
// #include <SFML/Graphics.hpp>
// #include <SFML/Audio.hpp>
// #include <iostream>
// int main()
// {
//     sf::RenderWindow window(sf::VideoMode(500, 500), "SFML works!");
//     sf::SoundBuffer buffer;
//     if (!buffer.loadFromFile("assets/damage.ogg")) {
//         std::cout << "Could not load audio";
//         return 0;
//     }
//     sf::Sound attackSound;
//     attackSound.setBuffer(buffer);
//     attackSound.play();
//     while (window.isOpen())
//     {
//         sf::Event event;
//         while (window.pollEvent(event))
//         {
//             if (event.type == sf::Event::Closed)
//                 window.close();
//         }
//         window.clear();
//         window.display();
//     }
//     return 0;
// }

// Handling user interactions
// #include <SFML/Graphics.hpp>
// #include <SFML/Audio.hpp>
// #include <iostream>
// int main()
// {
//     sf::RenderWindow window(sf::VideoMode(500, 500), "SFML works!");
//     while (window.isOpen())
//     {
//         sf::Event event;
//         while (window.pollEvent(event))
//         {
//             if (event.type == sf::Event::Closed) {
//                 window.close();
//             } else if (event.type == sf::Event::MouseButtonPressed) {
//                 std::cout << "Mouse button pressed" << std::endl;
//             } else if (event.type == sf::Event::KeyPressed) {
//                 if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
//                     std::cout << "Space bar pressed" << std::endl;
//                 }
//             }   
//         }
//         window.clear();
//         window.display();
//     }
//     return 0;
// }

// Handling Time
// #include <SFML/Graphics.hpp>
// #include <SFML/Audio.hpp>
// #include <iostream>
// int main()
// {
//     sf::RenderWindow window(sf::VideoMode(500, 500), "SFML works!");
//     sf::Clock clock;
//     clock.restart();
//     sf::Time time;
//     while (window.isOpen())
//     {
//         time = clock.getElapsedTime();
//         float seconds = time.asSeconds();
//         std::cout << seconds << std::endl;
//         sf::Event event;
//         while (window.pollEvent(event))
//         {
//             if (event.type == sf::Event::Closed) {
//                 window.close();
//             } 
//         }
//         window.clear();
//         window.display();
//     }
//     return 0;
// }