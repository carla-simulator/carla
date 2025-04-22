#include "CarlaChassisPublisher.h"

#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

bool CarlaChassisPublisher::UpdateData(int32_t seconds, uint32_t nanoseconds, ActorCallback callback) {
    ROS2CallbackData data(boost::variant2::in_place_type<VehicleChassisReport>);
    callback(this->GetActor(), data);

    const VehicleChassisReport& report = boost::variant2::get<VehicleChassisReport>(data);

    // TODO: Add header to CARLA chassis message
    // builtin_interfaces::msg::Time time;
    // time.sec(seconds);
    // time.nanosec(nanoseconds);

    // std_msgs::msg::Header header;
    // header.stamp(std::move(time));
    // header.frame_id("map");

    // _impl->GetMessage()->header(std::move(header));
    _impl->GetMessage()->engine_started(report.engine_started);
    _impl->GetMessage()->engine_rpm(report.engine_rpm);
    _impl->GetMessage()->speed_mps(report.speed_mps);
    _impl->GetMessage()->throttle_percentage(report.throttle_percentage);
    _impl->GetMessage()->brake_percentage(report.brake_percentage);
    _impl->GetMessage()->steering_percentage(report.steering_percentage);

    return true;
}

}  // namespace ros2
}  // namespace carla
