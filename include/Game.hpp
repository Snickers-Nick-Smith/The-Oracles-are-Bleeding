#ifndef GAME_HPP
#define GAME_HPP

#include "Player.hpp"
#include "Room.hpp"
#include "Shrine.hpp"
#include "SceneManager.hpp"
#include "Map.hpp"
#include "utils.hpp"
#include "Theme.hpp"   
#include "JournalManager.hpp"
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>

class Game {
public:
    Game();

    // Entry points
    void start();                     // main entry
    void displayMainMenu();
    void startLysaiaPrologue();       // sets up uncorrupted temple & runs prologue

    // Utilities already used elsewhere
    void describeCurrentRoom();
    void setAccessibility(const AccessibilitySettings& as) { accessibility_ = as; }

private:
    // ===== Prologue (Lysaia) =====
    std::unordered_set<int> lysaiaShrinesLogged_;
    void runLysaiaPrologue();         // small 7-day loop, returns to menu when done
    void lysaiaDay(int day, Shrine& shrine); // if you keep per-day hooks

    // ===== World / state =====
    Player player;
    std::vector<Room> rooms;

    // adjacency: currentRoomIndex -> (neighbor room name -> neighbor index)
    std::unordered_map<int, std::unordered_map<std::string, int>> roomConnections;

    std::unordered_map<int, Shrine> shrineRegistry; // shrineId -> Shrine
    TempleMap templeMap;
    JournalManager journalManager;
    bool isRunning = false;

    // ===== Setup =====
    void loadRooms();          // (if used by main game)
    void setupConnections();   // connect rooms in current map

    // ===== Main game (Melas, etc.) =====
    void beginMelasRun();
    void gameLoop();           // full loop (not used by prologue)
    void handleCommand(const std::string& input);
    void toggleAccessibility();
    void showMap();
    bool firstFramePrinted_ = false;
    int lastEnteredRoom_ = -1;   // <--- NEW: which room we last "entered" for side-effects
    // ...


    // ===== Helpers =====
    // deity inference
    Deity deityFromShrineName(const std::string& name) const;
    Deity deityFromRoomName(const std::string& name) const;

    // printing helpers (environmental narration)
    void printShrineText(const Shrine& shrine,
                         const std::string& text,
                         bool shake = false,
                         int intensity = 2,
                         int durationMs = 200);

    void printRoomDescriptionColored(const Room& room,
                                     const std::string& description);

    // default accessibility
    AccessibilitySettings accessibility_{ /*colorEnabled*/true,
                                          /*screenShakeEnabled*/true,
                                          /*textSpeed*/2 };
};

#endif // GAME_HPP
