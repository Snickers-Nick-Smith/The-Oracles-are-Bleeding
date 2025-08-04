#include "Player.hpp"

// --- Constructor ---
Player::Player() : currentRoom(0), sanity(100) {}

// --- Basic Access ---
int Player::getCurrentRoom() const { return currentRoom; }
void Player::setCurrentRoom(int index) { currentRoom = index; }
int Player::getSanity() const { return sanity; }

void Player::loseSanity(int amount) {
    sanity -= amount;
    if (sanity < 0) sanity = 0;
}

// --- Journal Integration ---
void Player::writeToJournal(const std::string& entry) {
    journal.write(entry);
}

void Player::writeCorruptedToJournal() {
    journal.writeCorrupted();
}

void Player::viewJournal() const {
    journal.view();
}

void Player::saveJournalToFile() const {
    journal.saveToFile();
}

void Player::loadJournalFromFile() {
    journal.loadFromFile();
}
