// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/SignalParser.h"

#include "carla/opendrive/parser/pugixml/pugixml.hpp"

#include "carla/road/MapBuilder.h"

namespace carla {
namespace opendrive {
namespace parser {

  using RoadID = char *;
  using SignalID = char *;
  using DependencyID = char *;
  using SignalReferenceID = char *;

  void SignalParser::Parse(
      const pugi::xml_document & xml,
      carla::road::MapBuilder &/* map_builder */) {
        //Extracting the OpenDRIVE
        pugi::xml_node opendrive_node = xml.child("OpenDRIVE");
        for (pugi::xml_node road_node = opendrive_node.child("road");
            road_node;
            road_node = road_node.next_sibling("road")) {
              RoadID road_id = (RoadID) road_node.attribute("id").value();
              pugi::xml_node signals_node = road_node.child("signals");
              for (pugi::xml_node signal_node = signals_node.child("signal");
                signal_node;
                signal_node = signal_node.next_sibling("signal")) {
                  double s_position = signal_node.attribute("s").as_double();
                  double t_position = signal_node.attribute("t").as_double();
                  SignalID signal_id = (SignalID) signal_node.attribute("id").value();
                  char * name = (char *) signal_node.attribute("name").value();
                  char * dynamic = (char *) signal_node.attribute("dynamic").value();
                  char * orientation = (char *) signal_node.attribute("orientation").value();
                  double zOffset = signal_node.attribute("zOffSet").as_double();
                  char * country = (char *) signal_node.attribute("country").value();
                  char * type = (char *) signal_node.attribute("type").value();
                  char * subtype = (char *) signal_node.attribute("subtype").value();
                  double value = signal_node.attribute("value").as_double();
                  char * unit = (char *) signal_node.attribute("unit").value();
                  double height = signal_node.attribute("height").as_double();
                  double width = signal_node.attribute("width").as_double();
                  char * text = (char *) signal_node.attribute("text").value();
                  double hOffset = signal_node.attribute("hOffset").as_double();
                  double pitch = signal_node.attribute("pitch").as_double();
                  double roll = signal_node.attribute("roll").as_double();
                  logging::log("Road: ", road_id, "Adding Signal: ", s_position, t_position, signal_id, name, dynamic, orientation, zOffset, country, type, subtype, value, unit, height, width, text, hOffset, pitch, roll);
                  //map_builder.AddSignal(road_id, s_position, t_position, signal_id, name, dynamic, orientation, zOffset, country, type, subtype, value, unit, height, width, text, hOffset, pitch, roll);
                  for (pugi::xml_node validity_node = signal_node.child("validity");
                    validity_node;
                    validity_node = validity_node.next_sibling("validity")) {
                      int from_lane = validity_node.attribute("fromLane").as_int();
                      int to_lane = validity_node.attribute("toLane").as_int();
                      logging::log("Added validity to signal ", signal_id, ":", from_lane, to_lane);
                      //map_builder.AddValidityToSignal(signal_id, from_lane, to_lane);
                    }
                  for (pugi::xml_node dependency_node = signal_node.child("dependency");
                    dependency_node;
                    dependency_node = dependency_node.next_sibling("validity")) {
                      DependencyID dependency_id = (DependencyID) dependency_node.attribute("id").value();
                      char * dependency_type = (char *) dependency_node.attribute("type").value();
                      logging::log("Added dependency to signal ", signal_id, ":", dependency_id, dependency_type);
                      //map_builder.AddDependencyToSignal(signal_id, dependency_id, dependency_type);
                    }
                }
                for (pugi::xml_node signalreference_node = signals_node.child("signalReference");
                    signalreference_node;
                    signalreference_node = signalreference_node.next_sibling("signalReference")) {
                      double s_position = signalreference_node.attribute("s").as_double();
                      double t_position = signalreference_node.attribute("t").as_double();
                      SignalReferenceID signal_reference_id = (SignalReferenceID) signalreference_node.attribute("id").value();
                      char * signal_reference_orientation = (char *) signalreference_node.attribute("orientation").value();
                      logging::log("Road: ", road_id, "Added SignalReference ", s_position, t_position, signal_reference_id, signal_reference_orientation);
                      //map_builder.AddDependencyToSignal(road_id, s_position, t_position, signal_reference_id, signal_reference_orientation);
                      for (pugi::xml_node sr_validity_node = signalreference_node.child("validity");
                        sr_validity_node;
                        sr_validity_node = sr_validity_node.next_sibling("validity")) {
                          int from_lane = sr_validity_node.attribute("fromLane").as_int();
                          int to_lane = sr_validity_node.attribute("toLane").as_int();
                          logging::log("Added validity to signal reference", signal_reference_id, ":", from_lane, to_lane);
                          //map_builder.AddValidityToSignalReference(signal_reference_id, from_lane, to_lane);
                        }
                    }
            }
        //Iterate over roads
          //Check if there is "Signals"
          //Iterate over signal

      }

} // namespace parser
} // namespace opendrive
} // namespace carla
