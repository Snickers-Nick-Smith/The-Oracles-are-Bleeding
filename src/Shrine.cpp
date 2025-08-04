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


player.writeToJournal("I knelt at the altar. The wheat whispered a name I no longer answer to.");
if (player.getSanity() < 40) {
    player.writeToJournal("I saw something behind me, written in my own voice.");
    player.writeCorruptedToJournal(); // or just: player.journal.writeCorrupted();
}
