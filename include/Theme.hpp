#ifndef THEME_HPP
#define THEME_HPP

#include <string>
#include <string_view>
#include "utils.hpp"

// High-level identity for coloring per shrine/speaker.
enum class Deity {
    Nyx,
    Eris,
    Pan,
    Demeter,
    Persephone,
    FalseHermes,
    Thanatos,
    Apollo,
    Hecate,
    Default
};

// Visual state of a shrine/scene.
enum class ShrineState {
    UNCORRUPTED,
    CORRUPTED
};

// Theme supports separate foreground and background colors
// for both Pristine and Decayed states.
struct Theme {
    std::string fgUNCORRUPTED; // e.g., ansi("\x1b[38;5;135m")
    std::string fgCORRUPTED;  // e.g., ansi("\x1b[38;5;60m")
    std::string bgUNCORRUPTED; // e.g., ansi("\x1b[48;5;99m")
    std::string bgCORRUPTED;  // e.g., ansi("\x1b[48;5;60m")
    std::string attrUNCORRUPTED; // e.g., ansi("[2m")  (faint)
    std::string attrCORRUPTED;   // e.g., ansi("[1m")  (bold)
};

class ThemeRegistry {
public:
    // Retrieve the Theme for a deity.
    static const Theme& get(Deity d);

    // Colorize with explicit shrine state (recommended).
    static std::string colorize(Deity d,
                                ShrineState state,
                                std::string_view text,
                                const AccessibilitySettings& as);

    // Convenience: uses the registry's default shrine state (configurable).
    static std::string colorize(Deity d,
                                std::string_view text,
                                const AccessibilitySettings& as);

    // Compose color + background + attributes into a single styled string.
    static std::string style(Deity d,
                             ShrineState state,
                             std::string_view text,
                             const AccessibilitySettings& as);
                             
    // Global default shrine state (useful if most of a chapter is one state).
    static void setDefaultShrineState(ShrineState s);
    static ShrineState getDefaultShrineState();
    static void printDeityLine(Deity d,
                               ShrineState state,
                               std::string_view text,
                               const AccessibilitySettings& as,
                               bool shake = false,
                               int intensity = 2,
                               int durationMs = 200);
                               
    static std::string style(Deity, ShrineState, std::string_view, const AccessibilitySettings&);

private:
    static ShrineState& defaultStateRef();
};

#endif // THEME_HPP
