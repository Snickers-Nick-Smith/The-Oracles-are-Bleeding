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

    // Getters
    const std::string& getName() const noexcept;           // deity name
    ShrineState getState() const;
    void setState(ShrineState newState);

    // If you prefer no copies, you can switch this to const std::string& too.
    std::string getDeityName() const;
    std::string getShrineRoomName() const;

    void addAssociatedRoom(const Room& room);
    const std::vector<Room>& getAssociatedRooms() const;

    void describeShrine() const;
    void activate(Player& player); // dynamic interaction

    static void activateByID(int shrineID, Player& player); // (declare only if you plan to define it)
};

#endif
