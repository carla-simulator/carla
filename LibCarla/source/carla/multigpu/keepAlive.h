// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/asio/ip/tcp.hpp>

#if defined(__linux__)
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#endif

namespace carla {
namespace multigpu {

  /// Lets either end notice a peer that vanished without closing the
  /// connection (a killed host, a cut link): on Linux, after 10 s of silence
  /// three probes 5 s apart, so about 25 s.
  inline void EnableKeepAlive(boost::asio::ip::tcp::socket &socket) {
    boost::system::error_code ec;
    socket.set_option(boost::asio::socket_base::keep_alive(true), ec);
#if defined(__linux__)
    const int idle_seconds = 10;
    const int interval_seconds = 5;
    const int probes = 3;
    const auto handle = socket.native_handle();
    ::setsockopt(handle, IPPROTO_TCP, TCP_KEEPIDLE, &idle_seconds, sizeof(idle_seconds));
    ::setsockopt(handle, IPPROTO_TCP, TCP_KEEPINTVL, &interval_seconds, sizeof(interval_seconds));
    ::setsockopt(handle, IPPROTO_TCP, TCP_KEEPCNT, &probes, sizeof(probes));
#endif
  }

} // namespace multigpu
} // namespace carla
