#include "Game.hpp"
#include "SceneManager.hpp"
#include "UI.hpp"
#include "Shrine.hpp"
#include "utils.hpp"
#include "Theme.hpp"
#include "Mechanics.hpp"
#include "ShrineBehavior.hpp"
#include "PersephoneFragments.hpp"
#include "FragmentPlacer.hpp"
#include "ShrineRunner.hpp"
#include "JournalManager.hpp"
#include "prologueController.hpp" 
#include <unordered_map>
#include <iostream>
#include <limits>
#include <algorithm>
#include <ctime>
#include <cctype>
#include <cstdlib>
#include <sstream>

// --- deity inference helpers -------------------------------------------------

Deity Game::deityFromShrineName(const std::string& name) const {
    const std::string n = toLower(name);
    if      (n == "nyx")          return Deity::Nyx;
    else if (n == "eris")         return Deity::Eris;
    else if (n == "pan")          return Deity::Pan;
    else if (n == "demeter")      return Deity::Demeter;
    else if (n == "persephone")   return Deity::Persephone;
    else if (n == "false hermes") return Deity::FalseHermes;
    else if (n == "thanatos")     return Deity::Thanatos;
    else if (n == "apollo")       return Deity::Apollo;
    else if (n == "hecate")       return Deity::Hecate;
    return Deity::Default;
}

// Map a room name to a deity by known titles you already use.
// (case-insensitive exact matches; easy to expand)
Deity Game::deityFromRoomName(const std::string& roomName) const {
    static const std::unordered_map<std::string, Deity> kRoomToDeity = {
        // ===== Demeter =====
        {"the garden of broken faces", Deity::Demeter},   // corrupted
        {"the threadbare womb",        Deity::Demeter},
        {"the hall of hunger",         Deity::Demeter},
        {"garden of blooming faces",   Deity::Demeter},   // uncorrupted
        {"threaded womb",              Deity::Demeter},
        {"hall of plenty",             Deity::Demeter},

        // ===== Nyx =====
        {"room with no corners",       Deity::Nyx},       // corrupted
        {"nest of wings",              Deity::Nyx},
        {"the starless well",          Deity::Nyx},
        {"room of gentle horizons",    Deity::Nyx},       // uncorrupted
        {"the star-bound well",        Deity::Nyx},

        // ===== Apollo =====  (same across both states)
        {"hall of echoes",             Deity::Apollo},
        {"room that remembers",        Deity::Apollo},
        {"echoing gallery",            Deity::Apollo},

        // ===== Hecate =====
        {"loom of names",              Deity::Hecate},
        {"listening chamber",          Deity::Hecate},
        {"the unlit path",             Deity::Hecate},    // corrupted
        {"the luminous path",          Deity::Hecate},    // uncorrupted

        // ===== Persephone =====
        {"hall of petals",             Deity::Persephone},
        {"orchard walk",               Deity::Persephone},
        {"the frozen spring",          Deity::Persephone},// corrupted
        {"the blooming spring",        Deity::Persephone},// uncorrupted

        // ===== Pan =====
        {"hall of shivering meat",     Deity::Pan},       // corrupted
        {"den of antlers",             Deity::Pan},
        {"wild rotunda",               Deity::Pan},
        {"hall of living wood",        Deity::Pan},       // uncorrupted
        {"verdant rotunda",            Deity::Pan},

        // ===== False Hermes ===== (same titles)
        {"room of borrowed things",    Deity::FalseHermes},
        {"whispering hall",            Deity::FalseHermes},
        {"gilded hallway",             Deity::FalseHermes},

        // ===== Thanatos =====
        {"room of waiting lights",     Deity::Thanatos},
        {"the room of waiting lights", Deity::Thanatos},
        {"waiting room",               Deity::Thanatos},
        {"the bloodclock",             Deity::Thanatos},
        {"sleepwalker’s alcove",       Deity::Thanatos},  // corrupted shrine
        {"hall of quiet rest",         Deity::Thanatos},  // uncorrupted shrine

        // ===== Eris =====
        {"oracle’s wake",              Deity::Eris},
        {"archivist’s cell",           Deity::Eris},
        {"throat of the temple",       Deity::Eris},
        {"the bone choir",             Deity::Eris},      // corrupted shrine
        {"hall of harmony",            Deity::Eris},      // uncorrupted shrine

        // ===== Hub =====
        {"main hall of the temple",    Deity::Default}
    };

    const std::string key = toLower(roomName);
    auto it = kRoomToDeity.find(key);
    return (it != kRoomToDeity.end()) ? it->second : Deity::Default;
}

static std::string toLocationId(const std::string& roomName);

// =================== Mechanics Integration Bridge ============================
static RNG                     g_rng;
static PlayerState             g_pstate;
static std::unordered_map<std::string, bool> g_flags;

// ---- Journal bridge (to your JournalManager) --------------------------------
struct JournalBridge : IJournalSink {
    JournalManager* jm = nullptr;  // set this in InitMechanics()
    void writeLysaia(const std::string& entry) override {
        if (jm) jm->writeLysaia(entry); else std::cout << "[Lysaia] " << entry << "\n";
    }
    void writeMelas(const std::string& entry) override {
        if (jm) jm->writeMelas(entry); else std::cout << "[Melas] " << entry << "\n";
    }
};
static JournalBridge g_journal;

// ---- Minimal UI lambdas for prompts ----------------------------------------
static UI g_ui {
    /*print*/ [](const std::string& s){ std::cout << s << "\n"; },
    /*choose*/[](const std::string& prompt, const std::vector<std::string>& opts){
        std::cout << prompt << "\n";
        for (size_t i=0;i<opts.size();++i) std::cout << "  " << (i+1) << ") " << opts[i] << "\n";
        int pick=0; std::cout << "> "; std::cin >> pick; return pick;
    },
    /*ask*/   [](const std::string& prompt){
        std::cout << prompt << "\n> ";
        std::string s; std::getline(std::cin >> std::ws, s); return s;
    },
    /*wait*/  [](){ std::cout << "[Press Enter]"; std::cin.get(); }
};

// ---- Context factory --------------------------------------------------------
static InteractionContext MakeCtx() {
    return InteractionContext{
        g_pstate,                  // PlayerState&
        g_rng,                     // RNG&
        g_journal,                 // IJournalSink&
        g_pstate.view,             // WorldView (Lysaia vs. Melas)
        ShrineState::UNCORRUPTED,  // default; ShrineRunner will override per-shrine
        g_flags                    // flags map
    };
}

[[maybe_unused]] static void SyncFromGamePlayer(const Player& /*p*/, PlayerState& /*s*/) {}
[[maybe_unused]] static void SyncToGamePlayer(const PlayerState& /*s*/, Player& /*p*/) {}


// ---- Public-ish helpers you will call from your flow ------------------------
static void InitMechanics(JournalManager* jm, bool isMelasPlaythrough) {
    g_journal.jm = jm;
    g_pstate.view = isMelasPlaythrough ? WorldView::Corrupted : WorldView::Uncorrupted;

    // Example starting stats; adjust as needed
    g_pstate.stats = {/*health*/5, /*will*/7, /*insight*/2, /*nerve*/2};
    g_pstate.corruption = isMelasPlaythrough ? 10 : 0;
}

static void OnRoomEntered(const std::string& roomTitle) {
    auto ctx = MakeCtx();

    // --- MELAS: auto-write a location entry once per room visit ---
    if (ctx.view == WorldView::Corrupted && g_journal.jm) {
        if (const std::string loc = toLocationId(roomTitle); !loc.empty()) {
            const std::string flag = "melas_visited:" + loc;
            if (!g_flags[flag]) {
                g_journal.jm->writeMelasAt(loc); // add location entry
                g_flags[flag] = true;            // de-dupe for future revisits
            }
        }
    }

    // Persephone letter fragment auto-pickups (already Melas-only inside)
    CheckPersephoneLetterPickupsForRoom(ctx, roomTitle);
}

static void OnShrineInteract(const Shrine& shrine, JournalManager* /* jm */) {
    auto ctx = MakeCtx();

    ShrineServices svc;
    // If your JournalManager exposes these, wire them; else leave nullptr
    // svc.takeMelasEntry = [jm]() -> std::optional<std::string> { return jm->takeLastMelasEntry(); };
    // svc.giveMelasEntry = [jm](const std::string& s) { jm->writeMelas(s); };

    Outcome out = RunShrine(shrine, ctx, g_ui, svc);

    // Apply result and log
    g_pstate.applyOutcome(out);
    if (!out.journalEntry.empty()) {
        if (ctx.view == WorldView::Corrupted) g_journal.writeMelas(out.journalEntry);
        else                               g_journal.writeLysaia(out.journalEntry);
    }

    // Endings check (optional)
    if (g_flags["false_hermes_endless_hall"] || g_flags["thanatos_sleep_end"] ||
        g_flags["ending_join_eris"] || g_flags["ending_save_lysaia"] ||
        g_flags["ending_overcome"] || g_flags["ending_claimed"]) {
        // TODO: return to menu / credits
        // e.g., SceneManager::instance().goToMainMenu();
    }
}
// ================= End Mechanics Integration Bridge ==========================

// --- public helpers ----------------------------------------------------------

// Color + (optional) shake for shrine text based on shrine's current state
void Game::printShrineText(const Shrine& shrine,
                           const std::string& text,
                           bool shake,
                           int intensity,
                           int durationMs) {
    const Deity d = deityFromShrineName(shrine.getName());
    const ShrineState st = shrine.getState(); // UNCORRUPTED/CORRUPTED
    const std::string styled = ThemeRegistry::style(d, st, text, accessibility_);
    if (shake) {
        shakeLine(styled, accessibility_, intensity, durationMs);
    } else {
        printWithSpeed(styled, accessibility_, /*endWithNewline*/true);
    }
}

// --- lookup & wiring helpers ---
int Game::indexByTitle(const std::string& title) const {
    for (int i = 0; i < static_cast<int>(rooms.size()); ++i)
        if (rooms[i].getName() == title) return i;
    return -1;
}

void Game::addEdge(int from, const std::string& dirLong, int to) {
    const std::string longKey = normalize_dir(dirLong); // "n" -> "north"
    roomConnections[from][longKey] = to;                // store only long key
}


void Game::addEdgeByTitle(const std::string& fromTitle, const std::string& dirLong,
                          const std::string& toTitle) {
    const int from = indexByTitle(fromTitle);
    const int to   = indexByTitle(toTitle);
    if (from < 0 || to < 0) return; // or assert/log
    addEdge(from, dirLong, to);
}

void Game::addEdgeBothByTitle(const std::string& fromTitle, const std::string& dirLong,
                              const std::string& toTitle,   const std::string& reverseDirLong) {
    addEdgeByTitle(fromTitle, dirLong, toTitle);
    addEdgeByTitle(toTitle, reverseDirLong, fromTitle);
}

void Game::setupPrologueConnectionsByTitle() {
    roomConnections.clear();

    const std::string MH = "Main Hall of the Temple";

    // Main Hall spokes
    addEdgeByTitle(MH, "north",     "Garden of Blooming Faces");   // Demeter entry
    addEdgeByTitle(MH, "northeast", "Room of Gentle Horizons");    // Nyx entry
    addEdgeByTitle(MH, "east",      "Hall of Echoes");             // Apollo entry
    addEdgeByTitle(MH, "southeast", "Loom of Names");              // Hecate entry
    addEdgeByTitle(MH, "south",     "Hall of Petals");             // Persephone entry
    addEdgeByTitle(MH, "southwest", "Hall of Living Wood");        // Pan entry
    addEdgeByTitle(MH, "west",      "Room of Borrowed Things");    // False Hermes entry
    addEdgeByTitle(MH, "northwest", "Room of Waiting Lights");     // Thanatos entry
    addEdgeByTitle(MH, "up",        "Throat of the Temple");       // Eris entry

    // Quick return to Main Hall from each room in a wing (matches your index layout)
    addEdgeByTitle("Garden of Blooming Faces", "south", MH);
    addEdgeByTitle("Threaded Womb",            "south", MH);
    addEdgeByTitle("Hall of Plenty",           "south", MH);

    addEdgeByTitle("Room of Gentle Horizons",  "southwest", MH);
    addEdgeByTitle("Nest of Wings",            "southwest", MH);
    addEdgeByTitle("The Star-Bound Well",      "southwest", MH);

    addEdgeByTitle("Hall of Echoes",           "west", MH);
    addEdgeByTitle("Room That Remembers",      "west", MH);
    addEdgeByTitle("Echoing Gallery",          "west", MH);

    addEdgeByTitle("Loom of Names",            "northwest", MH);
    addEdgeByTitle("Listening Chamber",        "northwest", MH);
    addEdgeByTitle("The Luminous Path",        "northwest", MH);

    addEdgeByTitle("Hall of Petals",           "north", MH);
    addEdgeByTitle("Orchard Walk",             "north", MH);
    addEdgeByTitle("The Blooming Spring",      "north", MH);

    addEdgeByTitle("Hall of Living Wood",      "northeast", MH);
    addEdgeByTitle("Den of Antlers",           "northeast", MH);
    addEdgeByTitle("Verdant Rotunda",          "northeast", MH);

    addEdgeByTitle("Room of Borrowed Things",  "north", MH);
    addEdgeByTitle("Whispering Hall",          "north", MH);
    addEdgeByTitle("Gilded Hallway",           "north", MH);

    addEdgeByTitle("Room of Waiting Lights",   "southeast", MH);
    addEdgeByTitle("Waiting Room",             "southeast", MH);
    addEdgeByTitle("Hall of Quiet Rest",       "southeast", MH);

    addEdgeByTitle("Throat of the Temple",     "down", MH);
    addEdgeByTitle("Oracle’s Wake",            "down", MH);
    addEdgeByTitle("Archivist’s Cell",         "down", MH);
    addEdgeByTitle("Hall of Harmony",          "down", MH);

    // Linear paths inside each wing (bidirectional)
    addEdgeBothByTitle("Garden of Blooming Faces", "east", "Threaded Womb", "west");
    addEdgeBothByTitle("Threaded Womb",            "east", "Hall of Plenty", "west");

    addEdgeBothByTitle("Room of Gentle Horizons",  "east", "Nest of Wings", "west");
    addEdgeBothByTitle("Nest of Wings",            "east", "The Star-Bound Well", "west");

    addEdgeBothByTitle("Hall of Echoes",           "east", "Room That Remembers", "west");
    addEdgeBothByTitle("Room That Remembers",      "east", "Echoing Gallery", "west");

    addEdgeBothByTitle("Loom of Names",            "east", "Listening Chamber", "west");
    addEdgeBothByTitle("Listening Chamber",        "east", "The Luminous Path", "west");

    addEdgeBothByTitle("Hall of Petals",           "east", "Orchard Walk", "west");
    addEdgeBothByTitle("Orchard Walk",             "east", "The Blooming Spring", "west");

    addEdgeBothByTitle("Hall of Living Wood",      "east", "Den of Antlers", "west");
    addEdgeBothByTitle("Den of Antlers",           "east", "Verdant Rotunda", "west");

    addEdgeBothByTitle("Room of Borrowed Things",  "east", "Whispering Hall", "west");
    addEdgeBothByTitle("Whispering Hall",          "east", "Gilded Hallway", "west");

    addEdgeBothByTitle("Room of Waiting Lights",   "east", "Waiting Room", "west");
    addEdgeBothByTitle("Waiting Room",             "east", "Hall of Quiet Rest", "west");

    addEdgeBothByTitle("Throat of the Temple",     "east", "Oracle’s Wake", "west");
    addEdgeBothByTitle("Oracle’s Wake",            "east", "Archivist’s Cell", "west");
    addEdgeBothByTitle("Archivist’s Cell",         "east", "Hall of Harmony", "west");
}

// Color a room description line using its deity (if we can infer one).
void Game::printRoomDescriptionColored(const Room& room,
                                       const std::string& description) {
    // If it's a shrine room, prefer the actual shrine's deity & state.
    if (room.isShrine()) {
        const int shrineID = room.getShrineID();
        auto it = shrineRegistry.find(shrineID);
        if (it != shrineRegistry.end()) {
            printShrineText(it->second, description, /*shake*/false);
            return;
        }
    }

    // Otherwise try to infer by room name.
    const Deity d = deityFromRoomName(room.getName());
    // If we matched a deity, assume the section's shrine state is what you'll use most:
    // We'll check if there is a known shrine for that deity in shrineRegistry; otherwise default CORRUPTED.
    ShrineState st = ShrineState::CORRUPTED;
    for (const auto& kv : shrineRegistry) {
        if (deityFromShrineName(kv.second.getName()) == d) {
            st = kv.second.getState();
            break;
        }
    }

    if (d != Deity::Default) {
        const std::string styled = ThemeRegistry::style(d, st, description, accessibility_);
        printWithSpeed(styled, accessibility_, true);
    } else {
        // Fallback: no deity mapping; print plain
        printWithSpeed(description, accessibility_, true);
    }
}

static std::string toLocationId(const std::string& roomName) {
    static const std::unordered_map<std::string, std::string> kRoomToLocId = {
        // ===== Demeter =====
        {"The Garden of Broken Faces", "demeter/room/garden_of_broken_faces"},
        {"The Threadbare Womb",        "demeter/room/threadbare_womb"},
        {"The Hall of Hunger",         "demeter/shrine"},
        {"Garden of Blooming Faces",   "demeter/room/garden_of_blooming_faces"},
        {"Threaded Womb",              "demeter/room/threaded_womb"},
        {"Hall of Plenty",             "demeter/shrine_uncorrupted"},

        // ===== Nyx =====
        {"Room With No Corners",       "nyx/room/no_corners"},
        {"Nest of Wings",              "nyx/room/nest_of_wings"},
        {"The Starless Well",          "nyx/shrine"},
        {"Room of Gentle Horizons",    "nyx/room/gentle_horizons"},
        {"The Star-Bound Well",        "nyx/shrine_uncorrupted"},

        // ===== Apollo =====
        {"Hall of Echoes",             "apollo/room/hall_of_echoes"},
        {"Room That Remembers",        "apollo/room/room_that_remembers"},
        {"Echoing Gallery",            "apollo/shrine_uncorrupted"}, // FIX: single uncorrupted entry

        // ===== Hecate =====
        {"Loom of Names",              "hecate/room/loom_of_names"},
        {"Listening Chamber",          "hecate/room/listening_chamber"},
        {"The Unlit Path",             "hecate/shrine"},
        {"The Luminous Path",          "hecate/shrine_uncorrupted"},

        // ===== Persephone =====
        {"Hall of Petals",             "persephone/room/hall_of_petals"},
        {"Orchard Walk",               "persephone/room/orchard_walk"},
        {"The Frozen Spring",          "persephone/shrine"},
        {"The Blooming Spring",        "persephone/shrine_uncorrupted"},

        // ===== Pan =====
        {"Hall of Shivering Meat",     "pan/room/hall_of_shivering_meat"},
        {"Den of Antlers",             "pan/room/den_of_antlers"},
        {"Wild Rotunda",               "pan/shrine"},
        {"Hall of Living Wood",        "pan/room/hall_of_living_wood"},
        {"Verdant Rotunda",            "pan/shrine_uncorrupted"},

        // ===== False Hermes =====
        {"Room of Borrowed Things",    "false_hermes/room/borrowed_things"},
        {"Whispering Hall",            "false_hermes/room/whispering_hall"},
        {"Gilded Hallway",             "false_hermes/shrine_uncorrupted"}, // FIX: correct key; remove typo line

        // ===== Thanatos =====
        {"Room of Waiting Lights",     "thanatos/room/room_of_waiting_lights"},
        {"The Room of Waiting Lights", "thanatos/room/room_of_waiting_lights"},
        {"Waiting Room",               "thanatos/room/waiting_room"},
        {"The Bloodclock",             "thanatos/room/bloodclock"},
        {"Sleepwalker’s Alcove",       "thanatos/shrine"},
        {"Hall of Quiet Rest",         "thanatos/shrine_uncorrupted"},

        // ===== Eris =====
        {"Oracle’s Wake",              "eris/room/oracles_wake"},
        {"Archivist’s Cell",           "eris/room/archivists_cell"},
        {"Throat of the Temple",       "eris/room/throat_of_temple"},
        {"The Bone Choir",             "eris/shrine"},
        {"Hall of Harmony",            "eris/shrine_uncorrupted"},

        // ===== Hub =====
        {"Main Hall of the Temple",    ""}
    };

    auto it = kRoomToLocId.find(roomName);
    return (it != kRoomToLocId.end()) ? it->second : std::string{};
}


// Room Descriptor
void Game::describeCurrentRoom() {
    // Gate: allow rendering if we're InGame OR we're in the prologue
    if (!inPrologue_ && phase_ != Phase::InGame) return;

    const int id = player.getCurrentRoom();
    if (id < 0 || id >= static_cast<int>(rooms.size())) return;

    const Room& current = rooms[id];

    // Only fire Melas mechanics/journal when actually in the main run.
    if (!inPrologue_ && id != lastEnteredRoom_) {
        OnRoomEntered(current.getName());   // or OnRoomEntered(id);
        lastEnteredRoom_ = id;
    }

    // Print the room description once
    printRoomDescriptionColored(current, current.getDescription());

    // Exits
    auto it = roomConnections.find(id);
    if (it != roomConnections.end() && !it->second.empty()) {
        std::vector<std::string> exits;
        exits.reserve(it->second.size());
        for (const auto& kv : it->second) exits.push_back(kv.first);
        std::sort(exits.begin(), exits.end());
        std::cout << "Exits: " << join(exits, ", ") << "\n";
    }
}



void Game::runLysaiaPrologue() {
     inPrologue_ = true;  
     phase_      = Phase::Intro;
    PrologueController::Hooks hooks;

    hooks.describe = [this]() {
        describeCurrentRoom(); // prints room (and exits if your describe() does)
    };

   hooks.listExits = [this]() {
    const int cur = player.getCurrentRoom();
    auto it = roomConnections.find(cur);
    if (it == roomConnections.end() || it->second.empty()) { std::cout << "No obvious exits.\n"; return; }

    std::vector<std::string> longs;
    longs.reserve(it->second.size());
    for (const auto& kv : it->second) longs.push_back(kv.first);
    std::sort(longs.begin(), longs.end());
    longs.erase(std::unique(longs.begin(), longs.end()), longs.end());
    std::cout << "Exits: " << join(longs, ", ") << "\n";
};

   hooks.moveTo = [this](const std::string& target) -> bool {
    if (auto dir = normalize_dir(target); !dir.empty()) {
        const int cur = player.getCurrentRoom();
        auto it = roomConnections.find(cur);
        if (it == roomConnections.end()) { std::cout << "You can't move from here.\n"; return false; }

        // resolve direction to a stored key (long or short)
        auto jt = it->second.find(dir);
        if (jt == it->second.end()) {
            // try normalized alias
            for (const auto& kv : it->second) {
                if (normalize_dir(kv.first) == dir) { jt = it->second.find(kv.first); break; }
            }
        }
        if (jt == it->second.end()) { std::cout << "No exit that way.\n"; return false; }

        // perform move (Player::move prints its own “No exit” if needed)
        player.move(jt->first, roomConnections);

        // DO NOT describe here; controller will call hooks.describe() after success
        return true;
    }

    // room-name teleport among neighbors
    const int cur = player.getCurrentRoom();
    auto it = roomConnections.find(cur);
    if (it == roomConnections.end()) { std::cout << "You can't move from here.\n"; return false; }

    const std::string t = toLower(target);
    for (const auto& kv : it->second) {
        const int idx = kv.second;
        if (toLower(rooms[idx].getName()) == t) {
            player.setCurrentRoom(idx);
            return true;
        }
    }
    std::cout << "No path to '" << target << "'. Try 'exits'.\n";
    return false;
};

   hooks.writeJournal = [this](int day) {
    const int cur = player.getCurrentRoom();
    const Room& r = rooms[cur];

    // Local helper that has access to 'this' (so we can call the private member)
    auto shrineKeyForLysaia = [this](const Room& room) -> std::string {
        switch (deityFromRoomName(room.getName())) {
            case Deity::Demeter:     return "demeter/shrine_uncorrupted";
            case Deity::Nyx:         return "nyx/shrine_uncorrupted";
            case Deity::Apollo:      return "apollo/shrine_uncorrupted";
            case Deity::Hecate:      return "hecate/shrine_uncorrupted";
            case Deity::Persephone:  return "persephone/shrine_uncorrupted";
            case Deity::Pan:         return "pan/shrine_uncorrupted";
            case Deity::FalseHermes: return "false_hermes/shrine_uncorrupted";
            case Deity::Thanatos:    return "thanatos/shrine_uncorrupted";
            case Deity::Eris:        return "eris/shrine_uncorrupted";
            default:                 return std::string{};
        }
    };

    std::vector<std::string> keys;
    keys.reserve(2);

    if (r.isShrine()) {
        // Prefer explicit room mapping; fall back by deity if missing/wrong
        std::string key = toLocationId(r.getName());
        if (key.empty() || key.find("/shrine") == std::string::npos) {
            key = shrineKeyForLysaia(r);
        }
        if (!key.empty()) keys.push_back(key);

        // If you want “first time at this shrine” gating, keep your set:
        // if (!lysaiaShrinesLogged_.count(cur)) lysaiaShrinesLogged_.insert(cur);
    } else {
        if (const std::string loc = toLocationId(r.getName()); !loc.empty()) {
            keys.push_back(loc);
        } else {
            journalManager.writeLysaia(
                "I wrote in an unmarked place, to keep it from becoming strange.");
        }
    }

    // De-dup and write
    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
    for (const auto& k : keys) journalManager.writeLysaiaAt(k);

    // Day-specific beat
    journalManager.writeLysaiaGuiltBeat(day);

    std::cout << "You light the candle and write. The ink dries in steady lines.\n";
    journalManager.printLastLysaia(std::cout);
};




    hooks.showJournal = [this]() { journalManager.printLysaia(std::cout); };

    hooks.promptPrefix = [this]() {
        std::ostringstream oss;
        oss << "[" << rooms[player.getCurrentRoom()].getName() << "] > ";
        return oss.str();
    };

    // IMPORTANT: one controller, one run. No pre-describe, no second run.
    PrologueController prologue(hooks);
    prologue.run();
}




Game::Game() : isRunning(true) {
}


void Game::startLysaiaPrologue() {
    inPrologue_ = true;  
    ThemeRegistry::setDefaultShrineState(ShrineState::UNCORRUPTED);

    InitMechanics(&journalManager, /*isMelasPlaythrough=*/false);
    journalManager.seedLysaiaPrologueText();
    journalManager.unlockLysaiaJournal();

    rooms.clear();
    shrineRegistry.clear();
    roomConnections.clear();
    lastEnteredRoom_ = -1; 

    // ===== Main Hall =====
    rooms.push_back(Room(
        "Main Hall of the Temple",
        "Sunlight streams through high windows, casting bright patterns across polished marble. The air is warm, "
        "and the faint sound of lyres drifts from unseen corridors."
    ));
    // (temporary start removed; we’ll set start by title later)

    // ===== Demeter =====
    Shrine demeter("Demeter", "Hall of Plenty");
    demeter.setState(ShrineState::UNCORRUPTED);
    shrineRegistry[0] = demeter;
    rooms.push_back(Room("Garden of Blooming Faces",
        "A peaceful garden of carved masks, each smiling serenely. Ivy and flowers weave gently between them, "
        "and the air is heavy with the scent of ripe fruit."));
    rooms.push_back(Room("Threaded Womb",
        "Soft woven cloth drapes the walls, dyed in warm golds and greens. In the center rests a cradle, "
        "adorned with fresh flowers and resting quietly."));
    rooms.push_back(Room("Hall of Plenty",
        "Rows of tables are laden with bread, grain, and ripe fruit. The distant hum of bees echoes softly.", true, 0));

    // ===== Nyx =====
    Shrine nyx("Nyx", "The Star-Bound Well");
    nyx.setState(ShrineState::UNCORRUPTED);
    shrineRegistry[1] = nyx;
    rooms.push_back(Room("Room of Gentle Horizons",
        "The walls curve seamlessly into floor and ceiling. A faint, soft starlight fills the air, "
        "as though the sky itself has come inside."));
    rooms.push_back(Room("Nest of Wings",
        "Feathers drift lazily from above, white and clean. In the center, a nest woven of pale reeds rests, "
        "warm from the touch of something unseen."));
    rooms.push_back(Room("The Star-Bound Well",
        "A perfectly round well reflects the stars, even in daylight. The water is still, yet seems impossibly deep.", true, 1));

    // ===== Apollo =====
    Shrine apollo("Apollo", "Echoing Gallery");
    apollo.setState(ShrineState::UNCORRUPTED);
    shrineRegistry[2] = apollo;
    rooms.push_back(Room("Hall of Echoes",
        "Marble columns sing softly when touched by the wind. Every sound here returns as music, "
        "layered and harmonious."));
    rooms.push_back(Room("Room That Remembers",
        "Polished stone reflects your image clearly. When you move, your reflection follows perfectly, "
        "and the air smells faintly of cedar and sunlight."));
    rooms.push_back(Room("Echoing Gallery",
        "A long hallway of golden mosaics, each panel telling a story in vibrant color. "
        "A warm breeze stirs the air.", true, 2));

    // ===== Hecate =====
    Shrine hecate("Hecate", "The Luminous Path");
    hecate.setState(ShrineState::UNCORRUPTED);
    shrineRegistry[3] = hecate;
    rooms.push_back(Room("Loom of Names",
        "Threads of silk stretch across a great frame, each glowing faintly. The sound of weaving is calm and steady."));
    rooms.push_back(Room("Listening Chamber",
        "Shells line the walls, carrying the sound of the sea. When you speak, the shells sing your words back in harmony."));
    rooms.push_back(Room("The Luminous Path",
        "Lanterns guide the way forward, their flames steady. The path is straight, and the air feels safe.", true, 3));

    // ===== Persephone =====
    Shrine persephone("Persephone", "The Blooming Spring");
    persephone.setState(ShrineState::UNCORRUPTED);
    shrineRegistry[4] = persephone;
    rooms.push_back(Room("Hall of Petals",
        "Petals drift down from unseen branches, gathering softly on the floor. Their fragrance is sweet and light."));
    rooms.push_back(Room("Orchard Walk",
        "Rows of fruit trees stand heavy with blossoms, their branches gently swaying in a warm breeze."));
    rooms.push_back(Room("The Blooming Spring",
        "A clear spring flows gently, surrounded by flowers in full bloom. The air hums with bees and distant laughter.", true, 4));

    // ===== Pan =====
    Shrine pan("Pan", "Verdant Rotunda");
    pan.setState(ShrineState::UNCORRUPTED);
    shrineRegistry[5] = pan;
    rooms.push_back(Room("Hall of Living Wood",
        "The walls are carved from living trees, their leaves whispering overhead. The smell of earth and moss is fresh and clean."));
    rooms.push_back(Room("Den of Antlers",
        "Antlers adorn the walls, polished and unbroken. The floor is covered in soft ferns, and somewhere, a flute plays."));
    rooms.push_back(Room("Verdant Rotunda",
        "A round chamber open to the sky, where ivy climbs the stone walls and birds nest in the beams.", true, 5));

    // ===== False Hermes =====
    Shrine falseHermes("False Hermes", "Gilded Hallway");
    falseHermes.setState(ShrineState::UNCORRUPTED);
    shrineRegistry[6] = falseHermes;
    rooms.push_back(Room("Room of Borrowed Things",
        "Neatly arranged items rest on shelves, each labeled with care. A faint smell of parchment fills the air."));
    rooms.push_back(Room("Whispering Hall",
        "Words are etched in flowing script across the walls, each telling a gentle tale. The sound of quills scratching is faintly heard."));
    rooms.push_back(Room("Gilded Hallway",
        "Golden panels reflect your image in warm light. The floor is swept clean, and the air smells of incense.", true, 6));

    // ===== Thanatos =====
    Shrine thanatos("Thanatos", "Hall of Quiet Rest");
    thanatos.setState(ShrineState::UNCORRUPTED);
    shrineRegistry[7] = thanatos;
    rooms.push_back(Room("Room of Waiting Lights",
        "Lanterns hang in still air, each burning steadily. The silence here is peaceful and complete."));
    rooms.push_back(Room("Waiting Room",
        "Cushioned benches face a great window where clouds drift by slowly. A pot of tea sits untouched on a table."));
    rooms.push_back(Room("Hall of Quiet Rest",
        "Tall doors open onto a calm garden where no wind stirs. The only sound is the quiet hum of the earth.", true, 7));

    // ===== Eris =====
    Shrine eris("Eris", "Hall of Harmony");
    eris.setState(ShrineState::UNCORRUPTED);
    shrineRegistry[8] = eris;
    rooms.push_back(Room("Throat of the Temple",
        "A wide, bright corridor where banners sway gently. Sunlight spills in through high arches."));
    rooms.push_back(Room("Oracle’s Wake",
        "A polished altar draped in white cloth. Candles burn steadily, their wax dripping slowly onto silver trays."));
    rooms.push_back(Room("Archivist’s Cell",
        "A tidy desk stacked with neatly bound books. The air smells of ink and lavender, and a quill rests in an open journal."));
    rooms.push_back(Room("Hall of Harmony",
        "A vaulted chamber filled with soft music and the glow of stained glass. Dust motes drift in the warm light.", true, 8));

    player.setCurrentRoom(indexByTitle("Main Hall of the Temple"));
    setupPrologueConnectionsByTitle();
    lastEnteredRoom_ = -1; // ensure OnRoomEntered won't suppress first render

    // now run the 7-day loop
    runLysaiaPrologue();

    // wrap up
    inPrologue_ = false;
    // phase will be set to MainMenu by your caller before showing menu
}


void Game::syncInputAfterPrologue() {
    // If the prologue set fail/eof, recover.
    if (std::cin.fail()) std::cin.clear();

    // If a newline is sitting there, eat exactly one.
    if (std::cin.peek() == '\n') { std::cin.get(); return; }

    // If there’s buffered junk up to a newline, drain it non-blockingly.
    // NOTE: rdbuf()->in_avail() may be 0 on TTY; we only ignore when data exists.
    std::streamsize avail = std::cin.rdbuf()->in_avail();
    if (avail > 0) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void Game::beginMelasRun() {
    SceneManager::introScene();

    // Fresh state for a clean run
    g_flags.clear();
    lastEnteredRoom_ = -1;
    firstFramePrinted_ = false;

    InitMechanics(&journalManager, /*isMelasPlaythrough=*/true);
    ThemeRegistry::setDefaultShrineState(ShrineState::CORRUPTED);

    loadRooms();

    // Do NOT pre-print here; gameLoop will print once and trigger OnRoomEntered
    gameLoop();
}
void Game::beginDescent() {
    phase_ = Phase::Intro;

    printTitleBlock();

    // Your intro text (deliberately blank player name)
    std::cout
        << "They gave her a name that was not hers: Cassandra.\n"
        << "You are _____, a former oracle, cast out.\n"
        << "Now the temple is open again.\n\n"
        << "> Press ENTER to descend.\n";

    waitForEnter();

    // After ENTER, we are officially in-game. Print the starting room ONCE.
    phase_ = Phase::InGame;
    describeCurrentRoom();
    firstFramePrinted_ = true;
}
void Game::printTitleBlock() {
    std::cout << "THE ORACLES ARE BLEEDING\n\n";
}

void Game::waitForEnter() {
    std::string _;
    std::getline(std::cin, _);
}



void Game::displayMainMenu() {
    phase_ = Phase::MainMenu;

    for (;;) {
        std::cout
            << "==== THE ORACLES ARE BLEEDING ====\n"
            << "1. Begin Descent\n"
            << "2. Accessibility Options\n"
            << "3. View Temple Map\n"
            << "4. Exit\n"
            << "Choice: ";

        std::string choice;
        if (!std::getline(std::cin, choice)) {
            // Recover from stray EOF/fail and re-prompt
            if (std::cin.fail() || std::cin.eof()) {
                std::cin.clear();
                continue; // redraw menu
            }
            return; // truly broken input; exit
        }

        // normalize to first non-space character
        char c = 0;
        for (char ch : choice) {
            if (!std::isspace(static_cast<unsigned char>(ch))) { c = ch; break; }
        }

        if (c == '1') {
            beginMelasRun();         // setup only; NO prints
            lastEnteredRoom_ = -1;
            firstFramePrinted_ = false;

            beginDescent();          // title + intro; waits for ENTER; calls describe once
            gameLoop();              // main run loop
            // when gameLoop returns, fall back to menu (for replays)
        }
        else if (c == '2') {
            toggleAccessibility();
        }
        else if (c == '3') {
            showMap();
        }
        else if (c == '4') {
            return;
        }
        else {
            std::cout << "The gods do not understand that choice.\n";
        }
    }
}




void Game::showMap() {
    std::cout << "\n";
    templeMap.printAscii();
    templeMap.printAdjacency();
}

void Game::start() {
    startLysaiaPrologue();      // plays 7 days and returns
    syncInputAfterPrologue();   // <-- clear any leftover input/EOF
    phase_ = Phase::MainMenu;
    displayMainMenu();          // menu loop
}



void Game::loadRooms() {
    rooms.clear();
    shrineRegistry.clear();
    roomConnections.clear();
    lastEnteredRoom_ = -1;  
    // ===== Main Hall =====
    rooms.push_back(Room(
        "Main Hall of the Temple",
        "Massive pillars rise toward a shadowed ceiling. Faded mosaics depict gods whose eyes seem to follow you. "
        "Eight arched corridors lead away into darkness, each humming faintly with a presence."
    ));
    player.setCurrentRoom(0);

    // ===== Demeter =====
    Shrine demeter("Demeter", "The Hall of Hunger");
    demeter.setState(ShrineState::CORRUPTED);
    shrineRegistry[0] = demeter;
    rooms.push_back(Room("The Garden of Broken Faces", "Masks litter the overgrown path—some smiling, some cracked in despair. A vine-covered mirror stands at the center, reflecting only strangers", false));
    rooms.push_back(Room("The Threadbare Womb", "The walls are made of fibrous, pulsing material—almost alive. A faint heartbeat hums under your feet. An empty cradle sits in the center, rocking gently though no one is near.", false));
    rooms.push_back(Room("The Hall of Hunger", "Withered olive trees claw at the cracked marble. Bowls overflow with bloated grain—writhing, weeping, moving. The air stinks of soured milk and blood turned syrup-thick. Vines sprawl across the floor like the intestines of slaughtered offerings, knotted and twitching. You hear chewing—but nothing moves.", true, 0));

    // ===== Nyx =====
    Shrine nyx("Nyx", "The Starless Well");
    nyx.setState(ShrineState::CORRUPTED);
    shrineRegistry[1] = nyx;
    rooms.push_back(Room("Room With No Corners", "The walls curve softly into one another. There are no shadows, no edges. You always feel like you’re at the center—even when walking. Something breathes in rhythm with you.", false));
    rooms.push_back(Room("Nest of Wings", "The ceiling is unseen. Black feathers drift downward. A nest of glass bones sits abandoned. You’re certain you heard wings—but only once.", false));
    rooms.push_back(Room("The Starless Well", "A smooth pit swallows light and sound. Glyphs etched into obsidian pulse faintly—recognizable and wrong. When you lean over the edge, your shadow vanishes. Something down there watches, not with eyes, but with intention. You forget why you’re breathing.", true, 1));

    // ===== Apollo =====
    Shrine apollo("Apollo", "Echoing Gallery");
    apollo.setState(ShrineState::CORRUPTED);
    shrineRegistry[2] = apollo;
    rooms.push_back(Room("Hall of Echoes", "Every step you take repeats a second later—just slightly out of sync. A chorus murmurs words you almost recognize. If you speak, something replies from behind.", false));
    rooms.push_back(Room("Room That Remembers", "Every surface is mirrored, but you’re never alone. Sometimes your reflection lags. Sometimes it moves first. Sometimes it’s gone entirely—but you still feel watched.", false));
    rooms.push_back(Room("Echoing Gallery", "Mirrors line the walls, angled just wrong. They show you—but older, injured, smiling. The statues have mouths but no faces. You swear one whispered your name, the one you haven’t heard since childhood. But it didn’t speak. Or did it?", true, 2));

    // ===== Hecate =====
    Shrine hecate("Hecate", "The Unlit Path");
    hecate.setState(ShrineState::CORRUPTED);
    shrineRegistry[3] = hecate;
    rooms.push_back(Room("Loom of Names", "Threads hang like veins, each labeled in ink. One bears your name. Another is frayed. The loom creaks but never stops. Something is weaving nearby, just out of sight.", false));
    rooms.push_back(Room("Listening Chamber", "Shells line the walls, hung like ears. Some whisper forgotten hymns. Others sob. When you breathe, a shell beside you repeats it a beat too late.", false));
    rooms.push_back(Room("The Unlit Path", "Three stone doors. One burns with blue flame, one drips something thick, one is just absence. Candle stubs mark the walls in patterns that shift when unobserved. The torchlight flickers—but the shadows don’t match your shape. One shadow walks when you don’t.", true, 3));

    // ===== Persephone =====
    Shrine persephone("Persephone", "The Frozen Spring");
    persephone.setState(ShrineState::CORRUPTED);
    shrineRegistry[4] = persephone;
    rooms.push_back(Room("Hall of Petals", "Petals fall stiff and brittle, shattering when they hit the floor. Frost rims each fragment, though the air smells of funeral incense.", false));
    rooms.push_back(Room("Orchard Walk", "Each tree weeps slow rivulets that freeze mid-drip, like tears caught in the act of falling.", false));
    rooms.push_back(Room("The Frozen Spring", "A fountain of vines now fossilized, curled in agony. Ice creeps up the edges of the walls, though the air is warm. A lone pomegranate seed rests in a cracked bowl. It has not rotted. It will not. The room smells of rotting flowers and ash...You feel mourned.", true, 4));

    // ===== Pan =====
    Shrine pan("Pan", "Wild Rotunda");
    pan.setState(ShrineState::CORRUPTED);
    shrineRegistry[5] = pan;
    rooms.push_back(Room("Hall of Shivering Meat", "Walls pulse with veins beneath translucent skin. Occasionally, a muscle twitches in the stone. A single pan flute lies on the ground—when touched, it plays a bleating cry.", false));
    rooms.push_back(Room("Den of Antlers", "Bones and antlers are fused into the architecture. The floor is covered in fur—not all of it animal. Something stalks just out of view, its gait rhythmic, almost... joyful.", false));
    rooms.push_back(Room("Wild Rotunda", "The walls pulse with root-veined moss, soft and warm as skin. Bones protrude from the growth—dancing mid-step, arms locked in joy or agony. Laughter echoes, then sobs, then silence. A damp breath tickles your neck, and no one is there.", true, 5));

    // ===== False Hermes =====
    Shrine falseHermes("False Hermes", "Gilded Hallway");
    falseHermes.setState(ShrineState::CORRUPTED);
    shrineRegistry[6] = falseHermes;
    rooms.push_back(Room("Room of Borrowed Things", "Shelves display small, mundane objects—combs, rings, sandals, letters. Each is labeled with a name you don’t recognize. One item is missing, but its tag reads your name. A drawer creaks open behind you.", false));
    rooms.push_back(Room("Whispering Hall", "Words are etched into every surface. None are repeated. The longer you stare, the more familiar the languages seem—until you find your own handwriting, carved deep and frantic.", false));
    rooms.push_back(Room("Gilded Hallway", "The marble gleams too clean. The walls shimmer like heatstroke. A friendly shrine waits at the end, grinning with a mouth it doesn’t have. You walk twenty-one steps. You always walk twenty-one steps. You don’t remember starting, but you’re always in motion.", true, 6));

    // ===== Thanatos =====
    Shrine thanatos("Thanatos", "Sleepwalker’s Alcove");
    thanatos.setState(ShrineState::CORRUPTED);
    shrineRegistry[7] = thanatos;
    rooms.push_back(Room("Room of Waiting Lights", "Hundreds of unlit candles line the floor. One flickers to life when you step inside, then another. None provide warmth. The air smells like burnt honey and salt.", false));
    rooms.push_back(Room("The Bloodclock", "A massive pendulum drips red into an unseen basin. It beats steadily—too slowly to match your pulse. On the wall: ■ν α■µατι χρ■νου. ('In the blood of time.')", false));
    rooms.push_back(Room("Sleepwalker’s Alcove", "A stone bed rests beneath an unlit arch. The room is warm—not comfort, but absence of discomfort. Laurel leaves line the floor, pale and dry. The silence here is full, whole. You think about lying down. Just for a moment. You imagine how easy it would be to stay. You do not remember why that’s a problem.", true, 7));

    // ===== Eris =====
    Shrine eris("Eris", "The Bone Choir");
    eris.setState(ShrineState::CORRUPTED);
    shrineRegistry[8] = eris;
    rooms.push_back(Room("Throat of the Temple", "The corridor narrows slowly behind you. The walls are damp and warm to the touch. You hear a low, slow heartbeat. Every step echoes like a swallowed breath.", false));
    rooms.push_back(Room("Oracle’s Wake", "Candles flicker in defiance of windless dark. A defaced altar bleeds wax. Someone scratched 'I won’t lie again' into the stone 27 times.", false));
    rooms.push_back(Room("Archivist’s Cell", "A rusted desk faces the wall. Dozens of inked notes are nailed above it—each crossed out violently. Scratched into the desk: 'It was true. That’s the problem.' The chair is still warm.", false));
    rooms.push_back(Room("The Bone Choir", "The bones are arranged in reverent poses, facing each other in song. Their mouths hang wide in eternal performance. The acoustics claw at your skull—discordant, divine, unending. Your ears bleed, or maybe your thoughts do. Their hymn harmonizes with your name.", true, 8));

    setupConnections();
}


void Game::setupConnections() {
    // Main Hall spokes
    addEdge(0, "north",     1);
    addEdge(0, "northeast", 4);
    addEdge(0, "east",      7);
    addEdge(0, "southeast", 10);
    addEdge(0, "south",     13);
    addEdge(0, "southwest", 16);
    addEdge(0, "west",      19);
    addEdge(0, "northwest", 22);
    addEdge(0, "up",        25);

    // Demeter (1–3)
    addEdge(1, "east", 2); addEdge(2, "west", 1);
    addEdge(2, "east", 3); addEdge(3, "west", 2);
    addEdge(1, "south", 0); addEdge(2, "south", 0); addEdge(3, "south", 0);

    // Nyx (4–6)
    addEdge(4, "east", 5); addEdge(5, "west", 4);
    addEdge(5, "east", 6); addEdge(6, "west", 5);
    addEdge(4, "southwest", 0); addEdge(5, "southwest", 0); addEdge(6, "southwest", 0);

    // Apollo (7–9)
    addEdge(7, "east", 8); addEdge(8, "west", 7);
    addEdge(8, "east", 9); addEdge(9, "west", 8);
    addEdge(7, "west", 0); addEdge(8, "west", 0); addEdge(9, "west", 0);

    // Hecate (10–12)
    addEdge(10, "east", 11); addEdge(11, "west", 10);
    addEdge(11, "east", 12); addEdge(12, "west", 11);
    addEdge(10, "northwest", 0); addEdge(11, "northwest", 0); addEdge(12, "northwest", 0);

    // Persephone (13–15)
    addEdge(13, "east", 14); addEdge(14, "west", 13);
    addEdge(14, "east", 15); addEdge(15, "west", 14);
    addEdge(13, "north", 0); addEdge(14, "north", 0); addEdge(15, "north", 0);

    // Pan (16–18)
    addEdge(16, "east", 17); addEdge(17, "west", 16);
    addEdge(17, "east", 18); addEdge(18, "west", 17);
    addEdge(16, "northeast", 0); addEdge(17, "northeast", 0); addEdge(18, "northeast", 0);

    // False Hermes (19–21)
    addEdge(19, "east", 20); addEdge(20, "west", 19);
    addEdge(20, "east", 21); addEdge(21, "west", 20);
    addEdge(19, "north", 0); addEdge(20, "north", 0); addEdge(21, "north", 0);

    // Thanatos (22–24)
    addEdge(22, "east", 23); addEdge(23, "west", 22);
    addEdge(23, "east", 24); addEdge(24, "west", 23);
    addEdge(22, "southeast", 0); addEdge(23, "southeast", 0); addEdge(24, "southeast", 0);

    // Eris (25–28)
    addEdge(25, "east", 26); addEdge(26, "west", 25);
    addEdge(26, "east", 27); addEdge(27, "west", 26);
    addEdge(27, "east", 28); addEdge(28, "west", 27);
    addEdge(25, "down", 0); addEdge(26, "down", 0); addEdge(27, "down", 0); addEdge(28, "down", 0);
}


void Game::handleCommand(const std::string& input) {
    const std::string raw = trim_copy(input);
    if (raw.empty()) { std::cout << "...\n"; return; }

    const auto [first, rest] = split_first(raw);
    const std::string cmd = toLower(raw); // full lowercased command for single-word checks

    // ===== Movement =====
    static const std::vector<std::string> directions = {
        "north","south","east","west",
        "northeast","northwest","southeast","southwest",
        "up","down"
    };

    // 2-word verbs like "go north", "move east", "head up"
    if (is_move_verb(first)) {
        const std::string dir = normalize_dir(rest);
        if (!dir.empty()) {
            player.move(dir, roomConnections);
            describeCurrentRoom();
            return;
        }
        std::cout << "Go where? (Try: " << join(directions, ", ") << ")\n";
        return;
    }

    // One-word directions and short forms: "n", "sw", "up", etc.
    if (auto dir = normalize_dir(cmd); !dir.empty()) {
        player.move(dir, roomConnections);
        describeCurrentRoom();
        return;
    }

   // ===== Shrine interaction =====
if (cmd == "shrine") {
    const int cur = player.getCurrentRoom();
    if (cur < 0 || cur >= static_cast<int>(rooms.size())) {
        std::cout << "You are nowhere near a shrine.\n";
        return;
    }

    Room& current = rooms[cur];
    if (!current.isShrine()) {
        std::cout << "There is no shrine here.\n";
        return;
    }

    const int shrineID = current.getShrineID();
    auto it = shrineRegistry.find(shrineID);
    if (it == shrineRegistry.end()) {
        std::cout << "The shrine seems dormant.\n";
        return;
    }

    // Optional flavor lead-in (styled per deity/state)
    printShrineText(it->second, "You approach the altar.", /*shake=*/false);

    // Mechanics dispatcher (runs the real shrine logic + outcomes/journal)
    OnShrineInteract(it->second, &journalManager);
    return;
}
    // ===== Look around =====
    if (first == "look" || cmd == "look around") {
       describeCurrentRoom();}


    // ===== Journal =====
    if (cmd == "journal") {
        player.printJournal();
        return;
    }
    else if (first == "note") {
        // keep original after 'note ' for free-form text
        std::istringstream iss(rest);
        int entryNumber;
        if (!(iss >> entryNumber)) {
            std::cout << "Usage: note <entry#> <text>\n";
            return;
        }
        std::string afterNum;
        std::getline(iss, afterNum);
        if (!afterNum.empty() && afterNum[0] == ' ') afterNum.erase(0, 1);
        if (afterNum.empty()) {
            std::cout << "Write something after the entry number.\n";
            return;
        }
        player.addJournalNote(entryNumber, afterNum);
        std::cout << "Noted.\n";
        return;
    }
    else if (first == "inspect") {
        std::istringstream iss(rest);
        int entryNumber;
        if (!(iss >> entryNumber)) {
            std::cout << "Usage: inspect <entry#>\n";
            return;
        }
        player.inspectJournalEntry(entryNumber - 1); // 0-based
        return;
    }

    // ===== Map (only in Main Hall) =====
    if (cmd == "map") {
        if (player.getCurrentRoom() == 0) {
            showMap();
        } else {
            std::cout << "You can only consult the map from the Main Hall.\n";
        }
        return;
    }

    // ===== Write (Melas free-write to current location) =====
    if (toLower(first) == "write" || cmd == "write") {
    const int cur = player.getCurrentRoom();
    if (cur >= 0 && cur < static_cast<int>(rooms.size())) {
        const std::string loc = toLocationId(rooms[cur].getName());
        if (!loc.empty()) {
            journalManager.writeMelasAt(loc);
            std::cout << "(Journal updated.)\n";
        } else {
            std::cout << "Your hand hesitates. Nothing here wants to be recorded.\n";
        }
    }
    return;
}
    // ===== Help =====
    if (cmd == "help") {
        std::cout << "Commands:\n"
                  << "  Movement: " << join(directions, ", ") << " (also: n, s, e, w, ne, nw, se, sw, u, d)\n"
                  << "            go/move/walk/run/head/travel <direction>\n"
                  << "  look / look around\n"
                  << "  shrine\n"
                  << "  journal\n"
                  << "  note <entry#> <text>\n"
                  << "  inspect <entry#>\n"
                  << "  map (Main Hall only)\n"
                  << "  write\n"
                  << "  help\n";
        return;
    }

    // ===== Unknown =====
    std::cout << "Unknown command. Type 'help' for a list of commands.\n";
}

// --- Temporary minimal implementations to satisfy linker ---
void Game::toggleAccessibility() {
    accessibility_.colorEnabled       = !accessibility_.colorEnabled;
    accessibility_.screenShakeEnabled = !accessibility_.screenShakeEnabled;
    std::cout << "Color: " << (accessibility_.colorEnabled ? "ON" : "OFF")
              << ", Shake: " << (accessibility_.screenShakeEnabled ? "ON" : "OFF") << "\n";
}

// ===== PATCH 5: print-once loop stays; minor safety flush =====
void Game::gameLoop() {
    isRunning = true;
    // Do NOT call describeCurrentRoom() here.
    while (isRunning) {
        std::cout << "\n> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") { isRunning = false; break; }
        handleCommand(line);
        // No automatic room reprint here.
    }
}

