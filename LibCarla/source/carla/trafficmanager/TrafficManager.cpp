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

  // Check if the singleton instance has been already created.
  if (!singleton_pointer) {

    // Check if the traffic manager instance is already registered with server.
    if(episodeProxy.Lock()->IsTrafficManagerRunning(port)) {

      // Get traffic manager server info (remote IP & PORT).
      std::pair<std::string, uint16_t> serverTM = episodeProxy.Lock()->GetTrafficManagerRunning(port);

      // Set remote traffic manager server IP and port.
      TrafficManagerRemote* tm_ptr = new(std::nothrow) TrafficManagerRemote(serverTM, episodeProxy);

      // Try to connect to remote traffic manager server.
      try {

        // Check if the memory is allocated or not.
        if(tm_ptr != nullptr) {

          #if DEBUG_PRINT_TM
          // Test print
          std::cout 	<< "OLD@: Registered TM at "
                << serverTM.first  << ":"
                << serverTM.second << " ..... TRY "
                << std::endl;
          #endif

          // Check if the remote traffic manager is alive
          tm_ptr->HealthCheckRemoteTM();

          // Test print
          std::cout 	<< "OLD@: Registered TM at "
                << serverTM.first  << ":"
                << serverTM.second << " ..... SUCCESS "
                << std::endl;

          // Set the pointer of the instance.
          singleton_pointer = std::unique_ptr<TrafficManagerBase>(tm_ptr);
        }
      }

      // If connection error occurred.
      catch (...) {

        // Clear previously allocated memory
        delete tm_ptr;

        #if DEBUG_PRINT_TM
        // Test print
        std::cout 	<< "OLD@: Registered TM at "
              << serverTM.first  << ":"
              << serverTM.second << " ..... FAILED "
              << std::endl;
        #endif
      }
    }
  }


  // If traffic manager server is not running.
  if(!singleton_pointer) {

    // Set default port.
    uint16_t RPCportTM = port;

    // Define local constants.
    const std::vector<float> longitudinal_param = {2.0f, 0.05f, 0.07f};
    const std::vector<float> longitudinal_highway_param = {4.0f, 0.02f, 0.03f};
    const std::vector<float> lateral_param = {10.0f, 0.02f, 1.0f};
    const std::vector<float> lateral_highway_param = {9.0f, 0.02f, 1.0f};
    const float perc_difference_from_limit = 30.0f;

    std::pair<std::string, uint16_t> serverTM;

    // Create local instance of traffic manager.
    TrafficManagerLocal* tm_ptr = new TrafficManagerLocal(
      longitudinal_param,
      longitudinal_highway_param,
      lateral_param,
      lateral_highway_param,
      perc_difference_from_limit,
      episodeProxy,
      RPCportTM);

    // Get traffic manager server info (local IP & PORT).
    serverTM = GetLocalIP(RPCportTM);

    // Set this client as the traffic manager to server.
    episodeProxy.Lock()->AddTrafficManagerRunning(serverTM);

    #if DEBUG_PRINT_TM
    // Test print
    std::cout 	<< "NEW@: Registered TM at "
          << serverTM.first  << ":"
          << serverTM.second << " ..... SUCCESS."
          << std::endl;
    #endif

    // Set the pointer of the instance.
    singleton_pointer = std::unique_ptr<TrafficManagerBase>(tm_ptr);
  }
}

void TrafficManager::Release() {
  if(singleton_pointer) {
    TrafficManagerBase *base_ptr = singleton_pointer.release();
    delete base_ptr;
  }
}

} // namespace traffic_manager
} // namespace carla
