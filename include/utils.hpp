#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>
#include <vector>
std::string join(const std::vector<std::string>& list, const std::string& sep);
std::string toLower(const std::string& s);
void slowPrint(const std::string& text, unsigned int ms = 20);

#endif
