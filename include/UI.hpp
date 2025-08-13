#pragma once
#include <string>
#include <vector>
#include <functional>

struct UI {
    std::function<void(const std::string&)> print;
    std::function<int(const std::string&, const std::vector<std::string>&)> choose;
    std::function<std::string(const std::string&)> ask;
    std::function<void()> wait;

    // conveniences / aliases
    void say(const std::string& s) const { if (print) print(s); }
    int  menu(const std::string& p, const std::vector<std::string>& o) const { return choose ? choose(p,o) : 0; }
    std::string input(const std::string& p) const { return ask ? ask(p) : std::string(); }
    void pause() const { if (wait) wait(); }
    void waitForKey() const { if (wait) wait(); }  // <-- used by ShrineBehavior.cpp
};
