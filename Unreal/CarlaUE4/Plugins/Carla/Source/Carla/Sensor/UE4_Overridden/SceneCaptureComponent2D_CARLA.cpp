#include "SceneCaptureComponent2D_CARLA.h"



USceneCaptureComponent2D_CARLA::USceneCaptureComponent2D_CARLA(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
}

const AActor* USceneCaptureComponent2D_CARLA::GetViewOwner() const
{
    return ViewActor;
}