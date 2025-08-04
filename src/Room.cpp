#include "Room.hpp"

Room::Room(const std::string& n, const std::string& d, bool shrine)
    : name(n), description(d), hasShrine(shrine) {}

std::string Room::getName() const {
    return name;
}

std::string Room::getDescription() const {
    return description;
}

bool Room::shrinePresent() const {
    return hasShrine;
}
