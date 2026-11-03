// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "XodrExport.h"

#include "PlanView.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <ctime>
#include <map>
#include <set>
#include <sstream>

#include <third-party/pugixml/pugixml.hpp>

namespace twinmodel {

  namespace {


    /// Deterministic integer ids: a numeric model id >= 1 is kept when it is
    /// free, anything else takes the next unused one. Roads are allocated
    /// first and junctions from the same pool, so the two never collide.
    struct IdMap {
      std::map<std::string, unsigned int> road;
      std::map<std::string, unsigned int> junction;

      const char *RoadOrMinusOne(const std::optional<std::string> &id, std::string &scratch) const {
        if (!id.has_value()) {
          return "-1";
        }
        const auto it = junction.find(*id);
        if (it == junction.end()) {
          return "-1";
        }
        scratch = std::to_string(it->second);
        return scratch.c_str();
      }
    };

    void Allocate(const std::vector<std::string> &ids,
                  std::set<unsigned int> &used,
                  std::map<std::string, unsigned int> &out) {
      std::vector<std::string> pending;
      for (const std::string &id : ids) {
        const bool numeric = !id.empty() &&
            id.find_first_not_of("0123456789") == std::string::npos;
        if (numeric) {
          const unsigned long value = std::stoul(id);
          if (value >= 1ul && used.count(static_cast<unsigned int>(value)) == 0u) {
            out[id] = static_cast<unsigned int>(value);
            used.insert(static_cast<unsigned int>(value));
            continue;
          }
        }
        pending.push_back(id);
      }
      unsigned int next = used.empty() ? 1u : (*used.rbegin() + 1u);
      for (const std::string &id : pending) {
        while (used.count(next) != 0u) {
          ++next;
        }
        out[id] = next;
        used.insert(next);
      }
    }

    IdMap BuildIdMap(const TwinModelData &model) {
      IdMap map;
      std::set<unsigned int> used;
      std::vector<std::string> road_ids;
      for (const Road &road : model.roads) {
        road_ids.push_back(road.id);
      }
      Allocate(road_ids, used, map.road);
      std::vector<std::string> junction_ids;
      for (const Junction &junction : model.junctions) {
        junction_ids.push_back(junction.id);
      }
      Allocate(junction_ids, used, map.junction);
      return map;
    }

    std::string Format(double value) {
      std::ostringstream out;
      out.precision(10);
      out << value;
      return out.str();
    }

    std::string Timestamp() {
      const std::time_t now = std::time(nullptr);
      std::tm utc{};
#if defined(_WIN32)
      gmtime_s(&utc, &now);
#else
      gmtime_r(&now, &utc);
#endif
      char buffer[32] = {0};
      std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &utc);
      return buffer;
    }

    void SetAttribute(pugi::xml_node node, const char *name, double value) {
      node.append_attribute(name).set_value(Format(value).c_str());
    }

    /// Lanes in OpenDRIVE order: left outward ascending, then centre, then
    /// right outward descending.
    void WriteLaneSection(pugi::xml_node lanes_node, const Road &road) {
      pugi::xml_node section = lanes_node.append_child("laneSection");
      SetAttribute(section, "s", 0.0);

      std::vector<const Lane *> left;
      std::vector<const Lane *> right;
      for (const Lane &lane : road.lanes) {
        if (lane.id > 0) {
          left.push_back(&lane);
        } else if (lane.id < 0) {
          right.push_back(&lane);
        }
      }
      std::sort(left.begin(), left.end(), [](const Lane *a, const Lane *b) {
        return a->id < b->id;
      });
      std::sort(right.begin(), right.end(), [](const Lane *a, const Lane *b) {
        return a->id > b->id;
      });

      auto write_lane = [&](pugi::xml_node parent, const Lane &lane) {
        pugi::xml_node node = parent.append_child("lane");
        node.append_attribute("id").set_value(lane.id);
        node.append_attribute("type").set_value(ToString(lane.type));
        node.append_attribute("level").set_value("false");
        if (lane.predecessor_lane.has_value() || lane.successor_lane.has_value()) {
          pugi::xml_node link = node.append_child("link");
          if (lane.predecessor_lane.has_value()) {
            link.append_child("predecessor").append_attribute("id")
                .set_value(*lane.predecessor_lane);
          }
          if (lane.successor_lane.has_value()) {
            link.append_child("successor").append_attribute("id")
                .set_value(*lane.successor_lane);
          }
        }
        pugi::xml_node width = node.append_child("width");
        SetAttribute(width, "sOffset", 0.0);
        SetAttribute(width, "a", lane.width);
        SetAttribute(width, "b", 0.0);
        SetAttribute(width, "c", 0.0);
        SetAttribute(width, "d", 0.0);
        if (lane.marking.has_value()) {
          pugi::xml_node mark = node.append_child("roadMark");
          SetAttribute(mark, "sOffset", 0.0);
          mark.append_attribute("type").set_value(ToString(lane.marking->kind));
          mark.append_attribute("color").set_value(ToString(lane.marking->color));
          SetAttribute(mark, "width", lane.marking->width);
          mark.append_attribute("laneChange").set_value("both");
        }
        if (lane.speed_limit.has_value()) {
          pugi::xml_node speed = node.append_child("speed");
          SetAttribute(speed, "sOffset", 0.0);
          SetAttribute(speed, "max", *lane.speed_limit);
          speed.append_attribute("unit").set_value("m/s");
        }
      };

      if (!left.empty()) {
        pugi::xml_node node = section.append_child("left");
        for (const Lane *lane : left) {
          write_lane(node, *lane);
        }
      }

      pugi::xml_node centre = section.append_child("center");
      pugi::xml_node centre_lane = centre.append_child("lane");
      centre_lane.append_attribute("id").set_value(0);
      centre_lane.append_attribute("type").set_value("none");
      centre_lane.append_attribute("level").set_value("false");
      if (road.center_marking.has_value()) {
        pugi::xml_node mark = centre_lane.append_child("roadMark");
        SetAttribute(mark, "sOffset", 0.0);
        mark.append_attribute("type").set_value(ToString(road.center_marking->kind));
        mark.append_attribute("color").set_value(ToString(road.center_marking->color));
        SetAttribute(mark, "width", road.center_marking->width);
        mark.append_attribute("laneChange").set_value("none");
      }

      if (!right.empty()) {
        pugi::xml_node node = section.append_child("right");
        for (const Lane *lane : right) {
          write_lane(node, *lane);
        }
      }
    }

    void WriteLink(pugi::xml_node road_node, const Road &road, const IdMap &ids) {
      if (!road.predecessor.has_value() && !road.successor.has_value()) {
        return;
      }
      pugi::xml_node link = road_node.append_child("link");
      auto write = [&](const char *name, const RoadLink &value) {
        pugi::xml_node node = link.append_child(name);
        node.append_attribute("elementType").set_value(
            value.element == LinkElement::Road ? "road" : "junction");
        const auto &pool = (value.element == LinkElement::Road) ? ids.road : ids.junction;
        const auto it = pool.find(value.id);
        node.append_attribute("elementId").set_value(it == pool.end() ? 0u : it->second);
        if (value.contact.has_value()) {
          node.append_attribute("contactPoint").set_value(ToString(*value.contact));
        }
      };
      if (road.predecessor.has_value()) {
        write("predecessor", *road.predecessor);
      }
      if (road.successor.has_value()) {
        write("successor", *road.successor);
      }
    }

    /// Piecewise cubic planview; a straight leg between two locked corners is
    /// written as a <line> instead.
    double WritePlanView(pugi::xml_node road_node, const Road &road) {
      pugi::xml_node plan = road_node.append_child("planView");
      double s = 0.0;
      for (const Geom &fitted : FitPlanView(road.reference_line)) {
        pugi::xml_node geometry = plan.append_child("geometry");
        SetAttribute(geometry, "s", fitted.s);
        SetAttribute(geometry, "x", fitted.x);
        SetAttribute(geometry, "y", fitted.y);
        SetAttribute(geometry, "hdg", fitted.hdg);
        SetAttribute(geometry, "length", fitted.length);
        if (fitted.is_line) {
          geometry.append_child("line");
        } else {
          pugi::xml_node poly = geometry.append_child("paramPoly3");
          static const char *names[8] = {"aU", "bU", "cU", "dU", "aV", "bV", "cV", "dV"};
          for (int i = 0; i < 8; ++i) {
            SetAttribute(poly, names[i], fitted.coeffs[static_cast<std::size_t>(i)]);
          }
          poly.append_attribute("pRange").set_value("normalized");
        }
        s = fitted.s + fitted.length;
      }
      return s;
    }

    void WriteElevation(pugi::xml_node road_node, const Road &road) {
      pugi::xml_node profile = road_node.append_child("elevationProfile");
      pugi::xml_node elevation = profile.append_child("elevation");
      SetAttribute(elevation, "s", 0.0);
      SetAttribute(elevation, "a", road.reference_line.empty() ? 0.0 : road.reference_line.front().z);
      SetAttribute(elevation, "b", 0.0);
      SetAttribute(elevation, "c", 0.0);
      SetAttribute(elevation, "d", 0.0);
    }


    /// <signals> on a road; a crosswalk is not a signal in CARLA but an
    /// <object type="crosswalk"> with a <cornerLocal> outline.
    void WriteSignalsAndObjects(pugi::xml_node road_node, const Road &road,
                                const TwinModelData &model, const IdMap &ids,
                                double crosswalk_half) {
      std::vector<const Signal *> signals;
      std::vector<const Signal *> crossings;
      for (const Signal &signal : model.signals) {
        if (signal.road_id != road.id) {
          continue;
        }
        (signal.kind == SignalKind::Crosswalk ? crossings : signals).push_back(&signal);
      }

      if (!signals.empty()) {
        pugi::xml_node node = road_node.append_child("signals");
        unsigned int index = 0u;
        for (const Signal *signal : signals) {
          const SignalType type = GetSignalType(signal->kind);
          pugi::xml_node element = node.append_child("signal");
          SetAttribute(element, "s", signal->s);
          SetAttribute(element, "t", signal->t);
          element.append_attribute("id").set_value(index++);
          element.append_attribute("name").set_value(signal->id.c_str());
          element.append_attribute("dynamic").set_value(type.dynamic ? "yes" : "no");
          element.append_attribute("orientation").set_value(signal->orientation.c_str());
          SetAttribute(element, "zOffset", 0.0);
          element.append_attribute("type").set_value(type.type);
          if (signal->kind == SignalKind::SpeedLimit && signal->value.has_value()) {
            // CARLA synthesises type 274 with the km/h value as the subtype
            const long kmh = std::lround(*signal->value * 3.6);
            element.append_attribute("subtype").set_value(std::to_string(kmh).c_str());
            SetAttribute(element, "value", static_cast<double>(kmh));
            element.append_attribute("unit").set_value("km/h");
          } else {
            element.append_attribute("subtype").set_value(type.subtype);
          }
          element.append_attribute("country").set_value("OpenDRIVE");
          SetAttribute(element, "hOffset", signal->h_offset);
          SetAttribute(element, "pitch", 0.0);
          SetAttribute(element, "roll", 0.0);
        }
      }

      if (!crossings.empty()) {
        pugi::xml_node node = road_node.append_child("objects");
        unsigned int index = 0u;
        for (const Signal *crossing : crossings) {
          pugi::xml_node element = node.append_child("object");
          SetAttribute(element, "s", crossing->s);
          SetAttribute(element, "t", 0.0);
          element.append_attribute("id").set_value(index++);
          element.append_attribute("name").set_value("crosswalk");
          element.append_attribute("type").set_value("crosswalk");
          SetAttribute(element, "zOffset", 0.0);
          SetAttribute(element, "hdg", 0.0);
          SetAttribute(element, "width", crosswalk_half * 2.0);
          // half the carriageway each side, so the outline spans the lanes
          double half = 0.0;
          for (const Lane &lane : road.lanes) {
            half = std::max(half, std::abs(static_cast<double>(lane.id)) * lane.width);
          }
          half = std::max(half, 3.5);
          SetAttribute(element, "length", half * 2.0);
          pugi::xml_node outline = element.append_child("outline");
          const double corners[4][2] = {{-crosswalk_half, -half}, {crosswalk_half, -half},
                                        {crosswalk_half, half}, {-crosswalk_half, half}};
          for (const auto &corner : corners) {
            pugi::xml_node point = outline.append_child("cornerLocal");
            SetAttribute(point, "u", corner[0]);
            SetAttribute(point, "v", corner[1]);
            SetAttribute(point, "z", 0.0);
            SetAttribute(point, "height", 0.0);
          }
        }
      }
      (void) ids;
    }

  } // namespace

  std::string ExportXodr(const TwinModelData &model, const LocalFrame &frame) {
    pugi::xml_document document;
    pugi::xml_node declaration = document.prepend_child(pugi::node_declaration);
    declaration.append_attribute("version").set_value("1.0");
    declaration.append_attribute("encoding").set_value("UTF-8");

    pugi::xml_node root = document.append_child("OpenDRIVE");
    pugi::xml_node header = root.append_child("header");
    header.append_attribute("revMajor").set_value("1");
    header.append_attribute("revMinor").set_value("6");
    header.append_attribute("name").set_value(model.name.c_str());
    header.append_attribute("version").set_value("1.00");
    header.append_attribute("date").set_value(Timestamp().c_str());
    header.append_attribute("vendor").set_value("twinmodel");

    pugi::xml_node geo = header.append_child("geoReference");
    geo.append_child(pugi::node_cdata).set_value(frame.GetProjString().c_str());

    const IdMap ids = BuildIdMap(model);

    for (const Road &road : model.roads) {
      pugi::xml_node node = root.append_child("road");
      node.append_attribute("name").set_value(road.name.c_str());
      node.append_attribute("id").set_value(ids.road.at(road.id));
      std::string scratch;
      node.append_attribute("junction").set_value(ids.RoadOrMinusOne(road.junction_id, scratch));
      node.append_attribute("rule").set_value("RHT");
      WriteLink(node, road, ids);
      const double length = WritePlanView(node, road);
      node.insert_attribute_after("length", node.attribute("name")).set_value(Format(length).c_str());
      WriteElevation(node, road);
      pugi::xml_node lanes = node.append_child("lanes");
      WriteLaneSection(lanes, road);
      WriteSignalsAndObjects(node, road, model, ids, 2.0);
    }

    for (const Junction &junction : model.junctions) {
      pugi::xml_node node = root.append_child("junction");
      node.append_attribute("name").set_value(junction.name.c_str());
      node.append_attribute("id").set_value(ids.junction.at(junction.id));
      unsigned int connection_index = 0u;
      for (const Connection &connection : junction.connections) {
        const auto incoming = ids.road.find(connection.incoming_road);
        const auto connecting = ids.road.find(connection.connecting_road);
        if (incoming == ids.road.end() || connecting == ids.road.end()) {
          continue;
        }
        pugi::xml_node entry = node.append_child("connection");
        entry.append_attribute("id").set_value(connection_index++);
        entry.append_attribute("incomingRoad").set_value(incoming->second);
        entry.append_attribute("connectingRoad").set_value(connecting->second);
        entry.append_attribute("contactPoint").set_value(ToString(connection.contact_point));
        for (const LaneLink &link : connection.lane_links) {
          pugi::xml_node lane = entry.append_child("laneLink");
          lane.append_attribute("from").set_value(link.from_lane);
          lane.append_attribute("to").set_value(link.to_lane);
        }
      }
    }

    unsigned int controller_index = 0u;
    for (const Controller &controller : model.controllers) {
      pugi::xml_node node = root.append_child("controller");
      node.append_attribute("id").set_value(controller_index++);
      node.append_attribute("name").set_value(controller.id.c_str());
      node.append_attribute("sequence").set_value(controller.sequence);
      for (const std::string &signal_id : controller.signal_ids) {
        unsigned int signal_index = 0u;
        for (const Signal &signal : model.signals) {
          if (signal.id == signal_id) {
            pugi::xml_node control = node.append_child("control");
            control.append_attribute("signalId").set_value(signal_index);
            control.append_attribute("type").set_value("");
            break;
          }
          ++signal_index;
        }
      }
    }

    std::ostringstream out;
    document.save(out, "    ");
    return out.str();
  }

} // namespace twinmodel
