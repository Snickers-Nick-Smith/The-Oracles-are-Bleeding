#include "prologueController.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>

// Note: we use split_first / normalize_dir / is_move_verb from utils.hpp

void PrologueController::run() {
    constexpr int kMaxDays = 7;
    std::cout << "\n(Prologue) Type 'help' for commands.\n";

    for (int day = 1; day <= kMaxDays; ++day) {
        std::cout << "\n— Day " << day << " —\n";
        hooks_.describe();

        bool wrote = false, endDay = false;
        while (!endDay) {
            std::cout << hooks_.promptPrefix();
            std::string line;
            if (!std::getline(std::cin, line)) break;

            auto [cmd, rest] = split_first(line);

            // 1) bare directions (n, north, sw, up...)
            if (auto dir = normalize_dir(cmd); !dir.empty()) {
                hooks_.moveTo(dir);
                continue;
            }

            // 2) verb + direction (go/move/walk/run/head/travel <dir>)
            if (is_move_verb(cmd)) {
                if (auto dir = normalize_dir(rest); !dir.empty()) {
                    hooks_.moveTo(dir);
                } else if (!rest.empty()) {
                    // allow neighbor room title
                    hooks_.moveTo(rest);
                } else {
                    std::cout << "Move where?\n";
                }
                continue;
            }

            if (cmd == "help") {
                std::cout
                    << "Commands: look, where, exits, "
                       "north/south/east/west/ne/nw/se/sw/up/down, "
                       "go|move|walk|run|head|travel <dir>, "
                       "write, journal, end, help\n";
                continue;
            }

            if (cmd == "look" || cmd == "where") {
                hooks_.describe();
                continue;
            }

            if (cmd == "exits") {
                hooks_.listExits();
                continue;
            }

            if (cmd == "write") {
                hooks_.writeJournal(day);
                wrote = true;
                continue;
            }

            if (cmd == "journal") {
                if (hooks_.showJournal) hooks_.showJournal();
                else std::cout << "(Journal is unavailable.)\n";
                continue;
            }

            if (cmd == "end") {
                if (!wrote) {
                    std::cout << "(You haven’t written today. Type 'write' or 'end' again to sleep anyway.)\n";
                    wrote = true;
                } else {
                    endDay = true;
                }
                continue;
            }

            if (!cmd.empty()) {
                std::cout << "Unknown command. Type 'help'.\n";
            }
        }

        if (day == 2) std::cout << "Somewhere, a page turns though no one is there.\n";
        if (day == 4) std::cout << "The corridors feel longer tonight, but you arrive all the same.\n";
        if (day == 6) std::cout << "You wake from a dream you can’t recall—only warmth and candlelight.\n";
    }

    std::cout << "\nThe candle gutters. The temple is not as it was.\nPrologue complete.\n";
}
