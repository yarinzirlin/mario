#ifndef PORTAL2D_ENTITY_H_
#define PORTAL2D_ENTITY_H_

#include "Components.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <Vec2.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

class Entity {
  friend class EntityManager;
  const size_t m_id = 0;
  const std::string m_tag = "Default";
  bool m_alive = true;

protected:
  Entity(const std::string &tag, size_t id) : m_id(id), m_tag(tag) {
    cTransform = std::make_shared<CTransform>();
  }
  sf::Texture m_texture;
  sf::Sprite m_sprite;

public:
  std::shared_ptr<CTransform> cTransform;
  void destroy() { m_alive = false; }
  const std::string &tag() { return m_tag; }
  int id() { return m_id; }
  sf::Sprite &sprite() { return m_sprite; }
};

class Player : public Entity {
  friend class EntityManager;
  Player(size_t id) : Entity("player", id) {
    m_texture.loadFromFile("resources/player/chell.png");
    m_sprite.setTexture(m_texture);
    m_sprite.setScale(0.25f, 0.25f);
    m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f,
                       m_sprite.getLocalBounds().height / 2.0f);
    cInput = std::make_shared<CInput>();
  }

public:
  std::shared_ptr<CInput> cInput;
};

enum PortalColor : char { Green = 'g', Purple = 'p' };

class StandbyPortal : public Entity {
  friend class EntityManager;
  PortalColor m_portalColor = Purple;
  const std::string m_portalsBasePath = "resources/portals/";
  const Vec2 m_resourcePosition = Vec2(212, 85);
  const Vec2 m_resourceSize = Vec2(25, 25);
  sf::Texture m_alternateTexture;

  StandbyPortal(size_t id) : Entity("standby_portal", id) {
    m_texture.loadFromFile(m_portalsBasePath + static_cast<char>(Purple));
    m_alternateTexture.loadFromFile(m_portalsBasePath +
                                    static_cast<char>(Green));

    m_sprite.setTexture(m_texture);
    m_sprite.setTextureRect(sf::IntRect(m_resourcePosition.x,
                                        m_resourcePosition.y, m_resourceSize.x,
                                        m_resourceSize.y));
  }

  void AlternateColor() { std::swap(m_texture, m_alternateTexture); }
};

#endif
