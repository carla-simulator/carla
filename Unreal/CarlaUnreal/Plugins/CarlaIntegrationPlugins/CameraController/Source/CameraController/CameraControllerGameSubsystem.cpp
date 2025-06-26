
#include "CameraControllerGameSubsystem.h"
#include "CarlaInterface/CarlaActor.h"
#include "ros2/ROS2CameraControl.h"

void UCameraControllerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    FCameraControllerActor::RegisterClassWithFactory();
    carla::ros2::ROS2CameraControl::GetInstance();
}

void UCameraControllerSubsystem::Deinitialize()
{

}
