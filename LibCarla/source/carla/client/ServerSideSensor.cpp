// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ServerSideSensor.h"

#include "carla/Logging.h"
#include "carla/client/detail/Simulator.h"

#include <exception>

constexpr size_t GBufferTextureCount = 13;

namespace carla {
namespace client {

  ServerSideSensor::~ServerSideSensor() {
    if (IsAlive() && IsListening()) {
      log_warning(
          "sensor object went out of the scope but the sensor is still alive",
          "in the simulation:",
          GetDisplayId());
    }
    if (IsListening() && GetEpisode().IsValid()) {
      try {
        for (uint32_t i = 1; i != GBufferTextureCount + 1; ++i) {
          if (listening_mask.test(i))
            StopGBuffer(i - 1);
        }
        Stop();
      } catch (const std::exception &e) {
        log_error("exception trying to stop sensor:", GetDisplayId(), ':', e.what());
      }
    }
  }

  void ServerSideSensor::Listen(CallbackFunctionType callback) {
    log_debug("calling sensor Listen() ", GetDisplayId());
    log_debug(GetDisplayId(), ": subscribing to stream");
    GetEpisode().Lock()->SubscribeToSensor(*this, std::move(callback));
    listening_mask.set(0);
  }

  void ServerSideSensor::Stop() {
    log_debug("calling sensor Stop() ", GetDisplayId());
    if (!IsListening()) {
      log_warning(
          "attempting to unsubscribe from stream but sensor wasn't listening:",
          GetDisplayId());
      return;
    }
    GetEpisode().Lock()->UnSubscribeFromSensor(*this);
    listening_mask.reset(0);
  }

  void ServerSideSensor::ListenToGBuffer(uint32_t GBufferId, CallbackFunctionType callback) {
    log_debug(GetDisplayId(), ": subscribing to gbuffer stream");
    RELEASE_ASSERT(GBufferId < GBufferTextureCount);
    if (GetActorDescription().description.id != "sensor.camera.rgb")
    {
      log_warning("GBuffer methods are not supported on non-RGB sensors (sensor.camera.rgb).");
      return;
    }
    GetEpisode().Lock()->SubscribeToGBuffer(*this, GBufferId, std::move(callback));
    listening_mask.set(0);
    listening_mask.set(GBufferId + 1);
  }

  void ServerSideSensor::StopGBuffer(uint32_t GBufferId) {
    log_debug(GetDisplayId(), ": unsubscribing from gbuffer stream");
    RELEASE_ASSERT(GBufferId < GBufferTextureCount);
    if (GetActorDescription().description.id != "sensor.camera.rgb")
    {
      log_warning("GBuffer methods are not supported on non-RGB sensors (sensor.camera.rgb).");
      return;
    }
    GetEpisode().Lock()->UnSubscribeFromGBuffer(*this, GBufferId);
    listening_mask.reset(GBufferId + 1);
  }

  void ServerSideSensor::EnableForROS() {
    GetEpisode().Lock()->EnableForROS(*this);
  }

  void ServerSideSensor::DisableForROS() {
    GetEpisode().Lock()->DisableForROS(*this);
  }

  bool ServerSideSensor::IsEnabledForROS(){
    return GetEpisode().Lock()->IsEnabledForROS(*this);
  }

  bool ServerSideSensor::Destroy() {
    log_debug("calling sensor Destroy() ", GetDisplayId());
    if (IsListening()) {
      for (uint32_t i = 1; i != GBufferTextureCount + 1; ++i) {
        if (listening_mask.test(i)) {
          StopGBuffer(i - 1);
        }
      }
      Stop();
    }
    return Actor::Destroy();
  }

} // namespace client
} // namespace carla
