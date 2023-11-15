#ifndef PORTAL2D_UTILS_H_
#define PORTAL2D_UTILS_H_

#include <SFML/Graphics/Rect.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <tmxlite/Layer.hpp>
#include <tmxlite/Map.hpp>
#include <tmxlite/Types.hpp>

#if DEBUG
#define DEBUGLOG(x)                                                            \
  std::cout << std::setprecision(6) << std::setw(10) << "[*] " << x            \
            << std::endl;
#else
#define DEBUGLOG(x)
#endif

#define COMP_EPSILON 0.01

static bool FloatEquals(float a, float b) {
  return fabs(a - b) <= COMP_EPSILON;
}

static bool Intersects(sf::FloatRect f, sf::FloatRect s) {
  auto f_is_to_the_right_of_s =
      f.left > s.left + s.width && !FloatEquals(f.left, s.left + s.width);
  auto f_is_to_the_left_of_s =
      f.left + f.width < s.left && !FloatEquals(f.left + f.width, s.left);
  auto f_is_above_s =
      f.top + f.height < s.top && !FloatEquals(f.top + f.height, s.top);
  auto f_is_below_s =
      f.top > s.top + s.height && !FloatEquals(f.top, s.top + s.height);
  return !(f_is_to_the_right_of_s || f_is_to_the_left_of_s || f_is_above_s ||
           f_is_below_s);
}

static sf::FloatRect BBTmxToSFML(const tmx::FloatRect &bb) {
  return sf::FloatRect(bb.left, bb.top, bb.width, bb.height);
}

static void PrintFloatRect(const sf::FloatRect &fr) {
  DEBUGLOG("T: " << fr.top << ", L: " << fr.left << ", W: " << fr.width
                 << ", H: " << fr.height)
}
static void PrintIntRect(const sf::IntRect &ir) {
  DEBUGLOG("T: " << ir.top << ", L: " << ir.left << ", W: " << ir.width
                 << ", H: " << ir.height)
}
static inline tmx::Colour ColourFromString(std::string str) {
  // removes preceding #
  auto result = str.find_last_of('#');
  if (result != std::string::npos) {
    str = str.substr(result + 1);
  }

  if (str.size() == 6 || str.size() == 8) {
    unsigned int value, r, g, b;
    unsigned int a = 255;
    std::stringstream input(str);
    input >> std::hex >> value;

    r = (value >> 16) & 0xff;
    g = (value >> 8) & 0xff;
    b = value & 0xff;

    if (str.size() == 8) {
      a = (value >> 24) & 0xff;
    }

    return {std::uint8_t(r), std::uint8_t(g), std::uint8_t(b), std::uint8_t(a)};
  }
  DEBUGLOG(str << ": not a valid colour string")
  return {};
}

static std::string LevelIndexToPath(int index) {
  return std::string("assets/maps/") + std::to_string(index + 1) +
         std::string(".tmx");
}

#endif
