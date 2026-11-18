// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Model.h"

namespace twinmodel {

  const char *ToString(LaneType type) {
    switch (type) {
      case LaneType::Driving:    return "driving";
      case LaneType::Sidewalk:   return "sidewalk";
      case LaneType::Shoulder:   return "shoulder";
      case LaneType::Parking:    return "parking";
      case LaneType::Biking:     return "biking";
      case LaneType::Border:     return "border";
      case LaneType::Restricted: return "restricted";
      case LaneType::Median:     return "median";
      case LaneType::None:       break;
    }
    return "none";
  }

  const char *ToString(Direction direction) {
    switch (direction) {
      case Direction::Forward:  return "forward";
      case Direction::Backward: return "backward";
      case Direction::Both:     break;
    }
    return "both";
  }

  const char *ToString(MarkingKind kind) {
    switch (kind) {
      case MarkingKind::Solid:        return "solid";
      case MarkingKind::Broken:       return "broken";
      case MarkingKind::SolidSolid:   return "solid solid";
      case MarkingKind::SolidBroken:  return "solid broken";
      case MarkingKind::BrokenSolid:  return "broken solid";
      case MarkingKind::BrokenBroken: return "broken broken";
      case MarkingKind::None:         break;
    }
    return "none";
  }

  const char *ToString(MarkingColor color) {
    return (color == MarkingColor::Yellow) ? "yellow" : "white";
  }

  SignalType GetSignalType(SignalKind kind) {
    switch (kind) {
      case SignalKind::TrafficLight:           return SignalType{"1000001", "-1", true};
      case SignalKind::TrafficLightPedestrian: return SignalType{"1000002", "-1", true};
      case SignalKind::Stop:                   return SignalType{"206", "-1", false};
      case SignalKind::Yield:                  return SignalType{"205", "-1", false};
      case SignalKind::PriorityRoad:           return SignalType{"306", "-1", false};
      case SignalKind::SpeedLimit:             return SignalType{"274", "", false};
      case SignalKind::Crosswalk:              break;
    }
    return SignalType{"", "-1", false};
  }

  const char *ToString(ContactPoint point) {
    return (point == ContactPoint::End) ? "end" : "start";
  }

} // namespace twinmodel
