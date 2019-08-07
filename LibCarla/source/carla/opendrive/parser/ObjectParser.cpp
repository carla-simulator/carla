// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/ObjectParser.h"

#include "carla/road/MapBuilder.h"

#include <pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  using RoadID = std::string;
  using ObjectID = std::string;
  using DependencyID = std::string;
  using SignalReferenceID = std::string;

  template <typename T>
  static void AddValidity(
      pugi::xml_node parent_node,
      const std::string &node_name,
      const ObjectID &objectID,
      T &&function) {
    for (pugi::xml_node validity_node = parent_node.child(node_name.c_str());
        validity_node;
        validity_node = validity_node.next_sibling("validity")) {
      const int from_lane = validity_node.attribute("fromLane").as_int();
      const int to_lane = validity_node.attribute("toLane").as_int();
      log_debug("Added validity to signal ", objectID, ":", from_lane, to_lane);
      function(objectID, from_lane, to_lane);
    }
  }

  void ObjectParser::Parse(
      const pugi::xml_document & /*xml*/,
      carla::road::MapBuilder &  /* map_builder */) {
    throw_exception(std::runtime_error("ObjectParser not ready"));
    // Extracting the OpenDRIVE
    // pugi::xml_node opendrive_node = xml.child("OpenDRIVE");
    // for (pugi::xml_node road_node = opendrive_node.child("road");
    //     road_node;
    //     road_node = road_node.next_sibling("road")) {
    //   const RoadID road_id = static_cast<RoadID>(road_node.attribute("id").value());
    //   const pugi::xml_node objects_node = road_node.child("objects");
    //   for (pugi::xml_node object_node = objects_node.child("object");
    //       object_node;
    //       object_node = object_node.next_sibling("object")) {
    //     const std::string type = object_node.attribute("type").value();
    //     const std::string name = object_node.attribute("name").value();
    //     const ObjectID object_id = object_node.attribute("id").value();
    //     const double s_position = object_node.attribute("s").as_double();
    //     const double t_position = object_node.attribute("t").as_double();
    //     const double z_offset = object_node.attribute("zOffset").as_double();
    //     const double valid_length = object_node.attribute("validLength").as_double();
    //     const std::string orientation = object_node.attribute("orientation").value();
    //     const double lenght = object_node.attribute("lenght").as_double();
    //     const double width = object_node.attribute("width").as_double();
    //     const double radius = object_node.attribute("radius").as_double();
    //     const double height = object_node.attribute("height").as_double();
    //     const double hdg = object_node.attribute("hdg").as_double();
    //     const double pitch = object_node.attribute("pitch").as_double();
    //     const double roll = object_node.attribute("roll").as_double();
    //     /*map_builder.AddObjectInRoad(road_id, type, name, object_id,
    //        s_position, t_position, z_offset, valid_length,
    //        orientation, lenght, width, radius, height, hdg, pitch, roll);*/
    //     log_debug("AddObjectInRoad",
    //         road_id,
    //         type,
    //         name,
    //         object_id,
    //         s_position,
    //         t_position,
    //         z_offset,
    //         valid_length,
    //         orientation,
    //         lenght,
    //         width,
    //         radius,
    //         height,
    //         hdg,
    //         pitch,
    //         roll);
    //     for (pugi::xml_node repeat_node = object_node.child("repeat");
    //         repeat_node;
    //         repeat_node = repeat_node.next_sibling("repeat")) {
    //       const double s_position_rep = repeat_node.attribute("s").as_double();
    //       const double lenght_rep = repeat_node.attribute("lenght").as_double();
    //       const double distance_rep = repeat_node.attribute("distance").as_double();
    //       const double t_start_rep = repeat_node.attribute("tStart").as_double();
    //       const double t_end_rep = repeat_node.attribute("tEnd").as_double();
    //       const double width_start_rep = repeat_node.attribute("widthStart").as_double();
    //       const double width_end_rep = repeat_node.attribute("widthEnd").as_double();
    //       const double height_start_rep = repeat_node.attribute("heightStart").as_double();
    //       const double z_offset_start_rep = repeat_node.attribute("zOffsetStart").as_double();
    //       const double z_offset_end_rep = repeat_node.attribute("zOffsetEnd").as_double();
    //       /*map_builder.AddRepeatRecordInObject(object_id, s_position_rep,
    //          lenght_rep, t_start_rep,
    //           t_start_rep, t_end_rep, width_start_rep, width_end_rep,
    //              height_start_rep, z_offset_start_rep,
    //             z_offset_end_rep);
    //        */
    //       log_debug("AddRepeatRecordInObject", object_id, s_position_rep, lenght_rep, distance_rep,
    //           t_start_rep, t_end_rep, width_start_rep, width_end_rep, height_start_rep, z_offset_start_rep,
    //           z_offset_end_rep);

    //     }
    //     pugi::xml_node outline_node = object_node.child("outline");
    //     for (pugi::xml_node corner_road_node = outline_node.child("cornerRoad");
    //         corner_road_node;
    //         corner_road_node = corner_road_node.next_sibling("cornerRoad")) {
    //       const double s_position_corner_road = corner_road_node.attribute("s").as_double();
    //       const double t_position_corner_road = corner_road_node.attribute("t").as_double();
    //       const double dz_position_corner_road = corner_road_node.attribute("dz").as_double();
    //       const double height_position_corner_road = corner_road_node.attribute("height").as_double();
    //       /*
    //          map_builder.AddCornerRoadToObject(object_id,
    //             s_position_corner_road, t_position_corner_road,
    //          dz_position_corner_road, height_position_corner_road);
    //        */
    //       log_debug("AddCornerRoadToObject", object_id, s_position_corner_road, t_position_corner_road,
    //           dz_position_corner_road, height_position_corner_road);
    //     }
    //     for (pugi::xml_node corner_local_node = outline_node.child("cornerLocal");
    //         corner_local_node;
    //         corner_local_node = corner_local_node.next_sibling("cornerLocal")) {
    //       const double u_coordinate_corner_local = corner_local_node.attribute("u").as_double();
    //       const double v_coordinate_corner_local = corner_local_node.attribute("v").as_double();
    //       const double z_coordinate_corner_local = corner_local_node.attribute("z").as_double();
    //       const double height_corner_local = corner_local_node.attribute("height").as_double();
    //       /*
    //          map_builder.AddCornerLocalToObject(object_id,
    //             u_coordinate_corner_local, v_coordinate_corner_local,
    //          z_coordinate_corner_local, height_corner_local);
    //        */
    //       log_debug("AddCornerLocalToObject", object_id, u_coordinate_corner_local, v_coordinate_corner_local,
    //           z_coordinate_corner_local, height_corner_local);
    //     }
    //     pugi::xml_node material_node = object_node.child("material");
    //     if (material_node != nullptr) {
    //       const std::string surface_material = material_node.attribute("surface").value();
    //       const double friction_material = material_node.attribute("friction").as_double();
    //       const double roughness_material = material_node.attribute("roughness").as_double();
    //       /*
    //          map_builder.AddMaterialToObject(object_id, surface_material,
    //             friction_material, roughness_material);
    //        */
    //       log_debug("AddMaterialToObject", object_id, surface_material, friction_material,
    //           roughness_material);
    //     }

    //     /* AddValidity(object_node, "validity", object_id,
    //        ([&map_builder](const ObjectID &object_id, const int16_t from_lane,
    //           const int16_t to_lane)
    //         { map_builder.AddValidityToObject(object_id, from_lane,
    //            to_lane);})); */

    //     pugi::xml_node parking_space_node = object_node.child("parkingSpace");
    //     if (parking_space_node != nullptr) {
    //       const std::string access_parking = parking_space_node.attribute("access").value();
    //       const std::string restrictions_parking = parking_space_node.attribute("restrictions").value();
    //       /*
    //          map_builder.AddParkingSpaceToObject(object_id, access_parking,
    //             restrictions_parking);
    //        */
    //       log_debug("AddParkingSpaceToObject", object_id, access_parking, restrictions_parking);
    //     }
    //     for (pugi::xml_node marking_node = parking_space_node.child("marking");
    //         marking_node;
    //         marking_node = marking_node.next_sibling("marking")) {
    //       const std::string side_marking = marking_node.attribute("side").value();
    //       const std::string type_marking = marking_node.attribute("type").value();
    //       const double width_marking = marking_node.attribute("width").as_double();
    //       const std::string color_marking = marking_node.attribute("color").value();
    //       /*
    //          Problem, there is no "Parking ID" or anything similar... how do we
    //             link them?
    //          map_builder.AddMarkingToParkingSpace(side_marking, type_marking,
    //          width_marking, color_marking);
    //        */
    //       log_debug("AddMarkingToParkingSpace", side_marking, type_marking,
    //           width_marking, color_marking);
    //     }
    //     for (pugi::xml_node obj_reference_node = object_node.child("objectReference");
    //         obj_reference_node;
    //         obj_reference_node = obj_reference_node.next_sibling("objectReference")) {
    //       const double s_position_obj_ref = obj_reference_node.attribute("s").as_double();
    //       const double t_position_obj_ref = obj_reference_node.attribute("t").as_double();
    //       const ObjectID id_obj_ref = obj_reference_node.attribute("id").value();
    //       const double z_offset_obj_ref = obj_reference_node.attribute("zOffset").as_double();
    //       const double valid_lenght_obj_ref = obj_reference_node.attribute("validLenght").as_double();
    //       const std::string orientation_obj_ref = obj_reference_node.attribute("orientation").value();
    //       /*
    //          map_builder.AddObjectReferenceToObject(object_id,
    //             s_position_obj_ref, t_position_obj_ref,
    //          id_obj_ref, z_offset_obj_ref, valid_lenght_obj_ref,
    //             orientation_obj_ref);
    //        */
    //       log_debug("AddObjectReferenceToObject", object_id, s_position_obj_ref, t_position_obj_ref,
    //           id_obj_ref, z_offset_obj_ref, valid_lenght_obj_ref, orientation_obj_ref);
    //       /* AddValidity(obj_reference_node, "validity", id_obj_ref,
    //          ([&map_builder](const ObjectID &id_obj_ref, const int16_t
    //             from_lane, const int16_t to_lane)
    //           { map_builder.AddValidityToObjectRef(id_obj_ref, from_lane,
    //              to_lane);})); */
    //     }

    //   }
    //   for (pugi::xml_node tunnel_node = objects_node.child("tunnel");
    //       tunnel_node;
    //       tunnel_node = tunnel_node.next_sibling("tunnel")) {
    //     const double s_position_tunnel = tunnel_node.attribute("s").as_double();
    //     const double lenght_tunnel = tunnel_node.attribute("lenght").as_double();
    //     const std::string name_tunnel = tunnel_node.attribute("name").value();
    //     const ObjectID id_tunnel = tunnel_node.attribute("id").value();
    //     const std::string type_tunnel = tunnel_node.attribute("type").value();
    //     const double lightning_tunnel = tunnel_node.attribute("lightning").as_double();
    //     const double daylight_tunnel = tunnel_node.attribute("daylight").as_double();
    //     /*
    //        map_builder.AddTunnel(s_position_tunnel, lenght_tunnel, name_tunnel,
    //           id_tunnel,
    //        type_tunnel, lightning_tunnel, daylight_tunnel);
    //      */
    //     log_debug("AddTunnel", s_position_tunnel, lenght_tunnel, name_tunnel, id_tunnel,
    //         type_tunnel, lightning_tunnel, daylight_tunnel);
    //     /* AddValidity(tunnel_node, "validity", id_tunnel,
    //        ([&map_builder](const ObjectID &id_tunnel, const int16_t from_lane,
    //           const int16_t to_lane)
    //         { map_builder.AddValidityToTunnel(id_tunnel, from_lane,
    //            to_lane);})); */
    //   }
    //   for (pugi::xml_node bridge_node = objects_node.child("bridge");
    //       bridge_node;
    //       bridge_node = bridge_node.next_sibling("bridge")) {
    //     const double s_position_bridge = bridge_node.attribute("s").as_double();
    //     const double lenght_bridge = bridge_node.attribute("lenght").as_double();
    //     const std::string name_bridge = bridge_node.attribute("name").value();
    //     const ObjectID id_bridge = bridge_node.attribute("id").value();
    //     const std::string type_bridge = bridge_node.attribute("type").value();
    //     /*
    //        map_builder.AddBridge(s_position_bridge, lenght_bridge, name_bridge,
    //           id_bridge,
    //        type_bridge);
    //      */
    //     log_debug("AddTunnel", s_position_bridge, lenght_bridge, name_bridge, id_bridge,
    //         type_bridge);
    //     /*AddValidity(bridge_node, "validity", id_bridge,
    //        ([&map_builder](const ObjectID &id_bridge, const int16_t from_lane,
    //           const int16_t to_lane)
    //         { map_builder.AddValidityToBridge(id_bridge, from_lane,
    //            to_lane);}));*/
    //   }
    // }
  }
} // namespace parser
} // namespace opendrive
} // namespace carla
