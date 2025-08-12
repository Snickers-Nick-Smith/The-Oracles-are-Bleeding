#include "prologueController.hpp"
#include <iostream>
#include <algorithm>

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return char(std::tolower(c)); });
    return s;
}

void PrologueController::run() {
    constexpr int kMaxDays = 7;
    std::cout << "\n(Prologue) Type 'help' for commands.\n";

    for (int day = 1; day <= kMaxDays; ++day) {
        std::cout << "\n— Day " << day << " —\n";
        hooks_.describe();

        bool wrote = false, endDay = false;
        while (!endDay) {
            std::cout << hooks_.promptPrefix();
            std::string in; if (!std::getline(std::cin, in)) break;
            auto low = toLower(in);

            if (low == "help") {
                std::cout << "Commands: look, where, exits, move <room|dir>, write, end, help\n";
            } else if (low == "look" || low == "where") {
                hooks_.describe();
            } else if (low == "exits") {
                hooks_.listExits();
            } else if (low.rfind("move ", 0) == 0) {
                hooks_.moveTo(in.substr(5));
            } else if (low == "write") {
                hooks_.writeJournal(day);
                wrote = true;
            } else if (low == "end") {
                if (!wrote) {
                    std::cout << "(You haven’t written today. Type 'write' or 'end' again to sleep anyway.)\n";
                    wrote = true;
                } else {
                    endDay = true;
                }
            } else if (!low.empty()) {
                std::cout << "Unknown command. Type 'help'.\n";
            }
        }

        if (day == 2) std::cout << "Somewhere, a page turns though no one is there.\n";
        if (day == 4) std::cout << "The corridors feel longer tonight, but you arrive all the same.\n";
        if (day == 6) std::cout << "You wake from a dream you can’t recall—only warmth and candlelight.\n";
    }

    std::cout << "\nThe candle gutters. The temple is not as it was.\nPrologue complete.\n";
}
