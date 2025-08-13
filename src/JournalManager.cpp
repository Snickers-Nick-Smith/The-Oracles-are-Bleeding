// JournalManager.cpp (Location-aware, dual journals, hallucinations)
#include "JournalManager.hpp"
#include "utils.hpp"
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <cstdlib>
#include <algorithm>

// -----------------------------
// Helper: generic hallucination pool
// -----------------------------
static const char* kGenericHallucinations[] = {
    "She was never gone.",
    "The temple sings when no one listens.",
    "Do not trust what you wrote.",
    "You were warned.",
    "Cassandra is a curse.",
    "You already died once.",
    "The shrines are listening.",
    "There were more of you. There aren't now.",
    "The walls remember every step you take.",
    "You have been here longer than the temple.",
    "The dust knows your name.",
    "You are not the only one wearing your skin.",
    "Every shadow is counting down.",
    "You left something breathing in the last room.",
    "You’re walking in the wrong direction.",
    "They are speaking about you in the walls.",
    "You forgot the word for leaving.",
    "The floor is warmer when you stand still.",
    "Something is following you inside your own breath.",
    "You carried this place inside you.",
    "The silence is watching for you to speak.",
    "You will not recognize your reflection next time.",
    "Your heartbeat does not belong to you anymore.",
    "You are the only one who thinks you’re alive.",
    "The light is bending away from you.",
    "Someone else wrote this before you.",
    "Your hands are not where you left them.",
    "The air here remembers your scent.",
    "You’ll see her again, but not how you want.",
    "The next door leads to where you started.",
    "You are filling someone else’s footsteps.",
    "The echo is getting ahead of you.",
    "You will not survive telling the truth.",
    "Your eyes will close before you mean them to.",
    "The next breath will not be yours.",
    "Every word you’ve written is already gone.",
    "You’ve been answering questions no one asked.",
    "The temple keeps count, even when you forget.",
};


// ---- Helpers ----------------------------------------------------------------
// If your JournalEntry uses a different field name than `actual`,
// change the return line below to e.text or e.content, etc.
static inline const std::string& getText(const JournalEntry& e) {
    return e.content;
}

// ---- Setup / Definitions -----------------------------------------------------

void JournalManager::defineLocationEntry(const std::string& id,
                                         const std::string& actual,
                                         const std::string& hallucination) {
    locationEntries[id] = EntryData{actual, hallucination};
}

// Optional: keep as-is if you already have this elsewhere.
// Loads the uncorrupted prologue lines + guilt beats.
void JournalManager::seedLysaiaPrologueText() {
    // Shrine attempts (uncorrupted)
    locationEntries["demeter/shrine_uncorrupted"]     = {"I brought offerings to Demeter and spoke plainly: feed what I starved. The grain did not bow. I bowed instead.", ""};
    locationEntries["nyx/shrine_uncorrupted"]         = {"At Nyx’s well I whispered my fear into the still water. The stars under the surface did not answer—perhaps they were listening.", ""};
    locationEntries["apollo/shrine_uncorrupted"]      = {"In Apollo’s gallery my voice doubled back as song. I asked for truth. The echo repeated only what I already knew.", ""};
    locationEntries["hecate/shrine_uncorrupted"]      = {"At the Luminous Path I asked for a door that opens only forward. The lanterns did not argue. I did.", ""};
    locationEntries["persephone/shrine_uncorrupted"]  = {"I asked Persephone how to hold two seasons at once. She remained kind, but silent.", ""};
    locationEntries["pan/shrine_uncorrupted"]         = {"I tried to speak softly to the earth. The earth spoke softly back, as if it pitied me.", ""};
    locationEntries["false_hermes/shrine_uncorrupted"]= {"I greeted the messenger who isn’t. He smiled without teeth. I smiled with mine and said nothing else.", ""};
    locationEntries["thanatos/shrine_uncorrupted"]    = {"I asked Thanatos if forgetting can be merciful. He made no promises, which felt like one.", ""};
    locationEntries["eris/shrine_uncorrupted"]        = {"I told Eris I would not play. She called that a move. I pretended not to hear the rules.", ""};

    // Guilt beats
    locationEntries["meta/guilt/day1"] = {"I wrote her new name to keep her safe. Names travel faster than truth.", ""};
    locationEntries["meta/guilt/day2"] = {"Cassandra was a kindness I could live with. Melas was a child I could not lose.", ""};
    locationEntries["meta/guilt/day3"] = {"Exile was supposed to be distance, not a sentence. I told myself the temple would quiet down.", ""};
    locationEntries["meta/guilt/day4"] = {"When the others turned against her, they were following me. I keep walking.", ""};
    locationEntries["meta/guilt/day5"] = {"If she returns, she will use the name I gave her. I have made a stranger whose face I know.", ""};
    locationEntries["meta/guilt/day6"] = {"If I confess, Eris will clap. If I deny, Eris will clap. I write instead.", ""};
    locationEntries["meta/guilt/day7"] = {"Release is not forgiveness. It is only the knife put down after the cut.", ""};
}

void JournalManager::writeLysaiaGuiltBeat(int day) {
    const std::string key = "meta/guilt/day" + std::to_string(day);
    auto it = locationEntries.find(key);
    if (it != locationEntries.end()) {
        writeLysaia(it->second.actual);
    }
}

// ---- Lysaia journal (read-only) ---------------------------------------------

void JournalManager::writeLysaia(const std::string& entry) {
    JournalEntry je{};
    je.content = entry;                 // <-- change if your text field is named differently
    lysaiaEntries.emplace_back(std::move(je));
}

void JournalManager::writeLysaiaAt(const std::string& locationID) {
    auto it = locationEntries.find(locationID);
    if (it != locationEntries.end()) {
        writeLysaia(it->second.actual);
    }
}

void JournalManager::viewLysaia() const {
    if (!showLysaiaJournal) {
        std::cout << "(Lysaia’s journal is locked.)\n";
        return;
    }
    // This mutates state; if you want corruption on view, remove 'const' and call it.
    // maybeCorruptOneOnView();
    printLysaia(std::cout);
}

void JournalManager::unlockLysaiaJournal() {
    showLysaiaJournal = true;
}

bool JournalManager::hasLysaia() const {
    return !lysaiaEntries.empty();
}

void JournalManager::printLysaia(std::ostream& out) const {
    if (lysaiaEntries.empty()) {
        out << "(Lysaia’s journal is empty.)\n";
        return;
    }
    out << "— Lysaia’s Journal —\n";
    for (size_t i = 0; i < lysaiaEntries.size(); ++i) {
        out << (i + 1) << ". " << getText(lysaiaEntries[i]) << "\n";
    }
}

void JournalManager::printLastLysaia(std::ostream& out) const {
    if (!lysaiaEntries.empty()) {
        out << "(Journal updated) " << getText(lysaiaEntries.back()) << "\n";
    }
}

void JournalManager::loadDefaultLocationEntries() {
    // NOTE: IDs use the form "deity/room/<slug>" or "deity/shrine"
    // Keep lines concise; you can revise anytime.

    // --- PERSEPHONE ---
    defineLocationEntry("persephone/room/hall_of_petals",
        "Petals drifted like snow. I wrote her name and it did not sting.",
        "The petals were ash. You coughed blood and called it perfume.");
    defineLocationEntry("persephone/room/orchard_walk",
        "Fruit hung heavy. Choice tasted sweet and strange.",
        "You spat the seeds into the well and wished for winter.");
    defineLocationEntry("persephone/shrine",
        "The fountain promised return without regret.",
        "The seed in the bowl has your tooth-marks.");

    // --- DEMETER ---
    defineLocationEntry("demeter/room/garden_of_broken_faces",
        "Offerings wore smiling masks; I remember the grain was gold.",
        "They were not masks. They watched you chew.");
    defineLocationEntry("demeter/room/threadbare_womb",
        "The shrine asked for patience, not blood.",
        "You rocked the empty cradle until it cried.");
    defineLocationEntry("demeter/shrine",
        "I prayed for harvest, not hunger.",
        "The bowls were already moving when you arrived.");

    // --- NYX ---
    defineLocationEntry("nyx/room/no_corners",
        "Edges softened; night held me without fear.",
        "You walked in circles and called it mercy.");
    defineLocationEntry("nyx/room/nest_of_wings",
        "A single feather fell and did not touch the floor.",
        "The wings were yours. You remember plucking.");
    defineLocationEntry("nyx/shrine",
        "I traded a memory for a quieter sky.",
        "You forgot how to breathe on purpose.");

    // --- APOLLO ---
    defineLocationEntry("apollo/room/hall_of_echoes",
        "The echo answered kindly, a half-beat late.",
        "It answered before you spoke.");
    defineLocationEntry("apollo/room/room_that_remembers",
        "Reflections kept time; mine looked brave enough.",
        "The mirror moved first and smiled with your broken teeth.");
    defineLocationEntry("apollo/shrine",
        "A riddle about light; I chose the honest lie.",
        "There was never an answer. You just stopped asking.");

    // --- HECATE ---
    defineLocationEntry("hecate/room/loom_of_names",
        "Threads hummed; my name held fast among them.",
        "Your thread was cut and you tucked the end into your sleeve.");
    defineLocationEntry("hecate/room/listening_chamber",
        "Lantern-breath on shells; old hymns remembered me.",
        "They repeated your breath after you stopped breathing.");
    defineLocationEntry("hecate/shrine",
        "Three ways; I lit the past to learn the present.",
        "You chose the door that opens into you.");

    // --- THANATOS ---
    defineLocationEntry("thanatos/room/room_of_waiting_lights",
    "No candles were lit when I entered, but one flared to life... then another, without my touch.",
    "Each flame pretended to warm me, but their light only deepened the cold.");
    
    defineLocationEntry("thanatos/room/waiting_room",
    "The chairs faced me in silent expectation; I could not bring myself to sit.",
    "Every chair knew my weight before I placed it there.");

    defineLocationEntry("thanatos/shrine",
        "I stood beside the bed and counted laurel leaves.",
        "You laid down already. The counting was the dream.");

    // --- FALSE HERMES ---
    defineLocationEntry("false_hermes/room/borrowed_things",
        "Names hung from trinkets; none were mine.",
        "Your tag was blank because you carved it off.");
    defineLocationEntry("false_hermes/room/whispering_hall",
        "Every word unique, like footprints in wet clay.",
        "Your handwriting shouted your old name until it bled.");
    defineLocationEntry("false_hermes/shrine",
        "I stopped after twenty steps and turned back.",
        "You never stopped. You are still counting.");

    // --- PAN ---
    defineLocationEntry("pan/room/hall_of_shivering_meat",
        "Stone twitched like muscle—alive, not cruel.",
        "It breathed on your neck and you answered.");
    defineLocationEntry("pan/room/den_of_antlers",
        "Bone and branch braided; the air smelled of pine.",
        "Not all the fur was animal. You kept some.");
    defineLocationEntry("pan/shrine",
        "The melody was simple; I learned the pauses.",
        "You missed a note. He noticed.");

    // --- ERIS ---
    defineLocationEntry("eris/room/throat_of_temple",
        "The passage narrowed kindly, like a throat before a song.",
        "You walked backwards without turning around.");
    defineLocationEntry("eris/room/oracles_wake",
        "Candles steadied in windless dark; the altar listened.",
        "You carved the sentence twenty-seven times and none were yours.");
    defineLocationEntry("eris/room/archivists_cell",
        "I faced the wall to write the truth smaller.",
        "The chair is warm because you never left.");
    defineLocationEntry("eris/shrine",
        "The choir remembered every vow I never sang.",
        "Their hymn is your name pronounced wrong on purpose.");
}

// -----------------------------
// Melas (interactive)
// -----------------------------
void JournalManager::writeMelas(const std::string& entry) {
    melasEntries.emplace_back(entry);
    // 50% chance to add a generic hallucination
    if (std::rand() % 2 == 0) {
        writeCorrupted();
    }

}

void JournalManager::writeMelasAt(const std::string& locationID, bool forceHallucination) {
    auto it = locationEntries.find(locationID);
    if (it == locationEntries.end()) {
        // fallback: write id as plain text to help debugging
        writeMelas("[" + locationID + "]");
        return;
    }

    // Write the true entry for this location
    melasEntries.emplace_back(it->second.actual);

    // Decide if we add a hallucination
    if (forceHallucination || (std::rand() % 2 == 0)) {
        if (!it->second.hallucination.empty()) {
            // Use the location-specific hallucination
            writeCorruptedLine(it->second.hallucination);
        } else {
            // Fallback to the generic hallucination pool
            writeCorrupted();
        }
    }
}


void JournalManager::addPlayerNoteToMelas(int index, const std::string& note) {
    if (index >= 0 && index < static_cast<int>(melasEntries.size())) {
        melasEntries[index].playerNote = note;
    }
}

void JournalManager::viewMelas() const {
    if (melasEntries.empty()) {
        std::cout << "Your journal is empty.\n";
        return;
    }

    std::cout << "\n=== Your Journal ===\n";
    for (size_t i = 0; i < melasEntries.size(); ++i) {
        std::cout << "\nEntry " << i + 1 << ":\n";
        std::cout << melasEntries[i].content << "\n";
        if (!melasEntries[i].playerNote.empty()) {
            std::cout << "[Your Note]: " << melasEntries[i].playerNote << "\n";
        }
    }
    std::cout << "====================\n";
}

// -----------------------------
// File I/O (Melas only)
// -----------------------------
void JournalManager::saveToFile(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) return;

    for (const auto& entry : melasEntries) {
        out << "[ENTRY]" << entry.content << ""; 
        if (!entry.playerNote.empty()) {
            out << "[NOTE]" << entry.playerNote << "";
        }
    }
    out.close();
}

void JournalManager::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return;

    std::string line, content, note;
    while (std::getline(in, line)) {
        if (line == "[ENTRY]") {
            std::getline(in, content);
            note = "";
        } else if (line == "[NOTE]") {
            std::getline(in, note);
            melasEntries.emplace_back(content, note);
        }
    }
    in.close();
}

// -----------------------------
// Hallucinations
// -----------------------------
void JournalManager::writeCorrupted() {
    const int n = static_cast<int>(sizeof(kGenericHallucinations)/sizeof(kGenericHallucinations[0]));
    int index = std::rand() % n;
    melasEntries.emplace_back(std::string("[HALLUCINATION] ") + kGenericHallucinations[index]);
}

void JournalManager::writeCorruptedLine(const std::string& line) {
    melasEntries.emplace_back(std::string("[HALLUCINATION] ") + line);
}
void JournalManager::maybeCorruptOneOnView() {
    // Only Melas’s journal mutates on view
    if (showLysaiaJournal) return;
    if (melasEntries.empty()) return;

    // 50% chance to skip corruption entirely
    if (std::rand() % 2 != 0) return;

    const int nHall = static_cast<int>(sizeof(kGenericHallucinations) / sizeof(kGenericHallucinations[0]));
    int numToCorrupt = 1 + (std::rand() % 3); // 1–3 entries

    std::vector<int> chosenIndexes;
    while ((int)chosenIndexes.size() < numToCorrupt && (int)chosenIndexes.size() < (int)melasEntries.size()) {
        int idx = std::rand() % static_cast<int>(melasEntries.size());
        if (std::find(chosenIndexes.begin(), chosenIndexes.end(), idx) == chosenIndexes.end()) {
            chosenIndexes.push_back(idx);
        }
    }

    for (int idx : chosenIndexes) {
        melasEntries[idx].content = std::string("[HALLUCINATION] ") + kGenericHallucinations[std::rand() % nHall];
        melasEntries[idx].playerNote.clear();
    }
}


void JournalManager::printJournal() {
    // Mutate-on-view behavior (affects only Melas)
    maybeCorruptOneOnView();

    if (showLysaiaJournal) {
        if (lysaiaEntries.empty()) {
            std::cout << "Lysaia's journal is empty.\n";
        } else {
            viewLysaia();
        }
    } else {
        if (melasEntries.empty()) {
            std::cout << "Your journal is empty.\n";
        } else {
            viewMelas();
        }
    }
}
void JournalManager::inspectEntry(int index) const {
    if (showLysaiaJournal) {
        // Inspect Lysaia entry
        if (index <= 0 || static_cast<size_t>(index) > lysaiaEntries.size()) {
            std::cout << "No such entry.\n";
            return;
        }
        const auto& e = lysaiaEntries[static_cast<size_t>(index) - 1];
        std::cout << "\n--- Inspecting Entry " << index << " ---\n";
        std::cout << e.content << "\n";
        std::cout << "---------------------------------\n";
        return;
    }

    // Inspect Melas entry
    if (index <= 0 || static_cast<size_t>(index) > melasEntries.size()) {
        std::cout << "No such entry.\n";
        return;
    }
    const auto& e = melasEntries[static_cast<size_t>(index) - 1];
    std::cout << "\n--- Inspecting Entry " << index << " ---\n";
    std::cout << e.content << "\n";
    if (!e.originalContent.empty()) {
        std::cout << "[Original Entry]: " << e.originalContent << "\n";
    }
    if (!e.playerNote.empty()) {
        std::cout << "[Your Note]: " << e.playerNote << "\n";
    }
    std::cout << "---------------------------------\n";
}

