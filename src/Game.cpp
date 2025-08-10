#include "Game.hpp"
#include "SceneManager.hpp"
#include "Shrine.hpp"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <sstream>

static std::string toLocationId(const std::string& roomName) {
    // Map ROOM NAMES -> JournalManager location IDs
    static const std::unordered_map<std::string, std::string> kRoomToLocId = {
        // Demeter
        {"The Garden of Broken Faces", "demeter/room/garden_of_broken_faces"},
        {"The Threadbare Womb",       "demeter/room/threadbare_womb"},
        {"The Hall of Hunger",        "demeter/shrine"},

        // Nyx
        {"Room With No Corners",      "nyx/room/no_corners"},
        {"Nest of Wings",             "nyx/room/nest_of_wings"},
        {"The Starless Well",         "nyx/shrine"},

        // Apollo
        {"Hall of Echoes",            "apollo/room/hall_of_echoes"},
        {"Room That Remembers",       "apollo/room/room_that_remembers"},
        {"Echoing Gallery",           "apollo/shrine"},

        // Hecate
        {"Loom of Names",             "hecate/room/loom_of_names"},
        {"Listening Chamber",         "hecate/room/listening_chamber"},
        {"The Unlit Path",            "hecate/shrine"},

        // Persephone (updated names)
        {"Hall of Petals",            "persephone/room/hall_of_petals"},
        {"Orchard Walk",              "persephone/room/orchard_walk"},
        {"The Frozen Spring",         "persephone/shrine"},

        // Pan
        {"Hall of Shivering Meat",    "pan/room/hall_of_shivering_meat"},
        {"Den of Antlers",            "pan/room/den_of_antlers"},
        {"Wild Rotunda",              "pan/shrine"},

        // False Hermes
        {"Room of Borrowed Things",   "false_hermes/room/borrowed_things"},
        {"Whispering Hall",           "false_hermes/room/whispering_hall"},
        {"Gilded Hallway",            "false_hermes/shrine"},

        // Thanatos (updated names, accept both)
        {"Room of Waiting Lights",    "thanatos/room/room_of_waiting_lights"},
        {"The Room of Waiting Lights","thanatos/room/room_of_waiting_lights"},
        {"Waiting Room",              "thanatos/room/waiting_room"},
        {"The Waiting Room",          "thanatos/room/waiting_room"},
        {"Sleepwalker’s Alcove",      "thanatos/shrine"},

        // Eris
        {"Oracle’s Wake",             "eris/room/oracles_wake"},
        {"Archivist’s Cell",          "eris/room/archivists_cell"},
        {"Throat of the Temple",      "eris/room/throat_of_temple"},
        {"The Bone Choir",            "eris/shrine"},

        // Hub
        {"Main Hall of the Temple",   ""} // No journal entry for hub (by design)
    };

    auto it = kRoomToLocId.find(roomName);
    return (it != kRoomToLocId.end()) ? it->second : std::string{};
}

Game::Game() : isRunning(true) {
}


void Game::startLysaiaPlaythrough() {
    std::vector<Shrine> shrines;

    // Day 1 – Demeter
    Shrine demeter("Demeter", "Hall of Hunger");
    demeter.setState(ShrineState::UNCORRUPTED);
    demeter.addAssociatedRoom(Room("Garden of Broken Faces", "Masks litter the overgrown path—some smiling, some cracked in despair. A vine-covered mirror stands at the center, reflecting only strangers."));
    demeter.addAssociatedRoom(Room("Threadbare Womb", "The walls are made of fibrous, pulsing material—almost alive. A faint heartbeat hums under your feet. An empty cradle sits in the center, rocking gently though no one is near."));
    shrines.push_back(demeter);

    // Day 2 – Nyx
    Shrine nyx("Nyx", "Starless Well");
    nyx.setState(ShrineState::UNCORRUPTED);
    nyx.addAssociatedRoom(Room("Room With No Corners", "The walls curve softly into one another. There are no shadows, no edges. You always feel like you’re at the center—even when walking. Something breathes in rhythm with you."));
    nyx.addAssociatedRoom(Room("Nest of Wings", "The ceiling is unseen. Black feathers drift downward. A nest of glass bones sits abandoned. You’re certain you heard wings—but only once."));
    shrines.push_back(nyx);

    // Day 3 – Apollo
    Shrine apollo("Apollo", "Echoing Gallery");
    apollo.setState(ShrineState::UNCORRUPTED);
    apollo.addAssociatedRoom(Room("Hall of Echoes", "Every step you take repeats a second later—just slightly out of sync. A chorus murmurs words you almost recognize. If you speak, something replies from behind."));
    apollo.addAssociatedRoom(Room("Room That Remembers", "Every surface is mirrored, but you’re never alone. Sometimes your reflection lags. Sometimes it moves first. Sometimes it’s gone entirely—but you still feel watched."));
    shrines.push_back(apollo);

    // Day 4 – Hecate
    Shrine hecate("Hecate", "The Unlit Path");
    hecate.setState(ShrineState::UNCORRUPTED);
    hecate.addAssociatedRoom(Room("Loom of Names", "Threads hang like veins, each labeled in ink. One bears your name. Another is frayed. The loom creaks but never stops. Something is weaving nearby, just out of sight."));
    hecate.addAssociatedRoom(Room("Listening Chamber", "Shells line the walls, hung like ears. Some whisper forgotten hymns. Others sob. When you breathe, a shell beside you repeats it a beat too late."));
    shrines.push_back(hecate);

    // Day 5 – False Hermes
    Shrine hermes("False Hermes", "Gilded Hallway");
    hermes.setState(ShrineState::UNCORRUPTED);
    hermes.addAssociatedRoom(Room("Room of Borrowed Things", "Shelves display small, mundane objects—combs, rings, sandals, letters. Each is labeled with a name you don’t recognize. One item is missing, but its tag reads your name. A drawer creaks open behind you."));
    hermes.addAssociatedRoom(Room("Whispering Hall", "Words are etched into every surface. None are repeated. The longer you stare, the more familiar the languages seem—until you find your own handwriting, carved deep and frantic."));
    shrines.push_back(hermes);

    // Day 6 – Pan
    Shrine pan("Pan", "Wild Rotunda");
    pan.setState(ShrineState::UNCORRUPTED);
    pan.addAssociatedRoom(Room("Hall of Shivering Meat", "Walls pulse with veins beneath translucent skin. Occasionally, a muscle twitches in the stone. A single pan flute lies on the ground—when touched, it plays a bleating cry."));
    pan.addAssociatedRoom(Room("Den of Antlers", "Bones and antlers are fused into the architecture. The floor is covered in fur—not all of it animal. Something stalks just out of view, its gait rhythmic, almost... joyful."));
    shrines.push_back(pan);

    // Day 7 – Eris
    Shrine eris("Eris", "Bone Choir");
    eris.setState(ShrineState::UNCORRUPTED);
    eris.addAssociatedRoom(Room("Throat of the Temple", "The corridor narrows slowly behind you. The walls are damp and warm to the touch. You hear a low, slow heartbeat. Every step echoes like a swallowed breath."));
    eris.addAssociatedRoom(Room("Oracle’s Wake", "Candles flicker in defiance of windless dark. A defaced altar bleeds wax. Someone scratched “I won’t lie again” into the stone 27 times."));
    eris.addAssociatedRoom(Room("Archivist’s Cell", "A rusted desk faces the wall. Dozens of inked notes are nailed above it—each crossed out violently. Scratched into the desk: “It was true. That’s the problem.” The chair is still warm."));
    shrines.push_back(eris);
    

    std::cout << "\nNo more candlelight. No more writing. The Bone Choir waits.\n";
    isRunning = false;
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





void Game::handleCommand(const std::string& input) {
    std::string cmd = toLower(input);

    // ===== Movement =====
    static const std::vector<std::string> directions = {
    "north","south","east","west",
    "northeast","northwest","southeast","southwest",
    "up","down"
};

if (std::find(directions.begin(), directions.end(), cmd) != directions.end()) {
    player.move(cmd, roomConnections);
    describeCurrentRoom();
    return;
}

    // ===== Shrine interaction =====
    if (cmd == "shrine") {
        Room& current = rooms[player.getCurrentRoom()];
        if (current.isShrine()) {
            int shrineID = current.getShrineID();
            if (shrineRegistry.count(shrineID)) {
                shrineRegistry[shrineID].activate(player);
            } else {
                std::cout << "The shrine seems dormant.\n";
            }
        } else {
            std::cout << "There is no shrine here.\n";
        }
        return;
    }

    // ===== Look around =====
    if (cmd == "look") {
        describeCurrentRoom();
        return;
    }

    // ===== Journal =====
     else if (input == "journal") {
        player.printJournal();
         } else if (input.rfind("note ", 0) == 0) {
            std::istringstream iss(input.substr(5));
             int entryNumber;
             if (!(iss >> entryNumber)) {
             std::cout << "Usage: note <entry#> <text>\n";
          } else {
            std::string rest;
            std::getline(iss, rest);
        if (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
        if (rest.empty()) {
        std::cout << "Write something after the entry number.\n";
        } else {
        player.addJournalNote(entryNumber, rest);
        std::cout << "Noted.\n";
    
    if (input.rfind("inspect ", 0) == 0) {
    std::istringstream iss(input.substr(8));
    int entryNumber;
    if (!(iss >> entryNumber)) {
        std::cout << "Usage: inspect <entry#>\n";
    } else {
        player.inspectJournalEntry(entryNumber - 1); // convert to 0-based
    }
}
                                                                                                                                  }

    }

    // ===== Map (only in Main Hall) =====
    if (cmd == "map") {
        if (player.getCurrentRoom() == 0) {
            TempleMap templeMap;
            templeMap.populateFromDesign();
            templeMap.printAscii();
        } else {
            std::cout << "You can only consult the map from the Main Hall.\n";
        }
        return;
    }

    // ===== Help =====
    if (cmd == "help") {
        std::cout << "Commands:\n"
                  << "  Movement: " << join(directions, ", ") << "\n"
                  << "  look - reprint current room description\n"
                  << "  shrine - interact with shrine (if present)\n"
                  << "  journal - view your journal entries\n"
                  << "  map - view temple map (Main Hall only)\n"
                  << "  help - show this help menu\n";
        return;
    }

    std::cout << "Unknown command. Type 'help' for a list of commands.\n";
}


