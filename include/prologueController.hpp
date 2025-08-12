#pragma once
#include <string>
#include <functional>

class PrologueController {
public:
    struct Hooks {
        std::function<void()> describe;
        std::function<void()> listExits;
        std::function<bool(const std::string&)> moveTo;
        std::function<void(int)> writeJournal;
        std::function<std::string()> promptPrefix;
    };

    explicit PrologueController(Hooks hooks) : hooks_(std::move(hooks)) {}
    void run();

private:
    Hooks hooks_;
};
