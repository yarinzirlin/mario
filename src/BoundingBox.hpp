#ifndef MARIO_BOUNDINGBOX_H_
#define MARIO_BOUNDINGBOX_H_

class BoundingBox {
  float m_top, m_left, m_width, m_height;

public:
  BoundingBox(float left_, float top, float width, float height)
      : m_top(top), m_left(left_), m_width(width), m_height(height) {}
  bool CollidesWith(BoundingBox &bb) {

    return m_left < bb.Left() + bb.Width() && m_left + m_width > bb.Left() &&
           m_top < bb.Top() + bb.Height() && m_top + m_height > bb.Top();
  }
  float Top() { return m_top; }
  float Left() { return m_left; }
  float Width() { return m_width; }
  float Height() { return m_height; }
};

#endif
