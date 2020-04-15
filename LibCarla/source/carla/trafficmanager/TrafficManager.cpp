// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

//#include "carla/client/Client.h"


#include "carla/Sockets.h"
#include "carla/trafficmanager/TrafficManager.h"
#include "carla/trafficmanager/TrafficManagerBase.h"
#include "carla/trafficmanager/TrafficManagerLocal.h"
#include "carla/trafficmanager/TrafficManagerRemote.h"

#define DEBUG_PRINT_TM  0
#define IP_DATA_BUFFER_SIZE     80

namespace carla {
namespace traffic_manager {

std::map<uint16_t, TrafficManagerBase*> TrafficManager::_tm_map;
std::mutex TrafficManager::_mutex;

TrafficManager::TrafficManager(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port)
  : _port(port) {

  if(!GetTM(_port)){
    // Check if a TM server already exists and connect to it
    if(!CreateTrafficManagerClient(episode_proxy, port)) {
      // As TM server not running, create one
      CreateTrafficManagerServer(episode_proxy, port);
    }
  }
}

void TrafficManager::Release() {
  std::lock_guard<std::mutex> lock(_mutex);
  for(auto& tm : _tm_map) {
    tm.second->Release();
    TrafficManagerBase *base_ptr = tm.second;
    delete base_ptr;
  }
  _tm_map.clear();
}

void TrafficManager::Reset() {
  std::lock_guard<std::mutex> lock(_mutex);
  for(auto& tm : _tm_map) {
    tm.second->Reset();
  }
}

void TrafficManager::Tick() {
  std::lock_guard<std::mutex> lock(_mutex);
  for(auto& tm : _tm_map) {
    tm.second->SynchronousTick();
  }
}

void TrafficManager::CreateTrafficManagerServer(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port) {

  // Get local IP details.
  auto GetLocalIP = [=](const uint16_t sport)-> std::pair<std::string, uint16_t> {
    std::pair<std::string, uint16_t> localIP;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == SOCK_INVALID_INDEX) {
      #if DEBUG_PRINT_TM
      std::cout << "Error number 1: " << errno << std::endl;
      std::cout << "Error message: " << strerror(errno) << std::endl;
      #endif
    } else {
      int err;
      sockaddr_in loopback;
      std::memset(&loopback, 0, sizeof(loopback));
      loopback.sin_family = AF_INET;
      loopback.sin_addr.s_addr = INADDR_LOOPBACK;
      loopback.sin_port = htons(9);
      err = connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback));
      if(err == SOCK_INVALID_INDEX) {
        #if DEBUG_PRINT_TM
        std::cout << "Error number 2: " << errno << std::endl;
        std::cout << "Error message: " << strerror(errno) << std::endl;
        #endif
      } else {
        socklen_t addrlen = sizeof(loopback);
        err = getsockname(sock, reinterpret_cast<struct sockaddr*> (&loopback), &addrlen);
        if(err == SOCK_INVALID_INDEX) {
          #if DEBUG_PRINT_TM
          std::cout << "Error number 3: " << errno << std::endl;
          std::cout << "Error message: " << strerror(errno) << std::endl;
          #endif
        } else {
          char buffer[IP_DATA_BUFFER_SIZE];
          const char* p = inet_ntop(AF_INET, &loopback.sin_addr, buffer, IP_DATA_BUFFER_SIZE);
          if(p != NULL) {
            localIP = std::pair<std::string, uint16_t>(std::string(buffer), sport);
          } else {
            #if DEBUG_PRINT_TM
            std::cout << "Error number 4: " << errno << std::endl;
            std::cout << "Error message: " << strerror(errno) << std::endl;
            #endif
          }
        }
      }
      #ifdef _WIN32
        closesocket(sock);
      #else
        close(sock);
      #endif
    }
    return localIP;
  };

  /// Define local constants
  const std::vector<float> longitudinal_param = {2.0f, 0.01f, 0.4f};
  const std::vector<float> longitudinal_highway_param = {4.0f, 0.02f, 0.2f};
  const std::vector<float> lateral_param = {9.0f, 0.02f, 1.0f};
  const std::vector<float> lateral_highway_param = {7.0f, 0.02f, 1.0f};
  const float perc_difference_from_limit = 30.0f;

  std::pair<std::string, uint16_t> serverTM;

  /// Create local instance of TM
  TrafficManagerLocal* tm_ptr = new TrafficManagerLocal(
    longitudinal_param,
    longitudinal_highway_param,
    lateral_param,
    lateral_highway_param,
    perc_difference_from_limit,
    episode_proxy,
    port);

  /// Get TM server info (Local IP & PORT)
  serverTM = GetLocalIP(port);

  /// Set this client as the TM to server
  episode_proxy.Lock()->AddTrafficManagerRunning(serverTM);

  #if DEBUG_PRINT_TM
  /// Print status
  std::cout << "NEW@: Registered TM at "
        << serverTM.first  << ":"
        << serverTM.second << " ..... SUCCESS."
        << std::endl;
  #endif

  /// Set the pointer of the instance
  _tm_map.insert(std::make_pair(port, tm_ptr));

}

bool TrafficManager::CreateTrafficManagerClient(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port) {

  bool result = false;

  if(episode_proxy.Lock()->IsTrafficManagerRunning(port)) {

    /// Get TM server info (Remote IP & PORT)
    std::pair<std::string, uint16_t> serverTM =
      episode_proxy.Lock()->GetTrafficManagerRunning(port);

    /// Set remote TM server IP and port
    TrafficManagerRemote* tm_ptr = new(std::nothrow)
      TrafficManagerRemote(serverTM, episode_proxy);

    /// Try to connect to remote TM server
    try {

      /// Check memory allocated or not
      if(tm_ptr != nullptr) {

        #if DEBUG_PRINT_TM
        // Test print
        std::cout << "OLD@: Registered TM at "
              << serverTM.first  << ":"
              << serverTM.second << " ..... TRY "
              << std::endl;
        #endif
        /// Try to reset all traffic lights
        tm_ptr->HealthCheckRemoteTM();

        /// Set the pointer of the instance
        _tm_map.insert(std::make_pair(port, tm_ptr));

        result = true;
      }
    }

    /// If Connection error occurred
    catch (...) {

      /// Clear previously allocated memory
      delete tm_ptr;

      #if DEBUG_PRINT_TM
      /// Test print
      std::cout 	<< "OLD@: Registered TM at "
            << serverTM.first  << ":"
            << serverTM.second << " ..... FAILED "
            << std::endl;
      #endif
    }

  }

  return result;
}

} // namespace traffic_manager
} // namespace carla
