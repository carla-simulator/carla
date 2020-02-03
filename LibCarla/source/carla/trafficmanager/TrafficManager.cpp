// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Client.h>
#include "carla/trafficmanager/TrafficManager.h"
#include "carla/trafficmanager/TrafficManagerBase.h"

namespace carla {
namespace traffic_manager {

/// Unique pointer to hold the TM instance
std::unique_ptr<TrafficManagerBase> TrafficManager::singleton_pointer = nullptr;

/// Private constructor for singleton life cycle management.
TrafficManager :: TrafficManager(carla::client::detail::EpisodeProxy episodeProxy) {

	/// Set default
	int counter = INVALID_INDEX;

	/// Check singleton instance already created or not
	if (!singleton_pointer) {

		/// Repeatedly check for TM services
		 while(true) {

			/// Set default
			bool tm_rpc_server_running = true;

			// parent process: user-client wait for TM to start (if any)
			std::this_thread::sleep_for(0.5s);

			/// Check TM instance already registered with server or not
			if(episodeProxy.Lock()->IsTrafficManagerRunning()) {

				/// Get TM server info (Remote IP & PORT)
				std::pair<std::string, std::string> serverTM = episodeProxy.Lock()->GetTrafficManagerRunning();

				/// Set IP and port
				TrafficManagerRemote* tm_ptr = new(std::nothrow)
				TrafficManagerRemote(serverTM, episodeProxy);

				/// Try to connect to remote TM server
				try {

					/// Check memory allocated or not
					if(tm_ptr != nullptr) {

						/// Test print
						std::cout 	<< "OLD[" << counter + 1 <<"]: Registered TM at "
									<< serverTM.first  << ":"
									<< serverTM.second << " ..... TRY "
									<< std::endl;

						/// Try to reset all traffic lights
						tm_ptr->HealthCheckRemoteTM();

						/// Test print
						std::cout 	<< "OLD[" << counter + 1 <<"]: Registered TM at "
									<< serverTM.first  << ":"
									<< serverTM.second << " ..... SUCCESS "
									<< std::endl;

						/// Set the pointer of the instance
						singleton_pointer = std::unique_ptr<TrafficManagerBase>(tm_ptr);
					}
				}

				/// If Connection error occurred
				catch (...) {

					/// Clear previously allocated memory
					delete tm_ptr;

					/// Set flag to indicate TM server not running
					tm_rpc_server_running = false;

					/// Test print
					std::cout 	<< "OLD[" << counter + 1 <<"]: Registered TM at "
								<< serverTM.first  << ":"
								<< serverTM.second << " ..... FAILED "
								<< std::endl;
				}
			} else {

				/// Test print
				std::cout 	<< "OLD[" << counter + 1 <<"]: No Registered TM." << std::endl;

				/// Set flag to indicate TM server not running
				tm_rpc_server_running = false;
			}

			/// As TM server not running
			if(counter == INVALID_INDEX && !tm_rpc_server_running) {

				/// Get default port
				uint16_t RPCportTM = TM_SERVER_PORT;

				/// Get server details
				std::string carlaServerDetails(episodeProxy.Lock()->GetEndpoint());

				/// Get new child process
				pid_t pid = fork();

				/// Put Traffic manager in child process
				if (pid == 0) {

					/// Get server details
					std :: string srthost(carlaServerDetails.substr(0, carlaServerDetails.find(":")));
					std :: string srtport(carlaServerDetails.substr(carlaServerDetails.find(":") + 1));
					uint16_t      rpcPort(boost::lexical_cast<uint16_t>(std::atoi(srtport.c_str())));

					/// Define local constants
					const std::vector<float> longitudinal_param 		= {2.0f, 0.05f, 0.07f};
					const std::vector<float> longitudinal_highway_param = {4.0f, 0.02f, 0.03f};
					const std::vector<float> lateral_param 				= {10.0f, 0.02f, 1.0f};
					const std::vector<float> lateral_highway_param 		= {9.0f, 0.02f, 1.0f};
					const float perc_difference_from_limit 				= 30.0f;

					/// Create new client
					auto client = carla::client::Client(srthost, rpcPort);
					auto episodeProxyLocal = client.GetCurrentEpisode();

					/// Create local instance of TM
					TrafficManagerLocal* tm_ptr = new TrafficManagerLocal
							( longitudinal_param
							, longitudinal_highway_param
							, lateral_param
							, lateral_highway_param
							, perc_difference_from_limit
							, episodeProxyLocal);

					/// Create RPC TM server
					TrafficManagerServer server = TrafficManagerServer
							( RPCportTM
							, static_cast<carla::traffic_manager::TrafficManagerBase *>(tm_ptr));

					/// Get Local IP details
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
										localIP = std::make_pair<std::string, std::string>
										(buffer, std::to_string(RPCportTM));
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
					std::pair<std::string, std::string> serverTM = GetLocalIp();

					/// Set this client as the TM to server
					episodeProxyLocal.Lock()->SetTrafficManagerRunning(serverTM);

					/// Print status
					std::cout 	<< "NEW[0]: Registered TM at "
								<< serverTM.first  << ":"
								<< serverTM.second << " ..... SUCCESS"
								<< std::endl;

					/// Sleep for 5 seconds to wait to check any vehicle registered or not
					do {
						/// Wait for vehicle registration
						std::this_thread::sleep_for(5s);

						/// Get valid registered vehicle count
						auto vcount = tm_ptr->GetRegisteredActorsCount();
						std::cout << "Total register vehicles:: " << vcount << std::endl;

						/// If no valid vehicle present
						if(vcount == 0) break;
					} while (true);

					/// If no vehicle registered stop the RPC TM server
					if(tm_ptr) {
						delete tm_ptr;
					}
				}

				/// TM as separate process creation failed
				else if (pid < 0) {
					std::cout << "FORK: Registered TM ..... FAILED" << std::endl;

					/// Throw error to notify calling client
					throw "RPC_Registered_TM_Error";
				}
			}

			/// Try for limited number of times
			if(singleton_pointer || (++counter >= MIN_TRY_COUNT)) break;
		}
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
