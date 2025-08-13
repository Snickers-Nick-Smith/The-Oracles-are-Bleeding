#include "SceneManager.hpp"
#include <iostream>


void SceneManager::introScene() {
    std::cout << "THE ORACLES ARE BLEEDING\n\n";
    std::cout << "They gave her a name that was not hers: Cassandra.\n";
    std::cout << "You are _____, a former oracle, cast out.\n";
    std::cout << "Now the temple is open again.\n";
    std::cout << "\n> Press ENTER to descend.\n";
    std::cin.ignore();
}

void SceneManager::erisFinalScene() {
    std::cout << "A mirror reflects someone else. The choir begins to hum.\n";
}

