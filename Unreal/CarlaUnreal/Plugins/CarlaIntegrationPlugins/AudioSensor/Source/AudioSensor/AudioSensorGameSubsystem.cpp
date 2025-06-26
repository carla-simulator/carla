
#include "AudioSensorGameSubsystem.h"

#include "AudioSensor/CarlaInterface/CarlaActor.h"

#include "AudioSensor/ros2/ROS2Audio.h"

void UAudioSensorSubsystem::Initialize(FSubsystemCollectionBase& Collection) 
{
    FAudioSensorActor::RegisterClassWithFactory();
    carla::ros2::ROS2Audio::GetInstance();
}

void UAudioSensorSubsystem::Deinitialize()
{

}
