// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"

namespace carla {
namespace road {
namespace element {

  /// Each lane within a road cross section may be provided with several entries
  /// defining the visibility in four directions relative to the lane’s
  /// direction. Each entry is valid until a new entry is defined. If multiple
  /// entries are defined, they must be listed in increasing order.
  ///
  /// For left lanes (positive ID), the forward direction is oriented opposite
  /// to the track’s direction, for right lanes, the forward direction and the
  /// track’s direction are identical.
  class RoadInfoLaneVisibility final : public RoadInfo {
  public:

    RoadInfoLaneVisibility(
        double s, // start position relative to the position of the preceding
                  // lane section
        double forward,
        double back,
        double left,
        double right)
      : RoadInfo(s),
        _forward(forward),
        _back(back),
        _left(left),
        _right(right) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    double GetForward() const {
      return _forward;
    }

    double GetBack() const {
      return _back;
    }

    double GetLeft() const {
      return _left;
    }

    double GetRight() const {
      return _right;
    }

  private:

    const double _forward;

    const double _back;

    const double _left;

    const double _right;
  };

} // namespace element
} // namespace road
} // namespace carla
