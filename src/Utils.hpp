#ifndef PORTAL2D_UTILS_H_
#define PORTAL2D_UTILS_H_

#include <SFML/Graphics/Rect.hpp>
#include <tmxlite/Layer.hpp>
#include <tmxlite/Map.hpp>


static sf::FloatRect BBTmxToSFML(const tmx::FloatRect & bb) {
    return sf::FloatRect(bb.left, bb.top, bb.width,
                                    bb.height);
}


#if DEBUG
#define DEBUGLOG(x) std::cout << "[*] " << x << std::endl;
#else
#define DEBUGLOG(x)
#endif





#endif


