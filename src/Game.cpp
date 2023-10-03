#include "Game.hpp"
#include "Entity.hpp"
#include "SFMLOrthogonalLayer.hpp"
#include "Utils.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <iostream>
#include <memory>
#include <tmxlite/Layer.hpp>
#include <tmxlite/Map.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/TileLayer.hpp>

#define DEFAULT_PLAYER_HORIZONTAL_VELOCITY 2
#define PLAYER_JUMP_VELOCITY 5
#define FRAMERATE_LIMIT 240
#define PORTAL_VELOCITY 15
#define GRAVITY_ACCELERATION 0.1f

Game::Game() {
  entities_ = std::make_shared<EntityManager>();
  window_ =
      std::make_shared<sf::RenderWindow>(sf::VideoMode(1280, 720), "Portal 2D");
  paused_ = true;
  running_ = false;
  font_.loadFromFile("assets/fonts/arial.ttf");
  Init();
}

void Game::Init() {

  paused_ = false;
  running_ = true;
  current_frame_ = 0;
  window_->create(sf::VideoMode(1280, 720), "Portal 2D");
  window_->setFramerateLimit(FRAMERATE_LIMIT);
  current_frame_ = 0;

  // m_backgroundTexture.loadFromFile("resources/backgrounds/4.png");
  // m_backgroundSprite.setTexture(m_backgroundTexture);
  // m_backgroundSprite.setScale(
  //     m_window->getSize().x / m_backgroundSprite.getLocalBounds().width,
  //     m_window->getSize().y / m_backgroundSprite.getLocalBounds().height);

  SpawnPlayer();
  DEBUGLOG("Initialization finished")
}

tmx::Map map;
#define COLLISION_LAYER "Collision"
void Game::Run() {
  map.load("assets/maps/test.tmx");
  DEBUGLOG(map.getTileSize())
  auto ts = map.getTilesets();
  for (auto t : ts) {
    auto margin = t.getMargin();
    DEBUGLOG("margin" << margin)
    auto spacing = t.getSpacing();
    DEBUGLOG("spacing" << spacing)
    auto props = t.getProperties();
    for (auto p : props) {
      DEBUGLOG("prop " << p.getName())
    }
    auto tilesize = t.getTileSize();
    DEBUGLOG("tilesize" << tilesize)
    auto tileone = t.getTile(1);
    DEBUGLOG("tileone" << tileone)
  }
  while (window_->isOpen()) {
    entities_->update();
    sf::Event event;
    while (window_->pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window_->close();
      }
      sUserInput(event);
    }
    sMovement();
    const auto &layers = map.getLayers();
    for (const auto &layer : layers) {
      if (layer->getName() == COLLISION_LAYER)
        sCollision(layer, map);
    }
    sRender();
    current_frame_++;
  }
}

void Game::sRender() {
  window_->clear();
  MapLayer layerZero(map, 0);
  // MapLayer layerOne(map, 1);
  window_->draw(layerZero);
  // m_window->draw(layerOne);
  for (auto e : entities_->getEntities()) {
    e->sprite().setPosition(e->transform_->pos_.x, e->transform_->pos_.y);
    e->sprite().setRotation(e->transform_->angle_);
    // Set flip state
    auto curTextRect = e->sprite().getTextureRect();
    if (e->transform_->flipped_ && curTextRect.width >= 0) {
      e->sprite().setTextureRect(
          sf::IntRect(curTextRect.left + curTextRect.width, curTextRect.top,
                      -curTextRect.width, curTextRect.height));
    } else if (!e->transform_->flipped_ && curTextRect.width < 0) {
      e->sprite().setTextureRect(
          sf::IntRect(curTextRect.left + curTextRect.width, curTextRect.top,
                      -curTextRect.width, curTextRect.height));
    }
#if DEBUG
    RenderEntityOutline(e);
    DrawInput();
#endif
    window_->draw(e->sprite());
  }
  window_->display();
}

void Game::DrawInput() {
  std::ostringstream oss;
  oss << "L: " << std::boolalpha << player_->cInput->left_
      << ", R: " << std::boolalpha << player_->cInput->right_
      << ", J: " << std::boolalpha << player_->cInput->jump_
      << ", F: " << std::boolalpha << player_->cInput->fire_
      << ", S: " << std::boolalpha << player_->cInput->switch_portal_
      << ", U: " << std::boolalpha << player_->cInput->up_
      << ", MA: " << std::boolalpha << player_->midair();
  DrawText(oss.str(), sf::Vector2f(0, 0), 24);
}

void Game::DrawText(const std::string &str, const sf::Vector2f &pos,
                    unsigned int size) {

  sf::Text text;
  text.setFont(font_);
  text.setString(str);
  text.setPosition(pos);
  text.setCharacterSize(size);
  text.setFillColor(sf::Color::Black);
  window_->draw(text);
}

void Game::RenderEntityOutline(std::shared_ptr<Entity> e) {
  sf::RectangleShape outline;
  outline.setPosition(e->bb().left, e->bb().top);
  outline.setSize(sf::Vector2f(e->bb().width, e->bb().height));
  outline.setFillColor(sf::Color::Transparent);
  outline.setOutlineColor(sf::Color::Black);
  outline.setOutlineThickness(1.0f);
  window_->draw(outline);
}

void Game::sCollision(const std::unique_ptr<tmx::Layer> &collision_layer,
                      const tmx::Map &map) {
  DEBUGLOG("~~~~In collision~~~~")
  const auto &collision_object_group =
      collision_layer->getLayerAs<tmx::ObjectGroup>();

  for (auto e1 : entities_->getEntities()) {
    for (auto e2 : entities_->getEntities()) {
      if (e1 == e2)
        continue;
      auto bb1 = e1->bb();
      auto bb2 = e2->bb();
      if (Intersects(bb1, bb2)) {
        HandleEntitiesCollision(e1, e2);
      }
    }

    auto colliding_objects = GetObjGroupColliders(e1, collision_object_group);
    std::vector<Collider> colliders;
    bool has_bottom_collider = false;
    for (auto obj : colliding_objects) {
      auto collider = IdentifyCollider(e1, obj);
      if (collider.direction == Bottom) {
        has_bottom_collider = true;
      }
      colliders.push_back(collider);
    }
    if (e1->tag() == "player") {
      DEBUGLOG("colliders:" << colliders.size())
    }
    for (auto collider : colliders) {
      HandleEntityCollisionWithMap(e1, collider);
    }

    if (!has_bottom_collider) {
      DEBUGLOG(e1->tag() << " no bottom collider")
      e1->set_midair(true);
    }
    if (IsEntityOutOfBounds(e1, map)) {
      HandleEntityOutOfBounds(e1);
    }
  }
}

std::vector<tmx::Object>
Game::GetObjGroupColliders(std::shared_ptr<Entity> entity,
                           const tmx::ObjectGroup &collision_layer) {
  std::vector<tmx::Object> colliders;
  for (const auto &obj : collision_layer.getObjects()) {
    auto object_bounding_box = BBTmxToSFML(obj.getAABB());
    if (Intersects(object_bounding_box, entity->bb())) {
      colliders.push_back(obj);
    }
  }
  return colliders;
}
Collider Game::IdentifyCollider(const std::shared_ptr<Entity> entity,
                                const tmx::Object &collidingObject) {
  Collider collider = {.collidingObject = collidingObject};
  if (IsBottomCollider(entity, collidingObject))
    collider.direction = Bottom;
  else if (IsTopCollider(entity, collidingObject))
    collider.direction = Top;
  else if (IsRightCollider(entity, collidingObject))
    collider.direction = Right;
  else if (IsLeftCollider(entity, collidingObject))
    collider.direction = Left;
  else
    collider.direction = Undetermined;
  return collider;
}

bool Game::IsEntityOutOfBounds(const std::shared_ptr<Entity> entity,
                               const tmx::Map &map) {
  auto bounds = map.getBounds();
  auto entity_bb = entity->bb();

  return entity_bb.left > bounds.width ||
         entity_bb.left + entity_bb.width < bounds.left ||
         entity_bb.top > bounds.height ||
         entity_bb.top + entity_bb.height < bounds.top;
}

void Game::HandleEntityOutOfBounds(const std::shared_ptr<Entity> entity) {
  if (entity->should_destroy_if_obb()) {
    entity->destroy();
  }
}

void Game::HandleEntityCollisionWithMap(const std::shared_ptr<Entity> entity,
                                        const Collider &collider) {
  DEBUGLOG(entity->tag() << " is colliding with "
                         << collider.collidingObject.getPosition())
  switch (collider.direction) {
  case Top:
    entity->transform_->pos_.y = collider.collidingObject.getAABB().top +
                                 collider.collidingObject.getAABB().height;
    entity->transform_->velocity_.y = 0;
    break;
  case Bottom:
    PrintFloatRect(entity->bb());
    entity->transform_->pos_.y =
        collider.collidingObject.getAABB().top - entity->bb().height;
    entity->transform_->velocity_.y = 0;
    entity->set_midair(false);
    break;

  case Left:
    entity->transform_->pos_.x = collider.collidingObject.getAABB().left +
                                 collider.collidingObject.getAABB().width + 1;
    entity->transform_->velocity_.x = 0;
    break;
  case Right:
    entity->transform_->pos_.x =
        collider.collidingObject.getAABB().left - entity->bb().width - 1;
    entity->transform_->velocity_.x = 0;

    break;
  }
}

bool Game::IsBottomCollider(const std::shared_ptr<Entity> entity,
                            const tmx::Object &collidingObject) {

  auto prev_bottom = entity->prev_bb().top + entity->prev_bb().height;
  auto new_bottom = entity->bb().top + entity->bb().height;
  auto collider_top = collidingObject.getAABB().top;
  DEBUGLOG("prev: " << prev_bottom << " new: " << new_bottom
                    << " collider: " << collider_top)
  return FloatEquals(new_bottom, collider_top) ||
         (prev_bottom <= collider_top && new_bottom > collider_top);
}
bool Game::IsTopCollider(const std::shared_ptr<Entity> entity,
                         const tmx::Object &collidingObject) {

  auto prev_top = entity->prev_bb().top;
  auto new_top = entity->bb().top;
  auto collider_bottom =
      collidingObject.getAABB().top + collidingObject.getAABB().height;
  return FloatEquals(new_top, collider_bottom) ||
         (prev_top <= collider_bottom && new_top > collider_bottom);
}
bool Game::IsLeftCollider(const std::shared_ptr<Entity> entity,
                          const tmx::Object &collidingObject) {

  auto prev_left = entity->prev_bb().left;
  auto new_left = entity->bb().left;
  auto collider_right =
      collidingObject.getAABB().left + collidingObject.getAABB().width;
  return FloatEquals(new_left, collider_right) ||
         (prev_left <= collider_right && new_left > collider_right);
}
bool Game::IsRightCollider(const std::shared_ptr<Entity> entity,
                           const tmx::Object &collidingObject) {

  auto prev_right = entity->prev_bb().left + entity->prev_bb().width;
  auto new_right = entity->bb().left + entity->bb().width;
  auto collider_left = collidingObject.getAABB().left;
  return FloatEquals(new_right, collider_left) ||
         (prev_right <= collider_left && new_right > collider_left);
}

bool Game::ShouldPlaceStandingEntityOnCollider(
    const std::shared_ptr<Entity> entity, const tmx::Object &collider) {
  auto prev_bottom = entity->prev_bb().top + entity->prev_bb().height;
  auto new_bottom = entity->bb().top + entity->bb().height;
  return prev_bottom <= collider.getAABB().top &&
         new_bottom > collider.getAABB().top;
}

void Game::HandleEntitiesCollision(std::shared_ptr<Entity> e1,
                                   std::shared_ptr<Entity> e2) {
  if (e1->tag() != "standby_portal" && e2->tag() != "standby_portal") {
    DEBUGLOG(e1->tag() << " is colliding with " << e2->tag())
  }
}

void Game::sUserInput(sf::Event event) {

  switch (event.key.code) {
  case sf::Keyboard::Left:
    player_->cInput->left_ = event.type == sf::Event::KeyPressed;
    break;
  case sf::Keyboard::Right:
    player_->cInput->right_ = event.type == sf::Event::KeyPressed;
    break;
  case sf::Keyboard::Up:
    player_->cInput->up_ = event.type == sf::Event::KeyPressed;
    break;
  case sf::Keyboard::F:
    player_->cInput->fire_ = event.type == sf::Event::KeyPressed;
    break;
  case sf::Keyboard::C:
    player_->cInput->switch_portal_ = event.type == sf::Event::KeyPressed;
    break;
  case sf::Keyboard::Space:
    player_->cInput->jump_ = event.type == sf::Event::KeyPressed;
  defualt:
    break;
  }
}

void Game::sMovement() {
  if (player_->cInput->right_) {
    player_->transform_->velocity_.x = DEFAULT_PLAYER_HORIZONTAL_VELOCITY;
    player_->transform_->flipped_ = false;
  }
  if (player_->cInput->left_) {
    player_->transform_->velocity_.x = -DEFAULT_PLAYER_HORIZONTAL_VELOCITY;
    player_->transform_->flipped_ = true;
  }
  if (!player_->cInput->left_ && !player_->cInput->right_)
    player_->transform_->velocity_.x = 0;

  if (!player_->midair() && player_->cInput->jump_) {
    player_->transform_->velocity_.y = -PLAYER_JUMP_VELOCITY;
  }

  if (player_->cInput->fire_) {
    firePortal();
  }

  for (auto e : entities_->getEntities()) {
    e->transform_->pos_ += e->transform_->velocity_;
    if (e->affected_by_gravity() && e->midair()) {
      e->transform_->velocity_.y += GRAVITY_ACCELERATION;
    }
  }
  // UpdateStandbyPortal();
  UpdateMidairPortals();
}

void Game::UpdateStandbyPortal() {
  int x_offset = player_->bb().width + sbportal_->bb().width / 2.25f;
  if (player_->transform_->flipped_) {
    x_offset -= player_->bb().width;
    x_offset *= -1;
  }
  sbportal_->transform_->flipped_ = player_->transform_->flipped_;
  Vec2 sbportal_pos;
  sbportal_pos.x = player_->bb().left + x_offset;
  sbportal_pos.y = player_->bb().top + player_->bb().height / 2.f;
  sbportal_->transform_->pos_ = sbportal_pos;

  if (player_->cInput->switch_portal_ &&
      current_frame_ > last_portal_switch_ + FRAMERATE_LIMIT / 10) {
    last_portal_switch_ = current_frame_;
    sbportal_->AlternateColor();
  }
}

void Game::UpdateMidairPortals() {
  for (auto e : entities_->getEntities("midair_portal")) {
    auto p = std::dynamic_pointer_cast<MidairPortal>(e);

    if (p->last_phase_change() + PhaseDuration < current_frame_) {
      p->NextPhase(current_frame_);
    }
  }
}

void Game::SpawnPlayer() {
  player_ = entities_->addPlayer();
  player_->transform_->pos_ = Vec2(5, 5);
  // sbportal_ = entities_->addEntity<StandbyPortal>();
}

void Game::firePortal() {
  if (last_portal_fired_ + PORTAL_COOLDOWN_TICKS > current_frame_) {
    return;
  }
  last_portal_fired_ = current_frame_;
  auto portal = entities_->addEntity<MidairPortal>();
  portal->transform_->pos_ = sbportal_->transform_->pos_;
  portal->transform_->velocity_ = Vec2(PORTAL_VELOCITY, 0);
  // if (portal->portal_color() != sbportal_->portal_color()) {
  //   portal->AlternateColor();
  // }

  if (player_->transform_->flipped_) {
    portal->transform_->velocity_ *= -1;
  }
  portal->NextPhase(current_frame_);
}
