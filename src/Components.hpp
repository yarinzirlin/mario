#ifndef PORTAL2D_COMPONENTS_H_
#define PORTAL2D_COMPONENTS_H_

#include "Vec2.hpp"

class CTransform {
public:
  Vec2 pos = {0.0, 0.0};
  float angle = 0.0f;
  Vec2 velocity = {0.0, 0.0};
  bool flipped = false;

  CTransform(const Vec2 &p, const Vec2 &v, float a, bool f)
      : pos(p), angle(a), velocity(v), flipped(f) {}
  CTransform() {}
};

class CCollision {
public:
  float radius;
};

class CScore {
public:
  int score;
};

class CInput {
public:
  bool up = false;
  bool down = false;
  bool right = false;
  bool left = false;
  bool fire = false;
  bool switchPortal = false;

  CInput() {}
};

#endif
