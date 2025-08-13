// ShrineBehavior.hpp
#pragma once
#include "Mechanics.hpp"
#include "UI.hpp"
#include <optional>
#include <functional>
#include <string>
#include <vector>

// -- type aliases expected by ShrineBehavior.cpp
using TakeMelasEntryFn = std::function<std::optional<std::string>()>;
using GiveMelasEntryFn = std::function<void(const std::string&)>;

// -- Riddle shape expected by ShrineBehavior.cpp
struct Riddle {
    std::string prompt;
    std::vector<std::string> options;
    int correctIndex1Based = 1;

    // compatibility aliases (if other code used different names)
    std::string& question() { return prompt; }
    const std::string& question() const { return prompt; }
    int correctIndex() const { return correctIndex1Based; }
};

// Demeter
Outcome RunDemeterLetter_FromInventory(InteractionContext& ctx, UI& ui);
Outcome ShowDemeterLetter_Uncorrupted(InteractionContext& ctx, UI& ui);
Outcome ShowDemeterLetter_Uncorrupted(InteractionContext& ctx, UI& ui,
                                      const std::vector<std::string>& choices);
// Nyx
Outcome RunNyxTrade(InteractionContext& ctx, UI& ui,
                    TakeMelasEntryFn take = {},
                    GiveMelasEntryFn give = {});

// Apollo
Outcome RunApolloRiddles(InteractionContext& ctx, UI& ui,
                         const std::vector<Riddle>& set);

// Hecate / Pan / False Hermes / Thanatos / Eris…
Outcome RunHecateDoors(InteractionContext& ctx, UI& ui, GiveMelasEntryFn give = {});
Outcome RunPanMemory(InteractionContext& ctx, UI& ui, int rounds, int noteRange);
Outcome RunFalseHermesEndlessHall(InteractionContext& ctx, UI& ui);
Outcome RunThanatosRest(InteractionContext& ctx, UI& ui);
Outcome RunErisFinal(InteractionContext& ctx, UI& ui);
