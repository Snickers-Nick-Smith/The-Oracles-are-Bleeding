#include "prologueController.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>

// Note: we use split_first / normalize_dir / is_move_verb from utils.hpp
namespace {
    void printPrologueHelpBanner() {
        std::cout
            << "\n— Lysaia’s Prologue —\n"
            << "Commands:\n"
            << "  look / look around      reprint the room\n"
            << "  exits                   list available directions\n"
            << "  where                   room + exits\n"
            << "  n,s,e,w,ne,nw,se,sw,u,d or go/move/run/head/travel <dir>\n"
            << "  journal                 read entries\n"
            << "  write                   add today’s entry\n"
            << "  end                     sleep / next day (confirm if unwritten)\n"
            << "  help                    show this help\n";
    }
}

void PrologueController::run() {
    constexpr int kMaxDays = 7;
    std::cout << "\n(Prologue) Type 'help' for commands.\n" << std::flush;

    for (int day = 1; day <= kMaxDays; ++day) {
        std::cout << "\n— Day " << day << " —\n" << std::flush;

        if (day == 1) {
            // Show the banner BEFORE any room text
            printPrologueHelpBanner();
        }

        // Show room, then exits immediately so directions are obvious
        hooks_.describe();
        hooks_.listExits();

        bool wrote = false, endDay = false;
        while (!endDay) {
    std::cout << hooks_.promptPrefix();

    std::string line;
    if (!std::getline(std::cin, line)) break;

    const std::string lineTrim = trim_copy(line);
    if (lineTrim.empty()) continue; // ignore blank lines

    auto [cmdTok, restRaw] = split_first(lineTrim);
    const std::string cmd = toLower(cmdTok);         // case-insensitive commands
    const std::string rest = restRaw;                // keep rest as-typed (room titles)
    const std::string wholeLower = toLower(lineTrim);

    // 1) bare directions (n, north, sw, up...)
    if (auto dir = normalize_dir(cmd); !dir.empty()) {
        hooks_.moveTo(dir);
        continue;
    }

    // 2) verb + direction (go/move/walk/run/head/travel <dir>), or neighbor title
    if (is_move_verb(cmd)) {
        if (auto dir = normalize_dir(rest); !dir.empty()) {
            hooks_.moveTo(dir);
        } else if (!rest.empty()) {
            hooks_.moveTo(rest); // neighbor room title
        } else {
            std::cout << "Move where?\n";
        }
        continue;
    }

    // help
 if (cmd == "help") {
    printPrologueHelpBanner();
    continue;
}
    // look / look around
    if (cmd == "look" || wholeLower == "look around") {
        hooks_.describe();
        continue;
    }
    // exits
    if (cmd == "exits") {
        hooks_.listExits();
        continue;
    }
    // where -> room + exits
    if (cmd == "where") {
        hooks_.describe();
        hooks_.listExits();
        continue;
    }
    // journal
    if (cmd == "journal") {
        if (hooks_.showJournal) hooks_.showJournal();
        else std::cout << "(Journal is unavailable.)\n";
        continue;
    }
    // write (only once per day)
    if (cmd == "write") {
        if (wrote) {
            std::cout << "(You’ve already written today.)\n";
        } else {
            hooks_.writeJournal(day);
            wrote = true;
        }
        continue;
    }
    // end-of-day (accept a few synonyms); require confirmation if not written
    if (cmd == "end" || cmd == "sleep" || cmd == "finish" || cmd == "next") {
        if (!wrote) {
            std::cout << "(You haven’t written today. Type 'end' again to sleep anyway.)\n";
            wrote = true; // confirmation latch
        } else {
            endDay = true;
        }
        continue;
    }

    std::cout << "Unknown command. Type 'help'.\n";
}


        if (day == 2) std::cout << "Somewhere, a page turns though no one is there.\n";
        if (day == 4) std::cout << "The corridors feel longer tonight, but you arrive all the same.\n";
        if (day == 6) std::cout << "You wake from a dream you can’t recall—only warmth and candlelight.\n";
    }

    std::cout << "\nThe candle gutters. The temple is not as it was.\nPrologue complete.\n";
}
