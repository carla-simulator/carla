#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Carla/Sensor/Sensor.h"
#include "Carla/Sensor/SceneCaptureSensor.h"
#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "CameraControl.h"

#include "CameraControllerActor.generated.h"

UCLASS()
class CAMERACONTROLLER_API ACameraControllerActor : public ASensor
{
    GENERATED_BODY()

public:

    ACameraControllerActor(const FObjectInitializer &ObjectInitializer);

    void Set(const FActorDescription &ActorDescription) override;

    void SetOwner(AActor *Owner) override;

    void PrePhysTick(float DeltaSeconds) override;

    void ApplyCameraControl(const FCameraControl& Control);

    void SetZoomValues(const FString& ZoomValuesString);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly)
    bool FirstTick = true;

    UPROPERTY(BlueprintReadOnly)
    FQuat CameraParentToCameraQuat;

    UPROPERTY(BlueprintReadOnly)
    FRotator CurrentRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly)
    FRotator TargetRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly)
    bool ReachedTargetRotation = true;

    UPROPERTY(BlueprintReadOnly)
    float OriginalFOV;

    UPROPERTY(BlueprintReadOnly)
    float TargetFOV;

    UPROPERTY(BlueprintReadOnly)
    bool ReachedTargetFOV = true;

public:

    UPROPERTY(BlueprintReadWrite)
    FString RosName;

    UPROPERTY(BlueprintReadWrite)
    float MinPanAngle = 0;

    UPROPERTY(BlueprintReadWrite)
    float MaxPanAngle = 0;

    UPROPERTY(BlueprintReadWrite)
    float MinTiltAngle = 0;

    UPROPERTY(BlueprintReadWrite)
    float MaxTiltAngle = 0;

    UPROPERTY(BlueprintReadWrite)
    float PanTiltSpeed = 0;

    UPROPERTY(BlueprintReadWrite)
    float ZoomSpeed = 0;

    UPROPERTY(BlueprintReadWrite)
    TArray<float> ZoomValues;
};
