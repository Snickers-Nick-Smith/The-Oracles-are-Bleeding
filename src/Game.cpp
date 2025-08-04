#include "Game.hpp"
#include <iostream>

Game::Game() : isRunning(true) {}

void Game::start() {
    SceneManager::introScene();
    loadRooms();
    gameLoop();
}

void Game::loadRooms() {
    // Placeholder room for now
    rooms.push_back(Room("The Hall of Hunger", "Withered olive trees claw at the walls..."));
    player.setCurrentRoom(0);
}

void Game::gameLoop() {
    while (isRunning) {
        const Room& currentRoom = rooms[player.getCurrentRoom()];
        std::cout << "\nLocation: " << currentRoom.getName() << "\n";
        std::cout << currentRoom.getDescription() << "\n";

        std::cout << "> ";
        std::string command;
        std::getline(std::cin, command);
        handleCommand(command);
    }
}

void Game::handleCommand(const std::string& input) {
    if (input == "quit") {
        isRunning = false;
    } else {
        std::cout << "The temple does not understand.\n";
    }
}
