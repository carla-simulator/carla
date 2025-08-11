// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/Stencil.h"
#include "carla/road/element/RoadInfo.h"

namespace carla {
namespace road {
namespace element {

  class RoadInfoStencil final : public RoadInfo {
  public:

    RoadInfoStencil(
        StencilId stencil_id,
        Stencil* stencil,
        RoadId road_id,
        double s,
        double t,
        std::string orientation)
      : RoadInfo(s),
        _stencil_id(stencil_id),
        _stencil(stencil),
        _road_id(road_id),
        _s(s),
        _t(t),
        _orientation(orientation) {}

    RoadInfoStencil(
        StencilId stencil_id,
        RoadId road_id,
        double s,
        double t,
        std::string orientation)
      : RoadInfo(s),
        _stencil_id(stencil_id),
        _road_id(road_id),
        _s(s),
        _t(t),
        _orientation(orientation) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    StencilId GetStencilId() const {
      return _stencil_id;
    }

    const Stencil* GetStencil() const {
      return _stencil;
    }

    RoadId GetRoadId() const {
      return _road_id;
    }

    double GetS() const {
      return _s;
    }

    double GetT() const {
      return _t;
    }

    StencilOrientation GetOrientation() const {
      if(_orientation == "+") {
        return StencilOrientation::Positive;
      } else if(_orientation == "-") {
        return StencilOrientation::Negative;
      } else {
        return StencilOrientation::Both;
      }
    }

    const std::vector<LaneValidity> &GetValidities() const {
      return _validities;
    }

  private:
    friend MapBuilder;

    StencilId _stencil_id;

    Stencil* _stencil;

    RoadId _road_id;

    double _s;

    double _t;

    std::string _orientation;

    std::vector<LaneValidity> _validities;
  };

} // namespace element
} // namespace road
} // namespace carla
