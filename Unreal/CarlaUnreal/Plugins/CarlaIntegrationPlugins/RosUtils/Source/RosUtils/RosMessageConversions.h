
#pragma once

#include "Engine.h"
#include "Logging/LogMacros.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct FTwist;

namespace geometry_msgs {
    namespace msg {
        class Transform;
        class Vector3;
        class Twist;
        class Point;
        class Quaternion;
    }
}


class ROSUTILS_API FRosMessageConversions : public UBlueprintFunctionLibrary
{
public:

    static FTransform RosToUnrealTransform(const geometry_msgs::msg::Transform& RosTransform);
    static FTransform RosToUnrealTransform(const geometry_msgs::msg::Transform& RosTransform, const geometry_msgs::msg::Vector3& RosScale);

    static FVector RosToUnrealPoint(const geometry_msgs::msg::Point& RosPoint);

    static FQuat RosToUnrealQuaternion(const geometry_msgs::msg::Quaternion& RosQuat);

    static FTwist RosToUnrealTwist(const geometry_msgs::msg::Twist& RosTransform);
};

