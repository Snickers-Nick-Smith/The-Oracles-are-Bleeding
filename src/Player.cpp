#include "Player.hpp"
#include "utils.hpp"

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

// --- Movement ---
void Player::move(const std::string& direction,
                  const std::unordered_map<int, std::unordered_map<std::string,int>>& roomConnections) {
    auto rcIt = roomConnections.find(currentRoom);
    if (rcIt == roomConnections.end()) { std::cout << "You can't move from here.\n"; return; }
    const auto& exits = rcIt->second;

    const std::string longDir  = normalize_dir(direction); // "n"->"north" or "" if invalid
    const std::string shortDir = to_short_dir(longDir.empty() ? direction : longDir);

    if (longDir.empty() && shortDir.empty()) { std::cout << "No exit that way.\n"; return; }

    auto it = exits.find(longDir);
    if (it == exits.end()) it = exits.find(shortDir);
    if (it == exits.end()) { std::cout << "No exit that way.\n"; return; }

    currentRoom = it->second;
}

// --- Journal Integration ---
void Player::writeToJournal(const std::string& entry) {
    // Melas can add notes and receive hallucinations
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
    journal.saveToFile("assets/journal.txt");
}

void Player::loadJournalFromFile() {
    journal.loadFromFile("assets/journal.txt");
}

void Player::writeMelasAt(const std::string& locationID, bool forceHallucination) {
    journal.writeMelasAt(locationID, forceHallucination);
}

void Player::addJournalNote(int entryIndexOneBased, const std::string& note) {
    // Journal expects zero-based index
    journal.addPlayerNoteToMelas(entryIndexOneBased - 1, note);
}

void Player::printJournal() {
    journal.printJournal();
}


