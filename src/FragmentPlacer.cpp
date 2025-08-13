#include "FragmentPlacer.hpp"
#include <unordered_map>
#include <algorithm>
#include <cctype>

static std::string toLowerCopy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

// silent one-time pickup
static bool PickupPersephoneFragment_Silent(InteractionContext& ctx, int index) {
    const std::string flag = "picked_perse_frag_" + std::to_string(index);
    if (ctx.flags[flag]) return false;

    Outcome o = PickupPersephoneFragmentInRoom(ctx, index, flag);
    ctx.player.applyOutcome(o);
    if (!o.journalEntry.empty()) ctx.journal.writeMelas(o.journalEntry);
    return true;
}

void CheckPersephoneLetterPickupsForRoom(InteractionContext& ctx, const std::string& roomTitle) {
     if (ctx.view != WorldView::Corrupted) return;

    static const std::unordered_map<std::string, std::vector<int>> PLACEMENT = {
        {"hall of petals",                {3,2}},
        {"orchard walk",                  {6,4}},
        {"the frozen spring",             {5,8}},
        {"the threadbare womb",    {7}},
        {"the hall of hunger",            {1}},
    };

    const std::string key = toLowerCopy(roomTitle);
    auto it = PLACEMENT.find(key);
    if (it == PLACEMENT.end()) return;

    for (int idx : it->second) {
        PickupPersephoneFragment_Silent(ctx, idx);
    }
}
