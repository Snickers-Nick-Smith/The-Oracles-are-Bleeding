#ifndef GAME_HPP
#define GAME_HPP

#include "Player.hpp"
#include "Room.hpp"
#include "Shrine.hpp"
#include "SceneManager.hpp"
#include "Map.hpp" 
#include "utils.hpp"
#include "Theme.hpp"       
#include <unordered_map>
#include <vector>
#include <string>

class Game {
private:
    Player player;
    std::vector<Room> rooms;
    std::unordered_map<int, std::unordered_map<std::string,int>> roomConnections;
    JournalManager journalManager;
    void setupConnections();
    std::unordered_map<int, Shrine> shrineRegistry;
    bool isRunning;
    TempleMap templeMap; 
    // utility: guess deity by shrine name or room name
    Deity deityFromShrineName(const std::string& name) const;
    Deity deityFromRoomName(const std::string& name) const;
    // print/stylize environmental text (not character dialogue)
    void printShrineText(const Shrine& shrine,
                         const std::string& text,
                         bool shake = false,
                         int intensity = 2,
                         int durationMs = 200);
    void printRoomDescriptionColored(const Room& room,
                                     const std::string& description);
    void loadRooms();
    void gameLoop();
    void handleCommand(const std::string& input);
    void toggleAccessibility();
    void showMap();
    // ← add this: default accessibility for printing
    AccessibilitySettings accessibility_{ /*colorEnabled*/true,
                                          /*screenShakeEnabled*/true,
                                          /*textSpeed*/2 };

public:
    Game();
    void startLysaiaPlaythrough();
    void lysaiaDay(int day, Shrine& shrine);
    void displayMainMenu();
    void start();
    void describeCurrentRoom();
    void setAccessibility(const AccessibilitySettings& as) { accessibility_ = as; }
    void printShrineText(const Shrine& shrine,
                         const std::string& text,
                         bool shake = false,
                         int intensity = 2,
                         int durationMs = 200);

    void printRoomDescriptionColored(const Room& room,
                                     const std::string& description);
    };

#endif
