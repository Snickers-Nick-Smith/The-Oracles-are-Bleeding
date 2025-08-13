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
#include <algorithm>
#include <ctime>
#include <cctype>
#include <cstdlib>
#include <sstream>

namespace {
    std::string normalize_dir(std::string d); // forward declare
}

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
        {"the waiting room",           Deity::Thanatos},
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

static void SyncFromGamePlayer(const Player& /*p*/, PlayerState& /*s*/) {}
static void SyncToGamePlayer(const PlayerState& /*s*/, Player& /*p*/) {}


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
    // Persephone letter fragment auto-pickups (Melas only)
    CheckPersephoneLetterPickupsForRoom(ctx, roomTitle);
    // (If any pickup happened, the bridge already wrote the journal and applied outcomes)
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
        {"The Threadbare Womb",       "demeter/room/threadbare_womb"},
        {"The Hall of Hunger",        "demeter/shrine"},
        {"Garden of Blooming Faces",  "demeter/room/garden_of_blooming_faces"}, // new
        {"Threaded Womb",             "demeter/room/threaded_womb"},            // new
        {"Hall of Plenty",            "demeter/shrine_uncorrupted"},            // new (use your preferred id)

        // ===== Nyx =====
        {"Room With No Corners",      "nyx/room/no_corners"},
        {"Nest of Wings",             "nyx/room/nest_of_wings"},
        {"The Starless Well",         "nyx/shrine"},
        {"Room of Gentle Horizons",   "nyx/room/gentle_horizons"},              // new
        {"The Star-Bound Well",       "nyx/shrine_uncorrupted"},                // new

        // ===== Apollo =====
        {"Hall of Echoes",            "apollo/room/hall_of_echoes"},
        {"Room That Remembers",       "apollo/room/room_that_remembers"},
        {"Echoing Gallery",           "apollo/shrine"},

        // ===== Hecate =====
        {"Loom of Names",             "hecate/room/loom_of_names"},
        {"Listening Chamber",         "hecate/room/listening_chamber"},
        {"The Unlit Path",            "hecate/shrine"},
        {"The Luminous Path",         "hecate/shrine_uncorrupted"},             // new

        // ===== Persephone =====
        {"Hall of Petals",            "persephone/room/hall_of_petals"},
        {"Orchard Walk",              "persephone/room/orchard_walk"},
        {"The Frozen Spring",         "persephone/shrine"},
        {"The Blooming Spring",       "persephone/shrine_uncorrupted"},         // new

        // ===== Pan =====
        {"Hall of Shivering Meat",    "pan/room/hall_of_shivering_meat"},
        {"Den of Antlers",            "pan/room/den_of_antlers"},
        {"Wild Rotunda",              "pan/shrine"},
        {"Hall of Living Wood",       "pan/room/hall_of_living_wood"},          // new
        {"Verdant Rotunda",           "pan/shrine_uncorrupted"},                // new

        // ===== False Hermes =====
        {"Room of Borrowed Things",   "false_hermes/room/borrowed_things"},
        {"Whispering Hall",           "false_hermes/room/whispering_hall"},
        {"Gilded Hallway",            "false_hermes/shrine"},

        // ===== Thanatos =====
        {"Room of Waiting Lights",    "thanatos/room/room_of_waiting_lights"},
        {"The Room of Waiting Lights","thanatos/room/room_of_waiting_lights"},
        {"Waiting Room",              "thanatos/room/waiting_room"},
        {"The Waiting Room",          "thanatos/room/waiting_room"},
        {"Sleepwalker’s Alcove",      "thanatos/shrine"},
        {"Hall of Quiet Rest",        "thanatos/shrine_uncorrupted"},           // new

        // ===== Eris =====
        {"Oracle’s Wake",             "eris/room/oracles_wake"},
        {"Archivist’s Cell",          "eris/room/archivists_cell"},
        {"Throat of the Temple",      "eris/room/throat_of_temple"},
        {"The Bone Choir",            "eris/shrine"},
        {"Hall of Harmony",           "eris/shrine_uncorrupted"},               // new

        // ===== Hub =====
        {"Main Hall of the Temple",   ""}
    };

    auto it = kRoomToLocId.find(roomName);
    return (it != kRoomToLocId.end()) ? it->second : std::string{};
}

// Room Descriptor
void Game::describeCurrentRoom() {
    const int id = player.getCurrentRoom();
    const Room& current = rooms[id];  
    OnRoomEntered(current.getName());

    // Color the room description based on deity
    printRoomDescriptionColored(current, current.getDescription());

    // (Optional) list visible exits
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
    PrologueController::Hooks hooks;

    hooks.describe = [this]() { describeCurrentRoom(); };

    hooks.listExits = [this]() {
        const int cur = player.getCurrentRoom();
        auto it = roomConnections.find(cur);
        if (it == roomConnections.end() || it->second.empty()) {
            std::cout << "No obvious exits.\n";
            return;
        }
        std::vector<std::string> dirs;
        dirs.reserve(it->second.size());
        for (const auto& kv : it->second) dirs.push_back(kv.first);
        std::sort(dirs.begin(), dirs.end());
        std::cout << "Exits: " << join(dirs, ", ") << "\n";
    };

    hooks.moveTo = [this](const std::string& target) -> bool {
        // Try direction first (supports n, sw, up, etc.) using your normalize_dir()
        if (auto dir = normalize_dir(target); !dir.empty()) {
            player.move(dir, roomConnections);
            describeCurrentRoom();
            return true;
        }
        // Try room title among visible neighbors
        const int cur = player.getCurrentRoom();
        auto it = roomConnections.find(cur);
        if (it == roomConnections.end()) { std::cout << "You can’t move from here.\n"; return false; }

        const std::string t = toLower(target);
        for (const auto& kv : it->second) {
            int idx = kv.second;
            if (toLower(rooms[idx].getName()) == t) {
                player.setCurrentRoom(idx);
                std::cout << "You move to: " << rooms[idx].getName() << "\n";
                describeCurrentRoom();
                return true;
            }
        }
        std::cout << "No path to '" << target << "'. Try 'exits'.\n";
        return false;
    };

hooks.writeJournal = [this](int day) {
    const int cur = player.getCurrentRoom();
    const Room& r = rooms[cur];

    // 1) Room-specific entry (if mapped)
    const std::string loc = toLocationId(r.getName());
    if (!loc.empty()) {
        journalManager.writeLysaiaAt(loc);
    } else {
        journalManager.writeLysaia("I wrote in an unmarked place, to keep it from becoming strange.");
    }

    // 2) Shrine attempt entry — first time only, if this room is a shrine
    if (r.isShrine() && !lysaiaShrinesLogged_.count(cur)) {
        // prefer uncorrupted shrine location id if available; fall back to generic shrine id
        std::string shrineKey = loc;
        if (shrineKey.empty()) {
            // Derive by deity if needed (matches IDs we seeded)
            const Deity d = deityFromRoomName(r.getName());
            switch (d) {
                case Deity::Demeter:    shrineKey = "demeter/shrine_uncorrupted"; break;
                case Deity::Nyx:        shrineKey = "nyx/shrine_uncorrupted"; break;
                case Deity::Apollo:     shrineKey = "apollo/shrine"; break;
                case Deity::Hecate:     shrineKey = "hecate/shrine_uncorrupted"; break;
                case Deity::Persephone: shrineKey = "persephone/shrine_uncorrupted"; break;
                case Deity::Pan:        shrineKey = "pan/shrine_uncorrupted"; break;
                case Deity::FalseHermes:shrineKey = "false_hermes/shrine"; break;
                case Deity::Thanatos:   shrineKey = "thanatos/shrine_uncorrupted"; break;
                case Deity::Eris:       shrineKey = "eris/shrine_uncorrupted"; break;
                default: break;
            }
        }
        if (!shrineKey.empty()) {
            journalManager.writeLysaiaAt(shrineKey);
        }
        lysaiaShrinesLogged_.insert(cur);
    }

    // 3) Day-specific guilt beat
    journalManager.writeLysaiaGuiltBeat(day);

    std::cout << "You light the candle and write. The ink dries in steady lines.\n";
};

    hooks.promptPrefix = [this]() -> std::string {
    std::ostringstream oss;
    oss << "[" << rooms[player.getCurrentRoom()].getName() << "] > ";
    return oss.str();
};



    PrologueController prologue(hooks);
    prologue.run();

    // Dump to main menu when done
    displayMainMenu();
}

Game::Game() : isRunning(true) {
}


void Game::startLysaiaPrologue() {
    InitMechanics(&journalManager, /*isMelasPlaythrough=*/false);
    rooms.clear();
    shrineRegistry.clear();
    roomConnections.clear();

    // ===== Main Hall =====
    rooms.push_back(Room(
        "Main Hall of the Temple",
        "Sunlight streams through high windows, casting bright patterns across polished marble. The air is warm, "
        "and the faint sound of lyres drifts from unseen corridors."
    ));
    player.setCurrentRoom(0);

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
    rooms.push_back(Room("The Waiting Room",
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

    setupConnections();
    runLysaiaPrologue();
    lysaiaShrinesLogged_.clear();
    journalManager.seedLysaiaPrologueText();
    runLysaiaPrologue();
}


void Game::displayMainMenu() {
    int choice;
    do {
        std::cout << "\n==== THE ORACLES ARE BLEEDING ====\n";
        std::cout << "1. Begin Descent\n";
        std::cout << "2. Accessibility Options\n";
        std::cout << "3. View Temple Map\n";
        std::cout << "4. Exit\n";
        std::cout << "Choice: ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1:
                start();  // begin game loop
                break;
            case 2:
                toggleAccessibility();
                break;
            case 3: showMap(); break;
            case 4:
                isRunning = false;
                break;
            default:
                std::cout << "The gods do not understand that choice.\n";
        }
    } while (isRunning);
}

void Game::showMap() {
    std::cout << "\n";
    templeMap.printAscii();
    templeMap.printAdjacency();
}

void Game::start() {
    SceneManager::introScene();
    InitMechanics(&journalManager, /*isMelasPlaythrough=*/true);
    loadRooms();
    gameLoop();
}

void Game::loadRooms() {
    rooms.clear();
    shrineRegistry.clear();

    roomConnections.clear();

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
    rooms.push_back(Room("The Garden of Broken Faces", "...", false));
    rooms.push_back(Room("The Threadbare Womb", "...", false));
    rooms.push_back(Room("The Hall of Hunger", "...", true, 0));

    // ===== Nyx =====
    Shrine nyx("Nyx", "The Starless Well");
    nyx.setState(ShrineState::CORRUPTED);
    shrineRegistry[1] = nyx;
    rooms.push_back(Room("Room With No Corners", "...", false));
    rooms.push_back(Room("Nest of Wings", "...", false));
    rooms.push_back(Room("The Starless Well", "...", true, 1));

    // ===== Apollo =====
    Shrine apollo("Apollo", "Echoing Gallery");
    apollo.setState(ShrineState::CORRUPTED);
    shrineRegistry[2] = apollo;
    rooms.push_back(Room("Hall of Echoes", "...", false));
    rooms.push_back(Room("Room That Remembers", "...", false));
    rooms.push_back(Room("Echoing Gallery", "...", true, 2));

    // ===== Hecate =====
    Shrine hecate("Hecate", "The Unlit Path");
    hecate.setState(ShrineState::CORRUPTED);
    shrineRegistry[3] = hecate;
    rooms.push_back(Room("Loom of Names", "...", false));
    rooms.push_back(Room("Listening Chamber", "...", false));
    rooms.push_back(Room("The Unlit Path", "...", true, 3));

    // ===== Persephone =====
    Shrine persephone("Persephone", "The Frozen Spring");
    persephone.setState(ShrineState::CORRUPTED);
    shrineRegistry[4] = persephone;
    rooms.push_back(Room("Hall of Petals", "...", false));
    rooms.push_back(Room("Orchard Walk", "...", false));
    rooms.push_back(Room("The Frozen Spring", "...", true, 4));

    // ===== Pan =====
    Shrine pan("Pan", "Wild Rotunda");
    pan.setState(ShrineState::CORRUPTED);
    shrineRegistry[5] = pan;
    rooms.push_back(Room("Hall of Shivering Meat", "...", false));
    rooms.push_back(Room("Den of Antlers", "...", false));
    rooms.push_back(Room("Wild Rotunda", "...", true, 5));

    // ===== False Hermes =====
    Shrine falseHermes("False Hermes", "Gilded Hallway");
    falseHermes.setState(ShrineState::CORRUPTED);
    shrineRegistry[6] = falseHermes;
    rooms.push_back(Room("Room of Borrowed Things", "...", false));
    rooms.push_back(Room("Whispering Hall", "...", false));
    rooms.push_back(Room("Gilded Hallway", "...", true, 6));

    // ===== Thanatos =====
    Shrine thanatos("Thanatos", "Sleepwalker’s Alcove");
    thanatos.setState(ShrineState::CORRUPTED);
    shrineRegistry[7] = thanatos;
    rooms.push_back(Room("Room of Waiting Lights", "...", false));
    rooms.push_back(Room("The Waiting Room", "...", false));
    rooms.push_back(Room("Sleepwalker’s Alcove", "...", true, 7));

    // ===== Eris =====
    Shrine eris("Eris", "The Bone Choir");
    eris.setState(ShrineState::CORRUPTED);
    shrineRegistry[8] = eris;
    rooms.push_back(Room("Oracle’s Wake", "...", false));
    rooms.push_back(Room("Archivist’s Cell", "...", false));
    rooms.push_back(Room("Throat of the Temple", "...", false));
    rooms.push_back(Room("The Bone Choir", "...", true, 8));

    setupConnections();
}


void Game::setupConnections() {
    // Main Hall spokes
    roomConnections[0]["north"]     = 1;   // Demeter start
    roomConnections[0]["northeast"] = 4;   // Nyx start
    roomConnections[0]["east"]      = 7;   // Apollo start
    roomConnections[0]["southeast"] = 10;  // Hecate start
    roomConnections[0]["south"]     = 13;  // Persephone start
    roomConnections[0]["southwest"] = 16;  // Pan start
    roomConnections[0]["west"]      = 19;  // False Hermes start
    roomConnections[0]["northwest"] = 22;  // Thanatos start
    roomConnections[0]["up"]        = 25;  // Eris start

    // Demeter (1–3)
    roomConnections[1]["east"] = 2; roomConnections[2]["west"] = 1;
    roomConnections[2]["east"] = 3; roomConnections[3]["west"] = 2;
    roomConnections[1]["south"] = 0; roomConnections[2]["south"] = 0; roomConnections[3]["south"] = 0;

    // Nyx (4–6)
    roomConnections[4]["east"] = 5; roomConnections[5]["west"] = 4;
    roomConnections[5]["east"] = 6; roomConnections[6]["west"] = 5;
    roomConnections[4]["southwest"] = 0; roomConnections[5]["southwest"] = 0; roomConnections[6]["southwest"] = 0;

    // Apollo (7–9)
    roomConnections[7]["east"] = 8; roomConnections[8]["west"] = 7;
    roomConnections[8]["east"] = 9; roomConnections[9]["west"] = 8;
    roomConnections[7]["west"] = 0; roomConnections[8]["west"] = 0; roomConnections[9]["west"] = 0;

    // Hecate (10–12)
    roomConnections[10]["east"] = 11; roomConnections[11]["west"] = 10;
    roomConnections[11]["east"] = 12; roomConnections[12]["west"] = 11;
    roomConnections[10]["northwest"] = 0; roomConnections[11]["northwest"] = 0; roomConnections[12]["northwest"] = 0;

    // Persephone (13–15)
    roomConnections[13]["east"] = 14; roomConnections[14]["west"] = 13;
    roomConnections[14]["east"] = 15; roomConnections[15]["west"] = 14;
    roomConnections[13]["north"] = 0; roomConnections[14]["north"] = 0; roomConnections[15]["north"] = 0;

    // Pan (16–18)
    roomConnections[16]["east"] = 17; roomConnections[17]["west"] = 16;
    roomConnections[17]["east"] = 18; roomConnections[18]["west"] = 17;
    roomConnections[16]["northeast"] = 0; roomConnections[17]["northeast"] = 0; roomConnections[18]["northeast"] = 0;

    // False Hermes (19–21)
    roomConnections[19]["east"] = 20; roomConnections[20]["west"] = 19;
    roomConnections[20]["east"] = 21; roomConnections[21]["west"] = 20;
    roomConnections[19]["east"] = 20; // already set above, removed overwrites
    roomConnections[19]["north"] = 0; roomConnections[20]["north"] = 0; roomConnections[21]["north"] = 0;

    // Thanatos (22–24)
    roomConnections[22]["east"] = 23; roomConnections[23]["west"] = 22;
    roomConnections[23]["east"] = 24; roomConnections[24]["west"] = 23;
    roomConnections[22]["southeast"] = 0; roomConnections[23]["southeast"] = 0; roomConnections[24]["southeast"] = 0;

    // Eris (25–28)
    roomConnections[25]["east"] = 26; roomConnections[26]["west"] = 25;
    roomConnections[26]["east"] = 27; roomConnections[27]["west"] = 26;
    roomConnections[27]["east"] = 28; roomConnections[28]["west"] = 27;
    roomConnections[25]["down"] = 0; roomConnections[26]["down"] = 0; roomConnections[27]["down"] = 0; roomConnections[28]["down"] = 0;
}

namespace {
    std::string trim_copy(std::string s) {
        auto not_space = [](int ch){ return !std::isspace(ch); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
        s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
        return s;
    }

    // Split into first word (lowercased) and the rest (trimmed, original case kept so notes work)
    std::pair<std::string,std::string> split_first(const std::string& input) {
        std::istringstream iss(input);
        std::string first;
        iss >> first;
        std::string rest;
        std::getline(iss, rest);
        if (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
        return { toLower(first), trim_copy(rest) };
    }

    // Normalize direction tokens: supports full, hyphenless, and short forms.
    // Returns empty string if not a direction.
    std::string normalize_dir(std::string d) {
        d = toLower(d);
        if (d == "n")  return "north";
        if (d == "s")  return "south";
        if (d == "e")  return "east";
        if (d == "w")  return "west";
        if (d == "ne") return "northeast";
        if (d == "nw") return "northwest";
        if (d == "se") return "southeast";
        if (d == "sw") return "southwest";
        if (d == "u" || d == "up") return "up";
        if (d == "d" || d == "down") return "down";
        // already full word? pass through if valid
        static const std::vector<std::string> dirs = {
            "north","south","east","west",
            "northeast","northwest","southeast","southwest",
            "up","down"
        };
        return (std::find(dirs.begin(), dirs.end(), d) != dirs.end()) ? d : std::string{};
    }

    bool is_move_verb(const std::string& w) {
        return w == "go" || w == "move" || w == "walk" || w == "run" || w == "head" || w == "travel";
    }
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
                  << "  help\n";
        return;
    }

    // ===== Unknown =====
    std::cout << "Unknown command. Type 'help' for a list of commands.\n";
}

// --- Temporary minimal implementations to satisfy linker ---
// (Adjust bodies later to your real logic.)

void Game::toggleAccessibility() {
    // TODO: flip your accessibility settings here if you have them stored.
    // e.g., settings.colorEnabled = !settings.colorEnabled;
}

void Game::gameLoop() {
    // TODO: implement your main loop. For now, a no-op keeps the linker happy.
}


