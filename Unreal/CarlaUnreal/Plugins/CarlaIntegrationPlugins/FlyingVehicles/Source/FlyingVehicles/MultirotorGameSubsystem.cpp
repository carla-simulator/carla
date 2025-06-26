
#include "MultirotorGameSubsystem.h"

#include "FlyingVehicles/CarlaInterface/CarlaActor.h"
#include "FlyingVehicles/ros2/ROS2Multirotor.h"

void UMultirotorSubsystem::Initialize(FSubsystemCollectionBase& Collection) 
{
    FMultirotorActor::RegisterClassWithFactory();
    carla::ros2::ROS2Multirotor::GetInstance(); // will register the instance
}

void UMultirotorSubsystem::Deinitialize()
{

}
