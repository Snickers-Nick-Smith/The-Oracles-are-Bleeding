#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <string_view>
#include <vector>
#include <utility>   // for std::pair

// Basic text helpers
std::string join(const std::vector<std::string>& list, const std::string& sep);
std::string toLower(const std::string& s);

// Command parsing helpers
std::string trim_copy(std::string s);
std::pair<std::string,std::string> split_first(const std::string& input); // {lowercased first, trimmed rest}
std::string normalize_dir(std::string d);  // "n"/"north" -> "north", "" if not a direction
bool is_move_verb(const std::string& w);   // go/move/walk/run/head/travel

// Accessibility settings
struct AccessibilitySettings {
    bool colorEnabled = true;
    bool screenShakeEnabled = true;
    int  textSpeed = 2; // 0=instant, 1=fast, 2=normal, 3=slow
};

// Console / platform helpers
bool isStdoutTTY();
bool enableVTSupport();
bool ansiCapable();
void sleepMillis(int ms);
void writeRaw(std::string_view s);
void flush();
inline void cr() { writeRaw("\r"); }

// ANSI helpers
std::string ansi(std::string_view seq);
inline std::string reset() { return ansi("\x1b[0m"); }

// Effects
void slowPrint(const std::string& text, unsigned int ms = 20);
void printWithSpeed(std::string_view text, const AccessibilitySettings& as, bool endWithNewline = true);
void shakeLine(std::string_view text,
               const AccessibilitySettings& as,
               int intensity = 2,
               int durationMs = 200,
               int baseIndent = 0,
               bool commitLine = true);

#endif // UTILS_HPP
