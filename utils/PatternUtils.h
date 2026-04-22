#ifndef PATTERN_UTILS_H
#define PATTERN_UTILS_H

#include <map>
#include <string>

using Pattern = std::map<std::string, float>;

std::string serializePattern(const Pattern& pat);
Pattern deserializePattern(const std::string& serialized);

#endif
