// Shrine.hpp
#ifndef SHRINE_HPP
#define SHRINE_HPP

#include <string>
#include <vector>
#include "Player.hpp"
#include "Room.hpp"
#include "Theme.hpp"   // for ShrineState

class Shrine {
private:
    std::string deityName;
    std::string shrineRoomName;
    ShrineState state = ShrineState::UNCORRUPTED;
    std::vector<Room> associatedRooms;

public:
    Shrine(const std::string& deity, const std::string& shrineRoom);

    // Getters / setters
    const std::string& getName() const noexcept;   // deity name (for UI coloring, etc.)
    std::string getDeityName() const;              // same as getName(), kept for compatibility
    std::string getShrineRoomName() const;

    ShrineState getState() const;
    void setState(ShrineState newState);

    // Room linkage (optional)
    void addAssociatedRoom(const Room& room);
    const std::vector<Room>& getAssociatedRooms() const;

    // Interaction
    void describeShrine() const;
    void activate(Player& player);
};

#endif // SHRINE_HPP
