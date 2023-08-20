#ifndef PORTAL2D_MAP_H_
#define PORTAL2D_MAP_H_

#include "Vec2.hpp"
#include <tmxlite/Map.hpp>
class Map {
  tmx::Map m_map;
  Vec2 m_spawnPoint;

public:
  Map(const std::string &assetName);
};

#endif
