#ifndef PORTAL2D_UTILS_H_
#define PORTAL2D_UTILS_H_

#include <SFML/Graphics/Rect.hpp>
#include <iostream>
#include <tmxlite/Layer.hpp>
#include <tmxlite/Map.hpp>
#include <tmxlite/Types.hpp>

#if DEBUG
#define DEBUGLOG(x) std::cout << "[*] " << x << std::endl;
#else
#define DEBUGLOG(x)
#endif

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
#endif
