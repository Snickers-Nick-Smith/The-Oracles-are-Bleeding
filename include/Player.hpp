#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "JournalManager.hpp"
#include <vector>
#include <string>

class Player {
private:
    int currentRoom;
    int sanity;
    std::vector<std::string> inventory;
    JournalManager journal;

public:
    Player();
    int getCurrentRoom() const;
    void setCurrentRoom(int index);
    int getSanity() const;
    void loseSanity(int amount);

    // Journal controls
    void writeToJournal(const std::string& entry);
    void writeCorruptedToJournal();
    void viewJournal() const;
    void saveJournalToFile() const;
    void loadJournalFromFile();
    void writeMelasAt(const std::string& locationID, bool forceHallucination = false);
    void addJournalNote(int entryIndexOneBased, const std::string& note);
    void printJournal();
    void inspectJournalEntry(int index) const { journal.inspectEntry(index); }


};

#endif