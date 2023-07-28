#ifndef PORTAL2D_ENTITY_H_
#define PORTAL2D_ENTITY_H_

#include "Components.hpp"
#include "Vec2.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transform.hpp>
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
  virtual ~Entity(){};
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

enum PortalColor : char { Green = 'g', Purple = 'p', None };
const std::string PortalsBasePath = "resources/portals/";
class StandbyPortal : public Entity {
  friend class EntityManager;
  PortalColor m_portalColor = Purple;
  const Vec2 m_resourcePos = Vec2(212, 85);
  const Vec2 m_resourceSize = Vec2(25, 25);
  sf::Texture m_alternateTexture;

  StandbyPortal(size_t id) : Entity("standby_portal", id) {
    m_texture.loadFromFile(PortalsBasePath + static_cast<char>(Purple) +
                           ".png");
    m_alternateTexture.loadFromFile(PortalsBasePath + static_cast<char>(Green) +
                                    ".png");

    m_sprite.setTexture(m_texture);
    m_sprite.setTextureRect(sf::IntRect(m_resourcePos.x, m_resourcePos.y,
                                        m_resourceSize.x, m_resourceSize.y));
    m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f,
                       m_sprite.getLocalBounds().height / 2.0f);
  }

public:
  void AlternateColor() { std::swap(m_texture, m_alternateTexture); }
};

#define InvalidFrame -99
#define PhaseDuration 40

class MidairPortal : public Entity {
  friend class EntityManager;
  PortalColor m_portalColor;
  sf::Texture m_alternateTexture;
  short m_curPhase = -1;
  const Vec2 m_baseResourcePos = Vec2(16, 92);
  const std::vector<Vec2> m_phasesPosOffsets = {Vec2(0, 0), Vec2(62, 0),
                                                Vec2(62 * 2, 0)};
  const Vec2 m_size = Vec2(35, 12);
  unsigned int m_lastPhaseChange = InvalidFrame;
  MidairPortal(size_t id) : Entity("midair_portal", id) {
    m_texture.loadFromFile(PortalsBasePath + static_cast<char>(Purple) +
                           ".png");
    m_alternateTexture.loadFromFile(PortalsBasePath + static_cast<char>(Green) +
                                    ".png");
    m_sprite.setTexture(m_texture);
    m_sprite.setTextureRect(sf::IntRect(
        m_baseResourcePos.x, m_baseResourcePos.y, m_size.x, m_size.y));
    m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.0f,
                       m_sprite.getLocalBounds().height / 2.0f);
  }

  void SetLastPhaseChange(unsigned int frame) {
    if (m_lastPhaseChange != InvalidFrame)
      return;
    m_lastPhaseChange = frame;
  }

public:
  unsigned int GetLastPhaseChange() { return m_lastPhaseChange; };
  void NextPhase() {
    if (m_curPhase >= m_phasesPosOffsets.size() - 1) {
      return;
    }
    m_curPhase++;
    m_sprite.setTextureRect(
        sf::IntRect(m_baseResourcePos.x + m_phasesPosOffsets[m_curPhase].x,
                    m_baseResourcePos.x + m_phasesPosOffsets[m_curPhase].y,
                    m_size.x, m_size.y));
  }
};

#endif
