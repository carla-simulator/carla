// Connects to a CARLA server and reports what it serves: usage is my-carla-client [host] [port].
#include <carla/client/ActorList.h>
#include <carla/client/Client.h>
#include <carla/client/Map.h>
#include <carla/client/World.h>

#include <chrono>
#include <iostream>
#include <string>

int main(int argc, const char *argv[]) try {
  const std::string host = argc > 1 ? argv[1] : "localhost";
  const uint16_t port = argc > 2 ? static_cast<uint16_t>(std::stoi(argv[2])) : 2000u;

  carla::client::Client client(host, port);
  client.SetTimeout(std::chrono::seconds(20));
  std::cout << "client " << client.GetClientVersion() << ", server " << client.GetServerVersion() << '\n';

  auto world = client.GetWorld();
  auto map = world.GetMap();
  std::cout << "map " << map->GetName() << ", " << map->GetRecommendedSpawnPoints().size() << " spawn points, "
            << world.GetActors()->size() << " actors\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << "my-carla-client: " << e.what() << '\n';
  return 1;
}
