// Map.cpp
#include "Map.hpp"
#include <iomanip>

int TempleMap::addNode(const std::string& name, bool shrine) {
    int id = static_cast<int>(nodes_.size());
    nodes_.push_back({id, name, shrine});
    edges_.push_back({});
    nameToId_[name] = id;
    return id;
}

void TempleMap::addEdgeByName(const std::string& a, const std::string& b) {
    auto ia = nameToId_.find(a), ib = nameToId_.find(b);
    if (ia == nameToId_.end() || ib == nameToId_.end()) return;
    int A = ia->second, B = ib->second;
    edges_[A].push_back(B);
    edges_[B].push_back(A);
}

void TempleMap::populateFromDesign() {
    nodes_.clear(); edges_.clear(); nameToId_.clear();

    // Clusters match Game::loadRooms() names
    // Demeter (id 0)
    addNode("The Garden of Broken Faces", false);
    addNode("The Threadbare Womb", false);
    addNode("The Hall of Hunger", true);

    // Nyx (id 1)
    addNode("Room With No Corners", false);
    addNode("Nest of Wings", false);
    addNode("The Starless Well", true);

    // Apollo (id 2)
    addNode("Hall of Echoes", false);
    addNode("Room That Remembers", false);
    addNode("Echoing Gallery", true);

    // Hecate (id 3)
    addNode("Loom of Names", false);
    addNode("Listening Chamber", false);
    addNode("The Unlit Path", true);

    // Persephone (id 4)
    addNode("Hall of Petals", false);
    addNode("Orchard Walk", false);
    addNode("The Frozen Spring", true);

    // Pan (id 5)
    addNode("Hall of Shivering Meat", false);
    addNode("Den of Antlers", false);
    addNode("Wild Rotunda", true);

    // False Hermes (id 6)
    addNode("Room of Borrowed Things", false);
    addNode("Whispering Hall", false);
    addNode("Gilded Hallway", true);

    // Thanatos (id 7)
    addNode("Room of Waiting Lights", false);
    addNode("Waiting Room", false);
    addNode("Sleepwalker’s Alcove", true);

    // Eris (id 8) — 3 non‑shrine + shrine
    addNode("Oracle’s Wake", false);
    addNode("Archivist’s Cell", false);
    addNode("Throat of the Temple", false);
    addNode("The Bone Choir", true);

    // Intra‑cluster links (room1 <-> room2 <-> shrine)
    auto linkTriad = [&](const std::string& a, const std::string& b, const std::string& s){
        addEdgeByName(a,b); addEdgeByName(b,s);
    };
    linkTriad("The Garden of Broken Faces","The Threadbare Womb","The Hall of Hunger");
    linkTriad("Room With No Corners","Nest of Wings","The Starless Well");
    linkTriad("Hall of Echoes","Room That Remembers","Echoing Gallery");
    linkTriad("Loom of Names","Listening Chamber","The Unlit Path");
    linkTriad("Hall of Petals","Orchard Walk","The Frozen Spring");
    linkTriad("Hall of Shivering Meat","Den of Antlers","Wild Rotunda");
    linkTriad("Room of Borrowed Things","Whispering Hall","Gilded Hallway");
    linkTriad("Room of Waiting Lights","Waiting Room","Sleepwalker’s Alcove");
    // Eris chain: wake -> cell -> throat -> shrine
    addEdgeByName("Oracle’s Wake","Archivist’s Cell");
    addEdgeByName("Archivist’s Cell","Throat of the Temple");
    addEdgeByName("Throat of the Temple","The Bone Choir");

    // Inter‑cluster spine (you can tweak later)
    addEdgeByName("The Hall of Hunger", "The Starless Well");
    addEdgeByName("The Starless Well", "Echoing Gallery");
    addEdgeByName("Echoing Gallery", "The Unlit Path");
    addEdgeByName("The Unlit Path", "The Frozen Spring");
    addEdgeByName("The Frozen Spring", "Wild Rotunda");
    addEdgeByName("Wild Rotunda", "Gilded Hallway");
    addEdgeByName("Gilded Hallway", "Sleepwalker’s Alcove");
    addEdgeByName("Sleepwalker’s Alcove", "Throat of the Temple");
}

void TempleMap::printAscii() const {
    // Compact cluster view (not to scale)
    auto pad = [](const std::string& s, int w){ return s.size()<static_cast<size_t>(w) ? s + std::string(w - s.size(),' ') : s.substr(0,w); };

    std::cout << "\n=== TEMPLE LAYOUT (CLUSTERS) ===\n";
    std::cout << "[Demeter]      [Nyx]          [Apollo]       [Hecate]\n";
    std::cout << pad("Garden of Broken Faces",22) << "  "
              << pad("Room With No Corners",22)   << "  "
              << pad("Hall of Echoes",22)         << "  "
              << pad("Loom of Names",22)          << "\n";
    std::cout << pad("Threadbare Womb",22)        << "  "
              << pad("Nest of Wings",22)          << "  "
              << pad("Room That Remembers",22)    << "  "
              << pad("Listening Chamber",22)      << "\n";
    std::cout << pad("== Hall of Hunger ==",22)   << "--"
              << pad("== Starless Well ==",22)    << "--"
              << pad("== Echoing Gallery ==",22)  << "--"
              << pad("== The Unlit Path ==",22)   << "\n\n";

    std::cout << "[Persephone]   [Pan]          [False Hermes] [Thanatos]\n";
    std::cout << pad("Hall of Petals",22)       << "  "
              << pad("Hall of Shivering Meat",22) << "  "
              << pad("Room of Borrowed Things",22)<< "  "
              << pad("Room of Waiting Lights",22) << "\n";
    std::cout << pad("Orchard Walk",22)     << "  "
              << pad("Den of Antlers",22)         << "  "
              << pad("Whispering Hall",22)        << "  "
              << pad("Waiting Room",22)         << "\n";
    std::cout << pad("== The Frozen Spring ==",22)<< "--"
              << pad("== Wild Rotunda ==",22)     << "--"
              << pad("== Gilded Hallway ==",22)   << "--"
              << pad("== Sleepwalker's Alcove ==",22) << "\n\n";

    std::cout << "[Eris]\n";
    std::cout << "Oracle's Wake -> Archivist's Cell -> Throat of the Temple -> == The Bone Choir ==\n\n";

    std::cout << "Spine (shrines): Hunger -> Well -> Gallery -> Path -> Spring -> Rotunda -> Hallway -> Alcove -> Throat -> Bone Choir\n\n";
}

void TempleMap::printAdjacency() const {
    std::cout << "=== ROOM GRAPH (Adjacency) ===\n";
    for (const auto& n : nodes_) {
        std::cout << (n.shrine ? "[S] " : "[ ] ") << n.id << " - " << n.name << " : ";
        const auto& nbrs = edges_[n.id];
        for (size_t i=0;i<nbrs.size();++i) {
            std::cout << nbrs[i] << (i+1<nbrs.size() ? ", " : "");
        }
        std::cout << "\n";
    }
    std::cout << "\nLegend: [S] = Shrine\n";
}

