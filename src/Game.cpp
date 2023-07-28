#include "Game.hpp"
#include "Entity.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <iostream>
#include <memory>

#define DEFAULT_PLAYER_HORIZONTAL_VELOCITY 5
#define FRAMERATE_LIMIT 120
#define PORTAL_VELOCITY 25

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
  m_currentFrame = 0;
  m_window->create(sf::VideoMode(1280, 1024), "Portal 2D");
  m_window->setFramerateLimit(FRAMERATE_LIMIT);
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
  while (m_window->isOpen()) {
    m_entities->update();
    sf::Event event;
    while (m_window->pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        m_window->close();
      }
      sUserInput(event);
    }
    sMovement();
    sCollision();
    sRender();
    m_currentFrame++;
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

void Game::sUserInput(sf::Event event) {
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
  case sf::Keyboard::F:
    m_player->cInput->fire = event.type == sf::Event::KeyPressed;
    break;
  case sf::Keyboard::C:
    m_player->cInput->switchPortal = event.type == sf::Event::KeyPressed;
    break;
  defualt:
    break;
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

  if (m_player->cInput->fire) {
    firePortal();
  }

  for (auto e : m_entities->getEntities()) {
    e->cTransform->pos += e->cTransform->velocity;
  }
  updateStandbyPortal();
  updateMidairPortals();
}

void Game::updateStandbyPortal() {
  int x_offset = 45;
  if (m_player->cTransform->flipped) {
    x_offset *= -1;
    x_offset += 5;
  }
  m_sbportal->cTransform->pos = m_player->cTransform->pos + Vec2(x_offset, 0);
  if (m_player->cInput->switchPortal &&
      m_currentFrame > m_lastPortalSwitch + FRAMERATE_LIMIT / 10) {
    m_lastPortalSwitch = m_currentFrame;
    m_sbportal->AlternateColor();
  }
}

void Game::updateMidairPortals() {
  for (auto e : m_entities->getEntities("midair_portal")) {
    auto p = std::dynamic_pointer_cast<MidairPortal>(e);
    if (p->GetLastPhaseChange() + PhaseDuration >= m_currentFrame) {
      p->NextPhase();
    }
  }
}

void Game::spawnPlayer() {
  m_player = m_entities->addPlayer();
  m_player->cTransform->pos = Vec2(500, 500);
  m_sbportal = m_entities->addEntity<StandbyPortal>();
  m_sbportal->cTransform->pos = Vec2(510, 500);
}

void Game::firePortal() {
  auto portal = m_entities->addEntity<MidairPortal>();
  portal->cTransform->pos = m_sbportal->cTransform->pos;
  portal->cTransform->velocity = Vec2(PORTAL_VELOCITY, 0);
  if (m_player->cTransform->flipped) {
    portal->cTransform->velocity *= -1;
  }
}
