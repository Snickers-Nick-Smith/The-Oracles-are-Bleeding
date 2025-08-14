// prologueController.hpp
#pragma once
#include <functional>
#include <string>

struct PrologueController {
    struct Hooks {
        std::function<void()> describe;
        std::function<void()> listExits;
        std::function<bool(const std::string&)> moveTo;
        std::function<void(int)> writeJournal;
        std::function<std::string()> promptPrefix;
        // NEW: show the Lysaia journal
        std::function<void()> showJournal;
        std::function<void()> showHelp;
    };

    explicit PrologueController(Hooks h) : hooks_(std::move(h)) {}
    void run();

private:
    Hooks hooks_;
};
