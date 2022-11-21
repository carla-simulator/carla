// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SensorFactory.h"

#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Sensor/Sensor.h"
#include "Carla/Sensor/SceneCaptureSensor.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/SensorRegistry.h>
#include <compiler/enable-ue4-macros.h>

#define LIBCARLA_SENSOR_REGISTRY_WITH_SENSOR_INCLUDES
#include <carla/sensor/SensorRegistry.h>
#undef LIBCARLA_SENSOR_REGISTRY_WITH_SENSOR_INCLUDES

#include <type_traits>

// =============================================================================
// -- FSensorDefinitionGatherer ------------------------------------------------
// =============================================================================

/// Retrieve the definitions of all the sensors registered in the
/// SensorRegistry by calling their static method
/// SensorType::GetSensorDefinition().
///
/// @note To make this class ignore a given sensor, define a public member
/// "not_spawnable" that defines a type. If so, the sensor won't be spawned by
/// this factory.
class FSensorDefinitionGatherer
{
  using Registry = carla::sensor::SensorRegistry;

public:

  static auto GetSensorDefinitions()
  {
    TArray<FActorDefinition> Definitions;
    Definitions.Reserve(Registry::size());
    AppendDefinitions(Definitions, std::make_index_sequence<Registry::size()>());
    return Definitions;
  }

private:

  // Type traits for detecting if a sensor is spawnable.

  template<typename T>
  struct void_type { typedef void type; };

  template<typename T, typename = void>
  struct is_spawnable : std::true_type {};

  template<typename T>
  struct is_spawnable<T, typename void_type<typename T::not_spawnable>::type > : std::false_type {};

  // AppendDefinitions implementations.

  template <typename SensorType>
  static typename std::enable_if<is_spawnable<SensorType>::value, void>::type
  AppendDefinitions(TArray<FActorDefinition> &Definitions)
  {
    auto Def = SensorType::GetSensorDefinition();
    // Make sure the class matches the sensor type.
    Def.Class = SensorType::StaticClass();
    Definitions.Emplace(Def);
  }

  template <typename SensorType>
  static typename std::enable_if<!is_spawnable<SensorType>::value, void>::type
  AppendDefinitions(TArray<FActorDefinition> &) {}

  template <size_t Index>
  static void AppendDefinitions(TArray<FActorDefinition> &Definitions)
  {
    using SensorPtrType = typename Registry::get_by_index<Index>::key;
    using SensorType = typename std::remove_pointer<SensorPtrType>::type;
    AppendDefinitions<SensorType>(Definitions);
  }

  template <size_t... Is>
  static void AppendDefinitions(
      TArray<FActorDefinition> &Definitions,
      std::index_sequence<Is...>)
  {
    std::initializer_list<int> ({(AppendDefinitions<Is>(Definitions), 0)...});
  }
};

// =============================================================================
// -- ASensorFactory -----------------------------------------------------------
// =============================================================================

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
    UE_LOG(LogCarla, Error, TEXT("ASensorFactory: cannot spawn sensor into an empty world."));
    return {};
  }

  UCarlaGameInstance *GameInstance = UCarlaStatics::GetGameInstance(World);
  if (GameInstance == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("ASensorFactory: cannot spawn sensor, incompatible game instance."));
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
    auto *Episode = GameInstance->GetCarlaEpisode();
    check(Episode != nullptr);
    
    Sensor->SetEpisode(*Episode);
    Sensor->Set(Description);
    Sensor->SetDataStream(GameInstance->GetServer().OpenStream());
    ASceneCaptureSensor * SceneCaptureSensor = Cast<ASceneCaptureSensor>(Sensor);
    if(SceneCaptureSensor)
    {
      SceneCaptureSensor->CameraGBuffers.SceneColor.SetDataStream(GameInstance->GetServer().OpenStream());
      SceneCaptureSensor->CameraGBuffers.SceneDepth.SetDataStream(GameInstance->GetServer().OpenStream());
      SceneCaptureSensor->CameraGBuffers.SceneStencil.SetDataStream(GameInstance->GetServer().OpenStream());
      SceneCaptureSensor->CameraGBuffers.GBufferA.SetDataStream(GameInstance->GetServer().OpenStream());
      SceneCaptureSensor->CameraGBuffers.GBufferB.SetDataStream(GameInstance->GetServer().OpenStream());
      SceneCaptureSensor->CameraGBuffers.GBufferC.SetDataStream(GameInstance->GetServer().OpenStream());
      SceneCaptureSensor->CameraGBuffers.GBufferD.SetDataStream(GameInstance->GetServer().OpenStream());
      SceneCaptureSensor->CameraGBuffers.GBufferE.SetDataStream(GameInstance->GetServer().OpenStream());
      SceneCaptureSensor->CameraGBuffers.GBufferF.SetDataStream(GameInstance->GetServer().OpenStream());
      SceneCaptureSensor->CameraGBuffers.Velocity.SetDataStream(GameInstance->GetServer().OpenStream());
      SceneCaptureSensor->CameraGBuffers.SSAO.SetDataStream(GameInstance->GetServer().OpenStream());
      SceneCaptureSensor->CameraGBuffers.CustomDepth.SetDataStream(GameInstance->GetServer().OpenStream());
      SceneCaptureSensor->CameraGBuffers.CustomStencil.SetDataStream(GameInstance->GetServer().OpenStream());
    }
  }
  UGameplayStatics::FinishSpawningActor(Sensor, Transform);
  return FActorSpawnResult{Sensor};
}
