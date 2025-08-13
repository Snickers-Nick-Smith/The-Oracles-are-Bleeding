// JournalManager.hpp (Location-aware, dual journals, hallucinations)
#ifndef JOURNALMANAGER_HPP
#define JOURNALMANAGER_HPP

#include <string>
#include <vector>
#include <iosfwd> 
#include <unordered_map>

// A single journal page as shown to the player
struct JournalEntry {
    std::string content;       // Canonical journal text (what the game writes)
    std::string originalContent; // Canonical journal text (overwritten by hallucination)
    std::string playerNote;    // Player-authored note (Melas only)
    JournalEntry(const std::string& c = "", const std::string& note = "")
        : content(c), playerNote(note) {}
};

struct EntryData {
    std::string actual;
    std::string hallucination{};
};




class JournalManager {
private:
    // Separate ledgers
    std::vector<JournalEntry> lysaiaEntries;   // read-only to player
    std::vector<JournalEntry> melasEntries;    // player can annotate

    // Location → entry data (actual + hallucination)
    std::unordered_map<std::string, EntryData> locationEntries;

    bool showLysaiaJournal = false; // access gate during Melas run
    void maybeCorruptOneOnView();
public:
    // -----------------------------
    // Setup / Definitions
    // -----------------------------
    void defineLocationEntry(const std::string& id,
                             const std::string& actual,
                             const std::string& hallucination);

    // Convenience: preload all defaults we know (shrines + rooms)
    void loadDefaultLocationEntries();

    // -----------------------------
    // Lysaia journal (read-only)
    // -----------------------------
    void seedLysaiaPrologueText();                    // preload shrine + guilt texts
    void writeLysaiaGuiltBeat(int day);               // optional day-specific guilt beat
    void writeLysaia(const std::string& entry);          // free-form append
    void writeLysaiaAt(const std::string& locationID);   // from registry
    void viewLysaia() const;
    void inspectEntry(int index) const;
    void unlockLysaiaJournal();
    bool hasLysaia() const;
    void printLysaia(std::ostream& out) const;
    void printLastLysaia(std::ostream& out) const;

    // -----------------------------
    // Melas journal (interactive)
    // -----------------------------
    void writeMelas(const std::string& entry);                 // free-form + 50% generic hallucination
    void writeMelasAt(const std::string& locationID, bool forceHallucination = false); // from registry + 50% hallucination
    void addPlayerNoteToMelas(int index, const std::string& note);
    void viewMelas() const;
    void printJournal();

    // -----------------------------
    // File I/O (Melas only for now)
    // -----------------------------
    void saveToFile(const std::string& filename = "assets/journal.txt") const;
    void loadFromFile(const std::string& filename = "assets/journal.txt");

    // -----------------------------
    // Hallucinations
    // -----------------------------
    void writeCorrupted();                    // generic, random line
    void writeCorruptedLine(const std::string& line); // explicit line
};

#endif // JOURNALMANAGER_HPP
