#pragma once

#include "CameraControllerBlueprintLibrary.h"
#include "CameraControllerActor.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla.h"
#include "Carla/Server/CarlaServer.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Sensor/Sensor.h"

#if WITH_EDITOR
#  define CARLA_ABFL_CHECK_ACTOR(ActorPtr)                    \
  if (!IsValid(ActorPtr))     \
  {                                                           \
    UE_LOG(LogCameraController, Error, TEXT("Cannot set empty actor!")); \
    return;                                                   \
  }
#else
#  define CARLA_ABFL_CHECK_ACTOR(ActorPtr) \
  IsValid(ActorPtr);
#endif // WITH_EDITOR

FActorDefinition UCameraControllerBlueprintLibrary::MakeCameraControllerDefinition(TSubclassOf<ACameraControllerActor> Class)
{
  FActorDefinition Definition;
  bool Success;
  MakeCameraControllerDefinition(Success, Definition, Class);
  check(Success);
  return Definition;
}

template <typename ... ARGS>
static FString JoinStrings(const FString &Separator, ARGS && ... Args)
{
  return FString::Join(TArray<FString>{std::forward<ARGS>(Args) ...}, *Separator);
}

template <typename ... TStrs>
static void FillIdAndTags(FActorDefinition &Def, TStrs && ... Strings)
{
  Def.Id = JoinStrings(TEXT("."), std::forward<TStrs>(Strings) ...).ToLower();
  Def.Tags = JoinStrings(TEXT(","), std::forward<TStrs>(Strings) ...).ToLower();

  // each actor gets an actor role name attribute (empty by default)
  FActorVariation ActorRole;
  ActorRole.Id = TEXT("role_name");
  ActorRole.Type = EActorAttributeType::String;
  ActorRole.RecommendedValues = { TEXT("default") };
  ActorRole.bRestrictToRecommended = false;
  Def.Variations.Emplace(ActorRole);

  // ROS2
  FActorVariation Var;
  Var.Id = TEXT("ros_name");
  Var.Type = EActorAttributeType::String;
  Var.RecommendedValues = { Def.Id };
  Var.bRestrictToRecommended = false;
  Def.Variations.Emplace(Var);
}

void UCameraControllerBlueprintLibrary::MakeCameraControllerDefinition(
    bool &Success,
    FActorDefinition &Definition,
    TSubclassOf<ACameraControllerActor> Class)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("other"), TEXT("camera_controller"));

  Definition.Class = Class;

  FActorVariation Tick;
  Tick.Id = TEXT("sensor_tick");
  Tick.Type = EActorAttributeType::Float;
  Tick.RecommendedValues = { TEXT("0.05") };
  Tick.bRestrictToRecommended = false;

  FActorVariation MinPanAngle;
  MinPanAngle.Id = TEXT("min_pan_angle");
  MinPanAngle.Type = EActorAttributeType::Float;
  MinPanAngle.RecommendedValues = { TEXT("-60") };
  MinPanAngle.bRestrictToRecommended = false;

  FActorVariation MaxPanAngle;
  MaxPanAngle.Id = TEXT("max_pan_angle");
  MaxPanAngle.Type = EActorAttributeType::Float;
  MaxPanAngle.RecommendedValues = { TEXT("60") };
  MaxPanAngle.bRestrictToRecommended = false;

  FActorVariation MinTiltAngle;
  MinTiltAngle.Id = TEXT("min_tilt_angle");
  MinTiltAngle.Type = EActorAttributeType::Float;
  MinTiltAngle.RecommendedValues = { TEXT("-60") };
  MinTiltAngle.bRestrictToRecommended = false;

  FActorVariation MaxTiltAngle;
  MaxTiltAngle.Id = TEXT("max_tilt_angle");
  MaxTiltAngle.Type = EActorAttributeType::Float;
  MaxTiltAngle.RecommendedValues = { TEXT("60") };
  MaxTiltAngle.bRestrictToRecommended = false;

  FActorVariation PanTiltSpeed;
  PanTiltSpeed.Id = TEXT("pan_tilt_speed"); // in deg/s
  PanTiltSpeed.Type = EActorAttributeType::Float;
  PanTiltSpeed.RecommendedValues = { TEXT("180.0") };
  PanTiltSpeed.bRestrictToRecommended = false;

  FActorVariation ZoomSpeed;
  ZoomSpeed.Id = TEXT("zoom_speed"); // in fov deg/sec
  ZoomSpeed.Type = EActorAttributeType::Float;
  ZoomSpeed.RecommendedValues = { TEXT("80.0") };
  ZoomSpeed.bRestrictToRecommended = false;

  FActorVariation ZoomValues;
  ZoomValues.Id = TEXT("zoom_values");
  ZoomValues.Type = EActorAttributeType::String;
  ZoomValues.RecommendedValues = { TEXT("1,2,5") };
  ZoomValues.bRestrictToRecommended = false;

  Definition.Variations.Append({
    Tick,
    MinPanAngle,
    MaxPanAngle,
    MinTiltAngle,
    MaxTiltAngle,
    PanTiltSpeed,
    ZoomSpeed,
    ZoomValues
  });

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("sensor_type"),
    EActorAttributeType::String,
    "camera_control"});

  Success = UActorBlueprintFunctionLibrary::CheckActorDefinition(Definition);
}

void UCameraControllerBlueprintLibrary::SetCameraController(
    const FActorDescription &Description,
    ACameraControllerActor *CameraController)
{
  CARLA_ABFL_CHECK_ACTOR(CameraController);

  CameraController->RosName = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToString("ros_name", Description.Variations, "");
  CameraController->MinPanAngle = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("min_pan_angle", Description.Variations, -60.0f);
  CameraController->MaxPanAngle = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("max_pan_angle", Description.Variations, 60.0f);
  CameraController->MinTiltAngle = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("min_tilt_angle", Description.Variations, -60.0f);
  CameraController->MaxTiltAngle = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("max_tilt_angle", Description.Variations, 60.0f);
  CameraController->PanTiltSpeed = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("pan_tilt_speed", Description.Variations, 180.0f);
  CameraController->ZoomSpeed = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("zoom_speed", Description.Variations, 80.0f);

  FString ZoomValuesString = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToString("zoom_values", Description.Variations, "");
  CameraController->SetZoomValues(ZoomValuesString);
}