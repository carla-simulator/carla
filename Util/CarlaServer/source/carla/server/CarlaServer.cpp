// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Debug.h"
#include "carla/Logging.h"
#include "carla/server/AgentServer.h"
#include "carla/server/CarlaServer.h"

using namespace carla;
using namespace carla::server;

// =============================================================================
// -- Static local functions ---------------------------------------------------
// =============================================================================

static inline carla::server::CarlaServer *Cast(CarlaServerPtr self) {
  return static_cast<carla::server::CarlaServer*>(self);
}

// =============================================================================
// -- Implementation of the C-interface of CarlaServer -------------------------
// =============================================================================

const int32_t CARLA_SERVER_SUCCESS = errc::success().value();;
const int32_t CARLA_SERVER_TRY_AGAIN = errc::try_again().value();
const int32_t CARLA_SERVER_TIMED_OUT = errc::timed_out().value();
const int32_t CARLA_SERVER_OPERATION_ABORTED = errc::operation_aborted().value();

CarlaServerPtr carla_make_server() {
  return new carla::server::CarlaServer;
}

void carla_free_server(CarlaServerPtr self) {
  delete Cast(self);
}

int32_t carla_server_connect(
    CarlaServerPtr self,
    const uint32_t world_port,
    const uint32_t timeout) {
  Cast(self)->Connect(world_port, timeout_t::milliseconds(timeout));
  return CARLA_SERVER_SUCCESS;
}

int32_t carla_server_connect_blocking(
    CarlaServerPtr self,
    const uint32_t world_port,
    const uint32_t timeout) {
  auto result = Cast(self)->Connect(world_port, timeout_t::milliseconds(timeout));
  return result.get().value();
}

void carla_disconnect_server(CarlaServerPtr self) {
  Cast(self)->Disconnect();
}

int32_t carla_read_request_new_episode(
      CarlaServerPtr self,
      carla_request_new_episode &values,
      const uint32_t timeout) {
  CARLA_PROFILE_SCOPE(C_API, RequestNewEpisode)
  auto ec = Cast(self)->TryRead(values, timeout_t::milliseconds(timeout));
  if (!ec) {
    log_debug("received valid request new episode");
    Cast(self)->KillAgentServer();
  }
  return ec.value();
}

int32_t carla_write_scene_description(
      CarlaServerPtr self,
      const carla_scene_description &values,
      const uint32_t timeout) {
  auto result = Cast(self)->Write(values);
  error_code ec = errc::timed_out();
  future::wait_and_get(result, ec, timeout_t::milliseconds(timeout));
  return ec.value();
}

int32_t carla_read_episode_start(
      CarlaServerPtr self,
      carla_episode_start &values,
      const uint32_t timeout) {
  return Cast(self)->TryRead(values, timeout_t::milliseconds(timeout)).value();
}

int32_t carla_write_episode_ready(
      CarlaServerPtr self,
      const carla_episode_ready &values,
      const uint32_t timeout) {
  if (values.ready) {
    Cast(self)->StartAgentServer();
  } else {
    log_error("start agent server cancelled: episode_ready = false");
  }
  error_code ec = errc::timed_out();
  auto result = Cast(self)->Write(values);
  future::wait_and_get(result, ec, timeout_t::milliseconds(timeout));
  Cast(self)->ResetProtocol();
  return ec.value();
}

int32_t carla_read_control(
      CarlaServerPtr self,
      carla_control &values,
      const uint32_t timeout) {
  CARLA_PROFILE_SCOPE(C_API, ReadControl);
  auto agent = Cast(self)->GetAgentServer();
  if (agent == nullptr) {
    log_debug("trying to read control but agent server is missing");
    return CARLA_SERVER_OPERATION_ABORTED;
  }
  return agent->ReadControl(values, timeout_t::milliseconds(timeout)).value();
}

int32_t carla_write_sensor_data(
    CarlaServerPtr self,
    const carla_sensor_data &sensor_data) {
  CARLA_PROFILE_SCOPE(C_API, WriteSensorData);
  auto agent = Cast(self)->GetAgentServer();
  if (agent == nullptr) {
    log_debug("trying to write sensor data but agent server is missing");
    return CARLA_SERVER_OPERATION_ABORTED;
  } else {
    return agent->WriteSensorData(sensor_data).value();
  }
}

int32_t carla_write_measurements(
    CarlaServerPtr self,
    const carla_measurements &measurements) {
  CARLA_PROFILE_FPS(FPS, SendMeasurements);
  CARLA_PROFILE_SCOPE(C_API, WriteMeasurements);
  auto agent = Cast(self)->GetAgentServer();
  if (agent == nullptr) {
    log_debug("trying to write measurements but agent server is missing");
    return CARLA_SERVER_OPERATION_ABORTED;
  } else {
    return agent->WriteMeasurements(measurements).value();
  }
}
