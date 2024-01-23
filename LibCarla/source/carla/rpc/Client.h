// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/Metadata.h"

#include <rpc/client.h>

namespace carla {
namespace rpc {

  class Client {
  public:

    template <typename... Args>
    explicit Client(Args &&... args)
      : _client(std::forward<Args>(args)...) {}

    void set_timeout(int64_t value) {
      _client.set_timeout(value);
    }

    auto get_timeout() const {
      return _client.get_timeout();
    }

    template <typename... Args>
    auto call(const std::string &function, Args &&... args) {
      return _client.call(function, Metadata::MakeSync(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void async_call(const std::string &function, Args &&... args) {
      _client.async_call(function, Metadata::MakeAsync(), std::forward<Args>(args)...);
    }

  private:

    ::rpc::client _client;
  };

} // namespace rpc
} // namespace carla
