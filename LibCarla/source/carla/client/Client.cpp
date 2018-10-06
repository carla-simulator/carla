// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Client.h"

namespace carla {
namespace client {

  Client::Client(const std::string &host, uint16_t port, size_t worker_threads)
    : _client_state(MakeShared<detail::Client>(host, port, worker_threads)) {}

} // namespace client
} // namespace carla
