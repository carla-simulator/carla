#include "CollisionStage.h"

namespace traffic_manager {

  const float SEARCH_RADIUS = 20.0;
  const float VERTICAL_OVERLAP_THRESHOLD = 2.0;
  const float ZERO_AREA = 0.0001;
  const float BOUNDARY_EXTENSION_MINIMUM = 2.0;
  const float EXTENSION_SQUARE_POINT = 7.0;
  const float TIME_HORIZON = 0.5;
  const float HIGHWAY_SPEED = 50 / 3.6;
  const float HIGHWAY_TIME_HORIZON = 5.0;

  CollisionStage::CollisionStage(
    std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger,
    std::shared_ptr<CollisionToPlannerMessenger> planner_messenger,
    int number_of_vehicle,
    int pool_size
  ):
    localization_messenger(localization_messenger),
    planner_messenger(planner_messenger),
    PipelineStage(pool_size, number_of_vehicle)
  {
    frame_selector = true;

    planner_frame_a = std::make_shared<CollisionToPlannerFrame>(number_of_vehicle);
    planner_frame_b = std::make_shared<CollisionToPlannerFrame>(number_of_vehicle);

    planner_frame_map.insert(std::pair<bool, std::shared_ptr<CollisionToPlannerFrame>>(true, planner_frame_a));
    planner_frame_map.insert(std::pair<bool, std::shared_ptr<CollisionToPlannerFrame>>(false, planner_frame_b));

    localization_messenger_state = localization_messenger->GetState();
    planner_messenger_state = planner_messenger->GetState() -1;

  }

  CollisionStage::~CollisionStage() {}

  void CollisionStage::Action(int start_index, int end_index) {

    for (int i=start_index; i <=end_index; i++) {
      break; // Remove this

      auto& data = localization_frame->at(i);
      auto ego_actor = data.actor;
      auto ego_actor_id = ego_actor->GetId();

      auto actor_id_list = vicinity_grid.GetActors(ego_actor);
      // std::cout << "Number of nearby vehicles : " << actor_id_list.size() << std::endl;
      float collision_hazard = -1;
      for (auto actor_id: actor_id_list) {
        if (actor_id != ego_actor_id) {
          auto ego_actor_location = ego_actor->GetLocation();
          auto actor = localization_frame->at(id_to_index.at(actor_id)).actor;
          float actor_distance = actor->GetLocation().Distance(ego_actor_location);
          if (actor_distance <= SEARCH_RADIUS) {
            try
            {
              if (negotiateCollision(ego_actor, actor)) {
                collision_hazard = 1;
                break;
              }
            }
            catch(const std::exception& e)
            {
              std::cout << "Unable to detect collision " << e.what() << '\n';
            }
          }
        }
      }

      auto& message = planner_frame_map.at(frame_selector)->at(i);
      message.hazard = collision_hazard > 0 ? true: false;

    }
  }

  void CollisionStage::DataReceiver() {
    // std::cout << "Running receiver" << std::endl; 
    auto packet = localization_messenger->ReceiveData(localization_messenger_state);
    localization_frame = packet.data;
    localization_messenger_state = packet.id;

    int index=0;
    for (auto& element: *localization_frame.get()) {
      id_to_index.insert(std::pair<int, int>(element.actor->GetId(), index));
    }
    // std::cout << "Finished receiver" << std::endl;
  }

  void CollisionStage::DataSender() {
    // std::cout << "Running sender" << std::endl;
    DataPacket<std::shared_ptr<CollisionToPlannerFrame>> packet{
      planner_messenger_state,
      planner_frame_map.at(frame_selector)
    };
    frame_selector = !frame_selector;
    planner_messenger_state = planner_messenger->SendData(packet);
    // std::cout << "Finished receiver" << std::endl;
  }

  bool CollisionStage::negotiateCollision(
      carla::SharedPtr<carla::client::Actor> ego_vehicle,
      carla::SharedPtr<carla::client::Actor> other_vehicle) const {

    bool hazard = false;
    auto overlap = checkGeodesicCollision(ego_vehicle, other_vehicle);
    if (overlap) {
      auto reference_heading_vector = ego_vehicle->GetTransform().GetForwardVector();
      reference_heading_vector.z = 0;
      reference_heading_vector = reference_heading_vector.MakeUnitVector();
      auto relative_other_vector = other_vehicle->GetLocation() - ego_vehicle->GetLocation();
      relative_other_vector.z = 0;
      relative_other_vector = relative_other_vector.MakeUnitVector();
      float reference_relative_dot = reference_heading_vector.x * relative_other_vector.x +
          reference_heading_vector.y * relative_other_vector.y;

      auto relative_reference_vector = ego_vehicle->GetLocation() - other_vehicle->GetLocation();
      relative_reference_vector.z = 0;
      relative_reference_vector = relative_reference_vector.MakeUnitVector();
      auto other_heading_vector = other_vehicle->GetTransform().GetForwardVector();
      other_heading_vector.z = 0;
      other_heading_vector = other_heading_vector.MakeUnitVector();
      float other_relative_dot = other_heading_vector.x * relative_reference_vector.x +
          other_heading_vector.y * relative_reference_vector.y;

      hazard = reference_relative_dot > other_relative_dot;
    }

    return hazard;
  }

  bool CollisionStage::checkGeodesicCollision(
      carla::SharedPtr<carla::client::Actor> reference_vehicle,
      carla::SharedPtr<carla::client::Actor> other_vehicle) const {
    bool overlap = false;
    auto reference_height = reference_vehicle->GetLocation().z;
    auto other_height = other_vehicle->GetLocation().z;
    if (abs(reference_height - other_height) < VERTICAL_OVERLAP_THRESHOLD) {
      auto reference_bbox = getBoundary(reference_vehicle);
      auto other_bbox = getBoundary(other_vehicle);
      auto reference_geodesic_boundary = getGeodesicBoundary(
          reference_vehicle, reference_bbox);
      auto other_geodesic_boundary = getGeodesicBoundary(
          other_vehicle, other_bbox);
      if (
        reference_geodesic_boundary.size() > 0
        and
        other_geodesic_boundary.size() > 0) {
        auto reference_polygon = getPolygon(reference_geodesic_boundary);
        auto other_polygon = getPolygon(other_geodesic_boundary);

        std::deque<polygon> output;
        boost::geometry::intersection(reference_polygon, other_polygon, output);

        BOOST_FOREACH(polygon const & p, output) {
          if (boost::geometry::area(p) > ZERO_AREA) {
            overlap = true;
            break;
          }
        }
      }
    }

    return overlap;
  }

  traffic_manager::polygon CollisionStage::getPolygon(const std::vector<carla::geom::Location> &boundary)
  const {
    std::string wkt_string;
    for (auto location: boundary) {
      wkt_string += std::to_string(location.x) + " " + std::to_string(location.y) + ",";
    }
    wkt_string += std::to_string(boundary[0].x) + " " + std::to_string(boundary[0].y);

    traffic_manager::polygon boundary_polygon;
    boost::geometry::read_wkt("POLYGON((" + wkt_string + "))", boundary_polygon);

    return boundary_polygon;
  }

  std::vector<carla::geom::Location> CollisionStage::getGeodesicBoundary(
      carla::SharedPtr<carla::client::Actor> actor,
      const std::vector<carla::geom::Location> &bbox) const {

    auto velocity = actor->GetVelocity().Length();
    int bbox_extension = static_cast<int>(
      std::max(std::sqrt(EXTENSION_SQUARE_POINT * velocity), BOUNDARY_EXTENSION_MINIMUM) +
      std::max(velocity * TIME_HORIZON, BOUNDARY_EXTENSION_MINIMUM) +
      BOUNDARY_EXTENSION_MINIMUM
    );

    std::vector<carla::geom::Location> geodesic_boundary;

      bbox_extension = velocity > HIGHWAY_SPEED ? HIGHWAY_TIME_HORIZON * velocity : bbox_extension;
      auto waypoint_buffer =  localization_frame->front().buffer_list->at(id_to_index.at(actor->GetId()));

      std::vector<carla::geom::Location> left_boundary;
      std::vector<carla::geom::Location> right_boundary;
      auto vehicle = boost::static_pointer_cast<carla::client::Vehicle>(actor);
      float width = vehicle->GetBoundingBox().extent.y;

      for (int i=0; i<bbox_extension; i++) {
        if (i >= waypoint_buffer.size()) break;
        auto swp = waypoint_buffer.at(i);
        auto vector = swp->getVector();
        auto location = swp->getLocation();
        auto perpendicular_vector = carla::geom::Vector3D(-1 * vector.y, vector.x, 0);
        perpendicular_vector = perpendicular_vector.MakeUnitVector();
        left_boundary.push_back(location + carla::geom::Location(perpendicular_vector * width));
        right_boundary.push_back(location - carla::geom::Location(perpendicular_vector * width));
      }

      std::reverse(left_boundary.begin(), left_boundary.end());
      geodesic_boundary.insert(geodesic_boundary.end(), left_boundary.begin(), left_boundary.end());
      geodesic_boundary.insert(geodesic_boundary.end(), bbox.begin(), bbox.end());
      geodesic_boundary.insert(geodesic_boundary.end(), right_boundary.begin(), right_boundary.end());
      std::reverse(geodesic_boundary.begin(), geodesic_boundary.end());

    return geodesic_boundary;
  }

  std::vector<carla::geom::Location> CollisionStage::getBoundary(
      carla::SharedPtr<carla::client::Actor> actor) const {
    auto vehicle = boost::static_pointer_cast<carla::client::Vehicle>(actor);
    auto bbox = vehicle->GetBoundingBox();
    auto extent = bbox.extent;
    auto location = vehicle->GetLocation();
    auto heading_vector = vehicle->GetTransform().GetForwardVector();
    heading_vector.z = 0;
    heading_vector = heading_vector.MakeUnitVector();
    auto perpendicular_vector = carla::geom::Vector3D(-1 * heading_vector.y,
        heading_vector.x,
        0);

    return {
             location + carla::geom::Location(heading_vector * extent.x + perpendicular_vector * extent.y),
             location +
             carla::geom::Location(-1 * heading_vector * extent.x + perpendicular_vector * extent.y),
             location +
             carla::geom::Location(-1 * heading_vector * extent.x - perpendicular_vector * extent.y),
             location + carla::geom::Location(heading_vector * extent.x - perpendicular_vector * extent.y)
    };
  }
}
