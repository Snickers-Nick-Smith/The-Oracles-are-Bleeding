// PersephoneFragments.hpp
#pragma once
#include "Mechanics.hpp"
#include <vector>
#include <string>
#include <utility>

// Create the 8 corrupted fragments as InventoryItems.
// id format: "perse_frag_<index>"
std::vector<InventoryItem> MakePersephoneFragments();

// Inventory helpers
bool HasAllPersephoneFragments(const PlayerState& ps);
std::vector<std::pair<int,std::string>> GetOwnedPersephoneFragments(const PlayerState& ps);

// Room pickup utility: call once when player enters the room that holds this fragment.
// Uses a unique flag (e.g., "picked_perse_frag_3") to ensure one-time pickup.
// NOTE: No UI parameter needed; the caller can print o.journalEntry if desired.
Outcome PickupPersephoneFragmentInRoom(InteractionContext& ctx, int index, const std::string& pickedFlagKey);

// Canonical, uncorrupted Persephone→Demeter letter,
// already split into lines (use for Demeter’s calm read).
extern const std::vector<std::string> kPersephoneLetterClean;