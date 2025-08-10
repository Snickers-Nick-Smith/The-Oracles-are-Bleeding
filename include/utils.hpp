#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <string_view>
#include <vector>

std::string join(const std::vector<std::string>& list, const std::string& sep);
std::string toLower(const std::string& s);
void slowPrint(const std::string& text, unsigned int ms = 20);

// ---------------------------
// Accessibility settings
// ---------------------------
struct AccessibilitySettings {
    bool colorEnabled = true; // true= Colors on false= Colors off
    bool screenShakeEnabled = true; //true= shake on false shake off
    int  textSpeed = 2; // 0=instant, 1=fast, 2=normal, 3=slow
};

// ---------------------------
// Console / platform helpers
// ---------------------------

// Detect if stdout is a TTY (terminal). If not, disable fancy effects/colors.
bool isStdoutTTY();

// Enable ANSI/VT sequences on Windows 10+ (noop on POSIX). Returns true if enabled/available.
bool enableVTSupport();

// Return true if we can safely output ANSI escapes (VT enabled AND stdout is a TTY).
bool ansiCapable();

// Sleep helper
void sleepMillis(int ms);

// Emit raw without newline
void writeRaw(std::string_view s);

// Flush stdout
void flush();

// Carriage return to start of line (no newline)
inline void cr() { writeRaw("\r"); }

// Build an ANSI sequence safely (returns "" if not ansiCapable()).
std::string ansi(std::string_view seq);

// Reset ANSI formatting (returns "" if not ansiCapable()).
inline std::string reset() { return ansi("\x1b[0m"); }

// ---------------------------
// Effects
// ---------------------------

// Typewriter-style printing that obeys AccessibilitySettings::textSpeed.
// speed map: 0=instant, 1=fast (~5 ms), 2=normal (~12 ms), 3=slow (~24 ms)
void printWithSpeed(std::string_view text, const AccessibilitySettings& as, bool endWithNewline = true);

// Shake a single line using horizontal indent jitter.
// intensity: 1..3   (maps to amplitude)
// durationMs: total effect duration (e.g., ~200ms feels punchy)
// baseIndent: fixed spaces kept before jitter (usually 0)
// commitLine: if true, finish with newline; otherwise leave settled without newline.
void shakeLine(std::string_view text,
               const AccessibilitySettings& as,
               int intensity = 2,
               int durationMs = 200,
               int baseIndent = 0,
               bool commitLine = true);

#endif
