#include "EntityManager.hpp"
#include <iostream>

// Todo make this use templates instead of "Entity"
std::shared_ptr<Entity> EntityManager::addEntity(const std::string &tag) {
  auto e = std::shared_ptr<Entity>(new Entity(tag, m_totalEntities++));
  return addEntity(e);
}
std::shared_ptr<Entity>
EntityManager::addEntity(std::shared_ptr<Entity> entity) {
  m_toAdd.push_back(entity);
  return entity;
}

std::shared_ptr<Player> EntityManager::addPlayer() {
  if (m_playerCreated) {
    return nullptr;
  }
  auto player = std::shared_ptr<Player>(new Player(m_totalEntities++));

  addEntity(player);

  return player;
}
void EntityManager::update() {
  for (auto e : m_toAdd) {
    std::cout << "[*] Entity added <" << e->tag() << ">" << std::endl;
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
