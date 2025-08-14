#include "Game.hpp"
#include <iostream>
#include <ctime>

int main() {
    // Fast, predictable console I/O
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    enableVTSupport();  // harmless on POSIX, best-effort on Windows
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    Game game;
    game.start();
    return 0;
}
