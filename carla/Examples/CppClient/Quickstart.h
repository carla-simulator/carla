#pragma once

#include <cstdint>
#include <string>
#include <tuple>

namespace carla::example {

  std::tuple<std::string, uint16_t> ParseEndpoint(int argc, const char *argv[]);

  void PrintClientAndServerVersions(const std::string &client_version, const std::string &server_version);

} // namespace carla::example
