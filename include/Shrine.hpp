#ifndef SHRINE_HPP
#define SHRINE_HPP

#include <string>
#include <vector>
#include "Player.hpp"
#include "Room.hpp"

enum class ShrineState {
    UNCORRUPTED,
    CORRUPTED
};

class Shrine {
private:
    std::string deityName;
    std::string shrineRoomName;
    ShrineState state;
    std::vector<Room> associatedRooms;

public:
    Shrine(const std::string& deity, const std::string& shrineRoom);

    void setState(ShrineState newState);
    ShrineState getState() const;

    std::string getDeityName() const;
    std::string getShrineRoomName() const;

    void addAssociatedRoom(const Room& room);
    const std::vector<Room>& getAssociatedRooms() const;

    void describeShrine() const;
    void activate(Player& player); // dynamic interaction

    static void activateByID(int shrineID, Player& player); // optional legacy static method
};

#endif
