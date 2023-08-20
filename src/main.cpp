#include "Game.hpp"
#include "Vec2.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>
#include <iostream>

void MainLoop() {}

int main() {
  auto game = Game();
  game.Run();
  return 0;
}
