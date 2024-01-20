// This file contains the implementation of [specific functionality]

#include "EntityManager.hpp"
#include "Utils.hpp"
#include <__algorithm/remove_if.h>
#include <iostream>
#include <iterator>
#include <memory>

bool IsEntityAlive(std::shared_ptr<Entity> e) { return !e->alive(); }

std::shared_ptr<Player> EntityManager::addPlayer() {
  if (player_created_) {
    return nullptr;
  }
  auto player = addEntity<Player>();

  return player;
}
void EntityManager::update() {
  for (auto e : to_add_) {
    DEBUGLOG("Entity added <" << e->tag() << ">")
    entities_.push_back(e);
    entity_map_[e->tag()].push_back(e);
  }

  auto old_count = entities_.size();
  auto new_end =
      std::remove_if(entities_.begin(), entities_.end(), IsEntityAlive);

  entities_.erase(new_end, entities_.end());
#if DEBUG
  if (old_count != entities_.size()) {
    DEBUGLOG("Destroyed " << old_count - entities_.size() << " entities")
  }
#endif

  // Iterating through each entity for updates
for (auto e : entities_) {
    e->set_prev_bb(e->bb());
  }
  to_add_.clear();
}

EntityVec &EntityManager::getEntities() { return entities_; }

EntityVec &EntityManager::getEntities(const std::string &tag) {
  return entity_map_[tag];
}
