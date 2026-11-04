// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Signals.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <sstream>

#include "Profiles.h"

namespace twinmodel {

  namespace {

    std::string MakeId(const char *prefix, std::size_t index) {
      std::ostringstream out;
      out << prefix << index;
      return out.str();
    }


    /// Rank of a highway class; the junction's major road is the highest rank
    /// among its approaches and gets the priority plate, the rest give way.
    int ClassRank(const std::string &highway) {
      if (highway == "motorway" || highway == "motorway_link") return 7;
      if (highway == "trunk" || highway == "trunk_link") return 6;
      if (highway == "primary" || highway == "primary_link") return 5;
      if (highway == "secondary" || highway == "secondary_link") return 4;
      if (highway == "tertiary" || highway == "tertiary_link") return 3;
      if (highway == "unclassified" || highway == "residential") return 2;
      if (highway == "living_street") return 1;
      return 0;
    }

    /// The road whose reference line passes closest to @a point, with the s of
    /// the projection. Connecting roads are skipped: a sign governs approach.
    const Road *NearestRoad(const TwinModelData &model, const geom::Point &point,
                            double limit, double &out_s, double &out_t) {
      const Road *best = nullptr;
      double best_distance = limit;
      for (const Road &road : model.roads) {
        if (road.junction_id.has_value() || road.reference_line.size() < 2u) {
          continue;
        }
        const geom::LineString flat = geom::To2D(road.reference_line);
        const double s = geom::Project(flat, point);
        const geom::Point on = geom::Interpolate(flat, s);
        const double distance = geom::Hypot(on, point);
        if (distance < best_distance) {
          best_distance = distance;
          best = &road;
          out_s = s;
          // signed offset: positive to the left of the reference line
          const double heading = geom::HeadingAt(flat, s);
          const double dx = geom::bg::get<0>(point) - geom::bg::get<0>(on);
          const double dy = geom::bg::get<1>(point) - geom::bg::get<1>(on);
          out_t = -std::sin(heading) * dx + std::cos(heading) * dy;
        }
      }
      return best;
    }

    void AddSignal(TwinModelData &model, SignalKind kind, const Road &road,
                   double s, double t, const geom::Point &position, double heading,
                   std::optional<double> value, long long node_id) {
      Signal signal;
      signal.id = MakeId("signal_", model.signals.size());
      signal.kind = kind;
      signal.road_id = road.id;
      signal.s = s;
      signal.t = t;
      signal.position = position;
      signal.heading = heading;
      signal.value = value;
      signal.osm_node_id = node_id;
      signal.orientation = (t < 0.0) ? "-" : "+";
      model.signals.push_back(std::move(signal));
    }

  } // namespace

  void BuildSignals(const OsmData &osm, const LocalFrame &frame,
                    TwinModelData &model, const SignalOptions &options) {

    auto to_local = [&](const OsmNode &node) {
      double x = 0.0;
      double y = 0.0;
      frame.ToLocal(node.lon, node.lat, x, y);
      return geom::Point(x, y);
    };

    // 1. node-tagged signs and lights
    for (const auto &entry : osm.nodes) {
      const OsmNode &node = entry.second;
      const std::string *highway = FindTag(node.tags, "highway");
      if (highway == nullptr) {
        continue;
      }
      SignalKind kind = SignalKind::Crosswalk;
      bool keep = false;
      if (*highway == "traffic_signals") {
        if (options.generate_traffic_lights || options.all_junctions_traffic_lights) {
          kind = SignalKind::TrafficLight;
          keep = true;
        }
      } else if (*highway == "stop") {
        kind = SignalKind::Stop;
        keep = true;
      } else if (*highway == "give_way") {
        kind = SignalKind::Yield;
        keep = true;
      } else if (*highway == "crossing") {
        kind = SignalKind::Crosswalk;
        keep = true;
      }
      if (!keep) {
        continue;
      }
      const geom::Point point = to_local(node);
      double s = 0.0;
      double t = 0.0;
      const Road *road = NearestRoad(model, point, options.snap_m, s, t);
      if (road == nullptr) {
        continue;
      }
      const geom::LineString flat = geom::To2D(road->reference_line);
      AddSignal(model, kind, *road, s, t, point, geom::HeadingAt(flat, s),
                std::nullopt, node.id);
    }

    // 2. a speed limit sign at the start of every way that carries maxspeed
    std::set<std::string> signed_roads;
    for (const Road &road : model.roads) {
      if (road.junction_id.has_value() || road.lanes.empty()) {
        continue;
      }
      const std::string *maxspeed = FindTag(road.tags, "maxspeed");
      if (maxspeed == nullptr) {
        continue;
      }
      const std::optional<double> speed = ParseMaxSpeed(*maxspeed);
      if (!speed.has_value() || signed_roads.count(road.id) != 0u) {
        continue;
      }
      signed_roads.insert(road.id);
      const geom::LineString flat = geom::To2D(road.reference_line);
      const geom::Point at = geom::Interpolate(flat, 0.0);
      AddSignal(model, SignalKind::SpeedLimit, road, 0.0, -1.0, at,
                geom::HeadingAt(flat, 0.0), speed, 0);
    }

    // 3. a light on every junction approach when the caller asked for them on
    // all junctions, with one controller per junction holding its signals
    if (options.all_junctions_traffic_lights) {
      for (const Junction &junction : model.junctions) {
        std::set<std::string> approaches;
        for (const Connection &connection : junction.connections) {
          approaches.insert(connection.incoming_road);
        }
        for (const std::string &road_id : approaches) {
          const Road *road = model.FindRoad(road_id);
          if (road == nullptr || road->reference_line.size() < 2u) {
            continue;
          }
          const geom::LineString flat = geom::To2D(road->reference_line);
          const double total = geom::Length(flat);
          const geom::Point at = geom::Interpolate(flat, total);
          AddSignal(model, SignalKind::TrafficLight, *road, total, -1.0, at,
                    geom::HeadingAt(flat, total), std::nullopt, 0);
        }
      }
    }

    // 4. sign the unsignalised junctions: the highest highway class through
    // the junction keeps priority, every lower approach gives way
    {
      std::set<std::string> signalised;
      for (const Signal &signal : model.signals) {
        if (signal.kind == SignalKind::TrafficLight ||
            signal.kind == SignalKind::TrafficLightPedestrian) {
          signalised.insert(signal.road_id);
        }
      }
      for (const Junction &junction : model.junctions) {
        std::set<std::string> approaches;
        for (const Connection &connection : junction.connections) {
          approaches.insert(connection.incoming_road);
        }
        if (approaches.size() < 2u) {
          continue;
        }
        int best = -1;
        bool any_signalised = false;
        for (const std::string &road_id : approaches) {
          const Road *road = model.FindRoad(road_id);
          if (road == nullptr) {
            continue;
          }
          best = std::max(best, ClassRank(road->highway));
          any_signalised = any_signalised || (signalised.count(road_id) != 0u);
        }
        if (any_signalised || best < 0) {
          continue;
        }
        bool mixed = false;
        for (const std::string &road_id : approaches) {
          const Road *road = model.FindRoad(road_id);
          if (road != nullptr && ClassRank(road->highway) != best) {
            mixed = true;
          }
        }
        for (const std::string &road_id : approaches) {
          const Road *road = model.FindRoad(road_id);
          if (road == nullptr || road->reference_line.size() < 2u) {
            continue;
          }
          // Where every approach is the same class there is no major road to
          // plate, so the junction is uncontrolled and each approach gives way.
          const bool major = mixed && ClassRank(road->highway) == best;
          const geom::LineString flat = geom::To2D(road->reference_line);
          const double total = geom::Length(flat);
          const double at = std::max(0.0, total - 1.0);
          AddSignal(model, major ? SignalKind::PriorityRoad : SignalKind::Yield,
                    *road, at, -1.0, geom::Interpolate(flat, at),
                    geom::HeadingAt(flat, at), std::nullopt, 0);
        }
      }
    }

    // 5. one controller per junction over the lights on its approaches
    std::map<std::string, std::vector<std::string>> by_junction;
    for (const Signal &signal : model.signals) {
      if (signal.kind != SignalKind::TrafficLight &&
          signal.kind != SignalKind::TrafficLightPedestrian) {
        continue;
      }
      for (const Junction &junction : model.junctions) {
        bool touches = false;
        for (const Connection &connection : junction.connections) {
          if (connection.incoming_road == signal.road_id) {
            touches = true;
            break;
          }
        }
        if (touches) {
          by_junction[junction.id].push_back(signal.id);
          break;
        }
      }
    }
    for (const auto &entry : by_junction) {
      Controller controller;
      controller.id = MakeId("controller_", model.controllers.size());
      controller.junction_id = entry.first;
      controller.signal_ids = entry.second;
      controller.sequence = 0;
      for (Signal &signal : model.signals) {
        if (std::find(entry.second.begin(), entry.second.end(), signal.id) !=
            entry.second.end()) {
          signal.controller_id = controller.id;
        }
      }
      model.controllers.push_back(std::move(controller));
    }
  }

} // namespace twinmodel
