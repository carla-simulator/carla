#include "SceneCaptureComponentCube_CARLA.h"



USceneCaptureComponentCube_CARLA::USceneCaptureComponentCube_CARLA(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
}

const AActor* USceneCaptureComponentCube_CARLA::GetViewOwner() const
{
    return ViewActor;
}
