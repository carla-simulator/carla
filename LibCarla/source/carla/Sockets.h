// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

#ifdef _WIN32
  #include <winsock2.h>   ///< socket
  #include <Ws2tcpip.h>
#else
  #include <sys/socket.h> ///< socket
  #include <netinet/in.h> ///< sockaddr_in
  #include <arpa/inet.h>  ///< getsockname
  #include <unistd.h>     ///< close
  #include <fcntl.h>      ///< fcntl, FD_CLOEXEC
  #include <dirent.h>     ///< opendir, readdir
  #include <cstdlib>      ///< strtol
#endif

#define SOCK_INVALID_INDEX  -1

namespace carla {

  /// Marks a socket file descriptor close-on-exec, so it is not inherited
  /// by child processes started via fork+exec (e.g. RecastBuilder, launched
  /// by UCarlaEpisode::LoadNewOpendriveEpisode through
  /// FPlatformProcess::CreateProc). Without this, a forked child that
  /// outlives the CARLA process keeps the listen socket bound and the next
  /// server boot fails in asio's bind(). No-op on Windows, where
  /// CreateProcess only inherits handles explicitly marked inheritable
  /// (bInheritHandles), which UE's process-spawning calls already leave off.
  template <typename NativeHandle>
  inline void SetSocketCloseOnExec(NativeHandle fd)
  {
#ifndef _WIN32
    if (fd != SOCK_INVALID_INDEX)
    {
      const int flags = fcntl(fd, F_GETFD, 0);
      if (flags != -1)
      {
        fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
      }
    }
#else
    (void) fd;
#endif // ifndef _WIN32
  }

#ifndef _WIN32
  /// Fallback for listening sockets we have no direct handle to (e.g. the
  /// vendored rpclib server's acceptor, hidden behind a PIMPL with no
  /// accessor). Walks this process's open file descriptors and marks
  /// close-on-exec any socket bound to @a Port, on any address family.
  /// Linux-only (relies on /proc/self/fd); intentionally not declared on
  /// Windows.
  inline void SetCloseOnExecForListeningPort(uint16_t Port)
  {
    DIR *FdDir = opendir("/proc/self/fd");
    if (FdDir == nullptr)
    {
      return;
    }
    struct dirent *Entry;
    while ((Entry = readdir(FdDir)) != nullptr)
    {
      char *EndPtr = nullptr;
      const long Fd = std::strtol(Entry->d_name, &EndPtr, 10);
      if (EndPtr == Entry->d_name || *EndPtr != '\0' || Fd < 0)
      {
        continue; // not a plain numeric fd entry (".", "..")
      }
      struct sockaddr_storage Addr;
      socklen_t AddrLen = sizeof(Addr);
      if (getsockname(static_cast<int>(Fd), reinterpret_cast<struct sockaddr *>(&Addr), &AddrLen) != 0)
      {
        continue; // not a socket, or we can't query it -- skip.
      }
      uint16_t BoundPort = 0;
      if (Addr.ss_family == AF_INET)
      {
        BoundPort = ntohs(reinterpret_cast<struct sockaddr_in *>(&Addr)->sin_port);
      }
      else if (Addr.ss_family == AF_INET6)
      {
        BoundPort = ntohs(reinterpret_cast<struct sockaddr_in6 *>(&Addr)->sin6_port);
      }
      if (BoundPort == Port)
      {
        SetSocketCloseOnExec(static_cast<int>(Fd));
      }
    }
    closedir(FdDir);
  }
#endif // ifndef _WIN32

} // namespace carla
