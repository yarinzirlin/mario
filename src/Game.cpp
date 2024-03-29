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
#include <tmxlite/Object.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/TileLayer.hpp>

#define DEFAULT_PLAYER_HORIZONTAL_VELOCITY 2
#define PLAYER_JUMP_VELOCITY 5
#define FRAMERATE_LIMIT 240
#define GRAVITY_ACCELERATION 0.1f

Game::Game() {
  entities_ = std::make_shared<EntityManager>();
  window_ =
      std::make_shared<sf::RenderWindow>(sf::VideoMode(1280, 720), "Mario Jump");
  current_level_index_ = 0;
  paused_ = true;
  running_ = false;
  hearts_ = 3;

  background_texture_.loadFromFile("assets/backgrounds/retro.png");
  background_sprite_.setTexture(background_texture_);

  font_.loadFromFile("assets/fonts/arial.ttf");
  auto window_size = window_->getSize();

// Scale the sprite to fit the window
  background_sprite_.setScale(
    float(window_size.x) / background_texture_.getSize().x,
    float(window_size.y) / background_texture_.getSize().y
);
  sf::Image heart_img;
  heart_img.loadFromFile("assets/general/heart.png");
  heart_sprites_ = {std::make_shared<sf::Sprite>(),
                    std::make_shared<sf::Sprite>(),
                    std::make_shared<sf::Sprite>()};
  heart_texture_.loadFromImage(heart_img);

  for (int i = 0; i < heart_sprites_.size(); i++) {

    heart_sprites_[i]->setTexture(heart_texture_);
    heart_sprites_[i]->setScale(0.03, 0.03);

    heart_sprites_[i]->setPosition(
        window_size.x - window_size.x / 20.f -
            (i * heart_sprites_[i]->getGlobalBounds().width),
        window_size.y / 100.f);
  }

  game_progress_ = std::make_shared<GameProgress>();
  game_progress_->loadProgress(current_level_index_, hearts_);

  Init();
}

void Game::Init() {

  paused_ = false;
  running_ = true;
  current_frame_ = 0;
  window_->create(sf::VideoMode(1280, 720), "Mario Jump");
  window_->setFramerateLimit(FRAMERATE_LIMIT);
  current_frame_ = 0;

  map_.load("assets/maps/" + std::string(levels_[current_level_index_]));
  SpawnPlayer();
  DEBUGLOG("Initialization finished")
}

#define COLLISION_LAYER "Collision"
#define DEATH_LAYER "Death"
#define LEVEL_ADVANCEMENT_LAYER "NextLevel"
void Game::Run() {

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
    const auto &layers = map_.getLayers();
    tmx::ObjectGroup collision_layer;
    tmx::ObjectGroup death_layer;
    tmx::ObjectGroup advancement_layer;

    for (const auto &layer : layers) {
      if (layer->getName() == COLLISION_LAYER)
        collision_layer = layer->getLayerAs<tmx::ObjectGroup>();
      if (layer->getName() == DEATH_LAYER)
        death_layer = layer->getLayerAs<tmx::ObjectGroup>();
      if (layer->getName() == LEVEL_ADVANCEMENT_LAYER)
        advancement_layer = layer->getLayerAs<tmx::ObjectGroup>();
    }
    sCollision(collision_layer, death_layer, advancement_layer, map_);
    sRender();
    current_frame_++;
  }
}

void Game::sRender() {
  window_->clear();
  MapLayer layerZero(map_, 0);
  window_->draw(background_sprite_);
  window_->draw(layerZero);
  for (auto e : entities_->getEntities()) {
    e->UpdateAnimation(current_frame_);
    e->sprite()->setPosition(e->transform_->pos_.x, e->transform_->pos_.y);
    e->sprite()->setRotation(e->transform_->angle_);
    // Set flip state
    auto curTextRect = e->sprite()->getTextureRect();
    if (e->transform_->flipped_ && curTextRect.width >= 0) {
      e->sprite()->setTextureRect(
          sf::IntRect(curTextRect.left + curTextRect.width, curTextRect.top,
                      -curTextRect.width, curTextRect.height));
    } else if (!e->transform_->flipped_ && curTextRect.width < 0) {
      e->sprite()->setTextureRect(
          sf::IntRect(curTextRect.left + curTextRect.width, curTextRect.top,
                      -curTextRect.width, curTextRect.height));
    }
#if DEBUG
    RenderEntityOutline(e);
    DrawInput();
#endif
    
    window_->draw(*e->sprite());
  }
  RenderHearts();
  window_->display();
}

void Game::DrawInput() {
  std::ostringstream oss;
  oss << "L: " << std::boolalpha << player_->cInput->left_
      << ", R: " << std::boolalpha << player_->cInput->right_
      << ", J: " << std::boolalpha << player_->cInput->jump_
      << ", F: " << std::boolalpha << player_->cInput->fire_
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

void Game::sCollision(const tmx::ObjectGroup &collision_layer,
                      const tmx::ObjectGroup &death_layer,
                      const tmx::ObjectGroup &advancement_layer,
                      const tmx::Map &map) {

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

    auto colliding_objects = GetObjGroupColliders(e1, collision_layer);
    std::vector<Collider> colliders;
    bool has_bottom_collider = false;
    for (const auto &obj : colliding_objects) {
      auto identified_collider = IdentifyCollider(e1, obj);
      if (identified_collider.direction == Bottom) {
        has_bottom_collider = true;
      }
      colliders.push_back(identified_collider);
    }
    for (auto collider : colliders) {
      HandleEntityCollisionWithMap(e1, collider);
    }

    auto colliding_death_objects = GetObjGroupColliders(e1, death_layer);
    if (colliding_death_objects.size() > 0) {
      HandleEntityCollisionWithDeathLayer(e1);
    }

    auto colliding_advancement_objects =
        GetObjGroupColliders(e1, advancement_layer);
    if (colliding_advancement_objects.size() > 0) {
      HandleEntityCollisionWithAdvancementLayer(e1);
    }

    if (!has_bottom_collider) {
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
  if (IsBottomCollider(entity, collidingObject)) {
    collider.direction = Bottom;
  } else if (IsTopCollider(entity, collidingObject))
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

void Game::HandleEntityCollisionWithDeathLayer(
    const std::shared_ptr<Entity> entity) {
  if (entity->tag() == "player") {
    entity->transform_->pos_ = SpawnPoint;
    entity->transform_->velocity_ = {0.0, 0.0};
    hearts_--;
    game_progress_->saveProgress(current_level_index_, hearts_);
  }

  if (hearts_ <= 0) {
    ResetGame();
  }
}

void Game::HandleEntityCollisionWithAdvancementLayer(
    const std::shared_ptr<Entity> entity) {
  current_level_index_++;
  if (current_level_index_ >= levels_.size()) {
    current_level_index_ = 0;
    hearts_ = 3;
    
  }
  game_progress_->saveProgress(current_level_index_, hearts_);

  
  auto next_level_path =
      "assets/maps/" + std::string(levels_[current_level_index_]);
  DEBUGLOG(next_level_path)
  map_.load(next_level_path);
  player_->transform_->pos_ = SpawnPoint;
  player_->transform_->velocity_ = {0.0, 0.0};
}
void Game::HandleEntityCollisionWithMap(const std::shared_ptr<Entity> entity,
                                        const Collider &collider) {
  switch (collider.direction) {
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

  // Handling top collision
  case Top:
    entity->transform_->pos_.y = collider.collidingObject.getAABB().top +
                                 collider.collidingObject.getAABB().height;
    entity->transform_->velocity_.y = 0;
    break;
  // Handling bottom collision
  case Bottom:
    entity->transform_->pos_.y =
        collider.collidingObject.getAABB().top - entity->bb().height;
    entity->transform_->velocity_.y = 0;
    entity->set_midair(false);
    break;
  case Undetermined:
    break;
  }
}

bool Game::IsBottomCollider(const std::shared_ptr<Entity> entity,
                            const tmx::Object &collidingObject) {

  auto new_bottom = entity->bb().top + entity->bb().height;
  auto prev_bottom = entity->prev_bb().top + entity->prev_bb().height;
  auto collider_top = collidingObject.getAABB().top;
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
         (prev_left >= collider_right && new_left < collider_right);
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
    DEBUGLOG(e1->tag() << " is colliding with " << e2->tag())
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

  for (auto e : entities_->getEntities()) {
    e->transform_->pos_ += e->transform_->velocity_;
    if (e->affected_by_gravity() && e->midair()) {
      e->transform_->velocity_.y += GRAVITY_ACCELERATION;
    }
  }
}


void Game::SpawnPlayer() {
  player_ = entities_->addPlayer();
  player_->transform_->pos_ = SpawnPoint;
}

void Game::RenderHearts() {
  for (int i = 0; i < hearts_; i++) {
    window_->draw(*heart_sprites_[i]);
  }
}
void Game::ResetGame() {
  hearts_ = 3;
  current_level_index_ = 0;
  game_progress_->saveProgress(current_level_index_, hearts_);
  map_.load("assets/maps/" + std::string(levels_[current_level_index_]));
}
