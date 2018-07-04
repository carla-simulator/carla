// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "UnrealString.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

#include <string>

namespace carla {
namespace rpc {

  class ActorBlueprint {
  public:

    ActorBlueprint(std::string type_id)
      : type_id(std::move(type_id)) {}

    ActorBlueprint() = default;
    ActorBlueprint(const ActorBlueprint &) = default;
    ActorBlueprint(ActorBlueprint &&) = default;
    ActorBlueprint &operator=(const ActorBlueprint &) = default;
    ActorBlueprint &operator=(ActorBlueprint &&) = default;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    // ActorBlueprint(const FString &Type)
    //   : type_id(TCHAR_TO_UTF8(*Type)) {}

    FString GetTypeIdAsFString() const {
      return FString(type_id.size(), UTF8_TO_TCHAR(type_id.c_str()));
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    std::string type_id;

    MSGPACK_DEFINE_ARRAY(type_id);
  };

} // namespace rpc
} // namespace carla
