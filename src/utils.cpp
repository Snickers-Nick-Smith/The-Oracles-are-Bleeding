#include "utils.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <iostream>
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
