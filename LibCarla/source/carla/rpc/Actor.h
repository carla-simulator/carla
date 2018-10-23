// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/rpc/ActorDescription.h"
#include "carla/streaming/Token.h"

#include <cstring>

namespace carla {
namespace rpc {

  class Actor {
  public:

    using id_type = uint32_t;

    Actor() = default;

    id_type id;

    ActorDescription description;

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

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    Actor(FActorView View)
      : id(View.GetActorId()),
        description(*View.GetActorDescription()) {
      DEBUG_ASSERT(View.IsValid());
    }

    Actor(FActorView View, const streaming::Token &StreamToken)
      : id(View.GetActorId()),
        description(*View.GetActorDescription()),
        stream_token(StreamToken.data.begin(), StreamToken.data.end()) {
      DEBUG_ASSERT(View.IsValid());
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(id, description, stream_token);
  };

} // namespace rpc
} // namespace carla
