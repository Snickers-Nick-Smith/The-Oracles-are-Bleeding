#include "Game.hpp"
#include "SceneManager.hpp"
#include "Shrine.hpp"
#include <iostream>

Game::Game() : isRunning(true) {}

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
        std::cout << "3. Exit\n";
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
            case 3:
                isRunning = false;
                break;
            default:
                std::cout << "The gods do not understand that choice.\n";
        }
    } while (isRunning);
}

void Game::start() {
    SceneManager::introScene();
    loadRooms();
    gameLoop();
}

void Game::loadRooms() {
    rooms.clear();
    shrineRegistry.clear();

    // Shrine 0: Demeter
    Shrine demeter("Demeter", "The Hall of Hunger");
    demeter.setState(ShrineState::CORRUPTED);
    shrineRegistry[0] = demeter;

    rooms.push_back(Room("The Garden of Broken Faces", "Masks litter the overgrown path—some smiling, some cracked in despair. A vine-covered mirror stands at the center, reflecting only strangers.", false));
    rooms.push_back(Room("The Threadbare Womb", "The walls are made of fibrous, pulsing material—almost alive. A faint heartbeat hums under your feet. An empty cradle sits in the center, rocking gently though no one is near.", false));
    rooms.push_back(Room("The Hall of Hunger", "Withered olive trees claw at the cracked marble. Bowls overflow with bloated grain—writhing, weeping, moving. The air stinks of soured milk and blood turned syrup-thick. Vines sprawl across the floor like the intestines of slaughtered offerings, knotted and twitching. You hear chewing—but nothing moves.", true, 0));
   
    // Shrine 1: Nyx
    Shrine nyx("Nyx", "The Starless Well");
    nyx.setState(ShrineState::CORRUPTED);
    shrineRegistry[1] = nyx;

    rooms.push_back(Room("Room With No Corners", "The walls curve softly into one another. There are no shadows, no edges. You always feel like you’re at the center—even when walking. Something breathes in rhythm with you.", false));
    rooms.push_back(Room("Nest of Wings", "The ceiling is unseen. Black feathers drift downward. A nest of glass bones sits abandoned. You’re certain you heard wings—but only once.", false));
    rooms.push_back(Room("The Starless Well", "A smooth pit swallows light and sound. Glyphs etched into obsidian pulse faintly—recognizable and wrong. When you lean over the edge, your shadow vanishes. Something down there watches, not with eyes, but with intention. You forget why you’re breathing.", true, 1));

    // Shrine 2: Apollo
    Shrine apollo("Apollo", "Echoing Gallery");
    apollo.setState(ShrineState::CORRUPTED);
    shrineRegistry[2] = apollo;

    rooms.push_back(Room("Hall of Echoes", "Every step you take repeats a second later—just slightly out of sync. A chorus murmurs words you almost recognize. If you speak, something replies from behind.", false));
    rooms.push_back(Room("Room That Remembers", "Every surface is mirrored, but you’re never alone. Sometimes your reflection lags. Sometimes it moves first. Sometimes it’s gone entirely—but you still feel watched.", false));
    rooms.push_back(Room("Echoing Gallery", "Mirrors line the walls, angled just wrong. They show you—but older, injured, smiling. The statues have mouths but no faces. You swear one whispered your name, a name you've not heard for years. But it didn’t. Or did it?", true, 2));

    // Shrine 3: Hecate
    Shrine hecate("Hecate", "The Unlit Path");
    hecate.setState(ShrineState::CORRUPTED);
    shrineRegistry[3] = hecate;

    rooms.push_back(Room("Loom of Names", "Threads hang like veins, each labeled in ink. One bears your name. Another is frayed. The loom creaks but never stops. Something is weaving nearby, just out of sight.", false));
    rooms.push_back(Room("Listening Chamber", "Shells line the walls, hung like ears. Some whisper forgotten hymns. Others sob. When you breathe, a shell beside you repeats it a beat too late.", false));
    rooms.push_back(Room("The Unlit Path", "Three stone doors. One burns with blue flame, one drips something thick, one is just absence. Candle stubs mark the walls in patterns that shift when unobserved. The torchlight flickers—but the shadows don’t match your shape. One shadow walks when you don’t", true, 3));

    // Shrine 4: Persephone
    Shrine persephone("Persephone", "The Frozen Spring");
    persephone.setState(ShrineState::CORRUPTED);
    shrineRegistry[4] = persephone;

    rooms.push_back(Room("Library of Teeth", "Shelves overflow with scrolls made from stretched skin, the ink still wet. Some pages have been bitten. Others twitch. A pile of broken quills sits beside a jar full of molars.", false));
    rooms.push_back(Room("Statuary of Regret", "Figures kneel in circles, their stone eyes hollow. Each statue wears a crown of dried flowers. A plaque beneath one reads: 'She warned them.' The air smells of the inside a tomb.", false));
    rooms.push_back(Room("The Frozen Spring", "A fountain of vines now fossilized, curled in agony. Ice creeps up the edges of the walls, though the air is warm. A lone pomegranate seed rests in a cracked bowl. It has not rotted. It will not. The room smells of rotting flowers and ash.", true, 4));
    
    // Shrine 5: Pan
    Shrine pan("Pan", "Wild Rotunda");
    pan.setState(ShrineState::CORRUPTED);
    shrineRegistry[5] = pan;

    rooms.push_back(Room("Hall of Shivering Meat", "Walls pulse with veins beneath translucent skin. Occasionally, a muscle twitches in the stone. A single pan flute lies on the ground—when touched, it plays a bleating cry.", false));
    rooms.push_back(Room("Den of Antlers", "Bones and antlers are fused into the architecture. The floor is covered in fur—not all of it animal. Something stalks just out of view, its gait rhythmic, almost... joyful.", false));
    rooms.push_back(Room("Wild Rotunda", "The walls pulse with root-veined moss, soft and warm as skin. Bones protrude from the growth—dancing mid-step, arms locked in joy or agony. Laughter echoes, then sobs, then silence. A damp breath tickles your neck, and no one is there.", true, 5));

    // Shrine 6: False Hermes
    Shrine falseHermes("False Hermes", "Gilded Hallway");
    falseHermes.setState(ShrineState::CORRUPTED);
    shrineRegistry[6] = falseHermes;

    rooms.push_back(Room("Room of Borrowed Things", "Shelves display small, mundane objects—combs, rings, sandals, letters. Each is labeled with a name you don’t recognize. One item is missing, but its tag reads your name. A drawer creaks open behind you.", false));
    rooms.push_back(Room("Whispering Hall", "Words are etched into every surface. None are repeated. The longer you stare, the more familiar the languages seem—until you find your own handwriting, carved deep and frantic.", false));
    rooms.push_back(Room("Gilded Hallway", "The marble gleams too clean. The walls shimmer like heatstroke. A friendly shrine waits at the end, grinning with a mouth it doesn’t have. You walk twenty-one steps. You always walk twenty-one steps. You don’t remember starting, but you’re always in motion.", true, 6));

    // Shrine 7: Thanatos
    Shrine thanatos("Thanatos", "Sleepwalker’s Alcove");
    thanatos.setState(ShrineState::CORRUPTED);
    shrineRegistry[7] = thanatos;

    rooms.push_back(Room("Room of Waiting Lights", "Hundreds of unlit candles line the floor. One flickers to life when you step inside, then another. None provide warmth. The air smells like burnt honey and salt.", false));
    rooms.push_back(Room("The Bloodclock", "A massive pendulum drips red into an unseen basin. It beats steadily—too slowly to match your pulse. On the wall: ■ν α■µατι χρ■νου. ('In the blood of time.')", false));
    rooms.push_back(Room("Sleepwalker’s Alcove", "A stone bed rests beneath an unlit arch. The room is warm—not comfort, but absence of discomfort. Laurel leaves line the floor, pale and dry. The silence here is full, whole. You think about lying down. Just for a moment. You imagine how easy it would be to stay. You do not remember why that’s a problem.", true, 7));

    // Shrine 8: Eris
    Shrine eris("Eris", "The Bone Choir");
    eris.setState(ShrineState::CORRUPTED);
    shrineRegistry[8] = eris;
    
    rooms.push_back(Room("Oracle’s Wake", "Candles flicker in defiance of windless dark. A defaced altar bleeds wax. Someone scratched 'I won’t lie again' into the stone 27 times.", false));
    rooms.push_back(Room("Archivist’s Cell", "A rusted desk faces the wall. Dozens of inked notes are nailed above it—each crossed out violently. Scratched into the desk: 'It was true. That’s the problem.' The chair is still warm.", false));
    rooms.push_back(Room("Throat of the Temple", "The corridor narrows slowly behind you. The walls are damp and warm to the touch. You hear a low, slow heartbeat. Every step echoes like a swallowed breath.", false));
    rooms.push_back(Room("The Bone Choir", "The bones are arranged in reverent poses, facing each other in song. Their mouths hang wide in eternal performance. The acoustics claw at your skull—discordant, divine, unending. Your ears bleed, or maybe your thoughts do. Their hymn harmonizes with your name.", true, 8));
}


void Game::gameLoop() {
    while (isRunning) {
        const Room& current = rooms[player.getCurrentRoom()];
        if (!current.isVisited()) {
            std::cout << "[First time here]\n";
            rooms[player.getCurrentRoom()].markVisited();
        }

        std::cout << current.getDescription() << "\n";

        if (current.shrinePresent()) {
            std::cout << "There is a shrine here. Type 'shrine' to interact.\n";
        } else {
            std::cout << "There are strange things here: ";
            for (const auto& obj : current.getObjects()) {
                std::cout << obj << ", ";
            }
            std::cout << "\n";
        }

        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);
        handleCommand(input);
    }
}

void Game::handleCommand(const std::string& input) {
    const Room& current = rooms[player.getCurrentRoom()];

    if (input == "quit") {
        isRunning = false;
    } else if (input == "journal") {
        player.viewJournal();
    } else if (input == "shrine") {
        if (current.shrinePresent()) {
            int shrineID = current.getShrineID();
            if (shrineRegistry.count(shrineID)) {
                shrineRegistry[shrineID].activate(player);
            } else {
                std::cout << "The shrine here feels... empty. As if forgotten.\n";
            }
        } else {
            std::cout << "There is no shrine here.\n";
        }
    } else {
        std::cout << "The temple does not understand.\n";
    }
}

