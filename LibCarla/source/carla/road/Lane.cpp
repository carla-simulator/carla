// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Lane.h"

#include <limits>

#include "carla/Debug.h"
#include "carla/geom/Math.h"
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
        tangent += negative_lane_id ? current_tang : -current_tang;
      } else {
        current_dist *= 0.5;
        dist += negative_lane_id ? current_dist : -current_dist;
        tangent += (negative_lane_id ? current_tang : -current_tang) * 0.5;
        break;
      }
    }
    return std::make_pair(dist, tangent);
  }

  geom::Transform Lane::ComputeTransform(const double s) const {
    const Road *road = GetRoad();
    DEBUG_ASSERT(road != nullptr);

    // must s be smaller (or eq) than road length and bigger (or eq) than 0?
    RELEASE_ASSERT(s <= road->GetLength());
    RELEASE_ASSERT(s >= 0.0);

    const auto *lane_section = GetLaneSection();
    DEBUG_ASSERT(lane_section != nullptr);
    const std::map<LaneId, Lane> &lanes = lane_section->GetLanes();

    // check that lane_id exists on the current s
    RELEASE_ASSERT(!lanes.empty());
    RELEASE_ASSERT(GetId() >= lanes.begin()->first);
    RELEASE_ASSERT(GetId() <= lanes.rbegin()->first);

    // These will accumulate the lateral offset (t) and lane heading of all
    // the lanes in between the current lane and lane 0, where the main road
    // geometry is described
    float lane_t_offset = 0.0f;
    float lane_tangent = 0.0f;

    if (GetId() < 0) {
      // right lane
      const auto side_lanes = MakeListView(
          std::make_reverse_iterator(lanes.lower_bound(0)), lanes.rend());
      const auto computed_width =
          ComputeTotalLaneWidth(side_lanes, s, GetId());
      lane_t_offset = static_cast<float>(computed_width.first);
      lane_tangent = static_cast<float>(computed_width.second);
    } else if (GetId() > 0) {
      // left lane
      const auto side_lanes = MakeListView(lanes.lower_bound(1), lanes.end());
      const auto computed_width =
          ComputeTotalLaneWidth(side_lanes, s, GetId());
      lane_t_offset = static_cast<float>(computed_width.first);
      lane_tangent = static_cast<float>(computed_width.second);
    }

    // Compute the tangent of the road's (lane 0) "laneOffset" on the current s
    const auto lane_offset_info = road->GetInfo<element::RoadInfoLaneOffset>(s);
    const auto lane_offset_tangent =
        static_cast<float>(lane_offset_info->GetPolynomial().Tangent(s));

    // Update the road tangent with the "laneOffset" information at current s
    lane_tangent -= lane_offset_tangent;

    // Get a directed point on the center of the current lane given an s
    element::DirectedPoint dp = road->GetDirectedPointIn(s);

    // Transform from the center of the road to the center of the lane
    dp.ApplyLateralOffset(lane_t_offset);

    // Update the lane tangent with the road "laneOffset" at current s
    dp.tangent -= lane_tangent;

    // Unreal's Y axis hack
    dp.location.y *= -1;
    dp.tangent    *= -1;

    geom::Rotation rot(
        geom::Math::ToDegrees(static_cast<float>(dp.pitch)),
        geom::Math::ToDegrees(static_cast<float>(dp.tangent)),
        0.0f);

    // Fix the direction of the possitive lanes
    if (GetId() > 0) {
      rot.yaw += 180.0f;
      rot.pitch = 360.0f - rot.pitch;
    }

    return geom::Transform(dp.location, rot);
  }

  std::pair<geom::Vector3D, geom::Vector3D> Lane::GetCornerPositions(
      const double s, const float extra_width) const {
    const Road *road = GetRoad();
    DEBUG_ASSERT(road != nullptr);

    const auto *lane_section = GetLaneSection();
    DEBUG_ASSERT(lane_section != nullptr);
    const std::map<LaneId, Lane> &lanes = lane_section->GetLanes();

    // check that lane_id exists on the current s
    RELEASE_ASSERT(!lanes.empty());
    RELEASE_ASSERT(GetId() >= lanes.begin()->first);
    RELEASE_ASSERT(GetId() <= lanes.rbegin()->first);

    float lane_t_offset = 0.0f;

    if (GetId() < 0) {
      // right lane
      const auto side_lanes = MakeListView(
          std::make_reverse_iterator(lanes.lower_bound(0)), lanes.rend());
      const auto computed_width =
          ComputeTotalLaneWidth(side_lanes, s, GetId());
      lane_t_offset = static_cast<float>(computed_width.first);
    } else if (GetId() > 0) {
      // left lane
      const auto side_lanes = MakeListView(lanes.lower_bound(1), lanes.end());
      const auto computed_width =
          ComputeTotalLaneWidth(side_lanes, s, GetId());
      lane_t_offset = static_cast<float>(computed_width.first);
    }

    float lane_width = static_cast<float>(GetWidth(s)) / 2.0f;
    if (extra_width != 0.f && road->IsJunction() && GetType() == Lane::LaneType::Driving) {
      lane_width += extra_width;
    }

    // Get two points on the center of the road on given s
    element::DirectedPoint dp_r, dp_l;
    dp_r = dp_l = road->GetDirectedPointIn(s);

    // Transform from the center of the road to each of lane corners
    dp_r.ApplyLateralOffset(lane_t_offset + lane_width);
    dp_l.ApplyLateralOffset(lane_t_offset - lane_width);

    // Unreal's Y axis hack
    dp_r.location.y *= -1;
    dp_l.location.y *= -1;

    // Apply an offset to the Sidewalks
    if (GetType() == LaneType::Sidewalk) {
      // RoadRunner doesn't export it right now and as a workarround where 15.24 cm
      // is the exact height that match with most of the RoadRunner sidewalks
      dp_r.location.z += 0.1524f;
      dp_l.location.z += 0.1524f;
      /// TODO: use the OpenDRIVE 5.3.7.2.1.1.9 Lane Height Record
    }

    return std::make_pair(dp_r.location, dp_l.location);
  }

} // road
} // carla
