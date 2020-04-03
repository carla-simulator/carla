// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Lane.h"

#include <limits>

#include "carla/Debug.h"
#include "carla/road/element/Geometry.h"
#include "carla/road/element/RoadInfoElevation.h"
#include "carla/road/element/RoadInfoGeometry.h"
#include "carla/road/element/RoadInfoLaneOffset.h"
#include "carla/road/element/RoadInfoLaneWidth.h"
#include "carla/road/LaneSection.h"
#include "carla/road/MapData.h"
#include "carla/road/Road.h"

namespace carla {
namespace road {

  const LaneSection *Lane::GetLaneSection() const {
    return _lane_section;
  }

  Road *Lane::GetRoad() const {
    DEBUG_ASSERT(_lane_section != nullptr);
    return _lane_section->GetRoad();
  }

  LaneId Lane::GetId() const {
    return _id;
  }

  Lane::LaneType Lane::GetType() const {
    return _type;
  }

  bool Lane::GetLevel() const {
    return _level;
  }

  double Lane::GetDistance() const {
    DEBUG_ASSERT(_lane_section != nullptr);
    return _lane_section->GetDistance();
  }

  double Lane::GetLength() const {
    const auto *road = GetRoad();
    DEBUG_ASSERT(road != nullptr);
    const auto s = GetDistance();
    return road->UpperBound(s) - s;
  }

  double Lane::GetWidth(const double s) const {
    RELEASE_ASSERT(s <= GetRoad()->GetLength());
    const auto width_info = GetInfo<element::RoadInfoLaneWidth>(s);
    RELEASE_ASSERT(width_info != nullptr);
    return width_info->GetPolynomial().Evaluate(s);
  }

  bool Lane::IsStraight() const {
    Road *road = GetRoad();
    RELEASE_ASSERT(road != nullptr);
    auto *geometry = road->GetInfo<element::RoadInfoGeometry>(GetDistance());
    DEBUG_ASSERT(geometry != nullptr);
    auto geometry_type = geometry->GetGeometry().GetType();
    if (geometry_type != element::GeometryType::LINE) {
      return false;
    }
    if(GetDistance() < geometry->GetDistance() ||
        GetDistance() + GetLength() >
        geometry->GetDistance() + geometry->GetGeometry().GetLength()) {
      return false;
    }
    auto lane_offsets = GetInfos<element::RoadInfoLaneOffset>();
    for (auto *lane_offset : lane_offsets) {
      if (std::abs(lane_offset->GetPolynomial().GetC()) > 0 ||
          std::abs(lane_offset->GetPolynomial().GetD()) > 0) {
        return false;
      }
    }
    auto elevations = road->GetInfos<element::RoadInfoElevation>();
    for (auto *elevation : elevations) {
      if (std::abs(elevation->GetPolynomial().GetC()) > 0 ||
          std::abs(elevation->GetPolynomial().GetD()) > 0) {
        return false;
      }
    }
    return true;
  }

  /// Returns a pair containing first = width, second = tangent,
  /// for an specific Lane given an s and a iterator over lanes
  template <typename T>
  static std::pair<double, double> ComputeTotalLaneWidth(
      const T container,
      const double s,
      const LaneId lane_id) {

    // lane_id can't be 0
    RELEASE_ASSERT(lane_id != 0);

    const bool negative_lane_id = lane_id < 0;
    double dist = 0.0;
    double tangent = 0.0;
    for (const auto &lane : container) {
      auto info = lane.second.template GetInfo<element::RoadInfoLaneWidth>(s);
      RELEASE_ASSERT(info != nullptr);
      const auto current_polynomial = info->GetPolynomial();
      auto current_dist = current_polynomial.Evaluate(s);
      auto current_tang = current_polynomial.Tangent(s);
      if (lane.first != lane_id) {
        dist += negative_lane_id ? current_dist : -current_dist;
        tangent += current_tang;
      } else if (lane.first == lane_id) {
        current_dist *= 0.5;
        dist += negative_lane_id ? current_dist : -current_dist;
        tangent += current_tang * 0.5;
        break;
      }
    }
    return std::make_pair(dist, tangent);
  }

  geom::Transform Lane::ComputeTransform(const double s) const {
    const Road *road = GetRoad();
    DEBUG_ASSERT(road != nullptr);

    // must s be smaller (or eq) than road lenght and bigger (or eq) than 0?
    RELEASE_ASSERT(s <= road->GetLength());
    RELEASE_ASSERT(s >= 0.0);

    const auto *lane_section = GetLaneSection();
    DEBUG_ASSERT(lane_section != nullptr);
    const std::map<LaneId, Lane> &lanes = lane_section->GetLanes();

    // check that lane_id exists on the current s
    RELEASE_ASSERT(!lanes.empty());
    RELEASE_ASSERT(GetId() >= lanes.begin()->first);
    RELEASE_ASSERT(GetId() <= lanes.rbegin()->first);

    float lane_width = 0.0f;
    float lane_tangent = 0.0f;

    if (GetId() < 0) {
      // right lane
      const auto side_lanes = MakeListView(
          std::make_reverse_iterator(lanes.lower_bound(0)), lanes.rend());
      const auto computed_width =
          ComputeTotalLaneWidth(side_lanes, s, GetId());
      lane_width = static_cast<float>(computed_width.first);
      lane_tangent = static_cast<float>(computed_width.second);
    } else if (GetId() > 0) {
      // left lane
      const auto side_lanes = MakeListView(lanes.lower_bound(1), lanes.end());
      const auto computed_width =
          ComputeTotalLaneWidth(side_lanes, s, GetId());
      lane_width = static_cast<float>(computed_width.first);
      lane_tangent = static_cast<float>(computed_width.second);
    }

    // get a directed point in s and apply the computed lateral offet
    element::DirectedPoint dp = road->GetDirectedPointIn(s);

    // compute the tangent of the laneOffset
    const auto lane_offset_info = road->GetInfo<element::RoadInfoLaneOffset>(s);
    const auto lane_offset_tangent = static_cast<float>(lane_offset_info->GetPolynomial().Tangent(s));

    lane_tangent -= lane_offset_tangent;

    // Unreal's Y axis hack
    lane_tangent *= -1;

    geom::Rotation rot(
        geom::Math::ToDegrees(static_cast<float>(dp.pitch)),
        geom::Math::ToDegrees(-static_cast<float>(dp.tangent)), // Unreal's Y axis hack
        0.0f);

    dp.ApplyLateralOffset(lane_width);

    if (GetId() > 0) {
      rot.yaw += 180.0f + geom::Math::ToDegrees(lane_tangent);
      rot.pitch = 360.0f - rot.pitch;
    } else {
      rot.yaw -= geom::Math::ToDegrees(lane_tangent);
    }

    // Unreal's Y axis hack
    dp.location.y *= -1;

    return geom::Transform(dp.location, rot);
  }

  std::pair<geom::Vector3D, geom::Vector3D> Lane::GetCornerPositions(
      const double s, const float extra_width) const {
    DEBUG_ASSERT(GetRoad() != nullptr);

    float lane_width = static_cast<float>(GetWidth(s)) / 2.0f;
    if (extra_width != 0.f && GetRoad()->IsJunction() && GetType() == Lane::LaneType::Driving) {
      lane_width += extra_width;
    }
    lane_width = GetId() > 0 ? -lane_width : lane_width;

    const geom::Transform wp_trnsf = ComputeTransform(s);

    const auto wp_right_distance = wp_trnsf.GetRightVector() * lane_width;

    auto loc_r = static_cast<geom::Vector3D>(wp_trnsf.location) + wp_right_distance;
    auto loc_l = static_cast<geom::Vector3D>(wp_trnsf.location) - wp_right_distance;

    // Apply an offset to the Sidewalks
    if (GetType() == LaneType::Sidewalk) {
      // RoadRunner doesn't export it right now and as a workarround where 15.24 cm
      // is the exact height that match with most of the RoadRunner sidewalks
      loc_r.z += 0.1524f;
      loc_l.z += 0.1524f;
      /// TODO: use the OpenDRIVE 5.3.7.2.1.1.9 Lane Height Record
    }

    return std::make_pair(loc_r, loc_l);
  }

} // road
} // carla
