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
    // Melas can add notes and receive hallucinations
    // (writeMelas already handles the 50% generic hallucination)
    journal.writeMelas(entry);
}

void Player::writeCorruptedToJournal() {
    journal.writeCorrupted();
}

void Player::viewJournal() const {
    // During Melas run we show the interactive journal
    journal.viewMelas();
}

void Player::saveJournalToFile() const {
    journal.saveToFile(); // uses default "assets/journal.txt"
}

void Player::loadJournalFromFile() {
    journal.loadFromFile();
}


