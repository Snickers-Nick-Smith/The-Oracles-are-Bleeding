#include "utils.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cctype>
#include <algorithm>
#include <sstream>
void slowPrint(const std::string& text, unsigned int ms) {
    for (char c : text) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    std::cout << '\n';
}
std::string join(const std::vector<std::string>& list, const std::string& sep) {
    std::ostringstream oss;
    for (size_t i = 0; i < list.size(); ++i) {
        oss << list[i];
        if (i + 1 < list.size()) oss << sep;
    }
    return oss.str();
}

std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return result;
}
