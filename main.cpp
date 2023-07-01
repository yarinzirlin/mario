#include <iostream>
#include <SFML/Graphics.hpp>


void MainLoop() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "My window");
    // Set window background to white color
    window.clear(sf::Color::White);

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { window.close(); }
        }

        window.clear();
        window.draw(shape);
        window.display();
    }
}


int main() {
    MainLoop();
    return 0;
}

