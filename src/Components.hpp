#ifndef PORTAL2D_COMPONENTS_H_
#define PORTAL2D_COMPONENTS_H_

#include "Vec2.hpp"

class CTransform {
public:
  Vec2 pos_ = {0.0, 0.0};
  float angle_ = 0.0f;
  Vec2 velocity_ = {0.0, 0.0};
  bool flipped_ = false;

  CTransform(const Vec2 &p, const Vec2 &v, float a, bool f)
      : pos_(p), angle_(a), velocity_(v), flipped_(f) {}
  CTransform() {}
};

class CCollision {
public:
  float radius_;
};

class CScore {
public:
  int score_;
};

class CInput {
public:
  bool up_ = false;
  bool down_ = false;
  bool right_ = false;
  bool left_ = false;
  bool fire_ = false;
  bool switch_portal_ = false;

  CInput() {}
};

#endif
