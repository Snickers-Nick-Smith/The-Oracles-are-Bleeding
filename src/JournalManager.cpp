// JournalManager.cpp (Split System Implementation)
#include "JournalManager.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>

// --- Lysaia Journal (Read-only) ---

void JournalManager::writeLysaia(const std::string& entry) {
    lysaiaEntries.emplace_back(entry);
    showLysaiaJournal = true; // Enable visibility during Lysaia's playthrough
}

void JournalManager::viewLysaia() const {
    if (!showLysaiaJournal) {
        std::cout << "You have no access to these entries right now.\n";
        return;
    }
    std::cout << "\n--- Lysaia's Journal ---\n";
    for (size_t i = 0; i < lysaiaEntries.size(); ++i) {
        std::cout << "\nDay " << i + 1 << ":\n";
        std::cout << lysaiaEntries[i].content << "\n";
    }
}

void JournalManager::unlockLysaiaJournal() {
    showLysaiaJournal = true;
}

// --- Melas Journal (Interactive) ---

void JournalManager::writeMelas(const std::string& entry) {
    melasEntries.emplace_back(entry);

    // 50% chance to add a hallucination
    if (rand() % 2 == 0) {
        writeCorrupted();
    }
}

void JournalManager::addPlayerNoteToMelas(int index, const std::string& note) {
    if (index >= 0 && index < static_cast<int>(melasEntries.size())) {
        melasEntries[index].playerNote = note;
    }
}

void JournalManager::viewMelas() const {
    std::cout << "\n--- Your Journal ---\n";
    for (size_t i = 0; i < melasEntries.size(); ++i) {
        std::cout << "\nEntry " << i + 1 << ":\n";
        std::cout << melasEntries[i].content << "\n";
        if (!melasEntries[i].playerNote.empty()) {
            std::cout << "\n[Your Note]: " << melasEntries[i].playerNote << "\n";
        }
    }
}

// --- File I/O (shared) ---

void JournalManager::saveToFile(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) return;

    for (const auto& entry : melasEntries) {
        out << "[ENTRY]\n" << entry.content << "\n";
        if (!entry.playerNote.empty()) {
            out << "[NOTE]\n" << entry.playerNote << "\n";
        }
    }
    out.close();
}

void JournalManager::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return;

    std::string line, content, note;
    while (std::getline(in, line)) {
        if (line == "[ENTRY]") {
            std::getline(in, content);
            note = "";
        } else if (line == "[NOTE]") {
            std::getline(in, note);
            melasEntries.emplace_back(content, note);
        }
    }
    in.close();
}

// --- Corrupted Entry (hallucination) ---

void JournalManager::writeCorrupted() {
    static const std::vector<std::string> hallucinations = {
        "She was never gone.",
        "The temple sings when no one listens.",
        "Do not trust what you wrote.",
        "You were warned.",
        "Cassandra is a curse.",
        "You already died once.",
        "The shrines are listening.",
        "There were more of you. There aren't now."
    };
    int index = rand() % hallucinations.size();
    melasEntries.emplace_back("[HALLUCINATION] " + hallucinations[index]);
}
