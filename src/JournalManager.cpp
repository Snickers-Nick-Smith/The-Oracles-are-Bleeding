#include "JournalManager.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>

void JournalManager::write(const std::string& entry) {
    entries.push_back(entry);
    std::cout << "[Journal updated]\n";
}

void JournalManager::view() const {
    std::cout << "\n--- Journal Entries ---\n";
    for (const std::string& line : entries) {
        std::cout << "- " << line << "\n";
    }
    std::cout << "------------------------\n";
}

void JournalManager::saveToFile(const std::string& filename) const {
    std::ofstream outFile(filename, std::ios::app);
    if (outFile) {
        for (const std::string& line : entries) {
            outFile << line << "\n";
        }
        outFile.close();
    } else {
        std::cerr << "[Error writing to journal file.]\n";
    }
}

void JournalManager::loadFromFile(const std::string& filename) {
    entries.clear();
    std::ifstream inFile(filename);
    std::string line;
    while (std::getline(inFile, line)) {
        if (!line.empty())
            entries.push_back(line);
    }
    inFile.close();
}

void JournalManager::writeCorrupted() {
    std::vector<std::string> corrupted = {
        "The pomegranate was not meant for her.",
        "She spoke my name. It was not my name.",
        "I am the dream she wakes from.",
        "She is listening through the bones.",
        "The mirror blinked first."
    };
    int index = rand() % corrupted.size();
    write(corrupted[index]);
}
