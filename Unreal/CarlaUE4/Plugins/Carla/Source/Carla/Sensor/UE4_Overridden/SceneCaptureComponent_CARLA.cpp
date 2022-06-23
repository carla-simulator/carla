#include "SceneCaptureComponent_CARLA.h"



USceneCaptureComponent_CARLA::USceneCaptureComponent_CARLA(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
}

const AActor* USceneCaptureComponent_CARLA::GetViewOwner() const
{
    return ViewActor;
}