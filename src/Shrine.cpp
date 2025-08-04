#include "Shrine.hpp"
#include <iostream>

void Shrine::activate(int shrineID, Player& player) {
    switch (shrineID) {
        case 0:
            std::cout << "The shrine bleeds. You hear nothing.\n";
            player.loseSanity(5);
            break;
        default:
            std::cout << "The altar is cold and unresponsive.\n";
    }
}
