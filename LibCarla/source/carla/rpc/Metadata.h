// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

namespace carla {
namespace rpc {

  /// Metadata of an RPC function call.
  class Metadata {
  public:

    Metadata() = default;

    static Metadata MakeSync() {
      return { false };
    }

    static Metadata MakeAsync() {
      return { true };
    }

    bool IsResponseIgnored() const {
      return _asynchronous_call;
    }

  private:

    Metadata(bool asynchronous_call) : _asynchronous_call(asynchronous_call) {}

    bool _asynchronous_call = false;

  public:

    MSGPACK_DEFINE_ARRAY(_asynchronous_call);
  };

} // namespace rpc
} // namespace carla
