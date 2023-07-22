#include "Game.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <iostream>

#define DEFAULT_PLAYER_HORIZONTAL_VELOCITY 5

Game::Game() {
  m_entities = std::make_shared<EntityManager>();
  m_window = std::make_shared<sf::RenderWindow>(sf::VideoMode(1280, 1024),
                                                "Portal 2D");
  m_paused = true;
  m_running = false;

  init();
}

void Game::init() {

  m_paused = false;
  m_running = true;
  m_window->create(sf::VideoMode(1280, 1024), "Portal 2D");
  m_window->setFramerateLimit(120);
  m_currentFrame = 0;

  m_backgroundTexture.loadFromFile("resources/backgrounds/4.png");
  m_backgroundSprite.setTexture(m_backgroundTexture);
  m_backgroundSprite.setScale(
      m_window->getSize().x / m_backgroundSprite.getLocalBounds().width,
      m_window->getSize().y / m_backgroundSprite.getLocalBounds().height);

  spawnPlayer();
  std::cout << "[*] Initialization finished" << std::endl;
}

void Game::run() {
  while (m_running) {
    m_entities->update();
    sMovement();
    sCollision();
    sUserInput();
    sRender();
  }
}

void Game::sRender() {
  m_window->clear();
  m_window->draw(m_backgroundSprite);
  for (auto e : m_entities->getEntities()) {
    e->sprite().setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
    e->sprite().setRotation(e->cTransform->angle);

    // Set flip state
    if (e->cTransform->flipped && e->sprite().getScale().x >= 0.f) {
      e->sprite().setScale(-e->sprite().getScale().x, e->sprite().getScale().y);
    } else if (!e->cTransform->flipped && e->sprite().getScale().x < 0.f) {
      e->sprite().setScale(-e->sprite().getScale().x, e->sprite().getScale().y);
    }

    m_window->draw(e->sprite());
  }
  m_window->display();
}

void Game::sCollision() {}

void Game::sUserInput() {
  sf::Event event;
  while (m_window->pollEvent(event)) {
    switch (event.key.code) {
    case sf::Keyboard::Left:
      m_player->cInput->left = event.type == sf::Event::KeyPressed;
      break;
    case sf::Keyboard::Right:
      m_player->cInput->right = event.type == sf::Event::KeyPressed;
      break;
    case sf::Keyboard::Up:
      m_player->cInput->up = event.type == sf::Event::KeyPressed;
      break;
    case sf::Keyboard::Space:
      m_player->cInput->fire = event.type == sf::Event::KeyPressed;
      break;
    case sf::Keyboard::C:
      m_player->cInput->switchPortal = event.type == sf::Event::KeyPressed;
      break;
    defualt:
      break;
    }
  }
}

void Game::sMovement() {
  if (m_player->cInput->right) {
    m_player->cTransform->velocity.x = DEFAULT_PLAYER_HORIZONTAL_VELOCITY;
    m_player->cTransform->flipped = false;
  }
  if (m_player->cInput->left) {
    m_player->cTransform->velocity.x = -DEFAULT_PLAYER_HORIZONTAL_VELOCITY;
    m_player->cTransform->flipped = true;
  }
  if (!m_player->cInput->left && !m_player->cInput->right)
    m_player->cTransform->velocity.x = 0;

  for (auto e : m_entities->getEntities()) {
    e->cTransform->pos += e->cTransform->velocity;
  }
}

void Game::spawnPlayer() {
  m_player = m_entities->addPlayer();
  m_player->cTransform->pos = Vec2(500, 500);
}
