// This file contains the implementation of [specific functionality]

#ifndef PORTAL2D_ENTITY_H_
#define PORTAL2D_ENTITY_H_

#include "BoundingBox.hpp"
#include "Components.hpp"
#include "Utils.hpp"
#include "Vec2.hpp"

#include "Animation.hpp"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <ios>
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
  std::shared_ptr<Animation> animation_;

protected:
  Entity(const std::string &tag, size_t id) : id_(id), tag_(tag) {
    transform_ = std::make_shared<CTransform>();
  }
  bool affected_by_gravity_ = false;
  bool should_destroy_if_obb_ = false;
  bool midair_ = true;
  sf::FloatRect prev_bb_;
  std::shared_ptr<Animation> default_animation_;
  std::shared_ptr<Animation> moving_animation_;
  std::shared_ptr<Animation> midair_animation_;
  sf::Texture texture_;

public:
  std::shared_ptr<CTransform> transform_;
  void destroy() {
    alive_ = false;
    DEBUGLOG("Destroyed <" << tag() << ">")
  }
  const std::string &tag() { return tag_; }
  int id() { return id_; }
  const std::shared_ptr<sf::Sprite> sprite() {
    if (animation_) {
      return animation_->sprite();
    }
    return default_animation_->sprite();
  }
  sf::FloatRect bb() {
    sf::FloatRect bb;
    bb.top = transform_->pos_.y - sprite()->getOrigin().y;
    bb.left = transform_->pos_.x - sprite()->getOrigin().x;
    bb.width = sprite()->getGlobalBounds().width;
    bb.height = sprite()->getGlobalBounds().height;
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
  bool should_destroy_if_obb() { return should_destroy_if_obb_; }
  bool midair() { return midair_; }
  void set_midair(bool midair) {
    if (midair_ == midair) {
      return;
    }
    DEBUGLOG("set " << tag() << "midair to " << midair)
    midair_ = midair;
  }
  void set_prev_bb(const sf::FloatRect &prev_bb) { prev_bb_ = prev_bb; }
  bool alive() { return alive_; }
  void UpdateAnimation(unsigned int current_frame) {
    animation_ = default_animation_;
    if (midair() && midair_animation_) {
      animation_ = midair_animation_;
    } else if (transform_->velocity_.x != 0 && moving_animation_) {
      animation_ = moving_animation_;
    }
    animation_->Update(current_frame);
  }
  virtual ~Entity(){};
};

class Player : public Entity {
  friend class EntityManager;
  Player(size_t id) : Entity("player", id) {
    sf::Image img;
    img.loadFromFile("assets/player/player.png");
    img.createMaskFromColor({0, 116, 116, 255});
    auto sprite = std::make_shared<sf::Sprite>();
    texture_.loadFromImage(img);
    sprite->setTexture(texture_);
    sprite->setScale(2.f, 2.f);

    std::vector<sf::IntRect> idling_rects = {{964, 48, 16, 24}};
    default_animation_ = std::make_shared<Animation>(sprite, idling_rects, 0);

    std::vector<sf::IntRect> walking_rects = {{808, 48, 16, 24},
                                              {808 - 52, 48, 16, 24},
                                              {808 - (52 * 2), 48, 16, 24}};
    moving_animation_ = std::make_shared<Animation>(sprite, walking_rects, 15);

    std::vector<sf::IntRect> jumping_rects = {{912, 112, 16, 24}};
    midair_animation_ = std::make_shared<Animation>(sprite, jumping_rects);
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
    std::shared_ptr<sf::Sprite> sprite = std::make_shared<sf::Sprite>();
    sprite->setTexture(texture_);
    sprite->setOrigin(resource_size_.x / 2.0f, resource_size_.y / 2.0f);
    sprite->setScale(0.75, 0.75f);
    sprite->setTextureRect(sf::IntRect(resource_pos_.x, resource_pos_.y,
                                       resource_size_.x, resource_size_.y));
    std::vector<sf::IntRect> default_rects = {{213, 89, 19, 19}};
    default_animation_ = std::make_shared<Animation>(sprite, default_rects, 0);
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
    std::shared_ptr<sf::Sprite> sprite = std::make_shared<sf::Sprite>();
    sprite->setTexture(texture_);
    sprite->setScale(2.f, 2.f);
    std::vector<sf::IntRect> animation_rects = {
        {16, 90, 35, 12}, {16 + 62, 90, 35, 12}, {16 + (62 * 2), 90, 35, 12}};
    default_animation_ =
        std::make_shared<Animation>(sprite, animation_rects, 10, false);
    should_destroy_if_obb_ = true;
  }

public:
  unsigned int last_phase_change() { return last_phase_change_; };

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

class ActivePortal : public Entity {
  friend class EntityManager;
  PortalColor color_;

protected:
  ActivePortal(const std::string &tag, size_t id, PortalColor color)
      : Entity(tag, id) {
    color_ = color;
    // texture_.loadFromFile(PortalsBasePath + static_cast<char>(color_) +
    // ".png");
  }
};
class ActivePurplePortal : public ActivePortal {
  friend class EntityManager;
};
class ActiveGreenPortal : public ActivePortal {
  friend class EntityManager;
};

#endif
