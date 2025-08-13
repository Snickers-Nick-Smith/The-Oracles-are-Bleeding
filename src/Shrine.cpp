// Shrine.cpp
#include "Shrine.hpp"
#include <iostream>

// Constructor
Shrine::Shrine(const std::string& deity, const std::string& shrineRoom)
    : deityName(deity), shrineRoomName(shrineRoom) {}

// State
void Shrine::setState(ShrineState newState) { state = newState; }
ShrineState Shrine::getState() const { return state; }

// Names
const std::string& Shrine::getName() const noexcept { return deityName; }
std::string Shrine::getDeityName() const { return deityName; }
std::string Shrine::getShrineRoomName() const { return shrineRoomName; }

// Rooms
void Shrine::addAssociatedRoom(const Room& room) { associatedRooms.push_back(room); }
const std::vector<Room>& Shrine::getAssociatedRooms() const { return associatedRooms; }

// Presentation
void Shrine::describeShrine() const {
    std::cout << "Shrine of " << deityName << " — " << shrineRoomName << "\n";
    std::cout << "Current state: "
              << (state == ShrineState::UNCORRUPTED ? "Uncorrupted" : "Corrupted")
              << "\n";
    std::cout << "You feel a presence...\n";

    if (!associatedRooms.empty()) {
        std::cout << "\nAssociated rooms:\n";
        for (const Room& r : associatedRooms) {
            std::cout << "- " << r.getName() << "\n";
        }
    }
}

// Simple interaction
void Shrine::activate(Player& /*player*/) {
    describeShrine();

    if (state == ShrineState::CORRUPTED) {
        std::cout << "The shrine hums with something unnatural.\n";
    } else {
        std::cout << "The shrine feels sacred. Familiar. Almost peaceful.\n";
    }

    std::cout << "You kneel. You listen. Nothing answers.\n";
}
