#include "Quickstart.h"

#include <iostream>
#include <stdexcept>

namespace carla::example {

  std::tuple<std::string, uint16_t> ParseEndpoint(int argc, const char *argv[]) {
    if (!((argc == 1) || (argc == 3))) {
      throw std::runtime_error("Usage: carla-example-client [host port]");
    }

    if (argc == 1) {
      return std::make_tuple(std::string{"localhost"}, static_cast<uint16_t>(2000u));
    }

    return std::make_tuple(std::string{argv[1]}, static_cast<uint16_t>(std::stoi(argv[2])));
  }

  void PrintClientAndServerVersions(const std::string &client_version, const std::string &server_version) {
    std::cout << "Client API version : " << client_version << '\n';
    std::cout << "Server API version : " << server_version << '\n';
  }

} // namespace carla::example
