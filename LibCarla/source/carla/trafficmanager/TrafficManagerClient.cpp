// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/TrafficManagerClient.h"
#include "carla/trafficmanager/TrafficManager.h"

void TrafficManagerClient::Stop() {
  carla::log_info("TrafficManagerClient::Stop");
  carla::traffic_manager::TrafficManager::Release();
}
