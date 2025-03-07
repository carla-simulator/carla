#include "CameraControllerActor.h"
#include "CameraControllerBlueprintLibrary.h"
#include "CameraController/ros2/ROS2CameraControl.h"

#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetMathLibrary.h"

ACameraControllerActor::ACameraControllerActor(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
// Add code that is required on startup
  PrimaryActorTick.bCanEverTick = true;
}

void ACameraControllerActor::BeginPlay()
{
    Super::BeginPlay();
}

void ACameraControllerActor::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  UCameraControllerBlueprintLibrary::SetCameraController(ActorDescription, this);
}

void ACameraControllerActor::SetOwner(AActor* OwningActor)
{
  Super::SetOwner(OwningActor);
}

// ROS Callback function to control the camera
void ACameraControllerActor::ApplyCameraControl(const FCameraControl& Control)
{
  // Invert yaw (pan) and pitch (tilt) since Unreal uses the left-handed rule with Z-up convention.
  // Then convert to degrees.
  float Pan = -Control.Pan * 180.0 / PI;
  float Tilt = -Control.Tilt * 180.0 / PI;

  if (Pan >= MinPanAngle && Pan <= MaxPanAngle)
    {
      TargetRotation.Yaw = Pan;
      ReachedTargetRotation = false;
      UE_LOG(LogCameraController, Log, TEXT("Pan Command '%f' deg has been set!"), Pan);
    }

    if (Tilt >= MinTiltAngle && Tilt <= MaxTiltAngle)
    {
      TargetRotation.Pitch = Tilt;
      ReachedTargetRotation = false;
      UE_LOG(LogCameraController, Log, TEXT("Tilt Command '%f' deg has been set!"), Tilt);
    }

    if (ZoomValues.Contains(Control.Zoom))
    {
      TargetFOV = OriginalFOV / Control.Zoom;
      ReachedTargetFOV = false;
      UE_LOG(LogCameraController, Log, TEXT("Zoom Command '%f' deg has been set!"), Control.Zoom);
    }
}

// Takes a string (ex."1,2,5") containing zoom values and creates an array of floats (ex. [1.0, 2.0, 5.0])
void ACameraControllerActor::SetZoomValues(const FString& ZoomValuesString)
{
  ZoomValues.Empty();

  TArray<FString> stringParts;
  ZoomValuesString.ParseIntoArray(stringParts, TEXT(","));

  for (const FString& part : stringParts)
  {
    float value = FCString::Atof(*part);
    ZoomValues.Add(value);
  }
}

void ACameraControllerActor::PrePhysTick(float DeltaSeconds){

  ASceneCaptureSensor* CameraActor = Cast<ASceneCaptureSensor>(GetAttachParentActor());
  static int throttle_cntr = 0;
  if (CameraActor == nullptr)
  {
    // Only print the warning the first 10 times so it doesn't spam the terminal
    if (throttle_cntr < 10)
    {
      UE_LOG(LogCameraController, Warning, TEXT("No compatible camera was found for the '%s' actor to control."), *RosName);
      throttle_cntr++;
    }
    return;
  }

  float CurrentFOV = CameraActor->GetFOVAngle();

  if (FirstTick)
  {
    OriginalFOV = CurrentFOV;
    FTransform CameraParentToCameraTransform;
    AActor* CameraParentActor = CameraActor->GetAttachParentActor();

    if (CameraParentActor)
    {
      UE_LOG(LogCameraController, Log, TEXT("Camera has an attached parent actor. Getting relative transform..."));
      CameraParentToCameraTransform = CameraActor->GetActorTransform().GetRelativeTransform(CameraParentActor->GetActorTransform());
    }
    else
    {
      UE_LOG(LogCameraController, Log, TEXT("Camera does not have an attached parent actor. Using its own transform relative to world..."));
      CameraParentToCameraTransform = CameraActor->GetActorTransform();
    }

    CameraParentToCameraQuat = CameraParentToCameraTransform.GetRotation();

    FirstTick = false;
  }

  float CurrentZoom = OriginalFOV / CurrentFOV;

  if (!ReachedTargetRotation)
  {
    if (!CurrentRotation.Equals(TargetRotation, 0.01))
    {
      FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaSeconds, PanTiltSpeed);
      FQuat NewQuat = NewRotation.Quaternion();
      CameraActor->SetActorRelativeRotation(CameraParentToCameraQuat * NewQuat);
      CurrentRotation = NewRotation;
    }
    else
    {
      ReachedTargetRotation = true;
      UE_LOG(LogCameraController, Log, TEXT("Reached Target Rotation!"));
    }
  }

  if (!ReachedTargetFOV)
  {
    if (CurrentFOV != TargetFOV)
    {
      float NewFOV = FMath::FInterpConstantTo(CurrentFOV, TargetFOV, DeltaSeconds, ZoomSpeed);
      CameraActor->SetFOVAngle(NewFOV);
    }
    else
    {
      ReachedTargetFOV = true;
      UE_LOG(LogCameraController, Log, TEXT("Reached Target Zoom"));
    }
  }

  auto ROS2CameraControl = carla::ros2::ROS2CameraControl::GetInstance();
  auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();

  FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(CameraActor->GetActorTransform());
  ROS2CameraControl->ProcessDataFromCameraControl(StreamId, LocalTransformRelativeToParent, CurrentRotation.Yaw, CurrentRotation.Pitch, CurrentZoom, this);
}
