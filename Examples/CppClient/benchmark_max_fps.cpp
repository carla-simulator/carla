#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <chrono>
#include <cstdint>

#include <carla/client/Client.h>
#include <carla/client/Map.h>
#include <carla/client/TimeoutException.h>
#include <carla/client/World.h>

namespace cc = carla::client;
namespace cg = carla::geom;
namespace csd = carla::sensor::data;

using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std;
using namespace chrono;

using ms = duration<float, milli>;
using us = duration<float, micro>;

#define NUM_TIMESTEPS 1000
#define REPORT_EVERY 100




#define EXPECT_TRUE(pred) if (!(pred)) { throw std::runtime_error(#pred); }


static auto ParseArguments(int argc, const char *argv[]) {
  EXPECT_TRUE((argc == 1u) || (argc == 3u));
  using ResultType = std::tuple<std::string, uint16_t>;
  return argc == 3u ?
      ResultType{argv[1u], std::stoi(argv[2u])} :
      ResultType{"localhost", 2000u};
}

int main(int argc, const char *argv[]) {
  try {

    std::string host;
    uint16_t port;
    std::tie(host, port) = ParseArguments(argc, argv);

    std::mt19937_64 rng((std::random_device())());

    auto client = cc::Client(host, port);
    client.SetTimeout(1s);

    std::cout << "Client API version : " << client.GetClientVersion() << '\n';
    std::cout << "Server API version : " << client.GetServerVersion() << '\n';

    auto world = client.GetWorld();
    auto settings = world.GetSettings();
    settings.synchronous_mode = true;
    settings.no_rendering_mode = true;
    settings.fixed_delta_seconds = (1.0 / 100.0);
    world.ApplySettings(settings);

    int i=0;
    auto start = std::chrono::high_resolution_clock::now();

    while( i < NUM_TIMESTEPS)
    {
      i++;
      auto frame = world.Tick();

      if(i % REPORT_EVERY == 0)
      {
        auto end = std::chrono::high_resolution_clock::now();
        auto dur_usec = std::chrono::duration_cast<us>(end-start).count()/(float)REPORT_EVERY;
        if (dur_usec >= 1)
        {
          start = end;
          std::cout << "Frame "<<frame<<"  "<<dur_usec << "us  |  FPS: " << 1000000.0f/dur_usec << endl;
        }
      }
    }

    settings.synchronous_mode = false;
    settings.no_rendering_mode = false;
    world.ApplySettings(settings);


  } catch (const cc::TimeoutException &e) {
    std::cout << '\n' << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cout << "\nException: " << e.what() << std::endl;
    return 2;
  }
}
