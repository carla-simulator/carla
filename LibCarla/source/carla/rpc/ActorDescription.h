// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/ActorAttribute.h"
#include "carla/rpc/MsgPack.h"
#include "carla/rpc/String.h"

#include <vector>

namespace carla {
namespace rpc {

  class ActorDescription {
  public:

    ActorDescription() = default;

    uint32_t uid = 0u;

    std::string id;

    std::vector<ActorAttributeValue> attributes;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    operator FActorDescription() const {
      FActorVariation Description;
      Description.UId = uid;
      Description.Id = ToFString(id);
      Description.Variations.Reserve(attributes.size());
      for (const auto &item : attributes) {
        Description.Variations.Add(item);
      }
      return Description;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(uid, id, attributes);
  };

} // namespace rpc
} // namespace carla
