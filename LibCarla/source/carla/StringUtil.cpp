// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/StringUtil.h"

#include <algorithm>

#ifdef _WIN32
  #include <Windows.h>
  #pragma comment(lib, "User32.lib")
#else
  #include <fnmatch.h>
#endif // _WIN32

namespace carla {

#ifdef _WIN32
namespace {

  std::string FoldWindowsCase(const char *text) {
    std::string result(text);
    size_t offset = 0u;
    while (offset < result.size()) {
      const auto count = static_cast<DWORD>((std::min)(
          result.size() - offset, static_cast<size_t>(MAXDWORD)));
      CharLowerBuffA(result.data() + offset, count);
      offset += count;
    }
    return result;
  }

  unsigned char ReadPatternCharacter(const char *&pattern, const char *end) {
    if (*pattern == '\\' && pattern + 1 < end) {
      ++pattern;
    }
    return static_cast<unsigned char>(*pattern++);
  }

  bool MatchCharacter(const char *&pattern, unsigned char value) {
    const auto token = *pattern++;
    if (token == '?') {
      return true;
    }
    if (token == '\\') {
      if (*pattern == '\0') {
        return false;
      }
      return value == static_cast<unsigned char>(*pattern++);
    }
    if (token != '[') {
      return value == static_cast<unsigned char>(token);
    }

    auto cursor = pattern;
    const bool negate = *cursor == '!' || *cursor == '^';
    if (negate) {
      ++cursor;
    }
    if (*cursor == ']') {
      ++cursor;
    }
    while (*cursor != '\0' && *cursor != ']') {
      if (*cursor == '\\' && cursor[1] != '\0') {
        ++cursor;
      }
      ++cursor;
    }
    if (*cursor != ']') {
      // An unclosed bracket is a literal, as in the existing POSIX backend.
      return value == '[';
    }

    const auto end = cursor;
    cursor = pattern + (negate ? 1u : 0u);
    bool matched = false;
    while (cursor < end) {
      const auto first = ReadPatternCharacter(cursor, end);
      if (cursor + 1 < end && *cursor == '-') {
        ++cursor;
        const auto last = ReadPatternCharacter(cursor, end);
        matched = matched || (first <= value && value <= last);
      } else {
        matched = matched || first == value;
      }
    }
    pattern = end + 1;
    return negate ? !matched : matched;
  }

  bool MatchWindowsGlob(const char *input, const char *pattern) {
    const char *star_pattern = nullptr;
    const char *star_input = nullptr;
    while (*input != '\0') {
      if (*pattern == '*') {
        do {
          ++pattern;
        } while (*pattern == '*');
        if (*pattern == '\0') {
          return true;
        }
        star_pattern = pattern;
        star_input = input;
        continue;
      }
      auto next = pattern;
      if (*pattern != '\0' && MatchCharacter(next, static_cast<unsigned char>(*input))) {
        pattern = next;
        ++input;
      } else if (star_pattern != nullptr) {
        pattern = star_pattern;
        input = ++star_input;
      } else {
        return false;
      }
    }
    while (*pattern == '*') {
      ++pattern;
    }
    return *pattern == '\0';
  }

} // namespace
#endif // _WIN32

  bool StringUtil::Match(const char *str, const char *test) {
#ifdef _WIN32
    const auto input = FoldWindowsCase(str);
    const auto pattern = FoldWindowsCase(test);
    return MatchWindowsGlob(input.c_str(), pattern.c_str());
#else
    return 0 == fnmatch(test, str, 0);
#endif // _WIN32
  }

} // namespace carla
