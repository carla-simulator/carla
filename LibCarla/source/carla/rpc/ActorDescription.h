// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/rpc/ActorAttribute.h"
#include "carla/rpc/ActorId.h"
#include "carla/rpc/String.h"

#include <vector>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <util/enable-ue4-macros.h>
#include "Carla/Actor/ActorDescription.h"
#include <util/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  class ActorDescription {
  public:

    ActorDescription() = default;

    ActorId uid = 0u;

    std::string id;

    std::vector<ActorAttributeValue> attributes;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    ActorDescription(const FActorDescription &Description)
      : uid(Description.UId),
        id(FromFString(Description.Id)) {
      attributes.reserve(Description.Variations.Num());
      for (const auto &Item : Description.Variations) {
        attributes.emplace_back(Item.Value);
      }
    }

    operator FActorDescription() const {
      FActorDescription Description;
      Description.UId = uid;
      Description.Id = ToFString(id);
      Description.Variations.Reserve(attributes.size());
      for (const auto &item : attributes) {
        Description.Variations.Emplace(ToFString(item.id), item);
      }
      return Description;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(uid, id, attributes);
  };

} // namespace rpc
} // namespace carla
