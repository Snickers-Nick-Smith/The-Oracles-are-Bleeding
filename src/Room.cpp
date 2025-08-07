#include "Room.hpp"

// Constructor
Room::Room(const std::string& n, const std::string& d, bool shrine, int id)
    : name(n), description(d), hasShrine(shrine), shrineID(id), visited(false) {}

// Getters
std::string Room::getName() const {
    return name;
}

std::string Room::getDescription() const {
    return description;
}

bool Room::shrinePresent() const {
    return hasShrine;
}

int Room::getShrineID() const {
    return shrineID;
}

const std::vector<std::string>& Room::getObjects() const {
    return objects;
}

bool Room::isVisited() const {
    return visited;
}

void Room::markVisited() {
    visited = true;
}

