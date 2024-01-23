// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#if _WIN32
  #include <winsock2.h>   ///< socket
  #include <Ws2tcpip.h>
#else
  #include <sys/socket.h> ///< socket
  #include <netinet/in.h> ///< sockaddr_in
  #include <arpa/inet.h>  ///< getsockname
  #include <unistd.h>     ///< close
#endif

#define SOCK_INVALID_INDEX  -1