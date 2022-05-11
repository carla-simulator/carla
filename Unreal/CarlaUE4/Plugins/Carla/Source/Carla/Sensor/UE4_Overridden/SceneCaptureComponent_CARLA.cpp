#include "SceneCaptureComponent_CARLA.h"



USceneCaptureComponent_CARLA::USceneCaptureComponent_CARLA(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
}

const AActor* USceneCaptureComponent_CARLA::GetViewOwner() const
{
    return ViewActor;
}

USceneCaptureComponent2D_CARLA::USceneCaptureComponent2D_CARLA(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
}

const AActor* USceneCaptureComponent2D_CARLA::GetViewOwner() const
{
    return ViewActor;
}

USceneCaptureComponentCube_CARLA::USceneCaptureComponentCube_CARLA(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
}

const AActor* USceneCaptureComponentCube_CARLA::GetViewOwner() const
{
    return ViewActor;
}
