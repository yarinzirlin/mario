#ifndef PORTAL2D_ANIMATION_H_
#define PORTAL2D_ANIMATION_H_

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <vector>
class Animation {
  std::shared_ptr<sf::Sprite> sprite_;
  std::vector<sf::IntRect> rects_;
  bool rotation_;
  unsigned int interval_;
  unsigned int last_phase_change_;
  size_t current_rect_;

  size_t NextRectIndex() {
    if (rects_.size() - 1 == current_rect_) {
      if (rotation_) {
        return 0;
      } else {
        return rects_.size() - 1;
      }
    }
    return current_rect_ + 1;
  }

public:
  std::shared_ptr<sf::Sprite> sprite() { return sprite_; }
  Animation(std::shared_ptr<sf::Sprite> sprite, std::vector<sf::IntRect> rects,
            unsigned int interval = -1, bool rotation = true) {
    sprite_ = sprite;
    rects_ = rects;
    interval_ = interval;
    rotation_ = rotation;
    current_rect_ = 0;
    last_phase_change_ = -1;
    sprite->setTextureRect(rects[0]);
  }

  void Update(unsigned int current_tick) {
    if (rects_.size() == 1) {
      if (sprite_->getTextureRect() != rects_[0]) {
        sprite_->setTextureRect(rects_[0]);
      }
      return;
    }

    if (last_phase_change_ != -1 &&
        current_tick - last_phase_change_ < interval_)
      return;
    current_rect_ = NextRectIndex();
    auto next_rect = rects_[current_rect_];
    sprite_->setTextureRect(next_rect);
    last_phase_change_ = current_tick;
  }
};

#endif
