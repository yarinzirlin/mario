#ifndef PORTAL2D_VEC2_H_
#define PORTAL2D_VEC2_H_

class Vec2 {
public:
  float x = 0, y = 0;

  Vec2();
  Vec2(float x, float y);

  bool operator==(const Vec2 &rhs) const;
  bool operator!=(const Vec2 &rhs) const;

  Vec2 operator+(const Vec2 &rhs) const;
  Vec2 operator-(const Vec2 &rhs) const;
  Vec2 operator*(const float val) const;
  Vec2 operator/(const float val) const;

  void operator+=(const Vec2 &rhs);
  void operator-=(const Vec2 &rhs);
  void operator*=(const float val);
  void operator/=(const float val);

  float dist(const Vec2 &rhs) const;

private:
};

#endif
