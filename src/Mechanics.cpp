#include "Mechanics.hpp"
#include <algorithm>

bool PlayerState::hasItem(const std::string& id) const {
    return std::any_of(bag.begin(), bag.end(), [&](const auto& it){ return it.id == id && it.charges > 0; });
}

bool PlayerState::consumeItem(const std::string& id) {
    for (auto& it : bag) {
        if (it.id == id && it.charges > 0) { --it.charges; return true; }
    }
    return false;
}

void PlayerState::addItem(const InventoryItem& it) {
    for (auto& x : bag) {
        if (x.id == it.id) { x.charges += it.charges; return; }
    }
    bag.push_back(it);
}

void PlayerState::applyOutcome(const Outcome& out) {
    stats.health  += out.healthDelta;
    stats.will    += out.willDelta;
    stats.insight += out.insightDelta;
    stats.nerve   += out.nerveDelta;
    corruption    = std::clamp(corruption + out.corruptionDelta, 0, 100);
    stats.clamp();
    for (const auto& it : out.itemsGained) addItem(it);
}

static int pickRoll(RNG& rng, bool adv, bool dis) {
    if (adv && !dis) { int a=rng.d10(), b=rng.d10(); return std::max(a,b); }
    if (dis && !adv) { int a=rng.d10(), b=rng.d10(); return std::min(a,b); }
    return rng.d10();
}

bool SkillCheck::resolve(RNG& rng, int dc, int statScore, const CheckMods& mods, int* outRoll) {
    int base  = pickRoll(rng, mods.advantage, mods.disadvantage);
    int total = base + statScore + mods.flat;
    if (outRoll) *outRoll = base;
    return total >= dc;
}

