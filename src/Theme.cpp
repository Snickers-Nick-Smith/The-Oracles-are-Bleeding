#include "Theme.hpp"

// ---- Internal helpers -------------------------------------------------------

static const Theme& defaultTheme() {
    static const Theme t{ "", "", "", "", "", "" };
    return t;
}

// Notes:
// - We keep backgrounds empty for most deities (foreground-only).
// - Nyx demonstrates black text on a colored background.
// - ansi(...) returns "" if ANSI isn't supported, so colorize() will gracefully fallback.

// Nyx — lighter black to darker black text, blue/violet backgrounds
static const Theme& nyxTheme() {
    static const Theme t{
        ansi("\x1b[38;5;234m"),   // fg Uncorrupted: warm black
        ansi("\x1b[38;5;232m"),   // fg Corrupted: cold black
        ansi("\x1b[48;5;61m"),  // bg Uncorrupted: scampi blue
        ansi("\x1b[48;5;98m"),   // bg Corrupted: medium violet
        ansi("\x1b[2m"),
        ansi("\x1b[2m")
    };
    return t;
}

// Eris — rust → orange (no bg)
static const Theme& erisTheme() {
    static const Theme t{
        ansi("\x1b[38;5;88m"),
        ansi("\x1b[38;5;160m"),
        "", "",
        ansi("\x1b[2m"),        // attr UNCORRUPTED: faint
        ansi("\x1b[1m")         // attr CORRUPTED:   bold
    };
    return t;
}

// Pan — kelly green → apple lime (no bg)
static const Theme& panTheme() {
    static const Theme t{
        ansi("\x1b[38;5;70m"),
        ansi("\x1b[38;5;106m"),
        "", "",
    };
    return t;
}

// Demeter — corn → dark olive (no bg)
static const Theme& demeterTheme() {
    static const Theme t{
        ansi("\x1b[38;5;184m"), // corn
        ansi("\x1b[38;5;58m"), // dark olive yellow
        "", "",
        "", ""
    };
    return t;
}

// Persephone — mauve → pomegranate (no bg)
static const Theme& persephoneTheme() {
    static const Theme t{
        ansi("\x1b[38;5;176m"), // mauve
        ansi("\x1b[38;5;52m"),  // pomegranate
        "", "",
        "", ""
    };
    return t;
}

// False Hermes mercury — scorpion →  (no bg)
static const Theme& fhermesTheme() {
    static const Theme t{
        ansi("\x1b[38;5;254m"), // mercury
        ansi("\x1b[38;5;59m"),  // scorpion
        "", "",
        "", ""
    };
    return t;
}

// Thanatos — gray → dark gray 
static const Theme& thanatosTheme() {
    static const Theme t{
        ansi("\x1b[38;5;240m"), // davys_gray
        ansi("\x1b[38;5;236m"), // dark charcoal
        ansi("\x1b[48;5;69m"),  // bg Uncorrupted: blueberry 
        ansi("\x1b[48;5;105m")  // bg Corrupted: violets_are_blue 
    };
    return t;
}

// Apollo — gold → brass (no bg)
static const Theme& apolloTheme() {
    static const Theme t{
        ansi("\x1b[38;5;220m"), // gold
        ansi("\x1b[38;5;136m"), // brass
        "", "",
        "", ""
    };
    return t;
}

// Hecate — outerspace grey → blue‑violet (no bg)
static const Theme& hecateTheme() {
    static const Theme t{
        ansi("\x1b[38;5;238m"), // outerspace grey
        ansi("\x1b[38;5;60m"), // blue-violet
        "", "",
        "", ""
    };
    return t;
}

// ---- ThemeRegistry impl -----------------------------------------------------

const Theme& ThemeRegistry::get(Deity d) {
    switch (d) {
        case Deity::Nyx:         return nyxTheme();
        case Deity::Eris:        return erisTheme();
        case Deity::Pan:         return panTheme();
        case Deity::Demeter:     return demeterTheme();
        case Deity::Persephone:  return persephoneTheme();
        case Deity::FalseHermes: return fhermesTheme();
        case Deity::Thanatos:    return thanatosTheme();
        case Deity::Apollo:      return apolloTheme();
        case Deity::Hecate:      return hecateTheme();
        default:                 return defaultTheme();
    }
}

std::string ThemeRegistry::colorize(Deity d,
                                    ShrineState state,
                                    std::string_view text,
                                    const AccessibilitySettings& as) {
    if (!as.colorEnabled || !ansiCapable()) {
        return std::string(text);
    }

    const Theme& th = get(d);
    const std::string& fg = (state == ShrineState::UNCORRUPTED) ? th.fgUNCORRUPTED : th.fgCORRUPTED;
    const std::string& bg = (state == ShrineState::UNCORRUPTED) ? th.bgUNCORRUPTED : th.bgCORRUPTED;

    if (fg.empty() && bg.empty()) {
        return std::string(text);
    }

    std::string out;
    out.reserve(fg.size() + bg.size() + text.size() + 4);
    if (!fg.empty()) out.append(fg);
    if (!bg.empty()) out.append(bg);
    out.append(text);
    out.append(reset()); // reset() -> "" if ANSI not supported
    return out;
}

std::string ThemeRegistry::colorize(Deity d,
                                    std::string_view text,
                                    const AccessibilitySettings& as) {
    return colorize(d, getDefaultShrineState(), text, as);
}

void ThemeRegistry::setDefaultShrineState(ShrineState s) {
    defaultStateRef() = s;
}

ShrineState ThemeRegistry::getDefaultShrineState() {
    return defaultStateRef();
}

ShrineState& ThemeRegistry::defaultStateRef() {
    static ShrineState s = ShrineState::UNCORRUPTED;
    return s;
}

// Convenience printer: wraps style() + typewriter + optional shake
void ThemeRegistry::printDeityLine(Deity d,
                                   ShrineState state,
                                   std::string_view text,
                                   const AccessibilitySettings& as,
                                   bool shake,
                                   int intensity,
                                   int durationMs) {
    const std::string styled = style(d, state, text, as);
    if (shake) {
        // Use immediate print inside shake; it handles its own redraw.
        shakeLine(styled, as, intensity, durationMs);
    } else {
        printWithSpeed(styled, as, /*endWithNewline*/true);
    }
}

