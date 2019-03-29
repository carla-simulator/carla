// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include <string>
#include <vector>
#include "carla/road/RoadTypes.h"
#include "carla/road/general/Validity.h"

namespace carla {
namespace road {
namespace signal {

  class SignalReference : private MovableNonCopyable {
  public:

    SignalReference(
        int32_t road_id,
        uint32_t id,
        double s,
        double t,
        std::string orientation)
      : _road_id(road_id),
        _signal_id(id),
        _s(s),
        _t(t),
        _orientation(orientation) {}

    void AddValidity(general::Validity &&validity) {
      _validities.push_back(std::move(validity));
    }

  private:

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-private-field"
#endif
    int32_t _road_id;
    int32_t _signal_id;
    double _s;
    double _t;
    std::string _orientation;
    std::vector<general::Validity> _validities;
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif
  };

} // object
} // road
} // carla
