#include "prologueController.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>

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
    // Auto-flush every insertion during the prologue so banners/prompt lines appear immediately.
    std::cout << std::unitbuf;

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

    // Print header + banner ONCE before the loop.
    std::cout << "\n(Prologue) Type 'help' for commands.\n";
    printPrologueHelpBanner();

    for (int day = 1; day <= kMaxDays; ++day) {
        std::cout << "\n— Day " << day << " —\n";

        // Ensure header is visibly out before the (chatty) describe path runs.
        std::cout.flush();
        callDescribe();          // (your describe also prints Exits)

        bool wrote = false, endDay = false;
        while (!endDay) {
            std::cout << safePrompt();

            std::string line;
            if (!std::getline(std::cin, line)) {
                std::cout << std::nounitbuf; // restore
                return;
            }

            const std::string lineTrim = trim_copy(line);
            if (lineTrim.empty()) continue;

            auto [cmdTok, restRaw] = split_first(lineTrim);
            const std::string cmd = toLower(cmdTok);
            const std::string rest = restRaw;
            const std::string wholeLower = toLower(lineTrim);

            if (auto dir = normalize_dir(cmd); !dir.empty()) {
                const bool moved = callMoveTo(dir);
                if (moved) callDescribe();
                continue;
            }
            if (is_move_verb(cmd)) {
                if (auto dir = normalize_dir(rest); !dir.empty()) {
                    const bool moved = callMoveTo(dir);
                    if (moved) callDescribe();
                } else if (!rest.empty()) {
                    const bool moved = callMoveTo(rest);
                    if (moved) callDescribe();
                } else {
                    std::cout << "Move where?\n";
                }
                continue;
            }
            if (cmd == "help") { printPrologueHelpBanner(); continue; }
            if (cmd == "look" || wholeLower == "look around") { callDescribe(); continue; }
            if (cmd == "exits") { callListExits(); continue; }
            if (cmd == "where") { callDescribe(); callListExits(); continue; }
            if (cmd == "journal") { callShowJournal(); continue; }
            if (cmd == "write") {
                if (wrote) std::cout << "(You’ve already written today.)\n";
                else { callWriteJournal(day); wrote = true; }
                continue;
            }
            if (cmd == "end" || cmd == "sleep" || cmd == "finish" || cmd == "next") {
                if (!wrote) { std::cout << "(You haven’t written today. Type 'end' again to sleep anyway.)\n"; wrote = true; }
                else { endDay = true; }
                continue;
            }
            std::cout << "Unknown command. Type 'help'.\n";
        }

        if (day == 2) std::cout << "Somewhere, a page turns though no one is there.\n";
        if (day == 4) std::cout << "The corridors feel longer tonight, but you arrive all the same.\n";
        if (day == 6) std::cout << "You wake from a dream you can’t recall—only warmth and candlelight.\n";
    }

    std::cout << "\nThe candle gutters. The temple is not as it was.\nPrologue complete.\n";

    // Restore normal buffering once we exit the prologue.
    std::cout << std::nounitbuf;
}
