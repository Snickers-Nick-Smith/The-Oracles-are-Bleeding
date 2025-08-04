#ifndef GAME_HPP
#define GAME_HPP

#include "Player.hpp"
#include "Room.hpp"
#include "Shrine.hpp"
#include "SceneManager.hpp"
#include "utils.hpp"
#include <vector>
#include <string>

enum class GameState {
    MENU,
    RUNNING,
    QUITa
};


class Game {
private:
    Player player;
    std::vector<Room> rooms;
    bool isRunning;

    void loadRooms();
    void gameLoop();
    void handleCommand(const std::string& input);

public:
    Game();
    void start();
};

#endif
