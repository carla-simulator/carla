#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/RoadTypes.h"
#include "carla/road/LaneValidity.h"
#include "carla/geom/Transform.h"

#include <string>
#include <vector>

namespace carla {
namespace road {

  enum StencilOrientation {
    Positive,
    Negative,
    Both
  };

  class Stencil : private MovableNonCopyable {
  public:
    Stencil(
        RoadId road_id,
        StencilId stencil_id,
        double s,
        double t,
        std::string name,
        std::string orientation,
        std::string type,
        std::string text,
        double zOffset,
        double length,
        double width,
        double pitch,
        double roll)
      : _road_id(road_id),
        _stencil_id(stencil_id),
        _s(s),
        _t(t),
        _name(name),
        _orientation(orientation),
        _type(type),
        _text(text),
        _length(length),
        _width(width),
        _pitch(pitch),
        _roll(roll) {}

    RoadId GetRoadId() const {
      return _road_id;
    }

    const StencilId &GetStencilId() const {
      return _stencil_id;
    }

    double GetS() const {
      return _s;
    }

    double GetT() const {
      return _t;
    }

    const std::string &GetName() const {
      return _name;
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

    double GetZOffset() const {
      return _zOffset;
    }

    const std::string &GetType() const {
      return _type;
    }

    double GetLength() const {
      return _height;
    }

    double GetWidth() const {
      return _width; 
    }

    const std::string &GetText() const {
      return _text;
    }

    double GetPitch() const {
      return _pitch;
    }

    double GetRoll() const {
      return _roll;
    }

    const geom::Transform &GetTransform() const {
      return _transform;
    }

  private:
    friend MapBuilder;

    RoadId _road_id;

    StencilId _signal_id;

    double _s;

    double _t;

    std::string _name;

    std::string _orientation;

    std::string _type;

    std::string _text;

    double _zOffset;

    double _length;

    double _width;

    double _pitch;

    double _roll;

    geom::Transform _transform;
  };

} // road
} // carla
