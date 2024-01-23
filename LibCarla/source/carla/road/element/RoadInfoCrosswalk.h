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

  struct CrosswalkPoint {
    double u            { 0.0 };
    double v            { 0.0 };
    double z            { 0.0 };
    CrosswalkPoint(double _u, double _v, double _z) : u(_u), v(_v), z(_z) {};
  };

  class RoadInfoCrosswalk final : public RoadInfo {
  public:

    RoadInfoCrosswalk(
        const double s,
        const std::string name,
        const double t,
        const double zOffset,
        const double hdg,
        const double pitch,
        const double roll,
        const std::string orientation,
        const double width,
        const double length,
        const std::vector<CrosswalkPoint> points)
      : RoadInfo(s),
        _name(name),
        _t(t),
        _zOffset(zOffset),
        _hdg(hdg),
        _pitch(pitch),
        _roll(roll),
        _orientation(orientation),
        _width(width),
        _length(length),
        _points(points) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    double GetS() const { return GetDistance(); };
    double GetT() const { return _t; };
    double GetWidth() const { return _width; };
    double GetLength() const { return _length; };
    double GetHeading() const { return _hdg; };
    double GetPitch() const { return _pitch; };
    double GetRoll() const { return _roll; };
    double GetZOffset() const { return _zOffset; };
    std::string GetOrientation() const { return _orientation; };
    const std::vector<CrosswalkPoint> &GetPoints() const { return _points; };

  private:
    std::string _name;
    double _t;
    double _zOffset;
    double _hdg;
    double _pitch;
    double _roll;
    std::string _orientation;
    double _width;
    double _length;
    std::vector<CrosswalkPoint> _points;
  };

} // namespace element
} // namespace road
} // namespace carla
