#pragma once
#include <cstddef>

// Builds a string from multiple null-terminated strings.
char* strbuild(char* dest, ...);

// Returns true if s2 is a prefix of s1.
bool strprefix(const char* s1, const char* s2);

// Safe memcpy that handles overlapping ranges.
void ovbcopy(const char* from, char* to, int bytes);

// Finds the first occurrence of c in s.
char* index(const char* s, char c);


