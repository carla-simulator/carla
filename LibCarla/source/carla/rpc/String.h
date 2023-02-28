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

  static inline std::string FromFString(const FString &Str) {
    return TCHAR_TO_UTF8(*Str);
  }

  static inline FString ToFString(const std::string &str) {
    return FString(str.size(), UTF8_TO_TCHAR(str.c_str()));
  }

#endif // LIBCARLA_INCLUDED_FROM_UE4

} // namespace rpc
} // namespace carla
