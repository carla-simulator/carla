// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficLightManager.h"
#include "Game/CarlaStatics.h"
#include <string>

ATrafficLightManager::ATrafficLightManager()
{
  PrimaryActorTick.bCanEverTick = false;
  SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
  RootComponent = SceneComponent;

  static ConstructorHelpers::FObjectFinder<UBlueprint> TrafficLightFinder( TEXT( "Blueprint'/Game/Carla/Blueprints/Test.Test'" ) );
  TrafficLightModel = TrafficLightFinder.Object->GeneratedClass;
}

void ATrafficLightManager::RegisterLightComponent(UTrafficLightComponent * TrafficLight)
{
  // Cast to std::string
  carla::road::SignId SignId(TCHAR_TO_UTF8(*(TrafficLight->GetSignId())));

  // Get OpenDRIVE signal
  if (GetMap()->GetSignals().count(SignId) == 0)
  {
    carla::log_warning("Error: missing signal with id:", SignId);
    return;
  }
  const auto &Signal = GetMap()->GetSignals().at(SignId);
  if(Signal->GetControllers().empty())
  {
    carla::log_warning("Error: no controllers in signal", SignId);
    return;
  }
  // Only one controller per signal
  auto ControllerId = *(Signal->GetControllers().begin());

  // Get controller
  const auto &Controller = GetMap()->GetControllers().at(ControllerId);
  if(Controller->GetJunctions().empty())
  {
    carla::log_warning("Error: no junctions in controller", ControllerId);
    return;
  }
  // Get junction of the controller
  auto JunctionId = *(Controller->GetJunctions().begin());

  // Search/create TrafficGroup (junction traffic light manager)
  if(!TrafficGroups.Contains(JunctionId))
  {
    auto * TrafficLightGroup =
        GetWorld()->SpawnActor<ATrafficLightGroup>();
    TrafficLightGroup->JunctionId = JunctionId;
    TrafficGroups.Add(JunctionId, TrafficLightGroup);
    carla::log_warning("Spawn TrafficLight Group");
  }
  auto * TrafficLightGroup = TrafficGroups[JunctionId];

  // Search/create controller in the junction
  if(!TrafficControllers.Contains(ControllerId.c_str()))
  {
    auto *TrafficLightController = NewObject<UTrafficLightController>();
    TrafficLightController->SetControllerId(ControllerId.c_str());
    TrafficLightGroup->GetControllers().Add(TrafficLightController);
    TrafficControllers.Add(ControllerId.c_str(), TrafficLightController);
    carla::log_warning("Created Controller");
  }
  auto *TrafficLightController = TrafficControllers[ControllerId.c_str()];

  TrafficLight->TrafficLightGroup = TrafficLightGroup;

  // Add signal to controller
  TrafficLightController->AddTrafficLight(TrafficLight);
  TrafficLightController->ResetState();

  // Add signal to map
  TrafficLights.Add(TrafficLight->GetSignId(), TrafficLight);
}

const boost::optional<carla::road::Map>& ATrafficLightManager::GetMap()
{
  // if(!GameMode) {
  //   GameMode = UCarlaStatics::GetGameMode(this);
  // }
  // return GameMode->GetMap();
  if (!Map.has_value())
  {
    FString MapName = GetWorld()->GetName();
    std::string opendrive_xml = carla::rpc::FromFString(UOpenDrive::LoadXODR(MapName));
    Map = carla::opendrive::OpenDriveParser::Load(opendrive_xml);
    if (!Map.has_value()) {
      UE_LOG(LogCarla, Error, TEXT("Invalid Map"));
    }
  }
  return Map;
}

void ATrafficLightManager::GenerateTrafficLights()
{
  if(!TrafficLightModel)
  {
    carla::log_warning("Error: no traffic light model.");
    return;
  }
  const auto& Signals = GetMap()->GetSignals();
  for(const auto& ControllerPair : GetMap()->GetControllers())
  {
    const auto& Controller = ControllerPair.second;
    for(const auto& SignalId : Controller->GetSignals())
    {
      const auto& Signal = Signals.at(SignalId);
      auto CarlaTransform = Signal->GetTransform();
      FTransform SpawnTransform(CarlaTransform);
      carla::log_warning("Creating Signal", SignalId);

      FActorSpawnParameters SpawnParams;
      SpawnParams.Owner = this;
      SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
      AActor * TrafficLight = GetWorld()->SpawnActor<AActor>(TrafficLightModel, SpawnTransform.GetLocation(), FRotator(SpawnTransform.GetRotation()), SpawnParams);

      carla::log_warning("Actor Spawned");
      UTrafficLightComponent *TrafficLightComponent = NewObject<UTrafficLightComponent>(TrafficLight);
      TrafficLightComponent->SetSignId(SignalId.c_str());
      TrafficLightComponent->RegisterComponent();
      TrafficLightComponent->AttachToComponent(TrafficLight->GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
      carla::log_warning("Component Created");
    }
  }
}

// Called when the game starts
void ATrafficLightManager::BeginPlay()
{
  Super::BeginPlay();

}

ATrafficLightGroup* ATrafficLightManager::GetTrafficGroup(carla::road::JuncId JunctionId)
{
  if (TrafficGroups.Contains(JunctionId))
  {
    return TrafficGroups[JunctionId];
  }
  return nullptr;
}


UTrafficLightController* ATrafficLightManager::GetController(FString ControllerId)
{
  if (TrafficControllers.Contains(ControllerId))
  {
    return TrafficControllers[ControllerId];
  }
  return nullptr;
}

UTrafficLightComponent* ATrafficLightManager::GetTrafficLight(FString SignId)
{
  if (!TrafficLights.Contains(SignId))
  {
    return nullptr;
  }
  return TrafficLights[SignId];
}