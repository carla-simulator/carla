// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/Signal.h"
#include "carla/road/element/RoadInfo.h"

namespace carla {
namespace road {
namespace element {

  class RoadInfoSignal final : public RoadInfo {
  public:

    RoadInfoSignal(
        SignId signal_id,
        Signal* signal,
        double s,
        double t,
        std::string orientation)
      : RoadInfo(s),
        _signal_id(signal_id),
        _signal(signal),
        _s(s),
        _t(t),
        _orientation(orientation) {}

    RoadInfoSignal(
        SignId signal_id,
        double s,
        double t,
        std::string orientation)
      : RoadInfo(s),
        _signal_id(signal_id),
        _s(s),
        _t(t),
        _orientation(orientation) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    SignId GetSignalId() const {
      return _signal_id;
    }

    const Signal* GetSignal() const {
      return _signal;
    }

    bool IsDynamic() const {
      return _signal->GetDynamic();
    }

    double GetS() const {
      return _s;
    }

    double GetT() const {
      return _t;
    }

    SignalOrientation GetOrientation() const {
      if(_orientation == "+") {
        return SignalOrientation::Positive;
      } else if(_orientation == "-") {
        return SignalOrientation::Negative;
      } else {
        return SignalOrientation::Both;
      }
    }

    const std::vector<LaneValidity> &GetValidities() const {
      return _validities;
    }

  private:
    friend MapBuilder;

    SignId _signal_id;

    Signal* _signal;

    double _s;

    double _t;

    std::string _orientation;

    std::vector<LaneValidity> _validities;
  };

} // namespace element
} // namespace road
} // namespace carla
