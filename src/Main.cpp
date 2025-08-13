#include "Game.hpp"
#include <ctime>
int main() {
    enableVTSupport();  // harmless on POSIX, best-effort on Windows
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    Game game;
    game.start();
    return 0;
}

