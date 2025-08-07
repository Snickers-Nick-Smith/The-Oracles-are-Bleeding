#include "Shrine.hpp"
#include <iostream>

// Constructor
Shrine::Shrine(const std::string& deity, const std::string& shrineRoom)
    : deityName(deity), shrineRoomName(shrineRoom), state(ShrineState::UNCORRUPTED) {}

// State Handling
void Shrine::setState(ShrineState newState) {
    state = newState;
}

ShrineState Shrine::getState() const {
    return state;
}

// Getters
std::string Shrine::getDeityName() const {
    return deityName;
}

std::string Shrine::getShrineRoomName() const {
    return shrineRoomName;
}

void Shrine::addAssociatedRoom(const Room& room) {
    associatedRooms.push_back(room);
}

const std::vector<Room>& Shrine::getAssociatedRooms() const {
    return associatedRooms;
}

// Output logic for shrine (basic for now)
void Shrine::describeShrine() const {
    std::cout << "Shrine of " << deityName << ": " << shrineRoomName << std::endl;
    std::cout << "Current state: " << (state == ShrineState::UNCORRUPTED ? "Uncorrupted" : "Corrupted") << std::endl;
    std::cout << "You feel a presence..." << std::endl;

    std::cout << "\nAssociated rooms:\n";
    for (const Room& r : associatedRooms) {
        std::cout << "- " << r.getName() << std::endl;
    }
}

// Activate shrine: can be extended to call different logic
void Shrine::activate(Player& player) {
    describeShrine();

    if (state == ShrineState::CORRUPTED) {
        std::cout << "The shrine hums with something unnatural.\n";
        // You can add stat changes or dialogue here
    } else {
        std::cout << "The shrine feels sacred. Familiar. Almost peaceful.\n";
        // Early game behavior
    }

    // Placeholder interaction
    std::cout << "You kneel. You listen. Nothing answers.\n";
}
