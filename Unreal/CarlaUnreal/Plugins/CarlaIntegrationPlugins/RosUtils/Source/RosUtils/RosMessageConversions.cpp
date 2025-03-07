#include "RosMessageConversions.h"

#include "carla/ros2/plugin-utils/enable-fastdds-include.h" // start fastdds includes
#include "carla/ros2/types/Header.h"
#include "carla/ros2/types/Transform.h"
#include "carla/ros2/types/Vector3.h"
#include "carla/ros2/types/Twist.h"
#include "carla/ros2/types/Point.h"
#include "carla/ros2/types/Quaternion.h"
#include "carla/ros2/plugin-utils/disable-fastdds-include.h" // end fastdds includes

#include "StructDefinitions.h"

FTransform FRosMessageConversions::RosToUnrealTransform(const geometry_msgs::msg::Transform& RosTransform) {
    auto RosTranslate = RosTransform.translation();
    auto RosQuat = RosTransform.rotation();
    return FTransform{
        FRotator{FQuat(RosQuat.x(), RosQuat.y(), RosQuat.z(), RosQuat.w())},
        FVector{RosTranslate.x(), RosTranslate.y(), RosTranslate.z()}, 
        FVector{1.0, 1.0, 1.0} // scale
    };
}

FTransform FRosMessageConversions::RosToUnrealTransform(const geometry_msgs::msg::Transform& RosTransform, const geometry_msgs::msg::Vector3& RosScale) {
    FTransform Transform = RosToUnrealTransform(RosTransform);
    Transform.SetScale3D(FVector{RosScale.x(), RosScale.y(), RosScale.z()});
    return Transform;
}

FVector FRosMessageConversions::RosToUnrealPoint(const geometry_msgs::msg::Point& RosPoint){
    return FVector{RosPoint.x(), RosPoint.y(), RosPoint.z()};
}

FQuat FRosMessageConversions::RosToUnrealQuaternion(const geometry_msgs::msg::Quaternion& RosQuat){
    return FQuat(RosQuat.x(), RosQuat.y(), RosQuat.z(), RosQuat.w());
}

FTwist FRosMessageConversions::RosToUnrealTwist(const geometry_msgs::msg::Twist& RosTwist){
    FTwist Output;
    Output.Linear = FVector{RosTwist.linear().x(), RosTwist.linear().y(), RosTwist.linear().z()};
    Output.Angular = FVector{RosTwist.angular().x(), RosTwist.angular().y(), RosTwist.angular().z()};

    return Output;
}


