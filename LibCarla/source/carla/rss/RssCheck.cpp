// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rss/RssCheck.h"

#include "carla/client/Map.h"
#include "carla/client/Vehicle.h"
#include "carla/client/Waypoint.h"
#include "carla/road/element/RoadInfoLaneWidth.h"

#include "ad_rss/core/RssResponseResolving.hpp"
#include "ad_rss/core/RssResponseTransformation.hpp"
#include "ad_rss/core/RssSituationChecking.hpp"
#include "ad_rss/core/RssSituationExtraction.hpp"
#include "ad_rss/state/RssStateOperation.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace carla {
namespace rss {

  namespace csd = carla::sensor::data;

// constants for deg-> rad conversion PI / 180
  constexpr float toRadians = static_cast<float>(M_PI) / 180.0f;

  inline float calculateAngleDelta(const float angle1, const float angle2) {
    float delta = angle1 - angle2;
    delta -= std::floor((delta + 180.f) / 360.f) * 360.f;
    return delta;
  }

  RssCheck::RssCheck() {
    _rssSituationExtraction = std::make_shared<::ad_rss::core::RssSituationExtraction>();
    _rssSituationChecking = std::make_shared<::ad_rss::core::RssSituationChecking>();
    _rssResponseResolving = std::make_shared<::ad_rss::core::RssResponseResolving>();

    _egoVehicleDynamics = std::make_shared<::ad_rss::world::RssDynamics>();
    _otherVehicleDynamics = std::make_shared<::ad_rss::world::RssDynamics>();

    ::ad_rss::world::RssDynamics defaultDynamics;

    defaultDynamics.alphaLon.accelMax = ::ad_rss::physics::Acceleration(3.5);
    defaultDynamics.alphaLon.brakeMax = ::ad_rss::physics::Acceleration(8.);
    defaultDynamics.alphaLon.brakeMin = ::ad_rss::physics::Acceleration(4.);
    defaultDynamics.alphaLon.brakeMinCorrect = ::ad_rss::physics::Acceleration(3);
    defaultDynamics.alphaLat.accelMax = ::ad_rss::physics::Acceleration(0.2);
    defaultDynamics.alphaLat.brakeMin = ::ad_rss::physics::Acceleration(0.8);
    defaultDynamics.responseTime = ::ad_rss::physics::Duration(1.);

    *_egoVehicleDynamics = defaultDynamics;
    *_otherVehicleDynamics = defaultDynamics;
  }

  RssCheck::~RssCheck() = default;

  const ::ad_rss::world::RssDynamics &RssCheck::getEgoVehicleDynamics() const {
    return *_egoVehicleDynamics;
  }

  void RssCheck::setEgoVehicleDynamics(const ::ad_rss::world::RssDynamics &dynamics) {
    *_egoVehicleDynamics = dynamics;
  }

  const ::ad_rss::world::RssDynamics &RssCheck::getOtherVehicleDynamics() const {
    return *_otherVehicleDynamics;
  }

  void RssCheck::setOtherVehicleDynamics(const ::ad_rss::world::RssDynamics &dynamics) {
    *_otherVehicleDynamics = dynamics;
  }

  bool RssCheck::checkObjects(cc::Timestamp const & timestamp, cc::World &world,
  carla::SharedPtr<cc::ActorList> const & vehicles,
  carla::SharedPtr<cc::Actor> const & carlaEgoActor,
  carla::SharedPtr<cc::Map> const & clientMap,
  ::ad_rss::state::ProperResponse &response,
  ::ad_rss::world::AccelerationRestriction &accelerationRestriction,
  ::ad_rss::world::Velocity &rssEgoVelocity, bool visualizeResults) {
    const auto carlaEgoVehicle = boost::dynamic_pointer_cast<cc::Vehicle>(carlaEgoActor);

    ::ad_rss::world::WorldModel worldModel;
    worldModel.timeIndex = timestamp.frame;

    ::ad_rss::world::Object egoVehicle;
    ::ad_rss::world::RssDynamics egoDynamics;
    initVehicle(egoVehicle);
    initEgoVehicleDynamics(egoDynamics);
    egoVehicle.objectId = carlaEgoVehicle->GetId();
    egoVehicle.objectType = ::ad_rss::world::ObjectType::EgoVehicle;

    cg::Location egoVehicleLocation = carlaEgoVehicle->GetLocation();
    auto egoClientWaypoint = clientMap->GetWaypoint(egoVehicleLocation, false);

    if (egoClientWaypoint != nullptr) {
      // ego vehicle is located on a lane marked for driving
      auto egoWaypointLoc = egoClientWaypoint->GetTransform();
      auto egoLaneId = egoClientWaypoint->GetLaneId();
      auto egoVelocity = carlaEgoVehicle->GetVelocity();
      auto egoVehicleTransform = carlaEgoVehicle->GetTransform();

      // calculate driving direction
      auto yawDiff = calculateAngleDelta(egoWaypointLoc.rotation.yaw, egoVehicleTransform.rotation.yaw);

      bool drivingInLaneDirection = true;
      if (std::abs(yawDiff) > 45.f) {
        drivingInLaneDirection = false;
      }

      // calculate road direction
      bool drivingInRoadDirection = (egoLaneId > 0) ^ drivingInLaneDirection;

      auto &carlaRoadMap = clientMap->GetMap();
      auto egoWaypoint = carlaRoadMap.GetWaypoint(egoVehicleLocation);
      auto &egoLane = carlaRoadMap.GetLane(*egoWaypoint);
      auto egoRoad = egoLane.GetRoad();

      ::ad_rss::world::RoadArea roadArea;
      ::ad_rss::world::RoadSegment roadSegment;

      // generate road area
      for (auto &laneSection : egoRoad->GetLaneSections()) {
        for (const auto &pair : laneSection.GetLanes()) {
          const auto &lane = pair.second;
          if ((static_cast<uint32_t>(lane.GetType()) &
          static_cast<uint32_t>(carla::road::Lane::LaneType::Driving)) > 0) {
            ::ad_rss::world::LaneSegment laneSegment;
            // assumption: only one segment per road

            auto laneLength = lane.GetLength();
            // evaluate width at lane start
            double pos = laneSection.GetDistance();
            const auto lane_width_info = lane.GetInfo<carla::road::element::RoadInfoLaneWidth>(pos);
            double laneWidth = 0.0;
            if (lane_width_info != nullptr) {
              laneWidth = lane_width_info->GetPolynomial().Evaluate(pos);
            }

            convertAndSetLaneSegmentId(lane, laneSegment);
            if ((lane.GetId() < 0) ^ drivingInRoadDirection) {
              laneSegment.drivingDirection = ::ad_rss::world::LaneDrivingDirection::Negative;
            } else {
              laneSegment.drivingDirection = ::ad_rss::world::LaneDrivingDirection::Positive;
            }

            // lane segment is assumed to be strait and evenly wide, so minimum
            // and maximum length and width are set to the same values
            laneSegment.length.minimum = ::ad_rss::physics::Distance(laneLength);
            laneSegment.length.maximum = laneSegment.length.minimum;
            laneSegment.width.minimum = ::ad_rss::physics::Distance(laneWidth);
            laneSegment.width.maximum = laneSegment.width.minimum;

            roadSegment.push_back(laneSegment);
          }
        }
        std::sort(
        roadSegment.begin(), roadSegment.end(),
        [&drivingInRoadDirection](::ad_rss::world::LaneSegment const &f,
        ::ad_rss::world::LaneSegment const &s) {
          return (f.id <= s.id) ^ drivingInRoadDirection;
        });
        roadArea.push_back(roadSegment);
      }

      calculateLatLonVelocities(egoWaypointLoc, egoVelocity, egoVehicle, !drivingInLaneDirection);
      rssEgoVelocity = egoVehicle.velocity;

      auto egoBounds = getVehicleBounds(*carlaEgoVehicle);

      calculateOccupiedRegions(egoBounds, egoVehicle.occupiedRegions, carlaRoadMap, drivingInRoadDirection,
      egoVehicleTransform);

      for (const auto &actor : *vehicles) {
        const auto vehicle = boost::dynamic_pointer_cast<cc::Vehicle>(actor);
        if (vehicle == nullptr) {
          continue;
        }
        if (vehicle->GetId() == carlaEgoVehicle->GetId()) {
          continue;
        }
        ::ad_rss::world::Object otherVehicle;
        ::ad_rss::world::RssDynamics otherDynamics;
        initVehicle(otherVehicle);
        initOtherVehicleDynamics(otherDynamics);
        otherVehicle.objectId = vehicle->GetId();
        otherVehicle.objectType = ::ad_rss::world::ObjectType::OtherVehicle;

        auto otherVehicleLoc = vehicle->GetLocation();
        auto otherVehicleClientWaypoint = clientMap->GetWaypoint(otherVehicleLoc);

        // same road
        if (egoClientWaypoint->GetRoadId() == otherVehicleClientWaypoint->GetRoadId()) {
          auto otherVehicleTransform = vehicle->GetTransform();
          auto otherBounds = getVehicleBounds(*vehicle);
          auto otherWaypointLoc = otherVehicleClientWaypoint->GetTransform();
          auto otherVelocity = vehicle->GetVelocity();
          calculateLatLonVelocities(otherWaypointLoc, otherVelocity, otherVehicle, false);

          ::ad_rss::world::Scene scene;

          calculateOccupiedRegions(otherBounds, otherVehicle.occupiedRegions, carlaRoadMap, drivingInRoadDirection,
          otherVehicleTransform);

          auto vehicleDirectionDiff =
          calculateAngleDelta(otherVehicleTransform.rotation.yaw, egoVehicleTransform.rotation.yaw);

          bool drivingInSameDirection = true;
          if (std::abs(vehicleDirectionDiff) > 90.f) {
            drivingInSameDirection = false;
          }

          if (drivingInSameDirection) {
            scene.situationType = ad_rss::situation::SituationType::SameDirection;
          } else {
            scene.situationType = ad_rss::situation::SituationType::OppositeDirection;
          }

          scene.object = otherVehicle;
          scene.objectRssDynamics = otherDynamics;
          scene.egoVehicle = egoVehicle;
          scene.egoVehicleRoad = roadArea;
          worldModel.scenes.push_back(scene);
        }
      }
      worldModel.egoVehicleRssDynamics = egoDynamics;

      ::ad_rss::situation::SituationSnapshot situationSnapshot;
      bool result = _rssSituationExtraction->extractSituations(worldModel, situationSnapshot);

      ::ad_rss::state::RssStateSnapshot stateSnapshot;
      if (result) {
        result = _rssSituationChecking->checkSituations(situationSnapshot, stateSnapshot);
      }
      if (result) {
        result = _rssResponseResolving->provideProperResponse(stateSnapshot, response);
      }
      if (result) {
        result = ::ad_rss::core::RssResponseTransformation::transformProperResponse(worldModel, response,
            accelerationRestriction);
      }

      if (result && visualizeResults) {
        visualizeRssResults(stateSnapshot, egoVehicleLocation, egoVehicleTransform, world);
      }
      return result;
    }
    return false;
  }

  void RssCheck::initVehicle(::ad_rss::world::Object &vehicle) const {
    vehicle.velocity.speedLon = ::ad_rss::physics::Speed(0.);
    vehicle.velocity.speedLat = ::ad_rss::physics::Speed(0.);
  }

  void RssCheck::initEgoVehicleDynamics(::ad_rss::world::RssDynamics &egoDynamics) const {
    egoDynamics = getEgoVehicleDynamics();
  }

  void RssCheck::initOtherVehicleDynamics(::ad_rss::world::RssDynamics &dynamics) const {
    dynamics = getOtherVehicleDynamics();
  }

  void RssCheck::calculateLatLonVelocities(const cg::Transform &laneLocation,
  const cg::Vector3D &velocity, ::ad_rss::world::Object &rssObject,
  bool inverseDirection) const {
    double roadDirSine = std::sin(laneLocation.rotation.yaw * toRadians);
    double roadDirCosine = std::cos(laneLocation.rotation.yaw * toRadians);
    double factor = 1.0;
    if (inverseDirection) {
      factor = -1.0;
    }
    rssObject.velocity.speedLon =
    ::ad_rss::physics::Speed(std::abs(roadDirCosine * velocity.x + roadDirSine * velocity.y));
    rssObject.velocity.speedLat =
    ::ad_rss::physics::Speed(factor * (-1.0 * roadDirSine * velocity.x + roadDirCosine * velocity.y));
  }

  void RssCheck::convertAndSetLaneSegmentId(const carla::road::Lane &lane,
  ::ad_rss::world::LaneSegment &laneSegment) const {
    uint64_t laneSegmentId = calculateLaneSegmentId(lane);
    laneSegment.id = laneSegmentId;
  }

// return front right, back right, back left, front left bounds
  std::array<cg::Location,
  4u> RssCheck::getVehicleBounds(const cc::Vehicle &vehicle) const {
    const auto &box = vehicle.GetBoundingBox();
    const auto &transform = vehicle.GetTransform();
    const auto location = transform.location + box.location;
    const auto yaw = transform.rotation.yaw * toRadians;
    const float cosine = std::cos(yaw);
    const float sine = std::sin(yaw);
    cg::Location frontExtent{cosine *box.extent.x, sine *box.extent.x, 0.0f};
    cg::Location rightExtent{-sine * box.extent.y, cosine *box.extent.y, 0.0f};
    return {location + frontExtent + rightExtent, location - frontExtent + rightExtent,
            location - frontExtent - rightExtent, location + frontExtent - rightExtent};
  }

  uint64_t RssCheck::calculateLaneSegmentId(const carla::road::Lane &lane) const {
    uint64_t laneSegmentId = lane.GetRoad()->GetId();
    laneSegmentId = laneSegmentId << 32;
    uint32_t sectionAndLane = lane.GetLaneSection()->GetId();
    sectionAndLane <<= 16;
    int16_t laneId = static_cast<int16_t>(lane.GetId());
    sectionAndLane |= (0xffff & (laneId + 256));
    laneSegmentId |= sectionAndLane;
    return laneSegmentId;
  }

  void RssCheck::calculateOccupiedRegions(const std::array<cg::Location, 4u> &bounds,
  ::ad_rss::world::OccupiedRegionVector &occupiedRegions,
  const carla::road::Map &carlaRoadMap, const bool drivingInRoadDirection,
  const cg::Transform &vehicleTransform) const {
    bool regionValid = true;
    for (const auto &bound : bounds) {
      auto boundWaypoint = carlaRoadMap.GetWaypoint(bound);

      if (!boundWaypoint.has_value()) {
        // vehicle not entirely on road --> stop evaluation
        regionValid = false;
        break;
      }
      auto &lane = carlaRoadMap.GetLane(*boundWaypoint);
      auto road = lane.GetRoad();
      uint64_t laneSegmentId = calculateLaneSegmentId(carlaRoadMap.GetLane(*boundWaypoint));

      auto length = road->GetLength();
      auto lonPos = boundWaypoint->s;
      if (!drivingInRoadDirection) {
        lonPos = length - lonPos;
      }
      double lon = std::max(0.0, std::min(lonPos / length, 1.0));

      auto waypointLocation = carlaRoadMap.ComputeTransform(*boundWaypoint);
      double headingSine = std::sin(waypointLocation.rotation.yaw * toRadians);
      double headingCosine = std::cos(waypointLocation.rotation.yaw * toRadians);

      auto yawDelta = calculateAngleDelta(waypointLocation.rotation.yaw, vehicleTransform.rotation.yaw);
      bool inLaneDirection = true;
      if (std::abs(yawDelta) > 45.f) {
        inLaneDirection = false;
      }

      auto width = carlaRoadMap.GetLaneWidth(*boundWaypoint);
      double latOffset = (headingSine * (waypointLocation.location.x - bound.x) -
      headingCosine * (waypointLocation.location.y - bound.y));
      if (!inLaneDirection) {
        latOffset *= -1.0;
      }

      double lat = std::max(0.0, std::min(0.5 + latOffset / width, 1.0));

      // find or create occupied region
      auto regionIt = std::find_if(
          occupiedRegions.begin(), occupiedRegions.end(),
          [laneSegmentId](const ::ad_rss::world::OccupiedRegion &region) {
        return region.segmentId == laneSegmentId;
      });
      if (regionIt == occupiedRegions.end()) {
        ::ad_rss::world::OccupiedRegion newRegion;
        newRegion.segmentId = laneSegmentId;
        newRegion.lonRange.minimum = ::ad_rss::physics::ParametricValue(lon);
        newRegion.lonRange.maximum = newRegion.lonRange.minimum;
        newRegion.latRange.minimum = ::ad_rss::physics::ParametricValue(lat);
        newRegion.latRange.maximum = newRegion.latRange.minimum;
        occupiedRegions.push_back(newRegion);
      } else {
        ::ad_rss::physics::ParametricValue const lonParam(lon);
        ::ad_rss::physics::ParametricValue const latParam(lat);

        regionIt->lonRange.minimum = std::min(regionIt->lonRange.minimum, lonParam);
        regionIt->lonRange.maximum = std::max(regionIt->lonRange.maximum, lonParam);
        regionIt->latRange.minimum = std::min(regionIt->latRange.minimum, latParam);
        regionIt->latRange.maximum = std::max(regionIt->latRange.maximum, latParam);
      }
    }
    // expand regions if more than one, ordered from right to left
    ::ad_rss::physics::ParametricValue lonMinParam(1.0);
    ::ad_rss::physics::ParametricValue lonMaxParam(0.0);

    for (auto &region : occupiedRegions) {
      lonMinParam = std::min(lonMinParam, region.lonRange.minimum);
      lonMaxParam = std::max(lonMaxParam, region.lonRange.maximum);
      if (region != occupiedRegions.front()) {
        // not the most right one, so extend lat maximum
        region.latRange.maximum = ::ad_rss::physics::ParametricValue(1.0);
      }
      if (region != occupiedRegions.back()) {
        // not the most left one, so extend lat minimum
        region.latRange.minimum = ::ad_rss::physics::ParametricValue(0.0);
      }
    }
    for (auto &region : occupiedRegions) {
      region.lonRange.minimum = lonMinParam;
      region.lonRange.maximum = lonMaxParam;
    }
  }

  void RssCheck::visualizeRssResults(::ad_rss::state::RssStateSnapshot stateSnapshot,
  const cg::Location &egoVehicleLocation,
  const cg::Transform &egoVehicleTransform,
  cc::World &world) const {
    cc::DebugHelper dh = world.MakeDebugHelper();
    for (std::size_t i = 0; i < stateSnapshot.individualResponses.size(); ++i) {
      ::ad_rss::state::RssState &state = stateSnapshot.individualResponses[i];
      carla::rpc::ActorId vehicleId = static_cast<carla::rpc::ActorId>(state.objectId);
      carla::SharedPtr<cc::ActorList> vehicleList =
      world.GetActors(std::vector<carla::rpc::ActorId>{vehicleId});
      cg::Location egoPoint = egoVehicleLocation;
      egoPoint.z += 0.05f;
      const auto yaw = egoVehicleTransform.rotation.yaw;
      const float cosine = std::cos(yaw * toRadians);
      const float sine = std::sin(yaw * toRadians);
      cg::Location lineOffset{-sine * 0.1f, cosine * 0.1f, 0.0f};
      for (const auto &actor : *vehicleList) {
        const auto vehicle = boost::dynamic_pointer_cast<cc::Vehicle>(actor);
        cg::Location point = vehicle->GetLocation();
        point.z += 0.05f;
        csd::Color lonColor{0u, 255u, 0u};
        csd::Color latLColor = lonColor;
        csd::Color latRColor = lonColor;
        csd::Color indicatorColor = lonColor;
        bool dangerous = ::ad_rss::state::isDangerous(state);
        if (dangerous) {
          indicatorColor = csd::Color{255u, 0u, 0u};
        }
        if (!state.longitudinalState.isSafe) {
          lonColor.r = 255u;
          if (dangerous) {
            lonColor.g = 0u;
          } else {
            lonColor.g = 255u;
          }
        }
        if (!state.lateralStateLeft.isSafe) {
          latLColor.r = 255u;
          if (dangerous) {
            latLColor.g = 0u;
          } else {
            latLColor.g = 255u;
          }
        }
        if (!state.lateralStateRight.isSafe) {
          latRColor.r = 255u;
          if (dangerous) {
            latRColor.g = 0u;
          } else {
            latRColor.g = 255u;
          }
        }
        dh.DrawLine(egoPoint, point, 0.1f, lonColor, 0.02f, false);
        dh.DrawLine(egoPoint - lineOffset, point - lineOffset, 0.1f, latLColor, 0.02f, false);
        dh.DrawLine(egoPoint + lineOffset, point + lineOffset, 0.1f, latRColor, 0.02f, false);
        point.z += 3.f;
        dh.DrawPoint(point, 0.2f, indicatorColor, 0.02f, false);
      }
    }
  }

}  // namespace rss
}  // namespace carla
