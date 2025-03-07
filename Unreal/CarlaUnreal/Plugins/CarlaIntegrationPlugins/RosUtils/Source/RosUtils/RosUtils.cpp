#include "RosUtils.h"

#include "carla/ros2/plugin-utils/enable-fastdds-include.h" // start fastdds includes
#include "carla/ros2/types/Header.h"
#include "carla/ros2/types/Transform.h"
#include "carla/ros2/types/Vector3.h"
#include "carla/ros2/types/Twist.h"
#include "carla/ros2/plugin-utils/disable-fastdds-include.h" // end fastdds includes

#include "StructDefinitions.h"

#define LOCTEXT_NAMESPACE "FRosUtils"


DEFINE_LOG_CATEGORY(LogRosUtils)

void FRosUtils::StartupModule()
{
}

void FRosUtils::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRosUtils, RosUtils)

