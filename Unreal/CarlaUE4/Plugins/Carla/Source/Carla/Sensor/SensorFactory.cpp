// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SensorFactory.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Sensor/SceneCaptureCamera.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/SensorRegistry.h>
#include <compiler/enable-ue4-macros.h>
#define LIBCARLA_SENSOR_REGISTRY_WITH_SENSOR_INCLUDES
#include <carla/sensor/SensorRegistry.h>
#undef LIBCARLA_SENSOR_REGISTRY_WITH_SENSOR_INCLUDES

/// Call GetSensorDefinition() on each sensor registered in the SensorRegistry.
class FSensorDefinitionGatherer
{
  using Registry = carla::sensor::SensorRegistry;

  template <size_t Index>
  static void Append_impl(TArray<FActorDefinition> &Definitions)
  {
    using SensorPtrType = typename Registry::get_by_index<Index>::key;
    using SensorType = typename std::remove_pointer<SensorPtrType>::type;
    auto Def = SensorType::GetSensorDefinition();
    Def.Class = SensorType::StaticClass();
    Definitions.Add(Def);
  }

  template <size_t... Is>
  static void Append_impl(
      TArray<FActorDefinition> &Definitions,
      std::index_sequence<Is...>)
  {
    std::initializer_list<int> ({(Append_impl<Is>(Definitions), 0)...});
  }

public:

  static void AppendSensorDefinitions(TArray<FActorDefinition> &Definitions)
  {
    Append_impl(Definitions, std::make_index_sequence<Registry::size()>());
  }
};

TArray<FActorDefinition> ASensorFactory::GetDefinitions()
{
  TArray<FActorDefinition> Definitions;
  FSensorDefinitionGatherer::AppendSensorDefinitions(Definitions);
  return Definitions;
}

FActorSpawnResult ASensorFactory::SpawnActor(
    const FTransform &Transform,
    const FActorDescription &Description)
{
  FActorSpawnParameters Params;
  auto *World = GetWorld();
  if (World == nullptr)
  {
    return {};
  }
  /// @todo Here we may spawn something else than a scene capture.
  auto *Sensor = World->SpawnActorDeferred<ASceneCaptureSensor>(
      Description.Class,
      Transform,
      this,
      nullptr,
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if (Sensor != nullptr)
  {
    using ABFL = UActorBlueprintFunctionLibrary;
    Sensor->SetImageSize(
        ABFL::RetrieveActorAttributeToInt("image_size_x", Description.Variations, 800),
        ABFL::RetrieveActorAttributeToInt("image_size_y", Description.Variations, 600));
    Sensor->SetFOVAngle(
        ABFL::RetrieveActorAttributeToFloat("fov", Description.Variations, 90.0f));
  }
  UGameplayStatics::FinishSpawningActor(Sensor, Transform);
  return FActorSpawnResult{Sensor};
}
