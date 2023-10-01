#ifndef PORTAL2D_GAME_H_
#define PORTAL2D_GAME_H_

#include "Entity.hpp"
#include "EntityManager.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Event.hpp>
#include <tmxlite/Layer.hpp>
#include <tmxlite/Object.hpp>

#define PORTAL_COOLDOWN_TICKS 60

class Game {
  std::shared_ptr<sf::RenderWindow> m_window;
  std::shared_ptr<EntityManager> m_entities;
  std::shared_ptr<Player> m_player;
  std::shared_ptr<StandbyPortal> sbportal_;
  sf::Texture m_backgroundTexture;
  sf::Sprite m_backgroundSprite;
  bool paused_ = false;
  bool running_ = false;
  unsigned int last_portal_switch_ = 0;
  unsigned int last_portal_fired_ = 0;

  unsigned int current_frame_ = 0;
  sf::Font font_;

  void sMovement();
  void sUserInput(sf::Event event);
  void sRender();
  void DrawText(const std::string &str, const sf::Vector2f &pos,
                unsigned int size);
  void DrawInput();
  void RenderEntityOutline(std::shared_ptr<Entity> e);
  void sCollision(const std::unique_ptr<tmx::Layer> &collisionLayer,
                  const tmx::Map &map);
  void SpawnPlayer();
  void UpdateStandbyPortal();
  void UpdateMidairPortals();
  void firePortal();
  bool aabbCollisionCheck(BoundingBox &first, BoundingBox &second);
  void HandleEntitiesCollision(std::shared_ptr<Entity> e1,
                               std::shared_ptr<Entity> e2);
  bool IsEntityCollidingWithObjGroup(std::shared_ptr<Entity> entity,
                                     const tmx::ObjectGroup &collisionLayer,
                                     tmx::Object &outCollidingObject);
  bool IsEntityOutOfBounds(const std::shared_ptr<Entity> entity,
                           const tmx::Map &map);
  void HandleEntityOutOfBounds(const std::shared_ptr<Entity> entity);
  void HandleEntityCollisionWithMap(const std::shared_ptr<Entity> entity,
                                    const tmx::Object &collidingObject);
  bool ShouldPlaceStandingEntityOnCollider(const std::shared_ptr<Entity> entity,
                                           const tmx::Object &collider);
  void Init();

public:
  Game();
  void Run();
};

#endif // !PORTAL2D_GAME_H_
