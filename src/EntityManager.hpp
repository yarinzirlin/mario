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

public:
  EntityManager() {}
  void update();
  template <typename T> std::shared_ptr<T> addEntity() {
    static_assert(std::is_base_of<Entity, T>::value,
                  "T must be derived from Entity");
    auto e = std::shared_ptr<T>(new T(m_totalEntities++));
    m_toAdd.push_back(e);
    return e;
  }
  std::shared_ptr<Player> addPlayer();
  EntityVec &getEntities();
  EntityVec &getEntities(const std::string &tag);
};

#endif
