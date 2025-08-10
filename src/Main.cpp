#include "Game.hpp"
#include <ctime>
int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    Game game;
    game.displayMainMenu();
    return 0;
}

