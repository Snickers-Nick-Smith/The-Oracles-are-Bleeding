#ifndef ROOM_HPP
#define ROOM_HPP

#include <string>
#include <vector>

class Room {
private:
    std::string name;
    std::string description;
    bool hasShrine;
    int shrineID; // -1 if no shrine
    std::vector<std::string> objects;
    bool visited;

public:
    Room(const std::string& n, const std::string& d, bool shrine = false, int id = -1);
    std::string getName() const;
    std::string getDescription() const;
    bool shrinePresent() const;
    int getShrineID() const;
    const std::vector<std::string>& getObjects() const;
    bool isVisited() const;
    void markVisited();
    bool isShrine() const { return hasShrine; }
};

#endif
