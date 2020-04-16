// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficLightManager.h"
#include "Game/CarlaStatics.h"
#include "StopSignComponent.h"
#include "YieldSignComponent.h"
#include "Components/BoxComponent.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/road/SignalType.h>
#include <compiler/enable-ue4-macros.h>

#include <string>

ATrafficLightManager::ATrafficLightManager()
{
  PrimaryActorTick.bCanEverTick = false;
  SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
  RootComponent = SceneComponent;

  // Hard codded default traffic light blueprint
  static ConstructorHelpers::FClassFinder<AActor> TrafficLightFinder(
      TEXT( "/Game/Carla/Blueprints/TrafficLight/BP_TLOpenDrive" ) );
  if (TrafficLightFinder.Succeeded())
  {
    TSubclassOf<AActor> Model = TrafficLightFinder.Class;
    TrafficLightModel = Model;
  }
  // Default traffic signs models
  static ConstructorHelpers::FClassFinder<AActor> StopFinder(
      TEXT( "/Game/Carla/Static/TrafficSigns/BP_Stop" ) );
  if (StopFinder.Succeeded())
  {
    TSubclassOf<AActor> StopSignModel = StopFinder.Class;
    TrafficSignsModels.Add(carla::road::SignalType::StopSign().c_str(), StopSignModel);
    SignComponentModels.Add(carla::road::SignalType::StopSign().c_str(), UStopSignComponent::StaticClass());
  }
  static ConstructorHelpers::FClassFinder<AActor> YieldFinder(
      TEXT( "/Game/Carla/Static/TrafficSigns/BP_Yield" ) );
  if (YieldFinder.Succeeded())
  {
    TSubclassOf<AActor> YieldSignModel = YieldFinder.Class;
    TrafficSignsModels.Add(carla::road::SignalType::YieldSign().c_str(), YieldSignModel);
    SignComponentModels.Add(carla::road::SignalType::YieldSign().c_str(), UYieldSignComponent::StaticClass());
  }
  LoneTrafficLightsGroupControllerId = -1;
}

void ATrafficLightManager::RegisterLightComponent(UTrafficLightComponent * TrafficLightComponent)
{
  // Cast to std::string
  carla::road::SignId SignId(TCHAR_TO_UTF8(*(TrafficLightComponent->GetSignId())));

  ATrafficLightGroup* TrafficLightGroup;
  UTrafficLightController* TrafficLightController;

  const auto &Signal = GetMap()->GetSignals().at(SignId);
  if(Signal->GetControllers().size())
  {
    // Only one controller per signal
    auto ControllerId = *(Signal->GetControllers().begin());

    // Get controller
    const auto &Controller = GetMap()->GetControllers().at(ControllerId);
    if(Controller->GetJunctions().empty())
    {
      UE_LOG(LogCarla, Error, TEXT("No junctions in controller %d"), *(ControllerId.c_str()) );
      return;
    }
    // Get junction of the controller
    auto JunctionId = *(Controller->GetJunctions().begin());

    // Search/create TrafficGroup (junction traffic light manager)
    if(!TrafficGroups.Contains(JunctionId))
    {
      auto * NewTrafficLightGroup =
          GetWorld()->SpawnActor<ATrafficLightGroup>();
      NewTrafficLightGroup->JunctionId = JunctionId;
      TrafficGroups.Add(JunctionId, NewTrafficLightGroup);
    }
    TrafficLightGroup = TrafficGroups[JunctionId];

    // Search/create controller in the junction
    if(!TrafficControllers.Contains(ControllerId.c_str()))
    {
      auto *NewTrafficLightController = NewObject<UTrafficLightController>();
      NewTrafficLightController->SetControllerId(ControllerId.c_str());
      TrafficLightGroup->GetControllers().Add(NewTrafficLightController);
      TrafficControllers.Add(ControllerId.c_str(), NewTrafficLightController);
    }
    TrafficLightController = TrafficControllers[ControllerId.c_str()];
  }
  else
  {
    auto * NewTrafficLightGroup =
          GetWorld()->SpawnActor<ATrafficLightGroup>();
    NewTrafficLightGroup->JunctionId = LoneTrafficLightsGroupControllerId;
    TrafficGroups.Add(NewTrafficLightGroup->JunctionId, NewTrafficLightGroup);
    TrafficLightGroup = NewTrafficLightGroup;

    auto *NewTrafficLightController = NewObject<UTrafficLightController>();
    NewTrafficLightController->SetControllerId(FString::FromInt(LoneTrafficLightsGroupControllerId));
    // Set red time longer than the default 2s
    NewTrafficLightController->SetRedTime(10);
    TrafficLightGroup->GetControllers().Add(NewTrafficLightController);
    TrafficControllers.Add(NewTrafficLightController->GetControllerId(), NewTrafficLightController);
    TrafficLightController = NewTrafficLightController;

    --LoneTrafficLightsGroupControllerId;
  }

  TrafficLightComponent->TrafficLightGroup = TrafficLightGroup;
  TrafficLightComponent->TrafficLightController = TrafficLightController;

  // Add signal to controller
  TrafficLightController->AddTrafficLight(TrafficLightComponent);
  TrafficLightController->ResetState();

  // Add signal to map
  TrafficSignComponents.Add(TrafficLightComponent->GetSignId(), TrafficLightComponent);

  TrafficLightGroup->ResetGroup();

}

const boost::optional<carla::road::Map>& ATrafficLightManager::GetMap()
{
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

void ATrafficLightManager::GenerateSignalsAndTrafficLights()
{
  if(!TrafficLightsGenerated)
  {
    if(!TrafficLightModel)
    {
      UE_LOG(LogCarla, Error, TEXT("Missing TrafficLightModel"));
      return;
    }

    RemoveRoadrunnerProps();

    SpawnTrafficLights();

    SpawnSignals();

    TrafficLightsGenerated = true;
  }
}

void ATrafficLightManager::RemoveGeneratedSignalsAndTrafficLights()
{
  for(auto& Sign : TrafficSigns)
  {
    Sign->Destroy();
  }
  TrafficSigns.Empty();

  for(auto& TrafficGroup : TrafficGroups)
  {
    TrafficGroup.Value->Destroy();
  }
  TrafficGroups.Empty();

  TrafficControllers.Empty();

  TrafficLightsGenerated = false;
}

// Called when the game starts
void ATrafficLightManager::BeginPlay()
{
  Super::BeginPlay();

  if (TrafficLightsGenerated)
  {
    ResetTrafficLightObjects();
  }
  else
  {
    GenerateSignalsAndTrafficLights();
  }

}

void ATrafficLightManager::ResetTrafficLightObjects()
{
  LoneTrafficLightsGroupControllerId = -1;
  // Update TrafficLightGroups
  for(auto& It : TrafficGroups)
  {
    ATrafficLightGroup* Group = It.Value;
    Group->GetControllers().Empty();
    Group->Destroy();
  }
  TrafficGroups.Empty();

  for(auto& It : TrafficControllers)
  {
    UTrafficLightController* Controller = It.Value;
    Controller->EmptyTrafficLights();
  }
  TrafficControllers.Empty();

  for (TActorIterator<ATrafficSignBase> It(GetWorld()); It; ++It)
  {
    ATrafficSignBase* trafficSignBase = (*It);
    UTrafficLightComponent* TrafficLightComponent =
      trafficSignBase->FindComponentByClass<UTrafficLightComponent>();

    if(TrafficLightComponent)
    {
      RegisterLightComponent(TrafficLightComponent);
    }
  }
}

void ATrafficLightManager::SpawnTrafficLights()
{
  std::unordered_set<std::string> SignalsToSpawn;
  for(const auto& ControllerPair : GetMap()->GetControllers())
  {
    const auto& Controller = ControllerPair.second;
    for(const auto& SignalId : Controller->GetSignals())
    {
      SignalsToSpawn.insert(SignalId);
    }
  }
  const auto& Signals = GetMap()->GetSignals();
  for(const auto& SignalPair : Signals)
  {
    const auto& SignalId = SignalPair.first;
    const auto& Signal = SignalPair.second;
    if(!Signal->GetControllers().size() &&
       !GetMap()->IsJunction(Signal->GetRoadId()) &&
       carla::road::SignalType::IsTrafficLight(Signal->GetType()) &&
       !SignalsToSpawn.count(SignalId))
    {
      SignalsToSpawn.insert(SignalId);
    }
  }
  for(auto &SignalId : SignalsToSpawn)
  {
    const auto& Signal = Signals.at(SignalId);
    auto CarlaTransform = Signal->GetTransform();
    FTransform SpawnTransform(CarlaTransform);

    FVector SpawnLocation = SpawnTransform.GetLocation();
    FRotator SpawnRotation(SpawnTransform.GetRotation());
    // Blueprints are all rotated by 90 degrees
    SpawnRotation.Yaw += 90;
    // Remove road inclination
    SpawnRotation.Roll = 0;
    SpawnRotation.Pitch = 0;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ATrafficLightBase * TrafficLight = GetWorld()->SpawnActor<ATrafficLightBase>(
        TrafficLightModel,
        SpawnLocation,
        SpawnRotation,
        SpawnParams);

    // Hack to prevent mixing ATrafficLightBase and UTrafficLightComponent logic
    TrafficLight->SetTimeIsFrozen(true);

    TrafficSigns.Add(TrafficLight);

    UTrafficLightComponent *TrafficLightComponent =
        NewObject<UTrafficLightComponent>(TrafficLight);
    TrafficLightComponent->SetSignId(SignalId.c_str());
    TrafficLightComponent->RegisterComponent();
    TrafficLightComponent->AttachToComponent(
        TrafficLight->GetRootComponent(),
        FAttachmentTransformRules::KeepRelativeTransform);

    auto ClosestWaypointToSignal =
        GetMap()->GetClosestWaypointOnRoad(CarlaTransform.location);
    if (ClosestWaypointToSignal)
    {
      auto SignalDistanceToRoad =
          (GetMap()->ComputeTransform(ClosestWaypointToSignal.get()).location - CarlaTransform.location).Length();
      double LaneWidth = GetMap()->GetLaneWidth(ClosestWaypointToSignal.get());

      if(SignalDistanceToRoad < LaneWidth * 0.5)
      {
        carla::log_warning("Traffic light",
            TCHAR_TO_UTF8(*TrafficLightComponent->GetSignId()),
            "overlaps a driving lane. Disabling collision...");

        TArray<UPrimitiveComponent*> Primitives;
        TrafficLight->GetComponents(Primitives);
        for (auto* Primitive : Primitives)
        {
          Primitive->SetCollisionProfileName(TEXT("NoCollision"));
        }
      }
    }

    RegisterLightComponent(TrafficLightComponent);
    TrafficLightComponent->InitializeSign(GetMap().get());
  }
}

void ATrafficLightManager::SpawnSignals()
{
  const auto &Signals = GetMap()->GetSignals();
  for (auto& SignalPair : Signals)
  {
    auto &Signal = SignalPair.second;
    FString SignalType = Signal->GetType().c_str();
    if (TrafficSignsModels.Contains(SignalType))
    {
      auto CarlaTransform = Signal->GetTransform();
      FTransform SpawnTransform(CarlaTransform);

      FVector SpawnLocation = SpawnTransform.GetLocation();
      FRotator SpawnRotation(SpawnTransform.GetRotation());
      SpawnRotation.Yaw += 90;
      // Remove road inclination
      SpawnRotation.Roll = 0;
      SpawnRotation.Pitch = 0;

      FActorSpawnParameters SpawnParams;
      SpawnParams.Owner = this;
      SpawnParams.SpawnCollisionHandlingOverride =
          ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
      ATrafficSignBase * TrafficSign = GetWorld()->SpawnActor<ATrafficSignBase>(
          TrafficSignsModels[SignalType],
          SpawnLocation,
          SpawnRotation,
          SpawnParams);

      USignComponent *SignComponent =
          NewObject<USignComponent>(TrafficSign, SignComponentModels[SignalType]);
      SignComponent->SetSignId(Signal->GetSignalId().c_str());
      SignComponent->RegisterComponent();
      SignComponent->AttachToComponent(
          TrafficSign->GetRootComponent(),
          FAttachmentTransformRules::KeepRelativeTransform);
      SignComponent->InitializeSign(GetMap().get());

      auto ClosestWaypointToSignal =
          GetMap()->GetClosestWaypointOnRoad(CarlaTransform.location);
      if (ClosestWaypointToSignal)
      {
        auto SignalDistanceToRoad =
            (GetMap()->ComputeTransform(ClosestWaypointToSignal.get()).location - CarlaTransform.location).Length();
        double LaneWidth = GetMap()->GetLaneWidth(ClosestWaypointToSignal.get());

        if(SignalDistanceToRoad < LaneWidth * 0.5)
        {
          carla::log_warning("Traffic sign",
              TCHAR_TO_UTF8(*SignComponent->GetSignId()),
              "overlaps a driving lane. Disabling collision...");

          TArray<UPrimitiveComponent*> Primitives;
          TrafficSign->GetComponents(Primitives);
          for (auto* Primitive : Primitives)
          {
            Primitive->SetCollisionProfileName(TEXT("NoCollision"));
          }
        }
      }
      TrafficSignComponents.Add(SignComponent->GetSignId(), SignComponent);
      TrafficSigns.Add(TrafficSign);
    }
  }
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

USignComponent* ATrafficLightManager::GetTrafficSign(FString SignId)
{
  if (!TrafficSignComponents.Contains(SignId))
  {
    return nullptr;
  }
  return TrafficSignComponents[SignId];
}

void ATrafficLightManager::RemoveRoadrunnerProps() const
{
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);

    // Detect PropsNode Actor which is the father of all the Props imported from Roadrunner
    AActor* PropsNode = nullptr;
    for(AActor* Actor : Actors)
    {
      const FString Name = UKismetSystemLibrary::GetDisplayName(Actor);
      if(Name.Equals("PropsNode"))
      {
        PropsNode = Actor;
        break;
      }
    }

    if(PropsNode)
    {
      PropsNode->GetAttachedActors(Actors, true);
      RemoveAttachedProps(Actors);
      PropsNode->Destroy();
    }

}

void ATrafficLightManager::RemoveAttachedProps(TArray<AActor*> Actors) const
{
  for(AActor* Actor : Actors)
  {
    TArray<AActor*> AttachedActors;
    Actor->GetAttachedActors(AttachedActors, true);
    RemoveAttachedProps(AttachedActors);
    Actor->Destroy();
  }
}
