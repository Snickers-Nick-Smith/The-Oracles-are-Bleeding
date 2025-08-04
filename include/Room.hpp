#ifndef ROOM_HPP
#define ROOM_HPP

#include <string>

class Room {
private:
    std::string name;
    std::string description;
    bool hasShrine;

public:
    Room(const std::string& name, const std::string& description, bool shrine = false);
    std::string getName() const;
    std::string getDescription() const;
    bool shrinePresent() const;
};

#endif
