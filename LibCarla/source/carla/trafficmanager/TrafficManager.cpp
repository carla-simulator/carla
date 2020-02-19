// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Client.h"
#include "carla/client/World.h"
#include "carla/trafficmanager/TrafficManager.h"
#include "carla/trafficmanager/TrafficManagerBase.h"
#include "carla/Exception.h"

#define DEBUG_PRINT_TM  0

namespace carla {
namespace traffic_manager {

// Unique pointer to hold the traffic manager instance.
std::unique_ptr<TrafficManagerBase> TrafficManager::singleton_pointer = nullptr;

// Explicit constructor for singleton life cycle management.
TrafficManager::TrafficManager(
    carla::client::detail::EpisodeProxy episodeProxy,
    uint16_t port) {

  /// Check singleton instance already created or not
  if (!singleton_pointer) {
    CreateTrafficManagerClient(episodeProxy, port);
  }

  /// As TM server not running
  if(!singleton_pointer) {
    CreateTrafficManagerServer(episodeProxy, port);
  }
}

void TrafficManager::Release() {
  if(singleton_pointer) {
    TrafficManagerBase *base_ptr = singleton_pointer.release();
    delete base_ptr;
  }
}

void TrafficManager::Reset() {
  if(singleton_pointer) {
    // Detect wich type of TM has been spawned before
    bool tm_server = singleton_pointer->IsServer();

    // Update episode information
    carla::client::detail::EpisodeProxy episodeProxy = singleton_pointer->GetEpisodeProxy();
    episodeProxy = episodeProxy.Lock()->GetCurrentEpisode();

    // Save port to restart TM again after releasing using the same port
    uint16_t port = singleton_pointer->port();

    // Release
    Release();

    // Create again the TM
    if(tm_server) {
      CreateTrafficManagerServer(episodeProxy, port);
    } else {
      int count = 0;
      while(count < MIN_TRY_COUNT) {
        std::this_thread::sleep_for(500ms);
        if(CreateTrafficManagerClient(episodeProxy, port)){
          break;
        }
        count++;
      }
      if(count >= MIN_TRY_COUNT){
        log_error("Traffic Manager client could not reconnect");
      }
    }
  }
}

void TrafficManager::CreateTrafficManagerServer(
    carla::client::detail::EpisodeProxy episodeProxy,
    uint16_t port) {

  // Get local IP details.
  auto GetLocalIP = [=](const uint16_t sport)-> std::pair<std::string, uint16_t> {
    std::pair<std::string, uint16_t> localIP;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == INVALID_INDEX) {
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
      if(err == INVALID_INDEX) {
        #if DEBUG_PRINT_TM
        std::cout << "Error number 2: " << errno << std::endl;
        std::cout << "Error message: " << strerror(errno) << std::endl;
        #endif
      } else {
        socklen_t addrlen = sizeof(loopback);
        err = getsockname(sock, reinterpret_cast<struct sockaddr*> (&loopback), &addrlen);
        if(err == INVALID_INDEX) {
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

  /// Set default port
  uint16_t RPCportTM = port;

  /// Define local constants
  const std::vector<float> longitudinal_param = {2.0f, 0.05f, 0.07f};
  const std::vector<float> longitudinal_highway_param = {4.0f, 0.02f, 0.03f};
  const std::vector<float> lateral_param = {10.0f, 0.02f, 1.0f};
  const std::vector<float> lateral_highway_param = {9.0f, 0.02f, 1.0f};
  const float perc_difference_from_limit = 30.0f;

  std::pair<std::string, uint16_t> serverTM;

  /// Create local instance of TM
  TrafficManagerLocal* tm_ptr = new TrafficManagerLocal(
    longitudinal_param,
    longitudinal_highway_param,
    lateral_param,
    lateral_highway_param,
    perc_difference_from_limit,
    episodeProxy,
    RPCportTM);

  /// Get TM server info (Local IP & PORT)
  serverTM = GetLocalIP(RPCportTM);

  /// Set this client as the TM to server
  episodeProxy.Lock()->AddTrafficManagerRunning(serverTM);

  #if DEBUG_PRINT_TM
  /// Print status
  std::cout << "NEW@: Registered TM at "
        << serverTM.first  << ":"
        << serverTM.second << " ..... SUCCESS."
        << std::endl;
  #endif

  /// Set the pointer of the instance
  singleton_pointer = std::unique_ptr<TrafficManagerBase>(tm_ptr);
}

bool TrafficManager::CreateTrafficManagerClient(
    carla::client::detail::EpisodeProxy episodeProxy,
    uint16_t port) {

  bool result = false;

  if(episodeProxy.Lock()->IsTrafficManagerRunning(port)) {

    /// Get TM server info (Remote IP & PORT)
    std::pair<std::string, uint16_t> serverTM =
      episodeProxy.Lock()->GetTrafficManagerRunning(port);

    carla::log_info("TrafficManager running at", serverTM.first,":",serverTM.second);

    /// Set remote TM server IP and port
    TrafficManagerRemote* tm_ptr = new(std::nothrow)
      TrafficManagerRemote(serverTM, episodeProxy);

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
        singleton_pointer = std::unique_ptr<TrafficManagerBase>(tm_ptr);

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
