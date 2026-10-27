// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Exception.h"

#include <cerrno>
#include <cstdlib>
#include <cwchar>
#include <locale.h>
#include <new>
#include <stdexcept>
#include <string>

namespace carla {
namespace detail {

  // Serialized map numbers always use the C numeric locale. An explicit locale
  // preserves strtod's format/range semantics without changing application state.
#ifdef _WIN32
  using NumericLocaleHandle = _locale_t;
#else
  using NumericLocaleHandle = locale_t;
#endif

  inline NumericLocaleHandle CreateNumericCLocale() {
#ifdef _WIN32
    const auto locale = _create_locale(LC_NUMERIC, "C");
#else
    const auto locale = newlocale(LC_NUMERIC_MASK, "C", nullptr);
#endif
    if (locale == nullptr) {
      throw_exception(std::bad_alloc());
    }
    return locale;
  }

  inline NumericLocaleHandle GetNumericCLocale() {
    const auto previous_errno = errno;
    // Retain one handle for the process lifetime: XML parsing can also run in
    // global destructors after a function-static owning object was destroyed.
    static const auto locale = CreateNumericCLocale();
    // Successful lazy locale creation must not change strtod's errno contract.
    errno = previous_errno;
    return locale;
  }

  inline double StrtodC(const char *text, char **end) {
#ifdef _WIN32
    return _strtod_l(text, end, GetNumericCLocale());
#else
    return strtod_l(text, end, GetNumericCLocale());
#endif
  }

  inline double StrtodC(const wchar_t *text, wchar_t **end) {
#ifdef _WIN32
    return _wcstod_l(text, end, GetNumericCLocale());
#else
    return wcstod_l(text, end, GetNumericCLocale());
#endif
  }

  inline double StodC(const std::string &text) {
    char *end = nullptr;
    errno = 0;
    const auto result = StrtodC(text.c_str(), &end);
    if (end == text.c_str()) {
      throw_exception(std::invalid_argument("stod"));
    }
    if (errno == ERANGE) {
      throw_exception(std::out_of_range("stod"));
    }
    return result;
  }

} // namespace detail
} // namespace carla
