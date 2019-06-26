// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/geom/BoundingBox.h"
#include "carla/rpc/ActorDescription.h"
#include "carla/rpc/ActorId.h"
#include "carla/streaming/Token.h"

#include <cstring>

namespace carla {
namespace rpc {

  class Actor {
  public:

    Actor() = default;

    ActorId id = 0u;

    ActorId parent_id = 0u;

    ActorDescription description;

    geom::BoundingBox bounding_box;

    std::vector<uint8_t> semantic_tags;

    /// @todo This is only used by sensors actually.
    /// @name Sensor functionality
    /// @{

    std::vector<unsigned char> stream_token;

    bool HasAStream() const {
      return stream_token.size() == sizeof(streaming::Token::data);
    }

    streaming::Token GetStreamToken() const {
      DEBUG_ASSERT(HasAStream());
      streaming::Token token;
      std::memcpy(&token.data[0u], stream_token.data(), stream_token.size());
      return token;
    }

    /// @}

    MSGPACK_DEFINE_ARRAY(id, parent_id, description, bounding_box, semantic_tags, stream_token);
  };

} // namespace rpc
} // namespace carla
