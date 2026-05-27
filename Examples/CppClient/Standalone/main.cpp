// Minimal demonstration of consuming LibCarla's C++ client API from a project
// that lives outside CARLA's source tree. Closes carla-simulator/carla#6091.
//
// Build: see README.md in this directory.

#include <carla/client/Client.h>
#include <chrono>
#include <iostream>
#include <stdexcept>

int main() try {
  carla::client::Client client("localhost", 2000);
  client.SetTimeout(std::chrono::seconds(10));

  std::cout << "Connected to CARLA " << client.GetServerVersion() << "\n";
  std::cout << "Available maps:";
  for (const auto &m : client.GetAvailableMaps()) {
    std::cout << "\n  " << m;
  }
  std::cout << '\n';
  return 0;
} catch (const std::exception &e) {
  std::cerr << "carla-cpp-standalone: " << e.what() << '\n';
  return 1;
}
