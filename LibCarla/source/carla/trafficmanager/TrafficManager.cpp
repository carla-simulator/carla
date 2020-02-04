// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/client/Client.h>
#include "carla/trafficmanager/TrafficManager.h"
#include "carla/trafficmanager/TrafficManagerBase.h"

#define DEBUG_PRINT_TM		0

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

#if DEBUG_PRINT_TM
						/// Test print
						std::cout 	<< "OLD[" << counter + 1 <<"]: Registered TM at "
								<< serverTM.first  << ":"
								<< serverTM.second << " ..... TRY "
								<< std::endl;
#endif
						/// Try to reset all traffic lights
						tm_ptr->HealthCheckRemoteTM();

#if DEBUG_PRINT_TM
						/// Test print
						std::cout 	<< "OLD[" << counter + 1 <<"]: Registered TM at "
								<< serverTM.first  << ":"
								<< serverTM.second << " ..... SUCCESS "
								<< std::endl;
#endif
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

#if DEBUG_PRINT_TM
					/// Test print
					std::cout 	<< "OLD[" << counter + 1 <<"]: Registered TM at "
							<< serverTM.first  << ":"
							<< serverTM.second << " ..... FAILED "
							<< std::endl;
#endif
				}
			} else {

#if DEBUG_PRINT_TM
				/// Test print
				std::cout 	<< "OLD[" << counter + 1 <<"]: No Registered TM." << std::endl;
#endif
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
#if DEBUG_PRINT_TM
							std::cout << "Error number1: " << errno << std::endl;
							std::cout << "Error message: " << strerror(errno) << std::endl;
#endif
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
#if DEBUG_PRINT_TM
								std::cout << "Error number2: " << errno << std::endl;
								std::cout << "Error message: " << strerror(errno) << std::endl;
#endif
							} else {
								socklen_t addrlen = sizeof(loopback);
								err = getsockname
										( sock
												, reinterpret_cast<struct sockaddr*> (&loopback)
												, &addrlen);
								if(err == INVALID_INDEX) {
#if DEBUG_PRINT_TM
									std::cout << "Error number3: " << errno << std::endl;
									std::cout << "Error message: " << strerror(errno) << std::endl;
#endif
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
#if DEBUG_PRINT_TM
										std::cout << "Error number4: " << errno << std::endl;
										std::cout << "Error message: " << strerror(errno) << std::endl;
#endif
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

					/// Try to run TM as long as vehicles are present
					try {

						/// Filter to get all current episode vehicle information
						auto Filter = [&](auto &actors, auto &wildcard_pattern) {
							std::vector<carla::client::detail::ActorVariant> filtered;
							for (auto &&actor : actors) {
								if (carla::StringUtil::Match
										( carla::client::detail::ActorVariant(actor).GetTypeId()
										, wildcard_pattern)) {
									filtered.push_back(actor);
								}
							}
							return filtered;
						};

						/// Sleep for 5 seconds to wait to check any vehicle registered or not
						do {

							/// Temporary variables
							std::set<ActorId> worldVSet;
							bool noVehiclePresent = true;

							/// Wait for vehicle registration
							std::this_thread::sleep_for(5s);

							/// Get all actors of the world
							auto world_actorsList = episodeProxyLocal.Lock()->GetAllTheActorsInTheEpisode();

							/// Get all vehicles of the world
							auto world_vehicle = Filter(world_actorsList, "vehicle.*");
							std::cout << "Total WR register vehicles:: " << world_vehicle.size() << std::endl;
							for (auto actor: world_vehicle) {
								worldVSet.insert(actor.GetId());
							}

							/// Get all registered vehicles to TM
							const auto tmreg_vehicle = tm_ptr->GetRegisteredVehiclesIDs();
							std::cout << "Total TM register vehicles:: " << tmreg_vehicle.size() << std::endl;

							/// Check any registered vehicle present in the world
							for (auto &actor: tmreg_vehicle) {
								if(worldVSet.find(actor) != worldVSet.end()) {
									noVehiclePresent = false;
									break;
								}
							}

							/// If no valid vehicle present
							if(noVehiclePresent) break;

						/// Run for ever
						} while (true);

						/// Clear allocated TM memory
						if(tm_ptr) {
							delete tm_ptr;
						}
					} catch(...) {

						/// Print status
						std::cout 	<< "Errs: Registered TM at "
									<< serverTM.first  << ":"
									<< serverTM.second << " ..... CATCHED"
									<< std::endl;

					}

					/// If no vehicle registered stop the RPC TM server
					std::cout 	<< "EXIT: Registered TM at "
								<< serverTM.first  << ":"
								<< serverTM.second << " ..... STOPPED"
								<< std::endl;
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
