// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rss/RssSensor.h"

#include "carla/Logging.h"
#include "carla/client/detail/Simulator.h"

#include "carla/client/ActorList.h"
#include "carla/rss/RssCheck.h"
#include "carla/sensor/data/RssResponse.h"

#include <ad_rss/core/RssResponseResolving.hpp>
#include <ad_rss/core/RssSituationChecking.hpp>
#include <ad_rss/core/RssSituationExtraction.hpp>

#include <exception>

namespace carla {
namespace client {

RssSensor::~RssSensor() = default;

void RssSensor::Listen(CallbackFunctionType callback) {
  if (_is_listening) {
    log_error(GetDisplayId(), ": already listening");
    return;
  }

  if (GetParent() == nullptr) {
    throw_exception(std::runtime_error(GetDisplayId() + ": not attached to vehicle"));
    return;
  }

  _map = GetWorld().GetMap();
  DEBUG_ASSERT(_map != nullptr);

  mRssCheck = std::make_shared<::carla::rss::RssCheck>();

  auto self = boost::static_pointer_cast<RssSensor>(shared_from_this());

  log_debug(GetDisplayId(), ": subscribing to tick event");
  GetEpisode().Lock()->RegisterOnTickEvent(
      [cb = std::move(callback), weak_self = WeakPtr<RssSensor>(self)](const auto &timestamp) {
        auto self = weak_self.lock();
        if (self != nullptr) {
          auto data = self->TickRssSensor(timestamp);
          if (data != nullptr) {
            cb(std::move(data));
          }
        }
      });

  _is_listening = true;
}

SharedPtr<sensor::SensorData> RssSensor::TickRssSensor(const Timestamp &timestamp) {
  try {
    bool result = false;
    ::ad_rss::state::ProperResponse response;
    if (_processing_lock.try_lock()) {
      carla::client::World world = GetWorld();
      SharedPtr<carla::client::ActorList> actors = world.GetActors();
      SharedPtr<carla::client::ActorList> vehicles = actors->Filter("vehicle.*");

      // check all object<->ego pairs with RSS and calculate proper response
      result = mRssCheck->checkObjects(timestamp, world, vehicles, GetParent(), _map, response, visualize_results);
      _processing_lock.unlock();
    }
    if (result) {
      carla::sensor::data::LongitudinalResponse longitudinalResponse;
      carla::sensor::data::LateralResponse lateralResponseRight;
      carla::sensor::data::LateralResponse lateralResponseLeft;

      switch (response.longitudinalResponse) {
        case ::ad_rss::state::LongitudinalResponse::None:
          longitudinalResponse = carla::sensor::data::LongitudinalResponse::None;
          break;
        case ::ad_rss::state::LongitudinalResponse::BrakeMinCorrect:
          longitudinalResponse = carla::sensor::data::LongitudinalResponse::BrakeMinCorrect;
          break;
        case ::ad_rss::state::LongitudinalResponse::BrakeMin:
          longitudinalResponse = carla::sensor::data::LongitudinalResponse::BrakeMin;
          break;
      }
      switch (response.lateralResponseRight) {
        case ::ad_rss::state::LateralResponse::None:
          lateralResponseRight = carla::sensor::data::LateralResponse::None;
          break;
        case ::ad_rss::state::LateralResponse::BrakeMin:
          lateralResponseRight = carla::sensor::data::LateralResponse::BrakeMin;
          break;
      }
      switch (response.lateralResponseLeft) {
        case ::ad_rss::state::LateralResponse::None:
          lateralResponseLeft = carla::sensor::data::LateralResponse::None;
          break;
        case ::ad_rss::state::LateralResponse::BrakeMin:
          lateralResponseLeft = carla::sensor::data::LateralResponse::BrakeMin;
          break;
      }
      return MakeShared<sensor::data::RssResponse>(timestamp.frame_count, timestamp.elapsed_seconds, GetTransform(),
                                                   longitudinalResponse, lateralResponseRight, lateralResponseLeft);
    } else {
      return nullptr;
    }
  } catch (const std::exception &e) {
    /// @todo We need to unsubscribe the sensor.
    std::cout << e.what() << std::endl;
    _processing_lock.unlock();
    return nullptr;
  }
}

void RssSensor::Stop() {
  /// @todo We need unsubscribe from the world on tick.
  _is_listening = false;
}

}  // namespace client
}  // namespace carla
