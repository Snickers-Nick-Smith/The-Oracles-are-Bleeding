#include "ShrineRunner.hpp"
#include "ShrineBehavior.hpp"   // Run* helpers + Riddle
#include <algorithm>
#include <cctype>
#include <vector>

// --- helpers ---------------------------------------------------------------
static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

static Deity DeityFromName(const std::string& deityName) {
    const std::string d = lower(deityName);
    if (d == "demeter")                     return Deity::Demeter;
    if (d == "nyx")                         return Deity::Nyx;
    if (d == "apollo")                      return Deity::Apollo;
    if (d == "hecate")                      return Deity::Hecate;
    if (d == "pan")                         return Deity::Pan;
    if (d == "thanatos")                    return Deity::Thanatos;
    if (d == "eris")                        return Deity::Eris;
    if (d == "persephone")                  return Deity::Persephone;
    if (d == "falsehermes" || d == "false hermes" || d == "fake hermes")
                                            return Deity::FalseHermes;
    return Deity::Default; // safe fallback
}

// --- dispatcher ------------------------------------------------------------
Outcome RunShrine(const Shrine& shrine,
                  InteractionContext& ctx,
                  UI& ui,
                  const ShrineServices& svc)
{
    // Preserve caller's state, but use the shrine's state during this run
    const ShrineState prevState = ctx.shrineState;
    ctx.shrineState = shrine.getState();

    const Deity deity = DeityFromName(shrine.getDeityName());
    Outcome out;

    switch (deity) {
        case Deity::Demeter: {
            if (ctx.view == WorldView::Corrupted) {
                // Melas: assemble Persephone letter from inventory fragments
                out = RunDemeterLetter_FromInventory(ctx, ui);
            } else {
                // Lysaia: calm reading (no puzzle). No extra 'letter' var needed.
                out = ShowDemeterLetter_Uncorrupted(ctx, ui);
            }
        } break;

        case Deity::Nyx: {
            // If you haven't wired JournalManager hooks yet, svc.* may be empty (that’s fine)
            out = RunNyxTrade(ctx, ui, svc.takeMelasEntry, svc.giveMelasEntry);
        } break;

        case Deity::Apollo: {
            std::vector<Riddle> set = {
                {"What breaks the fastest silence?", {"A shout","A thought","A whisper","Footsteps"}, 3},
                {"What shines behind closed eyes?",  {"Sun","Dream","Candle","Window"},               2},
                {"What answers every question?",     {"Echo","Silence","Time","Nothing"},             4},
                {"What door has no hinge?",          {"Grave","Mouth","Storm","Threshold"},           2},
                {"What song ends all songs?",        {"Lullaby","Requiem","Anthem","Hum"},            2}
            };
            out = RunApolloRiddles(ctx, ui, set);
        } break;

        case Deity::Hecate: {
            out = RunHecateDoors(ctx, ui, svc.giveMelasEntry);
        } break;

        case Deity::Pan: {
            out = RunPanMemory(ctx, ui, /*rounds=*/5, /*noteRange=*/5);
        } break;

        case Deity::FalseHermes: {
            out = RunFalseHermesEndlessHall(ctx, ui);
        } break;

        case Deity::Thanatos: {
            out = RunThanatosRest(ctx, ui);
        } break;

        case Deity::Eris: {
            out = RunErisFinal(ctx, ui);
        } break;

        default: {
            out.journalEntry = "The altar is quiet. Nothing answers you.";
        } break;
    }

    // restore caller’s shrine state
    ctx.shrineState = prevState;
    return out;
}
