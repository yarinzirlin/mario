#include "EntityManager.hpp"
#include "Utils.hpp"
#include <iostream>

std::shared_ptr<Player> EntityManager::addPlayer() {
  if (m_playerCreated) {
    return nullptr;
  }
  auto player = addEntity<Player>();

  return player;
}
void EntityManager::update() {
  for (auto e : m_toAdd) {
    DEBUGLOG("Entity added <" << e->tag() << ">")
    m_entities.push_back(e);
    m_entityMap[e->tag()].push_back(e);
  }
  for (auto e : m_entities) {
  }
  m_toAdd.clear();
}

EntityVec &EntityManager::getEntities() { return m_entities; }

EntityVec &EntityManager::getEntities(const std::string &tag) {
  return m_entityMap[tag];
}
