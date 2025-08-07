// JournalManager.hpp (Split System with Lysaia/Melas Journals)
#ifndef JOURNALMANAGER_HPP
#define JOURNALMANAGER_HPP

#include <string>
#include <vector>

struct JournalEntry {
    std::string content;       // Canonical journal entry
    std::string playerNote;    // Player-authored note (Melas only)

    JournalEntry(const std::string& c, const std::string& note = "")
        : content(c), playerNote(note) {}
};

class JournalManager {
private:
    std::vector<JournalEntry> lysaiaEntries;
    std::vector<JournalEntry> melasEntries;

    bool showLysaiaJournal = false; // Controls access during Melas' run

public:
    // --- Lysaia-only journal (read-only) ---
    void writeLysaia(const std::string& entry);
    void viewLysaia() const;
    void unlockLysaiaJournal();

    // --- Melas journal (interactive) ---
    void writeMelas(const std::string& entry);
    void addPlayerNoteToMelas(int index, const std::string& note);
    void viewMelas() const;

    // --- Shared I/O ---
    void saveToFile(const std::string& filename = "assets/journal.txt") const;
    void loadFromFile(const std::string& filename = "assets/journal.txt");

    void writeCorrupted(); // For Melas hallucinations
};

#endif

