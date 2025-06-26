#include "CarlaTemplateExternalInterfaceActor.h"
#include "CarlaTemplateExternalInterfaceBlueprintLibrary.h"
#include "ros2/ROS2CarlaTemplate.h"

ACarlaTemplateExternalInterfaceActor::ACarlaTemplateExternalInterfaceActor(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
// Add code that is required on startup
}

void ACarlaTemplateExternalInterfaceActor::BeginPlay()
{
    Super::BeginPlay();
    TemplateFrameDelegate.BindUFunction(this, FName("TemplateFrameCallback"));
    Activate();
}

void ACarlaTemplateExternalInterfaceActor::SetSensorTick(double SensorTick){
    this->SensorTick = SensorTick;
}

void ACarlaTemplateExternalInterfaceActor::SetTemplateValue(double Value){
    this->TemplateValue = Value;
}

void ACarlaTemplateExternalInterfaceActor::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  UCarlaTemplateExternalInterfaceBlueprintLibrary::SetCarlaTemplateExternalInterface(ActorDescription, this);
}

void ACarlaTemplateExternalInterfaceActor::SetOwner(AActor* OwningActor)
{
  Super::SetOwner(OwningActor);
}

void ACarlaTemplateExternalInterfaceActor::TemplateFrameCallback(){
    if (!Active){
        return;
    }

    auto ROS2CarlaTemplate = carla::ros2::ROS2CarlaTemplate::GetInstance();
    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    FTransform ActorTransform = GetActorTransform();
    AActor* ParentActor = GetAttachParentActor();
    if (ParentActor)
    {
        ActorTransform = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
    }
    char const *data = "Hello World";

    ROS2CarlaTemplate->ProcessDataFromTemplate(StreamId, ActorTransform, data, this);
}

void ACarlaTemplateExternalInterfaceActor::Activate(){
    Active = true;
    if (!GetWorldTimerManager().IsTimerActive(TemplateFrameTimer)){
        GetWorldTimerManager().SetTimer(TemplateFrameTimer, TemplateFrameDelegate, SensorTick, true);
    }
}

void ACarlaTemplateExternalInterfaceActor::Deactivate(){
    Active = false;
    if (GetWorldTimerManager().IsTimerActive(TemplateFrameTimer)){
        GetWorldTimerManager().ClearTimer(TemplateFrameTimer);
    }
}
