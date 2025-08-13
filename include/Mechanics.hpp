// Mechanics.hpp
#pragma once
#include "Theme.hpp"          // Deity, ShrineState
#include <string>
#include <vector>
#include <functional>
#include <random>
#include <chrono>
#include <unordered_map>
#include <optional>

// Lysaia vs. Melas
enum class WorldView { Uncorrupted, Corrupted };

// Optional local accessibility knobs (separate from your UI/theme ones)
struct Accessibility {
    bool colorDisabled = false;
    bool disableShake  = false;
    bool highContrast  = false;
};

struct Stats {
    int health = 5;
    int will   = 7;
    int insight= 2;
    int nerve  = 2;

    void clamp() {
        if (health < 0) health = 0; if (health > 10) health = 10;
        if (will   < 0) will   = 0; if (will   > 10) will   = 10;
        if (insight< 0) insight= 0; if (insight> 10) insight= 10;
        if (nerve  < 0) nerve  = 0; if (nerve  > 10) nerve  = 10;
    }
};

struct InventoryItem {
    std::string id;
    std::string name;
    std::string desc;
    int charges = 1;
};

struct CheckMods {
    int  flat = 0;
    bool advantage = false;
    bool disadvantage = false;
};

struct Outcome {
    std::string journalEntry;
    int healthDelta = 0;
    int willDelta   = 0;
    int insightDelta= 0;
    int nerveDelta  = 0;
    int corruptionDelta = 0;
    std::vector<InventoryItem> itemsGained;
    std::vector<std::string>  flagsSet;
};

class RNG {
public:
    RNG() {
        auto seed = static_cast<unsigned>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count());
        eng.seed(seed);
    }
    int roll(int minInclusive, int maxInclusive) {
        std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
        return dist(eng);
    }
    int d6()  { return roll(1,6); }
    int d10() { return roll(1,10); }
private:
    std::mt19937 eng;
};

class PlayerState {
public:
    Stats stats;
    int corruption = 0;
    std::vector<InventoryItem> bag;
    Accessibility access;
    WorldView view = WorldView::Uncorrupted;   // <-- fixed (had no member name)

    bool hasItem(const std::string& id) const;
    bool consumeItem(const std::string& id);
    void addItem(const InventoryItem& it);
    void applyOutcome(const Outcome& out);
    bool isAlive() const { return stats.health > 0 && stats.will > 0; }
};

class SkillCheck {
public:
    // 1d10 + stat + mods vs DC (ties succeed)
    static bool resolve(RNG& rng, int dc, int statScore, const CheckMods& mods, int* outRoll = nullptr);
};

// Journal bridge
class IJournalSink {
public:
    virtual ~IJournalSink() = default;
    virtual void writeLysaia(const std::string& entry) = 0;
    virtual void writeMelas (const std::string& entry) = 0;
};

// Interaction context used by shrine/room mechanics
struct InteractionContext {
    PlayerState& player;
    RNG& rng;
    IJournalSink& journal;
    WorldView view;                 // playthrough
    ShrineState shrineState;        // this room/shrine’s state
    std::unordered_map<std::string, bool>& flags;
};

// NOTE: Do NOT define Room or Shrine here.
// Use your existing Room.hpp and Shrine.hpp for those.
