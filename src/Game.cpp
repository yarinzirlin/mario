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
  m_entities = std::make_shared<EntityManager>();
  m_window =
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
  m_window->create(sf::VideoMode(1280, 720), "Portal 2D");
  m_window->setFramerateLimit(FRAMERATE_LIMIT);
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
  m_window->clear();
  MapLayer layerZero(map, 0);
  // MapLayer layerOne(map, 1);
  m_window->draw(layerZero);
  // m_window->draw(layerOne);
  for (auto e : m_entities->getEntities()) {
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
    m_window->draw(e->sprite());
  }
  m_window->display();
}

void Game::DrawInput() {
  std::ostringstream oss;
  oss << "L: " << std::boolalpha << m_player->cInput->left_
      << ", R: " << std::boolalpha << m_player->cInput->right_
      << ", J: " << std::boolalpha << m_player->cInput->jump_
      << ", F: " << std::boolalpha << m_player->cInput->fire_
      << ", S: " << std::boolalpha << m_player->cInput->switch_portal_
      << ", U: " << std::boolalpha << m_player->cInput->up_
      << ", MA: " << std::boolalpha << m_player->midair();
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
  m_window->draw(text);
}

void Game::RenderEntityOutline(std::shared_ptr<Entity> e) {
  sf::RectangleShape outline;
  outline.setPosition(e->bb().left, e->bb().top);
  outline.setSize(sf::Vector2f(e->bb().width, e->bb().height));
  outline.setFillColor(sf::Color::Transparent);
  outline.setOutlineColor(sf::Color::Black);
  outline.setOutlineThickness(1.0f);
  m_window->draw(outline);
}

void Game::sCollision(const std::unique_ptr<tmx::Layer> &collision_layer,
                      const tmx::Map &map) {
  const auto &collision_object_group =
      collision_layer->getLayerAs<tmx::ObjectGroup>();

  for (auto e1 : m_entities->getEntities()) {
    for (auto e2 : m_entities->getEntities()) {
      if (e1 == e2)
        continue;
      auto bb1 = e1->bb();
      auto bb2 = e2->bb();
      if (bb1.intersects(bb2)) {
        HandleEntitiesCollision(e1, e2);
      }
    }

    auto colliders = GetObjGroupColliders(e1, collision_object_group);
    for (auto collider : colliders) {
      HandleEntityCollisionWithMap(e1, collider);
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
    if (object_bounding_box.intersects(entity->bb())) {
      colliders.push_back(obj);
    }
  }
  return colliders;
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
                                        const tmx::Object &collidingObject) {
  DEBUGLOG(entity->tag() << " is colliding with "
                         << collidingObject.getPosition())
  if (IsBottomCollider(entity, collidingObject)) {

    DEBUGLOG("Collision with ground detected, checking if feet are high enough")
    entity->transform_->pos_.y =
        collidingObject.getAABB().top - entity->bb().height + 1;
    entity->transform_->velocity_.y = 0;
    DEBUGLOG("Placing entity on collider ******")
    entity->set_midair(false);
  } else {
    entity->set_midair(true);
  }
  if (IsRightCollider(entity, collidingObject)) {
    DEBUGLOG("Right collider")
    entity->transform_->pos_.x =
        collidingObject.getAABB().left - entity->bb().width - 1;
    entity->transform_->velocity_.x = 0;
  }
  if (IsLeftCollider(entity, collidingObject)) {
    DEBUGLOG("Left collider")
    entity->transform_->pos_.x =
        collidingObject.getAABB().left + collidingObject.getAABB().width + 1;
    entity->transform_->velocity_.x = 0;
  }
  if (IsTopCollider(entity, collidingObject)) {
    DEBUGLOG("Top collider")
    entity->transform_->pos_.y =
        collidingObject.getAABB().top + collidingObject.getAABB().height - 1;
    entity->transform_->velocity_.y = 0;
  }
}

bool Game::IsBottomCollider(const std::shared_ptr<Entity> entity,
                            const tmx::Object &collidingObject) {

  auto prev_bottom = entity->prev_bb().top + entity->prev_bb().height;
  auto new_bottom = entity->bb().top + entity->bb().height;
  return prev_bottom <= collidingObject.getAABB().top &&
         new_bottom > collidingObject.getAABB().top;
}
bool Game::IsTopCollider(const std::shared_ptr<Entity> entity,
                         const tmx::Object &collidingObject) {

  auto prev_top = entity->prev_bb().top;
  auto new_top = entity->bb().top;
  return prev_top <=
             collidingObject.getAABB().top + collidingObject.getAABB().height &&
         new_top >
             collidingObject.getAABB().top + collidingObject.getAABB().height;
}
bool Game::IsLeftCollider(const std::shared_ptr<Entity> entity,
                          const tmx::Object &collidingObject) {

  auto prev_left = entity->prev_bb().left;
  auto new_left = entity->bb().left;
  return prev_left <=
             collidingObject.getAABB().left + collidingObject.getAABB().width &&
         new_left >
             collidingObject.getAABB().left + collidingObject.getAABB().width;
}
bool Game::IsRightCollider(const std::shared_ptr<Entity> entity,
                           const tmx::Object &collidingObject) {

  auto prev_right = entity->prev_bb().left + entity->prev_bb().width;
  auto new_right = entity->bb().left + entity->bb().width;
  return prev_right <= collidingObject.getAABB().left &&
         new_right > collidingObject.getAABB().left;
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
    m_player->cInput->left_ = event.type == sf::Event::KeyPressed;
    break;
  case sf::Keyboard::Right:
    m_player->cInput->right_ = event.type == sf::Event::KeyPressed;
    break;
  case sf::Keyboard::Up:
    m_player->cInput->up_ = event.type == sf::Event::KeyPressed;
    break;
  case sf::Keyboard::F:
    m_player->cInput->fire_ = event.type == sf::Event::KeyPressed;
    break;
  case sf::Keyboard::C:
    m_player->cInput->switch_portal_ = event.type == sf::Event::KeyPressed;
    break;
  case sf::Keyboard::Space:
    m_player->cInput->jump_ = event.type == sf::Event::KeyPressed;
  defualt:
    break;
  }
}

void Game::sMovement() {
  if (m_player->cInput->right_) {
    m_player->transform_->velocity_.x = DEFAULT_PLAYER_HORIZONTAL_VELOCITY;
    m_player->transform_->flipped_ = false;
  }
  if (m_player->cInput->left_) {
    m_player->transform_->velocity_.x = -DEFAULT_PLAYER_HORIZONTAL_VELOCITY;
    m_player->transform_->flipped_ = true;
  }
  if (!m_player->cInput->left_ && !m_player->cInput->right_)
    m_player->transform_->velocity_.x = 0;

  if (!m_player->midair() && m_player->cInput->jump_) {
    m_player->transform_->velocity_.y = -PLAYER_JUMP_VELOCITY;
    m_player->set_midair(true);
  }

  if (m_player->cInput->fire_) {
    firePortal();
  }

  for (auto e : m_entities->getEntities()) {
    e->transform_->pos_ += e->transform_->velocity_;
    if (e->affected_by_gravity() && e->midair()) {
      e->transform_->velocity_.y += GRAVITY_ACCELERATION;
    }
  }
  UpdateStandbyPortal();
  UpdateMidairPortals();
}

void Game::UpdateStandbyPortal() {
  int x_offset = m_player->bb().width + sbportal_->bb().width / 2.25f;
  if (m_player->transform_->flipped_) {
    x_offset -= m_player->bb().width;
    x_offset *= -1;
  }
  sbportal_->transform_->flipped_ = m_player->transform_->flipped_;
  Vec2 sbportal_pos;
  sbportal_pos.x = m_player->bb().left + x_offset;
  sbportal_pos.y = m_player->bb().top + m_player->bb().height / 2.f;
  sbportal_->transform_->pos_ = sbportal_pos;

  if (m_player->cInput->switch_portal_ &&
      current_frame_ > last_portal_switch_ + FRAMERATE_LIMIT / 10) {
    last_portal_switch_ = current_frame_;
    sbportal_->AlternateColor();
  }
}

void Game::UpdateMidairPortals() {
  for (auto e : m_entities->getEntities("midair_portal")) {
    auto p = std::dynamic_pointer_cast<MidairPortal>(e);

    if (p->last_phase_change() + PhaseDuration < current_frame_) {
      p->NextPhase(current_frame_);
    }
  }
}

void Game::SpawnPlayer() {
  m_player = m_entities->addPlayer();
  m_player->transform_->pos_ = Vec2(5, 5);
  sbportal_ = m_entities->addEntity<StandbyPortal>();
}

void Game::firePortal() {
  if (last_portal_fired_ + PORTAL_COOLDOWN_TICKS > current_frame_) {
    return;
  }
  last_portal_fired_ = current_frame_;
  auto portal = m_entities->addEntity<MidairPortal>();
  portal->transform_->pos_ = sbportal_->transform_->pos_;
  portal->transform_->velocity_ = Vec2(PORTAL_VELOCITY, 0);
  if (portal->portal_color() != sbportal_->portal_color()) {
    portal->AlternateColor();
  }

  if (m_player->transform_->flipped_) {
    portal->transform_->velocity_ *= -1;
  }
  portal->NextPhase(current_frame_);
}
