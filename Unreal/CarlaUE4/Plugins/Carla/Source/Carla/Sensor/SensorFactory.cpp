// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SensorFactory.h"

#include "Carla/Sensor/Sensor.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/SensorRegistry.h>
#include <compiler/enable-ue4-macros.h>

#define LIBCARLA_SENSOR_REGISTRY_WITH_SENSOR_INCLUDES
#include <carla/sensor/SensorRegistry.h>
#undef LIBCARLA_SENSOR_REGISTRY_WITH_SENSOR_INCLUDES

class FSensorDefinitionGatherer
{
  using Registry = carla::sensor::SensorRegistry;

public:

  /// Retrieve the definitions of all the sensors registered in the
  /// SensorRegistry by calling their static method
  /// SensorType::GetSensorDefinition().
  static auto GetSensorDefinitions()
  {
    TArray<FActorDefinition> Definitions;
    Definitions.Reserve(Registry::size());
    AppendDefinitions(Definitions, std::make_index_sequence<Registry::size()>());
    return Definitions;
  }

private:

  template <size_t Index>
  static void AppendDefinitions(TArray<FActorDefinition> &Definitions)
  {
    using SensorPtrType = typename Registry::get_by_index<Index>::key;
    using SensorType = typename std::remove_pointer<SensorPtrType>::type;
    auto Def = SensorType::GetSensorDefinition();
    // Make sure the class matches the sensor type.
    Def.Class = SensorType::StaticClass();
    Definitions.Emplace(Def);
  }

  template <size_t... Is>
  static void AppendDefinitions(
      TArray<FActorDefinition> &Definitions,
      std::index_sequence<Is...>)
  {
    std::initializer_list<int> ({(AppendDefinitions<Is>(Definitions), 0)...});
  }
};

TArray<FActorDefinition> ASensorFactory::GetDefinitions()
{
  return FSensorDefinitionGatherer::GetSensorDefinitions();
}

FActorSpawnResult ASensorFactory::SpawnActor(
    const FTransform &Transform,
    const FActorDescription &Description)
{
  auto *World = GetWorld();
  if (World == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("ASensorFactory: cannot spawn sensor into empty world."));
    return {};
  }
  auto *Sensor = World->SpawnActorDeferred<ASensor>(
      Description.Class,
      Transform,
      this,
      nullptr,
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
  if (Sensor == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("ASensorFactory: spawn sensor failed."));
  }
  else
  {
    Sensor->Set(Description);
  }
  UGameplayStatics::FinishSpawningActor(Sensor, Transform);
  return FActorSpawnResult{Sensor};
}
