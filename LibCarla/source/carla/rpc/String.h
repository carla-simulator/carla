// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "UnrealString.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

#ifdef LIBCARLA_INCLUDED_FROM_UE4

  // Fast conversion from fstring
  static inline std::string FromFString(const FString &Str) {
    return TCHAR_TO_UTF8(*Str);
  }

  // Fast conversion to fstring
  static inline FString ToFString(const std::string &str) {
    return FString(str.size(), UTF8_TO_TCHAR(str.c_str()));
  }

  constexpr size_t MaxStringLength = 5000000;

  // Slower conversion from fstring for long text
  static inline std::string FromLongFString(const FString &Str) {
    std::string result;
    size_t i = 0;
    while(i + MaxStringLength < Str.Len()) {
      auto Substr = Str.Mid(i, MaxStringLength);
      std::string temp_string = TCHAR_TO_UTF8(*Substr);
      result += temp_string;
      i += MaxStringLength;
    }
    auto Substr = Str.Mid(i, Str.Len() - i);
    std::string temp_string = TCHAR_TO_UTF8(*Substr);
    result += temp_string;
    return result;
  }

  // Slower conversion to fstring for long text
  static inline FString ToLongFString(const std::string &str) {
    FString result = "";
    for (size_t i = 0; i < str.size(); i++)
    {
      result += str[i];
    }
    return result;
  }

#endif // LIBCARLA_INCLUDED_FROM_UE4

} // namespace rpc
} // namespace carla
