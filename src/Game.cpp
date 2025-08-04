#include "Game.hpp"
#include "SceneManager.hpp"
#include "Shrine.hpp"
#include <iostream>

Game::Game() : isRunning(true) {}

void Game::start() {
    SceneManager::introScene();
    loadRooms();
    gameLoop();
}

void Game::loadRooms() {
    rooms.push_back(Room(
        "The Hall of Hunger",
        "Withered olive trees claw at the walls. A bowl of grain weeps red.",
        true, 0  // Demeter's shrine
    ));

    rooms.push_back(Room(
        "The Starless Well",
        "A smooth pit carved into obsidian. There is no echo here.",
        true, 1  // Nyx's shrine
    ));

    rooms.push_back(Room(
        "The Library of Teeth",
        "Scrolls made of stretched skin hang from pegs. Some blink.",
        false // no shrine
    ));

    rooms.push_back(Room(
        "The Whispering Hall",
        "Words are etched into every surface. None are the same language.",
        false
    ));
}

void Game::gameLoop() {
    while (isRunning) {
        const Room& current = rooms[player.getCurrentRoom()];
        if (!current.isVisited()) {
            std::cout << "[First time here]\n";
            rooms[player.getCurrentRoom()].markVisited();
        }

        std::cout << current.getDescription() << "\n";

        if (current.shrinePresent()) {
            std::cout << "There is a shrine here. Type 'shrine' to interact.\n";
        } else {
            std::cout << "There are strange things here: ";
            for (const auto& obj : current.getObjects()) {
                std::cout << obj << ", ";
            }
            std::cout << "\n";
        }

        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);
        handleCommand(input);
    }
}

void Game::handleCommand(const std::string& input) {
    const Room& current = rooms[player.getCurrentRoom()];

    if (input == "quit") {
        isRunning = false;
    } else if (input == "journal") {
        player.viewJournal();
    } else if (input == "shrine") {
        if (current.shrinePresent()) {
            Shrine::activate(current.getShrineID(), player);
        } else {
            std::cout << "There is no shrine here.\n";
        }
    } else {
        std::cout << "The temple does not understand.\n";
    }
}
