#include "CarlaObstaclesPublisher.h"

#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

bool CarlaObstaclesPublisher::UpdateData(int32_t seconds, uint32_t nanoseconds, ActorCallback callback) {
    ROS2CallbackData data(boost::variant2::in_place_type<ObstacleReport>);
    callback(this->GetActor(), data);

    const ObstacleReport& report = boost::variant2::get<ObstacleReport>(data);

    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id("map");

    std::vector<carla_msgs::msg::CarlaPerceptionObstacle> obstacles;

    for (const auto& obstacle: report.obstacles) {
        carla_msgs::msg::CarlaPerceptionObstacle obs;
        obs.id(obstacle.id);

        // Position
        geometry_msgs::msg::Vector3 position;
        position.x(obstacle.pos_x);
        position.y(-obstacle.pos_y);
        position.z(obstacle.pos_z);
        obs.position(std::move(position));

        // Yaw
        obs.yaw(-obstacle.yaw);

        // Velocity
        geometry_msgs::msg::Vector3 velocity;
        velocity.x(obstacle.speed_x);
        velocity.y(-obstacle.speed_y);
        velocity.z(obstacle.speed_z);
        obs.velocity(std::move(velocity));

        // Dimensions
        obs.length(obstacle.length);
        obs.width(obstacle.width);
        obs.height(obstacle.height);

        // Polygon point
        std::vector<geometry_msgs::msg::Vector3> points;
        // point 1
        geometry_msgs::msg::Vector3 point1;
        point1.x(obstacle.corner_points[0]);
        point1.y(-obstacle.corner_points[1]);
        point1.z(obstacle.corner_points[2]);
        points.push_back(point1);
        // point 2
        geometry_msgs::msg::Vector3 point2;
        point2.x(obstacle.corner_points[3]);
        point2.y(-obstacle.corner_points[4]);
        point2.z(obstacle.corner_points[5]);
        points.push_back(point2);
        // point 3
        geometry_msgs::msg::Vector3 point3;
        point3.x(obstacle.corner_points[6]);
        point3.y(-obstacle.corner_points[7]);
        point3.z(obstacle.corner_points[8]);
        points.push_back(point3);
        // point 4
        geometry_msgs::msg::Vector3 point4;
        point4.x(obstacle.corner_points[9]);
        point4.y(-obstacle.corner_points[10]);
        point4.z(obstacle.corner_points[11]);
        points.push_back(point4);
        
        obs.polygon_point(std::move(points));

        // Tracking time
        obs.tracking_time(obstacle.tracking_time);

        // Type & Subtype
        obs.type(carla_msgs::msg::CarlaPerceptionObstacle_Constants::VEHICLE);

        // Timestamp
        obs.timestamp(obstacle.timestamp);

        // Acceleration
        geometry_msgs::msg::Vector3 acceleration;
        acceleration.x(obstacle.acc_x);
        acceleration.y(-obstacle.acc_y);
        acceleration.z(obstacle.acc_z);
        obs.acceleration(std::move(acceleration));

        // Anchor point
        geometry_msgs::msg::Vector3 anchor_point;
        anchor_point.x(obstacle.anchor_x);
        anchor_point.y(-obstacle.anchor_y);
        anchor_point.z(obstacle.anchor_z);
        obs.acceleration(std::move(acceleration));

        obstacles.push_back(obs);
    }

    _impl->GetMessage()->header(std::move(header));
    _impl->GetMessage()->obstacle(std::move(obstacles));

    return true;
}

}  // namespace ros2
}  // namespace carla
