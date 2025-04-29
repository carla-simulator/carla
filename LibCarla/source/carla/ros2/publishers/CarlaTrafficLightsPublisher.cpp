#include "CarlaTrafficLightsPublisher.h"

#include "carla/ros2/ROS2CallbackData.h"

namespace carla {
namespace ros2 {

bool CarlaTrafficLightsPublisher::UpdateData(int32_t seconds, uint32_t nanoseconds, ActorCallback callback) {
    ROS2CallbackData data(boost::variant2::in_place_type<TrafficLightReport>);
    callback(this->GetActor(), data);

    const TrafficLightReport& report = boost::variant2::get<TrafficLightReport>(data);

    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id("map");

    std::vector<carla_msgs::msg::CarlaTrafficLightStatus> traffic_lights;

    for (const auto& traffic_light: report.traffic_lights) {
      carla_msgs::msg::CarlaTrafficLightStatus tl;
      tl.id(traffic_light.id);
        
      switch (traffic_light.light_state) {
        case TrafficLightState::Red:
          tl.color(carla_msgs::msg::CarlaTrafficLightStatus_Constants::RED);
          break;
        case TrafficLightState::Yellow:
          tl.color(carla_msgs::msg::CarlaTrafficLightStatus_Constants::YELLOW);
          break;
        case TrafficLightState::Green:
          tl.color(carla_msgs::msg::CarlaTrafficLightStatus_Constants::GREEN);
          break;
        case TrafficLightState::Off:
          tl.color(carla_msgs::msg::CarlaTrafficLightStatus_Constants::BLACK);
          break;
        default:
          tl.color(carla_msgs::msg::CarlaTrafficLightStatus_Constants::UNKNOWN);
          break;
        }

        tl.confidence(traffic_light.confidence);
        tl.blink(traffic_light.blink);

        traffic_lights.push_back(tl);
    }

    _impl->GetMessage()->header(std::move(header));
    _impl->GetMessage()->traffic_light(std::move(traffic_lights));

    return true;
}

}  // namespace ros2
}  // namespace carla
