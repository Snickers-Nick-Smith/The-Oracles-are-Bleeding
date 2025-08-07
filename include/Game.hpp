#ifndef GAME_HPP
#define GAME_HPP

#include "Player.hpp"
#include "Room.hpp"
#include "Shrine.hpp"
#include "SceneManager.hpp"
#include "utils.hpp"
#include <unordered_map>
#include <vector>
#include <string>

class Game {
private:
    Player player;
    std::vector<Room> rooms;
    std::unordered_map<int, Shrine> shrineRegistry;
    bool isRunning;

    void loadRooms();
    void gameLoop();
    void handleCommand(const std::string& input);
    void displayMainMenu();
    void toggleAccessibility();

public:
    Game();
    void startLysaiaPlaythrough();
    void lysaiaDay(int day, Shrine& shrine);
    void start();
};


#endif