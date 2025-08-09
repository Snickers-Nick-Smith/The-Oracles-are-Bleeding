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
    journal.writeMelas(entry);
}

void Player::writeCorruptedToJournal() {
    journal.writeCorrupted();
}

void Player::viewJournal() const {
    journal.viewMelas();
}

void Player::saveJournalToFile() const {
    journal.saveToFile();
}

void Player::loadJournalFromFile() {
    journal.loadFromFile();
}

// ===== 2) Player.cpp — paste these implementations =====
void Player::writeMelasAt(const std::string& locationID, bool forceHallucination) {
    journal.writeMelasAt(locationID, forceHallucination);
}

void Player::addJournalNote(int entryIndexOneBased, const std::string& note) {
    int idx = entryIndexOneBased - 1;
    journal.addPlayerNoteToMelas(idx, note);
}

void Player::printJournal() {
    // Use the mutate-on-view behavior inside JournalManager::printJournal()
    journal.printJournal();
}

