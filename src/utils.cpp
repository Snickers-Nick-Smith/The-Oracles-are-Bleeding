#include "utils.hpp"
#include <iostream>
#include <thread>
#include <chrono>

void slowPrint(const std::string& text, unsigned int ms) {
    for (char c : text) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    std::cout << '\n';
}
