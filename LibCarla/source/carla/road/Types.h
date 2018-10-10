// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"

#include <cstdio>

namespace carla {
namespace road {

  using id_type = size_t;

// Geometry ////////////////////////////////////////////////////////////

  enum class GeometryType : unsigned int {
    LINE,
    ARC,
    SPIRAL
  };

  class Geometry {
  public:

    GeometryType GetTypeh() {
      return _type;
    }
    double GetLength() {
      return _length;
    }
    double GetStartPositionOffset() {
      return _start_position_offset;
    }
    double GetHeading() {
      return _heading;
    }

  private:

    GeometryType _type;            // geometry type
    double _length;                // length of the road section [meters]

    double _start_position_offset; // s-offset [meters]
    double _heading;               // start orientation [radians]

    geom::Location start_position; // [meters]

  protected:

    Geometry(GeometryType type) : _type(type) {}
  };

  class GeometryLine : public Geometry {
  public:

    GeometryLine() : Geometry(GeometryType::LINE) {}
  };

  class GeometryArc : public Geometry {
  public:

    GeometryArc(double curv)
      : Geometry(GeometryType::ARC),
        _curvature(curv) {}
    double GetCurvature() {
      return _curvature;
    }

  private:

    double _curvature;
  };

  class GeometrySpiral : public Geometry {
  public:

    GeometrySpiral(double curv_s, double curv_e)
      : Geometry(GeometryType::SPIRAL),
        _curve_start(curv_s),
        _curve_end(curv_e) {}
    double GetCurveStart() {
      return _curve_start;
    }
    double GetCurveEnd() {
      return _curve_end;
    }

  private:

    double _curve_start;
    double _curve_end;
  };

  struct RoadInfo {
    // distance from Road's start location
    double d = 0; // [meters]
  };

  struct SpeedLimit : public RoadInfo {
    double speed = 0; // [meters/second]
  };

  class RoadSegmentDefinition {
  public:

    const id_type &GetId() const
    {
      return _id;
    }

    RoadSegmentDefinition(id_type id) {
      assert(id > 0);
      _id = id;
    }

    void AddPredecessorID(const id_type &id) {
      _predecessor_id.emplace_back(id);
    }

    void AddSuccessorID(const id_type &id) {
      _successor_id.emplace_back(id);
    }

    void AddGeometry(const Geometry &geom) {
      _geom.emplace_back(geom);
    }

    void AddInfo(const RoadInfo &info) {
      _info.emplace_back(info);
    }

    const std::vector<id_type> &GetPredecessorID_Vector() const {
      return _predecessor_id;
    }
    const std::vector<id_type> &GetSuccessorID_Vector() const {
      return _successor_id;
    }
    const std::vector<Geometry> &GetGeometry_Vector() const {
      return _geom;
    }
    const std::vector<RoadInfo> &GetInfo_Vector() const {
      return _info;
    }

  private:

    id_type _id;
    std::vector<id_type> _predecessor_id;
    std::vector<id_type> _successor_id;
    std::vector<Geometry> _geom;
    std::vector<RoadInfo> _info;
  };

} // namespace road
} // namespace carla
