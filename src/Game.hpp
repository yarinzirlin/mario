#ifndef PORTAL2D_GAME_H_
#define PORTAL2D_GAME_H_

#include "Entity.hpp"
#include "EntityManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Event.hpp>
class Game {
  std::shared_ptr<sf::RenderWindow> m_window;
  std::shared_ptr<EntityManager> m_entities;
  std::shared_ptr<Player> m_player;
  std::shared_ptr<StandbyPortal> m_sbportal;
  sf::Texture m_backgroundTexture;
  sf::Sprite m_backgroundSprite;
  bool m_paused;
  bool m_running;
  unsigned int m_lastPortalSwitch;
  unsigned int m_lastPortalFired;

  unsigned int m_currentFrame;

  void sMovement();
  void sUserInput(sf::Event event);
  void sRender();
  void sCollision();
  void spawnPlayer();
  void updateStandbyPortal();
  void updateMidairPortals();
  void firePortal();

  void init();

public:
  Game();
  void run();
};

#endif // !PORTAL2D_GAME_H_
