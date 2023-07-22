#ifndef PORTAL2D_ENTITYMANAGER_H_
#define PORTAL2D_ENTITYMANAGER_H_

#include "Entity.hpp"
#include <map>
#include <memory>
#include <vector>

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<std::string, EntityVec> EntityMap;

class EntityManager {
  EntityVec m_entities;
  EntityMap m_entityMap;
  EntityVec m_toAdd;
  size_t m_totalEntities = 0;
  bool m_playerCreated = false;
  std::shared_ptr<Entity> addEntity(std::shared_ptr<Entity> entity);

public:
  EntityManager() {}
  void update();
  std::shared_ptr<Entity> addEntity(const std::string &tag);
  std::shared_ptr<Player> addPlayer();
  EntityVec &getEntities();
  EntityVec &getEntities(const std::string &tag);
};

#endif
