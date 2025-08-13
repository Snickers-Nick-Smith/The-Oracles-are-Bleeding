#include "ShrineBehavior.hpp"
#include "PersephoneFragments.hpp"
#include "UI.hpp"
#include <algorithm>
#include <iomanip>
// --------------------- DEMETER -----------------------------------------------
Outcome RunDemeterLetter_FromInventory(InteractionContext& ctx, UI& ui) {  Outcome out;

    if (!HasAllPersephoneFragments(ctx.player)) {
        auto owned = GetOwnedPersephoneFragments(ctx.player);
        int have = (int)owned.size();
        out.journalEntry =
            "Demeter’s altar waits for the whole letter. You have only " + std::to_string(have) +
            "/8 fragments. The grain will not answer yet. (-1 Will)";
        out.willDelta -= 1;
        return out;
    }

    auto owned = GetOwnedPersephoneFragments(ctx.player); // vector<(index,text)>
    // Build a presentation list (we don’t show indices to the player—just the text)
    std::vector<std::string> texts; texts.reserve(owned.size());
    for (auto& p : owned) texts.push_back(p.second);

    ui.print("Persephone’s scattered words lie before you. Put them in their true order.");

    std::vector<int> chosen; chosen.reserve(texts.size());
    std::vector<bool> used(texts.size(), false);

    while (chosen.size() < texts.size()) {
        ui.print("Fragments:");
        for (size_t i=0;i<texts.size();++i) {
            std::string mark = used[i] ? "[X]" : "[ ]";
            ui.print(mark + " " + std::to_string(i+1) + ": " + texts[i]);
        }
        int pick = ui.choose("Pick the next fragment in sequence:", {});
        if (pick < 1 || pick > (int)texts.size() || used[pick-1]) {
            ui.print("That fragment is not available. Try again.");
            continue;
        }
        used[pick-1] = true;
        chosen.push_back(pick);
    }

    // Map back to the *true indices* of what the player picked
    // owned[k] = (trueIndex, text)
    std::vector<int> pickedTrueOrder;
    pickedTrueOrder.reserve(chosen.size());
    for (int displayIdx : chosen) {
        pickedTrueOrder.push_back(owned[displayIdx-1].first);
    }

    // Correct order is 1..8
    bool correct = true;
    for (int i=0;i<8;++i) if (pickedTrueOrder[i] != i+1) { correct = false; break; }

    if (correct) {
        out.journalEntry =
            "The letter settles into sense — ragged, but undeniable. Persephone chose this path.\n"
            "The truth steels you. (+2 Will, +1 Nerve, +1 Insight)";
        out.willDelta    += 2;
        out.nerveDelta   += 1;
        out.insightDelta += 1;
        ctx.flags["demeter_letter_solved"] = true;
    } else {
        out.journalEntry =
            "Your arrangement scrapes like bone on stone. The message becomes a chant with no mercy.\n"
            "Doubt fills the seams. (-2 Will, -1 Nerve, -1 Health)";
        out.willDelta   -= 2;
        out.nerveDelta  -= 1;
        out.healthDelta -= 1;
        ctx.flags["demeter_letter_solved"] = false;
    }

    return out;
}

Outcome ShowDemeterLetter_Uncorrupted(InteractionContext& ctx, UI& ui) {
    return ShowDemeterLetter_Uncorrupted(ctx, ui, kPersephoneLetterClean);
}

// --------------------- NYX ----------------------------------------------------
Outcome RunNyxTrade(InteractionContext& ctx, UI& ui,
                    TakeMelasEntryFn takeOne, GiveMelasEntryFn giveOne)
{
    Outcome out;
    ui.print("Nyx’s bowl shows a page that is yours but never was. She asks for a trade.");

    if (!takeOne) {
        out.journalEntry = "You have nothing to give that Nyx will take. The water darkens. (-1 Will)";
        out.willDelta -= 1;
        return out;
    }

    auto offered = takeOne(); // remove one Melas journal entry
    if (!offered.has_value()) {
        out.journalEntry = "Your journal is silent. Nyx offers only silence back. (-1 Will)";
        out.willDelta -= 1;
        return out;
    }

    // 50/50 helpful or misleading
    bool helpful = (ctx.rng.roll(1,100) <= 50);
    std::string newEntry;

    if (helpful) {
        static const std::vector<std::string> boons = {
            "The bowl reveals a hidden latch behind the Archivist’s shelf.",
            "A cracked tile marks a crawlspace in Pan’s corridor.",
            "Apollo’s third riddle lies: choose what sounds wrong."
        };
        newEntry = boons[ctx.rng.roll(0, (int)boons.size()-1)];
        out.journalEntry =
            "You surrender a page to the dark. In return, stars arrange into instruction. (+1 Insight)";
        out.insightDelta += 1;
        ctx.flags["nyx_helpful_trade"] = true;
    } else {
        static const std::vector<std::string> banes = {
            "Follow the echo, not the voice. (It circles back to the false hall.)",
            "Count the doors that aren’t there.",
            "Sleep where the floor is warm."
        };
        newEntry = banes[ctx.rng.roll(0, (int)banes.size()-1)];
        out.journalEntry =
            "Your page sinks without a ripple. The mirror returns a crooked map. (+2 Corruption)";
        out.corruptionDelta += 2;
        ctx.flags["nyx_helpful_trade"] = false;
    }

    if (giveOne) giveOne(newEntry);
    return out;
}

// --------------------- APOLLO -------------------------------------------------
Outcome RunApolloRiddles(InteractionContext& ctx, UI& ui, const std::vector<Riddle>& set)
{
    Outcome out;
    ui.print("Apollo’s lyre hums out of tune. The sun points the wrong way.");
    int right = 0, total = (int)set.size();

    for (const auto& r : set) {
        int pick = ui.choose(r.prompt, r.options);
        if (pick == r.correctIndex1Based) {
            ui.print("The strings tighten—wrong feels right.");
            ++right;
        } else {
            ui.print("A bright chord snaps.");
        }
    }

    bool majority = right * 2 >= total; // 3/5 or better
    if (majority) {
        out.journalEntry =
            "You answer what no one sane would. The hymn completes. (+1 Insight, +1 Health, +1 Nerve, -1 Will)";
        out.insightDelta += 1;
        out.healthDelta  += 1;
        out.nerveDelta   += 1;
        out.willDelta    -= 1; // sanity cost
        ctx.flags["apollo_majority_right"] = true;
    } else {
        out.journalEntry =
            "Sense betrays you. Apollo’s light fractures. (-2 Will, +2 Corruption)";
        out.willDelta -= 2;
        out.corruptionDelta += 2;
        ctx.flags["apollo_majority_right"] = false;
    }
    return out;
}

// --------------------- FALSE HERMES ------------------------------------------
Outcome RunFalseHermesEndlessHall(InteractionContext& ctx, UI& ui)
{
    Outcome out;
    ui.print("A silver‑tongued path promises shortcuts and mercy. Your name sounds better there.");

    int successes = 0, failures = 0;
    while (successes < 5 && failures <= 5) {
        CheckMods mods; // you could inject item buffs here
        bool ok = SkillCheck::resolve(ctx.rng, /*dc=*/7 + (ctx.player.corruption/25),
                                      ctx.player.stats.will, mods);
        if (ok) {
            ++successes;
            ui.print("You avert your eyes. The corridor shortens by one lie. (" + std::to_string(successes) + "/5)");
            // small reward to keep momentum
            ctx.player.stats.will = std::min(10, ctx.player.stats.will + 1);
        } else {
            ++failures;
            ui.print("You look back. The hall lengthens. (" + std::to_string(failures) + " fails)");
            ctx.player.corruption = std::min(100, ctx.player.corruption + 2);
            if (failures % 2 == 0) ctx.player.stats.will = std::max(0, ctx.player.stats.will - 1);
        }
    }

    if (failures > 5) {
        out.journalEntry =
            "You turn one more time and the hall seals like a mouth. (Bad Ending: Endless Hall)";
        ctx.flags["false_hermes_endless_hall"] = true;
        out.corruptionDelta += 10;
        // You can handle ending outside by reading the flag.
    } else {
        out.journalEntry =
            "You keep walking even when the floor begs you to stop. At last the echoes thin. (+2 Nerve)";
        out.nerveDelta += 2;
        ctx.flags["false_hermes_endless_hall"] = false;
    }
    return out;
}

// --------------------- THANATOS ----------------------------------------------
Outcome RunThanatosRest(InteractionContext& ctx, UI& ui)
{
    Outcome out;
    int c = ui.choose(
        "Thanatos offers quiet: \"Lay down, and I will keep you.\"",
        {"Lay down and rest.", "Keep moving forward."}
    );

    if (c == 1) {
        out.journalEntry = "You sleep as if the world never asked for you. (Passive Ending)";
        ctx.flags["thanatos_sleep_end"] = true;
        // No stat deltas needed; caller should end game based on flag.
    } else {
        out.journalEntry = "You pass the offered bed. It feels like a kindness refused. (+1 Will)";
        out.willDelta += 1;
        ctx.flags["thanatos_sleep_end"] = false;
    }
    return out;
}

// --------------------- PAN ----------------------------------------------------
Outcome RunPanMemory(InteractionContext& ctx, UI& ui, int rounds, int noteRange)
{
    Outcome out;
    ui.print("A reed flute on the altar wheezes out a pattern. Then silence.");

    int correctRounds = 0;
    for (int r = 1; r <= rounds; ++r) {
        std::vector<int> seq;
        for (int i=0;i<r;++i) seq.push_back(ctx.rng.roll(1, noteRange));

        // show sequence
        {
            std::string show = "Notes: ";
            for (int n : seq) show += std::to_string(n) + " ";
            ui.print(show);
            ui.waitForKey(); // clear after
            ui.print(std::string(40,'\n')); // crude “erase”
        }

        // ask player
        std::string ans = ui.ask("Repeat the notes separated by spaces:");
        std::vector<int> got;
        {
            std::istringstream iss(ans);
            int x; while (iss >> x) got.push_back(x);
        }

        if (got == seq) {
            ++correctRounds;
            ui.print("Your fingers remember what your eyes forgot.");
        } else {
            ui.print("A sour squeal betrays your hesitation.");
        }
    }

    if (correctRounds * 2 >= rounds) {
        out.journalEntry = "Pan laughs through his teeth. Chaos approves. (+1 Health, +2 Nerve)";
        out.healthDelta += 1;
        out.nerveDelta += 2;
        ctx.flags["pan_memory_mastered"] = true;
    } else {
        out.journalEntry = "The pattern crawls away. Your certainty shakes. (-1 Nerve, -1 Insight)";
        out.nerveDelta -= 1;
        out.insightDelta -= 1;
        ctx.flags["pan_memory_mastered"] = false;
    }
    return out;
}


// --------------------- HECATE -------------------------------------------------
Outcome RunHecateDoors(InteractionContext& ctx, UI& ui, GiveMelasEntryFn giveOne)
{
    Outcome out;
    ui.print("Three doors stand before you, each marked only by a faint sigil.");
    int choice = ui.choose(
        "Which door do you open?",
        {"The First Door — to the Past", "The Second Door — to the Future", "The Third Door — to the Present"}
    );

    if (choice == 1) {
        out.journalEntry =
            "You step into memory’s embrace. The air smells of an old, safe place. "
            "Your wounds knit, and your mind steadies. (+2 Will, +1 Insight, +1 Health)";
        out.willDelta    += 2;
        out.insightDelta += 1;
        out.healthDelta  += 1;
        ctx.flags["hecate_choice"] = 1;
    }
    else if (choice == 2) {
        out.journalEntry =
            "A vision takes root — something that has not yet happened, but will. "
            "It scrawls itself into your journal.";
        if (giveOne) {
            static const std::vector<std::string> visions = {
                "A door with no frame. Do not knock.",
                "Two shadows pass over you, but the floor is empty.",
                "When you hear the third bell, hide."
            };
            giveOne(visions[ctx.rng.roll(0, (int)visions.size()-1)]);
        }
        ctx.flags["hecate_choice"] = 2;
    }
    else {
        out.journalEntry =
            "You open the door. There is only a hallway that swallows sound. "
            "Your chest tightens for no reason you can name. (-2 Will)";
        out.willDelta -= 2;
        ctx.flags["hecate_choice"] = 3;
    }

    return out;
}


// --------------------- ERIS ---------------------------------------------------
Outcome RunErisFinal(InteractionContext& ctx, UI& ui)
{
    Outcome out;
    ui.print("Eris arranges bones like wind chimes. Lysaia stands beside her, eyes bright and far.");

    // Score what the player learned/did. You can tune these weights.
    int score = 0;
    if (ctx.flags["demeter_letter_solved"])  score += 2;
    if (ctx.flags["apollo_majority_right"])  score += 2;
    if (ctx.flags["pan_memory_mastered"])    score += 2;
    if (ctx.flags["nyx_helpful_trade"])      score += 1;
    if (ctx.flags["false_hermes_endless_hall"]) score -= 999; // shouldn’t be here if trapped

    // Dialogue fork – very light; replace with your system later.
    int choice = ui.choose(
        "Three paths open:\n"
        "1) Resist them both.\n"
        "2) Speak to Lysaia alone.\n"
        "3) Accept Eris’ offer.",
        {"Resist", "Plead with Lysaia", "Join the Bone Choir"}
    );

    if (choice == 3) {
        out.journalEntry =
            "You step into the harmony of breaking. (Ending: Joined the Bone Choir)";
        ctx.flags["ending_join_eris"] = true;
        out.corruptionDelta += 10;
        return out;
    }

    if (choice == 2) {
        // Persuade Lysaia – base on score + Will
        int dc = 8;
        CheckMods mods; mods.flat = (score >= 4 ? 2 : 0); // strong prep helps
        bool ok = SkillCheck::resolve(ctx.rng, dc, ctx.player.stats.will, mods);
        if (ok) {
            out.journalEntry =
                "You call her by the name only you used. Something in her loosens. (Ending: Lysaia Turns)";
            ctx.flags["ending_save_lysaia"] = true;
            out.willDelta += 2;
            return out;
        } else {
            out.journalEntry =
                "Your words reach her and shatter anyway. Eris smiles with all her teeth. (-2 Will)";
            out.willDelta -= 2;
            ctx.flags["ending_save_lysaia"] = false;
            return out;
        }
    }

    // choice == 1: straight resist test using accumulated knowledge
    {
        int dc = 9;
        CheckMods mods;
        if (score >= 5) mods.advantage = true;
        bool ok = SkillCheck::resolve(ctx.rng, dc, ctx.player.stats.nerve, mods);
        if (ok) {
            out.journalEntry =
                "You refuse, and refuse, until refusal is all that remains. (Ending: Overcame the Offer)";
            ctx.flags["ending_overcome"] = true;
            out.nerveDelta += 2;
        } else {
            out.journalEntry =
                "Your stance wavers at the last word. She catches it. (Ending: Claimed by Discord)";
            ctx.flags["ending_claimed"] = true;
            out.corruptionDelta += 5;
            out.willDelta -= 2;
        }
        return out;
    }
}
