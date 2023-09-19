#ifndef PORTAL2D_ENTITY_H_
#define PORTAL2D_ENTITY_H_

#include "BoundingBox.hpp"
#include "Components.hpp"
#include "Utils.hpp"
#include "Vec2.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>

class Entity {
  friend class EntityManager;
  const size_t id_ = 0;
  const std::string tag_ = "Default";

  bool alive_ = true;

protected:
  Entity(const std::string &tag, size_t id) : id_(id), tag_(tag) {
    transform_ = std::make_shared<CTransform>();
  }
  sf::Texture texture_;
  sf::Sprite sprite_;
  bool affected_by_gravity_ = false;
  bool midair_ = true;
  sf::FloatRect prev_bb_;

public:
  std::shared_ptr<CTransform> transform_;
  void destroy() { alive_ = false; }
  const std::string &tag() { return tag_; }
  int id() { return id_; }
  sf::Sprite &sprite() { return sprite_; }
  sf::FloatRect bb() {
    sf::FloatRect bb;
    bb.top = transform_->pos_.y - sprite_.getOrigin().y;
    bb.left = transform_->pos_.x - sprite_.getOrigin().x;
    bb.width = sprite_.getGlobalBounds().width;
    bb.height = sprite_.getGlobalBounds().height;
    return bb;
  }
  sf::FloatRect prev_bb() { return prev_bb_; }
  sf::FloatRect feet_bb() {
    auto feet_bb = bb();
    feet_bb.top += feet_bb.height;
    feet_bb.height /= 5.f;
    feet_bb.top -= feet_bb.height;
    return feet_bb;
  }
  bool affected_by_gravity() { return affected_by_gravity_; }
  bool midair() { return midair_; }
  void set_midair(bool midair) {
    DEBUGLOG("set " << tag() << "midair to " << midair) midair_ = midair;
  }
  float width() { return sprite_.getLocalBounds().width; }
  float height() { return sprite_.getLocalBounds().height; }
  float top() { return sprite_.getLocalBounds().top; }
  float left() { return sprite_.getLocalBounds().left; }
  void set_prev_bb(const sf::FloatRect &prev_bb) { prev_bb_ = prev_bb; }
  virtual ~Entity(){};
};

class Player : public Entity {
  friend class EntityManager;
  Player(size_t id) : Entity("player", id) {
    texture_.loadFromFile("assets/player/chell.png");
    sprite_.setTexture(texture_);
    sprite_.setScale(0.15f, 0.15f);
    // sprite_.setOrigin(sprite_.getLocalBounds().width / 2.0f,
    //                    sprite_.getLocalBounds().height / 2.0f);
    cInput = std::make_shared<CInput>();
    affected_by_gravity_ = true;
  }

public:
  std::shared_ptr<CInput> cInput;
};

#define INIT_PORTAL_COLOR Purple
enum PortalColor : char { Green = 'g', Purple = 'p', None };
const std::string PortalsBasePath = "assets/portals/";
class StandbyPortal : public Entity {
  friend class EntityManager;
  PortalColor portal_color_ = INIT_PORTAL_COLOR;
  const Vec2 resource_pos_ = Vec2(213, 89);
  const Vec2 resource_size_ = Vec2(19, 19);
  sf::Texture alternate_texture_;

  StandbyPortal(size_t id) : Entity("standby_portal", id) {
    texture_.loadFromFile(PortalsBasePath + static_cast<char>(Purple) + ".png");
    alternate_texture_.loadFromFile(PortalsBasePath + static_cast<char>(Green) +
                                    ".png");

    sprite_.setTexture(texture_);
    sprite_.setTextureRect(sf::IntRect(resource_pos_.x, resource_pos_.y,
                                       resource_size_.x, resource_size_.y));
    sprite_.setOrigin(sprite_.getLocalBounds().width / 2.0f,
                      sprite_.getLocalBounds().height / 2.0f);
    sprite_.setScale(0.75, 0.75f);
  }

public:
  void AlternateColor() {
    std::swap(texture_, alternate_texture_);
    if (portal_color_ == Purple) {
      portal_color_ = Green;
    } else {
      portal_color_ = Purple;
    }
  }
  PortalColor portal_color() { return portal_color_; }
};

#define PhaseDuration 10

class MidairPortal : public Entity {
  friend class EntityManager;
  PortalColor portal_color_ = INIT_PORTAL_COLOR;
  sf::Texture altername_texture_;
  int cur_phase_ = -1;
  const Vec2 base_resource_pos_ = Vec2(16, 90);
  const std::vector<Vec2> phases_pos_offsets_ = {Vec2(0, 0), Vec2(62, 0),
                                                 Vec2(62 * 2, 0)};
  const Vec2 size_ = Vec2(35, 12);
  unsigned int last_phase_change_ = 0;
  MidairPortal(size_t id) : Entity("midair_portal", id) {
    texture_.loadFromFile(PortalsBasePath + static_cast<char>(Purple) + ".png");
    altername_texture_.loadFromFile(PortalsBasePath + static_cast<char>(Green) +
                                    ".png");
    sprite_.setTexture(texture_);
    sprite_.setScale(2.f, 2.f);
  }
  void SetTexture() {
    sprite_.setTextureRect(
        sf::IntRect(base_resource_pos_.x + phases_pos_offsets_[cur_phase_].x,
                    base_resource_pos_.y + phases_pos_offsets_[cur_phase_].y,
                    size_.x, size_.y));
    sprite_.setOrigin(sprite_.getLocalBounds().width / 2.0f,
                      sprite_.getLocalBounds().height / 2.0f);
  }

public:
  unsigned int last_phase_change() { return last_phase_change_; };

  void NextPhase(unsigned int current_frame) {
    if (cur_phase_ >= (int)phases_pos_offsets_.size() - 1) {
      return;
    }
    cur_phase_++;
    SetTexture();
    last_phase_change_ = current_frame;
  }
  void AlternateColor() {
    std::swap(texture_, altername_texture_);
    if (portal_color_ == Purple) {
      portal_color_ = Green;
    } else {
      portal_color_ = Purple;
    }
  }
  PortalColor portal_color() { return portal_color_; }
};

#endif
