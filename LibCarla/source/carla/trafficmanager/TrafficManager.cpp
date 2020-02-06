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

#define DEBUG_PRINT_TM		0

namespace carla {
namespace traffic_manager {

/// Unique pointer to hold the TM instance
std::unique_ptr<TrafficManagerBase> TrafficManager::singleton_pointer = nullptr;

/// Private constructor for singleton life cycle management.
TrafficManager::TrafficManager(
  carla::client::detail::EpisodeProxy episodeProxy,
  uint16_t port) {

  std::pair<std::string, uint16_t> serverTM;
  std::string port_str = std::to_string(port);

  /// Check singleton instance already created or not
  if (!singleton_pointer) {

    /// Check TM instance already registered with server or not
    if(episodeProxy.Lock()->IsTrafficManagerRunning(port)) {

      /// Get TM server info (Remote IP & PORT)
      serverTM = episodeProxy.Lock()->GetTrafficManagerRunning(port);

      /// Set IP and port
      TrafficManagerRemote* tm_ptr = new(std::nothrow)
        TrafficManagerRemote(serverTM, episodeProxy);

      try {

        /// Check memory allocated or not
        if(tm_ptr != nullptr) {

          tm_ptr->HealthCheckRemoteTM();

          /// Set the pointer of the instance
          singleton_pointer = std::unique_ptr<TrafficManagerBase>(tm_ptr);
        }
      }
      catch (...) {

        /// Clear previously allocated memory
        delete tm_ptr;
        std::cout 	<< "OLD: Registered TM at "
              << serverTM.first  << ":"
              << serverTM.second << " ..... FAILED "
              << std::endl;
      }
    }
  }

  /// If pointer is not allocated get Local TM details
  if (!singleton_pointer) {

    const std::vector<float> longitudinal_param   = {2.0f, 0.05f, 0.07f};
    const std::vector<float> longitudinal_highway_param = {4.0f, 0.02f, 0.03f};
    const std::vector<float> lateral_param        = {10.0f, 0.02f, 1.0f};
    const std::vector<float> lateral_highway_param    = {9.0f, 0.02f, 1.0f};
    const float perc_difference_from_limit        = 30.0f;

    carla::log_info("TrafficManager", port);

    TrafficManagerLocal* tm_ptr = new TrafficManagerLocal
        ( longitudinal_param
            , longitudinal_highway_param
            , lateral_param
            , lateral_highway_param
            , perc_difference_from_limit
            , episodeProxy
            , port);

    auto GetLocalIp = [=]()-> std::pair<std::string, uint16_t>
    {
      int err;
      std::pair<std::string, uint16_t> localIP;
      int sock = socket(AF_INET, SOCK_DGRAM, 0);
      if(sock == INVALID_INDEX) {
        std::cout << "Error number1: " << errno << std::endl;
        std::cout << "Error message: " << strerror(errno) << std::endl;
      } else {
        sockaddr_in loopback;
        std::memset(&loopback, 0, sizeof(loopback));
        loopback.sin_family = AF_INET;
        loopback.sin_addr.s_addr = INADDR_LOOPBACK;
        loopback.sin_port = htons(9);
        err = connect
            ( sock
            , reinterpret_cast<sockaddr*>(&loopback)
            , sizeof(loopback));
        if(err == INVALID_INDEX) {
          std::cout 	<< "Error number2: "
                << errno << std::endl;
          std::cout 	<< "Error message: "
                << strerror(errno) << std::endl;
        } else {
          socklen_t addrlen = sizeof(loopback);
          err = getsockname
              ( sock
              , reinterpret_cast<struct sockaddr*> (&loopback)
              , &addrlen);
          if(err == INVALID_INDEX) {
            std::cout 	<< "Error number3: "
                  << errno << std::endl;
            std::cout 	<< "Error message: "
                  << strerror(errno) << std::endl;
          } else {
            char buffer[IP_DATA_BUFFER_SIZE];
            const char* p = inet_ntop
                      ( AF_INET
                      , &loopback.sin_addr, buffer
                      , IP_DATA_BUFFER_SIZE);
            if(p != NULL) {
              localIP = std::make_pair(buffer, port);
            } else {
              std::cout 	<< "Error number4: "
                    << errno << std::endl;
              std::cout 	<< "Error message: "
                    << strerror(errno) << std::endl;
            }
          }
        }
        close(sock);
      }
      return localIP;
    };

    /// Get TM server info (Local IP & PORT)
    serverTM = GetLocalIp();

    /// Set this client as the TM to server
    episodeProxy.Lock()->AddTrafficManagerRunning(serverTM);

    /// Print status
    std::cout 	<< "NEW: Registered TM at "
          << serverTM.first  << ":"
          << serverTM.second << " ..... SUCCESS"
          << std::endl;

    /// Set the pointer of the instance
    singleton_pointer = std::unique_ptr<TrafficManagerBase>(tm_ptr);
  }
}

void TrafficManager::Release() {
  carla::log_info("TrafficManager::Release");
  if(singleton_pointer) {
    carla::log_info("TrafficManager::Release if");
    TrafficManagerBase *base_ptr = singleton_pointer.release();
    carla::log_info("TrafficManager::Release delete");
    delete base_ptr;
  }
}

void TrafficManager::Restart() {
  carla::log_info("TrafficManager::Restart");
  if(singleton_pointer) {
    carla::log_info("TrafficManager::Restart IsRunning", singleton_pointer->IsRunning()?"True":"False");
    if(singleton_pointer->IsRunning()){
      singleton_pointer->Stop();
    } else {
      singleton_pointer->Start();
    }
  }
}


} // namespace traffic_manager
} // namespace carla
