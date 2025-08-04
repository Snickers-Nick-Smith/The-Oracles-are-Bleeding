#include "Player.hpp"

Player::Player() : currentRoom(0), sanity(100) {}

int Player::getCurrentRoom() const {
    return currentRoom;
}

void Player::setCurrentRoom(int index) {
    currentRoom = index;
}

int Player::getSanity() const {
    return sanity;
}

void Player::loseSanity(int amount) {
    sanity -= amount;
    if (sanity < 0) sanity = 0;
}

void Player::writeToJournal(const std::string& entry) {
    journal.push_back(entry);
    // optional: write to file later
}
