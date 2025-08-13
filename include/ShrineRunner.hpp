#pragma once
#include <string>
#include "Shrine.hpp"              // your shrine
#include "Mechanics.hpp"          // InteractionContext, Outcome, Theme, Deity
#include "ShrineBehavior.hpp"    // RunDemeterLetter_FromInventory, RunNyxTrade, etc.

// Services for shrines that touch the journal (Nyx/Hecate: Future entry)
struct ShrineServices {
    TakeMelasEntryFn takeMelasEntry = nullptr; // remove+return one Melas entry
    GiveMelasEntryFn giveMelasEntry = nullptr; // append a Melas entry
};

// Dispatch the correct mechanic for a given shrine.
// Applies NO side effects; just returns the Outcome.
// (You can then apply it and write journal in your loop/manager.)
Outcome RunShrine(const Shrine& shrine, InteractionContext& ctx, UI& ui, const ShrineServices& svc = {});

// Helpers if you need them elsewhere
Deity DeityFromName(const std::string& deityName);
