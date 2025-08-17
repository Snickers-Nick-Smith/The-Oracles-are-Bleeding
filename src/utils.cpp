#include "utils.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <random>
#include <sstream>
#include <thread>
#include <iostream>
#include <vector>
#if defined(_WIN32)
  #include <io.h>
  #include <windows.h>
  #ifndef STDOUT_FILENO
    #define STDOUT_FILENO _fileno(stdout)
  #endif
  #define ISATTY _isatty
#else
  #include <unistd.h>
  #define ISATTY isatty
#endif



void slowPrint(const std::string& text, unsigned int ms) {
    for (char c : text) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    std::cout << '\n';
}

std::string join(const std::vector<std::string>& list, const std::string& sep) {
    std::ostringstream oss;
    for (size_t i = 0; i < list.size(); ++i) {
        oss << list[i];
        if (i + 1 < list.size()) oss << sep;
    }
    return oss.str();
}

std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return result;
}

// ============================================================================
// Console / platform helpers
// ============================================================================

static bool g_vtEnabledChecked = false;
static bool g_vtEnabled = false;

bool isStdoutTTY() {
#if defined(_WIN32)
    return ISATTY(STDOUT_FILENO) != 0;
#else
    return ISATTY(STDOUT_FILENO) == 1;
#endif
}

bool enableVTSupport() {
#if defined(_WIN32)
    if (g_vtEnabledChecked) return g_vtEnabled;
    g_vtEnabledChecked = true;

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) { g_vtEnabled = false; return false; }

    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) { g_vtEnabled = false; return false; }

    DWORD desired = mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, desired)) {
        g_vtEnabled = false; // Older consoles may fail; we'll just skip ANSI then.
        return false;
    }
    g_vtEnabled = true;
    return true;
#else
    // POSIX terminals generally support ANSI
    g_vtEnabledChecked = true;
    g_vtEnabled = true;
    return true;
#endif
}

bool ansiCapable() {
    if (!g_vtEnabledChecked) enableVTSupport();
    return g_vtEnabled && isStdoutTTY();
}

void sleepMillis(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void writeRaw(std::string_view s) {
    std::fwrite(s.data(), 1, s.size(), stdout);
}

void flush() {
    std::fflush(stdout);
}

 std::string trim_copy(std::string s) {
        auto not_space = [](int ch){ return !std::isspace(ch); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
        s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
        return s;
    }

    // Split into first word (lowercased) and the rest (trimmed, original case kept so notes work)
    std::pair<std::string,std::string> split_first(const std::string& input) {
        std::istringstream iss(input);
        std::string first;
        iss >> first;
        std::string rest;
        std::getline(iss, rest);
        if (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
        return { toLower(first), trim_copy(rest) };
    }

  // Normalize direction tokens: supports full, hyphenless, and short forms.
// Returns empty string if not a direction.
std::string normalize_dir(std::string d) {
    d = toLower(d);
    if (d == "n")  return "north";
    if (d == "s")  return "south";
    if (d == "e")  return "east";
    if (d == "w")  return "west";
    if (d == "ne") return "northeast";
    if (d == "nw") return "northwest";
    if (d == "se") return "southeast";
    if (d == "sw") return "southwest";
    if (d == "u" || d == "up") return "up";
    if (d == "d" || d == "down") return "down";

    // already full word? pass through if valid
    static const std::vector<std::string> dirs = {
        "north","south","east","west",
        "northeast","northwest","southeast","southwest",
        "up","down"
    };
    return (std::find(dirs.begin(), dirs.end(), d) != dirs.end()) ? d : std::string{};
}

// Convert a long form direction to its short alias (north -> n).
std::string to_short_dir(const std::string& longDir) {
    std::string d = toLower(longDir);
    if (d == "north")      return "n";
    if (d == "south")      return "s";
    if (d == "east")       return "e";
    if (d == "west")       return "w";
    if (d == "northeast")  return "ne";
    if (d == "northwest")  return "nw";
    if (d == "southeast")  return "se";
    if (d == "southwest")  return "sw";
    if (d == "up")         return "u";
    if (d == "down")       return "d";

    // Already short? return as-is.
    if (d == "n"||d=="s"||d=="e"||d=="w"||
        d=="ne"||d=="nw"||d=="se"||d=="sw"||
        d=="u"||d=="d")
        return d;

    return {};
}

// Convert a short alias to its long form (n -> north).
std::string expand_dir(const std::string& shortDir) {
    std::string d = toLower(shortDir);
    if (d == "n")  return "north";
    if (d == "s")  return "south";
    if (d == "e")  return "east";
    if (d == "w")  return "west";
    if (d == "ne") return "northeast";
    if (d == "nw") return "northwest";
    if (d == "se") return "southeast";
    if (d == "sw") return "southwest";
    if (d == "u")  return "up";
    if (d == "d")  return "down";
    return d; // already long or unknown
}

bool is_move_verb(const std::string& w) {
    return w == "go" || w == "move" || w == "walk" ||
           w == "run" || w == "head" || w == "travel";
}

std::string ansi(std::string_view seq) {
    if (!ansiCapable()) return std::string{};
    return std::string(seq);
}




// ============================================================================
// Effects
// ============================================================================

static int speedToDelayMs(int speed) {
    switch (speed) {
        case 0: return 0;   // instant
        case 1: return 5;   // fast
        case 2: return 12;  // normal
        case 3: return 24;  // slow
        default: return 12;
    }
}

static std::mt19937& rng() {
    thread_local std::mt19937 gen{ std::random_device{}() };
    return gen;
}

void printWithSpeed(std::string_view text, const AccessibilitySettings& as, bool endWithNewline) {
    const int delay = speedToDelayMs(as.textSpeed);
    if (delay <= 0 || !isStdoutTTY()) {
        writeRaw(text);
        if (endWithNewline) writeRaw("\n");
        flush();
        return;
    }

    for (char c : text) {
        writeRaw(std::string_view(&c, 1));
        flush();
        sleepMillis(delay);
    }
    if (endWithNewline) writeRaw("\n");
    flush();
}

void shakeLine(std::string_view text,
               const AccessibilitySettings& as,
               int intensity,
               int durationMs,
               int baseIndent,
               bool commitLine) {
    // Respect accessibility toggle and non-TTY environments
    if (!as.screenShakeEnabled || !isStdoutTTY() || intensity <= 0 || durationMs <= 0) {
        if (baseIndent > 0) writeRaw(std::string(baseIndent, ' '));
        writeRaw(text);
        if (commitLine) writeRaw("\n");
        flush();
        return;
    }

    // Clamp intensity
    if (intensity < 1) intensity = 1;
    if (intensity > 3) intensity = 3;

    // Map intensity to amplitude and frame pacing
    const int amplitude = intensity; // spaces of jitter
    const int frameMs   = 22;
    int frames = durationMs / frameMs;
    if (frames < 4) frames = 4;
    if (frames > 18) frames = 18;

    std::uniform_int_distribution<int> dist(-amplitude, amplitude);

    // Initial draw (settled) so there's always text visible
    {
        std::string prefix(baseIndent, ' ');
        writeRaw(prefix);
        writeRaw(text);
        cr();
        flush();
    }

    for (int i = 0; i < frames; ++i) {
        int off = dist(rng());
        int leftSpaces = baseIndent + (off > 0 ? off : 0);
        if (leftSpaces < 0) leftSpaces = 0;

        cr();
        if (leftSpaces > 0) writeRaw(std::string(leftSpaces, ' '));
        writeRaw(text);
        flush();
        sleepMillis(frameMs);
    }

    // Final settle
    cr();
    if (baseIndent > 0) writeRaw(std::string(baseIndent, ' '));
    writeRaw(text);
    if (commitLine) writeRaw("\n");
    flush();
}
