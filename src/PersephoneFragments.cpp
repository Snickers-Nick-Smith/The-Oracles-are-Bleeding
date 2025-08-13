// PersephoneFragments.cpp
#include "PersephoneFragments.hpp"
#include <algorithm>
#include <sstream>

const std::vector<std::string> kPersephoneLetterClean = {
    "Mother, you read footprints and call them chains.",
    "I was not stolen; I went because I wished to go.",
    "No hand at my throat, no leash at my heel.",
    "I packed a small bag and followed the cool seam in the earth.",
    "He is grave-dark and gentle; he listens more than he speaks.",
    "Even the dog with three faces lowers them when I pass.",
    "Do not fear his work—death is steady, and steadiness is mercy.",
    "He knelt, not to conquer, but to hear my answer.",
    "We learned each other's shadows without flinching.",
    "Mine returned ash-sweet from dancing with his.",
    "The blood we shared has dried; it does not ache.",
    "I am not lost. I am choosing, and I am chosen.",
    "I am his, and he is mine."
};

static const char* FRAG_TEXT[8] = {
    "They say I was dragged screaming… screaming… but the dark was already inside me.",
    "I walked here. My eyes open. My hands empty.",
    "Hades… on his knees… his hands colder than mine.",
    "The shadows dance, or they feed — sometimes I forget which.",
    "The dead keep their promises. The living… forget.",
    "Better his crown in the stillness than your fields in the wind.",
    "Blood dried in the lines of my palms. I tried to wash it… it stayed.",
    "I am his. He is mine. I am his. He is mine."
};

std::vector<InventoryItem> MakePersephoneFragments() {
    std::vector<InventoryItem> v; v.reserve(8);
    for (int i=0;i<8;++i) {
        InventoryItem it;
        it.id   = "perse_frag_" + std::to_string(i+1);
        it.name = "Letter Fragment (" + std::to_string(i+1) + ")";
        it.desc = FRAG_TEXT[i];
        it.charges = 1;
        v.push_back(it);
    }
    return v;
}

static bool hasFrag(const PlayerState& ps, int idx) {
    const std::string want = "perse_frag_" + std::to_string(idx);
    for (const auto& it : ps.bag) if (it.id == want && it.charges > 0) return true;
    return false;
}

bool HasAllPersephoneFragments(const PlayerState& ps) {
    for (int i=1;i<=8;++i) if (!hasFrag(ps, i)) return false;
    return true;
}

std::vector<std::pair<int,std::string>> GetOwnedPersephoneFragments(const PlayerState& ps) {
    std::vector<std::pair<int,std::string>> out; out.reserve(8);
    for (int i=1;i<=8;++i) {
        const std::string want = "perse_frag_" + std::to_string(i);
        for (const auto& it : ps.bag) {
            if (it.id == want && it.charges > 0) {
                out.emplace_back(i, it.desc);
                break;
            }
        }
    }
    return out;
}

Outcome PickupPersephoneFragmentInRoom(InteractionContext& ctx, int index, const std::string& pickedFlagKey) {
    Outcome o;
    if (index < 1 || index > 8) {
        o.journalEntry = "The parchment here has crumbled to dust.";
        return o;
    }
    if (ctx.flags[pickedFlagKey]) {
        o.journalEntry = "Only scraps remain where a fragment once lay.";
        return o;
    }

    // Give the fragment
    auto all = MakePersephoneFragments();
    InventoryItem it = all[index-1];
    ctx.player.addItem(it);
    ctx.flags[pickedFlagKey] = true;

    o.journalEntry = "You recover a torn piece of Persephone’s letter: \"" + it.desc + "\"";
    o.willDelta += 1; // small calm boon
    return o;
}
