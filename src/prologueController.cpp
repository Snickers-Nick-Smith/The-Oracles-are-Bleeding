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
    // ---- Safe wrappers so empty std::function never throws ----
    auto safePrompt = [this]() -> std::string {
        return hooks_.promptPrefix ? hooks_.promptPrefix() : std::string("> ");
    };
    auto callDescribe = [this]() {
        if (hooks_.describe) hooks_.describe();
        else std::cout << "(No description available.)\n";
    };
    auto callListExits = [this]() {
        if (hooks_.listExits) hooks_.listExits();
        else std::cout << "(No exit info available.)\n";
    };
    auto callShowJournal = [this]() {
        if (hooks_.showJournal) hooks_.showJournal();
        else std::cout << "(Journal is unavailable.)\n";
    };
    auto callWriteJournal = [this](int day) {
        if (hooks_.writeJournal) hooks_.writeJournal(day);
    };
    auto callMoveTo = [this](const std::string& dir) -> bool {
        return hooks_.moveTo ? hooks_.moveTo(dir) : false;
    };

    constexpr int kMaxDays = 7;
    std::cout << "\n(Prologue) Type 'help' for commands.\n" << std::flush;

    for (int day = 1; day <= kMaxDays; ++day) {
        std::cout << "\n— Day " << day << " —\n" << std::flush;

        if (day == 1) {
            // Show the banner BEFORE any room text
            printPrologueHelpBanner();
        }

        // Show room, then exits immediately so directions are obvious
        callDescribe();
        callListExits();

        bool wrote = false, endDay = false;
        while (!endDay) {
            std::cout << safePrompt();

            std::string line;
            if (!std::getline(std::cin, line)) return;

            const std::string lineTrim = trim_copy(line);
            if (lineTrim.empty()) continue; // ignore blank lines

            auto [cmdTok, restRaw] = split_first(lineTrim);
            const std::string cmd = toLower(cmdTok);         // case-insensitive commands
            const std::string rest = restRaw;                // keep rest as-typed (room titles)
            const std::string wholeLower = toLower(lineTrim);

            // 1) bare directions (n, north, sw, up...)
            if (auto dir = normalize_dir(cmd); !dir.empty()) {
                const bool moved = callMoveTo(dir);
                if (moved) callDescribe();
                continue;
            }

            // 2) verb + direction (go/move/walk/run/head/travel <dir>), or neighbor title
            if (is_move_verb(cmd)) {
                if (auto dir = normalize_dir(rest); !dir.empty()) {
                    const bool moved = callMoveTo(dir);
                    if (moved) callDescribe();
                } else if (!rest.empty()) {
                    const bool moved = callMoveTo(rest); // neighbor room title
                    if (moved) callDescribe();
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
                callDescribe();
                continue;
            }
            // exits
            if (cmd == "exits") {
                callListExits();
                continue;
            }
            // where -> room + exits
            if (cmd == "where") {
                callDescribe();
                callListExits();
                continue;
            }
            // journal
            if (cmd == "journal") {
                callShowJournal();
                continue;
            }
            // write (only once per day)
            if (cmd == "write") {
                if (wrote) {
                    std::cout << "(You’ve already written today.)\n";
                } else {
                    callWriteJournal(day);
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

        // your flavor text
        if (day == 2) std::cout << "Somewhere, a page turns though no one is there.\n";
        if (day == 4) std::cout << "The corridors feel longer tonight, but you arrive all the same.\n";
        if (day == 6) std::cout << "You wake from a dream you can’t recall—only warmth and candlelight.\n";
    }

    std::cout << "\nThe candle gutters. The temple is not as it was.\nPrologue complete.\n";
}
