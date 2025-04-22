#include "OdometryPublisher.h"

#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

bool OdometryPublisher::UpdateData(int32_t seconds, uint32_t nanoseconds, ActorCallback callback) {
    ROS2CallbackData data(boost::variant2::in_place_type<VehicleOdometryReport>);
    callback(this->GetActor(), data);

    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id("map");

    const VehicleOdometryReport& report = boost::variant2::get<VehicleOdometryReport>(data);

    // Position
    geometry_msgs::msg::Point position;
    position.x(report.x / 100.0f);
    position.y(-report.y / 100.0f);
    position.z(report.z / 100.0f);

    // Orientation
    geometry_msgs::msg::Quaternion orientation;
    const float roll = report.roll * (float(M_PI) / 180.0f);
    const float pitch = -report.pitch * (float(M_PI) / 180.0f);
    const float yaw = -report.yaw * (float(M_PI) / 180.0f);

    const float cr = cosf(roll * 0.5f);
    const float sr = sinf(roll * 0.5f);
    const float cp = cosf(pitch * 0.5f);
    const float sp = sinf(pitch * 0.5f);
    const float cy = cosf(yaw * 0.5f);
    const float sy = sinf(yaw * 0.5f);

    float w = cr * cp * cy + sr * sp * sy;
    float x = sr * cp * cy - cr * sp * sy;
    float y = cr * sp * cy + sr * cp * sy;
    float z = cr * cp * sy - sr * sp * cy;

    orientation.x(x);
    orientation.y(y);
    orientation.z(z);
    orientation.w(w);

    // Pose
    geometry_msgs::msg::Pose pose;
    pose.position(std::move(position));
    pose.orientation(std::move(orientation));

    // Pose with covariance
    geometry_msgs::msg::PoseWithCovariance pose_with_covariance;
    pose_with_covariance.pose(std::move(pose));
    pose_with_covariance.covariance({
        0.1,0.0,0.0,0.0,0.0,0.0,
        0.0,0.1,0.0,0.0,0.0,0.0,
        0.0,0.0,0.1,0.0,0.0,0.0,
        0.0,0.0,0.0,0.0,0.0,0.0,
        0.0,0.0,0.0,0.0,0.0,0.0,
        0.0,0.0,0.0,0.0,0.0,0.0
    });

    // Linear velocity
    geometry_msgs::msg::Vector3 linear;
    linear.x(report.linear_velocity_x / 100.0f);
    linear.y(-report.linear_velocity_y / 100.0f);
    linear.z(report.linear_velocity_z / 100.0f);

    // Angular velocity
    geometry_msgs::msg::Vector3 angular;
    angular.x(report.angular_velocity_x * float(M_PI) / 180.0f);
    angular.y(-report.angular_velocity_y * float(M_PI) / 180.0f);
    angular.z(report.angular_velocity_z * float(M_PI) / 180.0f);

    // Twist
    geometry_msgs::msg::Twist twist;
    twist.linear(std::move(linear));
    twist.angular(std::move(angular));

    // Twist with covariance
    geometry_msgs::msg::TwistWithCovariance twist_with_covariance;
    twist_with_covariance.twist(std::move(twist));
    twist_with_covariance.covariance({
        0.1,0.0,0.0,0.0,0.0,0.0,
        0.0,0.1,0.0,0.0,0.0,0.0,
        0.0,0.0,0.1,0.0,0.0,0.0,
        0.0,0.0,0.0,0.0,0.0,0.0,
        0.0,0.0,0.0,0.0,0.0,0.0,
        0.0,0.0,0.0,0.0,0.0,0.0
    });

    // Odometry
    _impl->GetMessage()->header(std::move(header));
    _impl->GetMessage()->pose(std::move(pose_with_covariance));
    _impl->GetMessage()->twist(std::move(twist_with_covariance));

    return true;
}

}  // namespace ros2
}  // namespace carla
