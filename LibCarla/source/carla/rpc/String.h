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

  constexpr size_t MaxStringLength = 5000000;

  static inline std::string FromFString(const FString &Str) {
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

  static inline FString ToFString(const std::string &str) {
    FString result = "";
    size_t i = 0;
    while(i + MaxStringLength < str.size()) {
      auto substr = str.substr(i, MaxStringLength);
      FString temp_string(substr.size(), UTF8_TO_TCHAR(substr.c_str()));
      result += temp_string;
      i += MaxStringLength;
    }
    auto substr = str.substr(i);
    FString temp_string(substr.size(), UTF8_TO_TCHAR(substr.c_str()));
    result += temp_string;
    return result;
  }

#endif // LIBCARLA_INCLUDED_FROM_UE4

} // namespace rpc
} // namespace carla
