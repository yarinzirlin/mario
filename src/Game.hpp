#ifndef PORTAL2D_GAME_H_
#define PORTAL2D_GAME_H_

#include "Entity.hpp"
#include "EntityManager.hpp"
#include "Vec2.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Event.hpp>
#include <string>
#include <tmxlite/Layer.hpp>
#include <tmxlite/Object.hpp>
#include <tmxlite/ObjectGroup.hpp>

#define PORTAL_COOLDOWN_TICKS 60

enum Direction { Undetermined, Top, Bottom, Left, Right };

struct Collider {
  const tmx::Object &collidingObject;
  Direction direction;
};

const Vec2 SpawnPoint = {5, 5};

class Game {
  std::shared_ptr<sf::RenderWindow> window_;
  const std::vector<std::string> levels_ = {"main.tmx", "1.tmx", "2.tmx",
                                            "3.tmx",    "4.tmx", "5.tmx"};
  int current_level_index_;
  tmx::Map map_;
  std::shared_ptr<EntityManager> entities_;
  std::shared_ptr<Player> player_;
  std::shared_ptr<StandbyPortal> sbportal_;
  sf::Texture background_texture_;
  sf::Sprite background_sprite_;
  bool paused_ = false;
  bool running_ = false;
  unsigned int last_portal_switch_ = 0;
  unsigned int last_portal_fired_ = 0;
  unsigned int current_frame_ = 0;
  unsigned int hearts_;
  sf::Texture heart_texture_;
  std::vector<std::shared_ptr<sf::Sprite>> heart_sprites_;
  sf::Font font_;

  void sMovement();
  void sUserInput(sf::Event event);
  void sRender();
  void DrawText(const std::string &str, const sf::Vector2f &pos,
                unsigned int size);
  void DrawInput();
  void RenderEntityOutline(std::shared_ptr<Entity> e);
  void sCollision(const tmx::ObjectGroup &collision_layer,
                  const tmx::ObjectGroup &death_layer,
                  const tmx::ObjectGroup &advancement_layer,
                  const tmx::Map &map);

  void sPortals(const std::unique_ptr<tmx::Layer> &portallable_layer,
                const tmx::Map &map);
  void SpawnPlayer();
  void UpdateStandbyPortal();
  void UpdateMidairPortals();
  void firePortal();
  bool aabbCollisionCheck(BoundingBox &first, BoundingBox &second);
  void HandleEntitiesCollision(std::shared_ptr<Entity> e1,
                               std::shared_ptr<Entity> e2);
  std::vector<tmx::Object>
  GetObjGroupColliders(std::shared_ptr<Entity> entity,
                       const tmx::ObjectGroup &collisionLayer);
  bool IsEntityOutOfBounds(const std::shared_ptr<Entity> entity,
                           const tmx::Map &map);
  void HandleEntityOutOfBounds(const std::shared_ptr<Entity> entity);
  Collider IdentifyCollider(const std::shared_ptr<Entity> entity,
                            const tmx::Object &collidingObject);
  void HandleEntityCollisionWithMap(const std::shared_ptr<Entity> entity,
                                    const Collider &collider);
  bool IsBottomCollider(const std::shared_ptr<Entity> entity,
                        const tmx::Object &collidingObject);
  bool IsTopCollider(const std::shared_ptr<Entity> entity,
                     const tmx::Object &collidingObject);
  bool IsLeftCollider(const std::shared_ptr<Entity> entity,
                      const tmx::Object &collidingObject);
  bool IsRightCollider(const std::shared_ptr<Entity> entity,
                       const tmx::Object &collidingObject);
  bool ShouldPlaceStandingEntityOnCollider(const std::shared_ptr<Entity> entity,
                                           const tmx::Object &collider);
  void Init();
  void
  HandleEntityCollisionWithDeathLayer(const std::shared_ptr<Entity> entity);
  void HandleEntityCollisionWithAdvancementLayer(
      const std::shared_ptr<Entity> entity);
  void RenderHearts();
  void ResetGame();

public:
  Game();
  void Run();
};

#endif // !PORTAL2D_GAME_H_
