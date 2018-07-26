// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SensorSpawner.h"

#include "Carla/Sensor/SceneCaptureCamera.h"

template <typename T>
static FActorDefinition MakeSensorDefinition(const FString &Id)
{
  FActorDefinition Definition;
  Definition.Id = Id;
  Definition.Class = T::StaticClass();
  Definition.Tags = TEXT("sensor,") + Id;
  return Definition;
}

TArray<FActorDefinition> ASensorSpawner::MakeDefinitions()
{
  // Cameras.
  auto Cameras = MakeSensorDefinition<ASceneCaptureCamera>("camera");
  {
    FActorVariation PostProcessing;
    PostProcessing.Id = TEXT("PostProcessing");
    PostProcessing.Type = EActorAttributeType::String;
    PostProcessing.RecommendedValues = {
        TEXT("None"),
        TEXT("SceneFinal"),
        TEXT("Depth"),
        TEXT("SemanticSegmentation")
    };
    PostProcessing.bRestrictToRecommended = true;
    FActorVariation FOV;
    FOV.Id = TEXT("FOV");
    FOV.Type = EActorAttributeType::Float;
    FOV.RecommendedValues = { TEXT("90.0") };
    FOV.bRestrictToRecommended = false;
    FActorVariation ResX;
    ResX.Id = TEXT("ImageSizeX");
    ResX.Type = EActorAttributeType::Int;
    ResX.RecommendedValues = { TEXT("800") };
    ResX.bRestrictToRecommended = false;
    FActorVariation ResY;
    ResY.Id = TEXT("ImageSizeY");
    ResY.Type = EActorAttributeType::Int;
    ResY.RecommendedValues = { TEXT("600") };
    ResY.bRestrictToRecommended = false;
    Cameras.Variations = {PostProcessing, ResX, ResY, FOV};
  }
  return {Cameras};
}

FActorSpawnResult ASensorSpawner::SpawnActor(
    const FTransform &Transform,
    const FActorDescription &Description)
{
  FActorSpawnParameters Params;
  static int32 COUNTER = 0u;
  Params.Name = FName(*(Description.Id + FString::FromInt(++COUNTER)));
  auto *World = GetWorld();
  if (World == nullptr)
  {
    return {};
  }
  auto *Sensor = World->SpawnActor<ASensor>(Description.Class, Transform, Params);
  /// @todo Set description: Actor->Set(Description);
  return FActorSpawnResult{Sensor};
}
