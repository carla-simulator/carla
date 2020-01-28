// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/TrafficManager.h"
#include "carla/trafficmanager/TrafficManagerBase.h"

namespace carla {
namespace traffic_manager {

std::unique_ptr<TrafficManagerBase> TrafficManager::singleton_pointer = nullptr;

/// Private constructor for singleton life cycle management.
TrafficManager :: TrafficManager(carla::client::detail::EpisodeProxy episodeProxy) {

	/// Check singleton instance already created or not
	if (!singleton_pointer) {

		/// Check TM instance already registered with server or not
		if(episodeProxy.Lock()->IsTrafficManagerRunning()) {

			/// Get TM server information
			std::pair<std::string, std::string> serverTM = episodeProxy.Lock()->GetTrafficManagerRunning();

			/// Set IP and port
			TrafficManagerRemote* tm_ptr = new TrafficManagerRemote(serverTM, episodeProxy);

			/// Set the pointer of the instance
			singleton_pointer = std::unique_ptr<TrafficManagerBase>(tm_ptr);
		}

		/// Get Local TM details
		else {
			const std::vector<float> longitudinal_param 		= {2.0f, 0.05f, 0.07f};
			const std::vector<float> longitudinal_highway_param = {4.0f, 0.02f, 0.03f};
			const std::vector<float> lateral_param 				= {10.0f, 0.02f, 1.0f};
			const std::vector<float> lateral_highway_param 		= {9.0f, 0.02f, 1.0f};
			const float perc_difference_from_limit 				= 30.0f;

			TrafficManagerLocal* tm_ptr = new TrafficManagerLocal
					( longitudinal_param
							, longitudinal_highway_param
							, lateral_param
							, lateral_highway_param
							, perc_difference_from_limit
							, episodeProxy);


			auto GetLocalIp = [=]()-> std::pair<std::string, std::string>
			{
				int err;
				std::pair<std::string, std::string> localIP;
				int sock = socket(AF_INET, SOCK_DGRAM, 0);
				if(sock == INVALID_INDEX) {
					std::cout << "Error number1: " << errno << std::endl;
					std::cout << "Error message: " << strerror(errno) << std::endl;
				} else {
					sockaddr_in loopback;
					std::memset(&loopback, 0, sizeof(loopback));
					loopback.sin_family = AF_INET;
					loopback.sin_addr.s_addr = INADDR_LOOPBACK;   // using loopback ip address
					loopback.sin_port = htons(9);                 // using debug port
					err = connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback));
					if(err == INVALID_INDEX) {
						std::cout << "Error number2: " << errno << std::endl;
						std::cout << "Error message: " << strerror(errno) << std::endl;
					} else {
						socklen_t addrlen = sizeof(loopback);
						err = getsockname(sock, reinterpret_cast<struct sockaddr*> (&loopback), &addrlen);
						if(err == INVALID_INDEX) {
							std::cout << "Error number3: " << errno << std::endl;
							std::cout << "Error message: " << strerror(errno) << std::endl;
						} else {
							char buffer[IP_DATA_BUFFER_SIZE];
							const char* p = inet_ntop(AF_INET, &loopback.sin_addr, buffer, IP_DATA_BUFFER_SIZE);
							if(p != NULL) {
								localIP = std::make_pair<std::string, std::string>(buffer, std::to_string(TM_SERVER_PORT));
								std::cout << "Local IP: " << localIP.first << " PORT: " << localIP.second << std::endl;
							} else {
								std::cout << "Error number4: " << errno << std::endl;
								std::cout << "Error message: " << strerror(errno) << std::endl;
							}
						}
					}
					close(sock);
				}
				return localIP;
			};

			/// Set this client as the TM to server
			episodeProxy.Lock()->SetTrafficManagerRunning(GetLocalIp());

			/// Set the pointer of the instance
			singleton_pointer = std::unique_ptr<TrafficManagerBase>(tm_ptr);
		}
	}
}

} // namespace traffic_manager
} // namespace carla
