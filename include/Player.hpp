#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>
#include <vector>

class Player {
private:
    int currentRoom;
    int sanity;
    std::vector<std::string> inventory;
    std::vector<std::string> journal;

public:
    Player();
    int getCurrentRoom() const;
    void setCurrentRoom(int index);
    int getSanity() const;
    void loseSanity(int amount);
    void writeToJournal(const std::string& entry);
};

#endif
