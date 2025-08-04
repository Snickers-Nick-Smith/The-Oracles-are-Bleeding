#ifndef JOURNALMANAGER_HPP
#define JOURNALMANAGER_HPP

#include <string>
#include <vector>

class JournalManager {
private:
    std::vector<std::string> entries;

public:
    void write(const std::string& entry);
    void view() const;
    void saveToFile(const std::string& filename = "assets/journal.txt") const;
    void loadFromFile(const std::string& filename = "assets/journal.txt");

    void writeCorrupted(); // adds a randomized hallucination
};

#endif
