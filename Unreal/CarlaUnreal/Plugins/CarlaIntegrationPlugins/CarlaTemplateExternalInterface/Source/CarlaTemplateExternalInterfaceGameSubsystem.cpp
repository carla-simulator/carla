
#include "CarlaTemplateExternalInterfaceGameSubsystem.h"
#include "CarlaInterface/CarlaActor.h"
#include "ros2/ROS2CarlaTemplate.h"

void UCarlaTemplateExternalInterfaceSubsystem::Initialize(FSubsystemCollectionBase& Collection) 
{
    FCarlaTemplateExternalInterfaceActor::RegisterClassWithFactory();
    carla::ros2::ROS2CarlaTemplate::GetInstance();
}

void UCarlaTemplateExternalInterfaceSubsystem::Deinitialize()
{

}
