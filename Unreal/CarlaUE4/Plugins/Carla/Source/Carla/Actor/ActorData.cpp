// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ActorData.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Traffic/SignComponent.h"
#include "Carla/Traffic/TrafficLightComponent.h"
#include "Carla/Traffic/TrafficLightController.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Carla/Walker/WalkerController.h"
#include "Carla/Walker/WalkerBase.h"
#include "Carla/Sensor/Sensor.h"
#include "CarlaActor.h"

AActor* FActorData::RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info)
{
  FTransform SpawnTransform = GetLocalTransform(CarlaEpisode);
  // hack to prevent wheel intersection with geometry
  SpawnTransform.AddToTranslation(FVector(0,0,15));
  return CarlaEpisode->ReSpawnActorWithInfo(SpawnTransform, Info.Description);
}

void FActorData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  AActor* Actor = CarlaActor->GetActor();
  FTransform Transform = Actor->GetTransform();
  Location = FDVector(Transform.GetLocation()) + CarlaEpisode->GetCurrentMapOrigin();
  Rotation = Transform.GetRotation();
  Scale = Transform.GetScale3D();
  UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
  if (Component)
  {
    bSimulatePhysics = Component->IsSimulatingPhysics();
    AngularVelocity = Component->GetPhysicsAngularVelocityInDegrees();
  }
  Velocity = Actor->GetVelocity();
}

void FActorData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  AActor* Actor = CarlaActor->GetActor();
  Actor->SetActorTransform(GetLocalTransform(CarlaEpisode));
  UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
  if (Component)
  {
    Component->SetPhysicsLinearVelocity(
        Velocity, false, "None");
    Component->SetPhysicsAngularVelocityInDegrees(
        AngularVelocity, false, "None");
  }
  auto* Character = Cast<ACharacter>(Actor);
  auto* CarlaVehicle = Cast<ACarlaWheeledVehicle>(Actor);
  // The physics in the vehicles works in a different way so to disable them.
  if (CarlaVehicle != nullptr){
    CarlaVehicle->SetSimulatePhysics(bSimulatePhysics);
  }
  // The physics in the walkers also works in a different way so to disable them,
  // we need to do it in the UCharacterMovementComponent.
  else if (Character != nullptr)
  {
    auto CharacterMovement = Cast<UCharacterMovementComponent>(Character->GetCharacterMovement());

    if(bSimulatePhysics) {
      CharacterMovement->SetDefaultMovementMode();
    }
    else {
      CharacterMovement->DisableMovement();
    }
  }
  // In the rest of actors, the physics is controlled with the UPrimitiveComponent, so we use
  // that for disable it.
  else
  {
    auto RootComponent = Component;
    if (RootComponent != nullptr)
    {
      RootComponent->SetSimulatePhysics(bSimulatePhysics);
      RootComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
  }
}

FTransform FActorData::GetLocalTransform(UCarlaEpisode* CarlaEpisode) const
{
  FVector LocalLocation = (Location - CarlaEpisode->GetCurrentMapOrigin()).ToFVector();
  return FTransform(Rotation, LocalLocation, Scale);
}

void FVehicleData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
  if (bSimulatePhysics)
  {
    PhysicsControl = Vehicle->GetVehiclePhysicsControl();
  }
  Control = Vehicle->GetVehicleControl();
  LightState = Vehicle->GetVehicleLightState();
  auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
  if (Controller)
  {
    SpeedLimit = Controller->GetSpeedLimit();
  }
}

void FVehicleData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
  Vehicle->SetSimulatePhysics(bSimulatePhysics);
  if (bSimulatePhysics)
  {
    Vehicle->ApplyVehiclePhysicsControl(PhysicsControl);
  }
  Vehicle->ApplyVehicleControl(Control, EVehicleInputPriority::Client);
  Vehicle->SetVehicleLightState(LightState);
  auto Controller = Cast<AWheeledVehicleAIController>(Vehicle->GetController());
  if (Controller)
  {
    Controller->SetSpeedLimit(SpeedLimit);
  }
}

void FWalkerData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  auto Walker = Cast<APawn>(Actor);
  auto Controller = Walker != nullptr ? Cast<AWalkerController>(Walker->GetController()) : nullptr;
  if (Controller != nullptr)
  {
    WalkerControl = carla::rpc::WalkerControl{Controller->GetWalkerControl()};
  }
}

void FWalkerData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  auto Walker = Cast<APawn>(Actor);
  auto Controller = Walker != nullptr ? Cast<AWalkerController>(Walker->GetController()) : nullptr;
  if (Controller != nullptr)
  {
    Controller->ApplyWalkerControl(WalkerControl);
  }
}

AActor* FTrafficSignData::RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info)
{
  FTransform SpawnTransform = GetLocalTransform(CarlaEpisode);
  FActorSpawnParameters SpawnParams;
  SpawnParams.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  return CarlaEpisode->GetWorld()->SpawnActor<ATrafficSignBase>(
        Model,
        SpawnTransform,
        SpawnParams);
}

void FTrafficSignData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  Model = Actor->GetClass();
  ATrafficSignBase* TrafficSign = Cast<ATrafficSignBase>(Actor);
  USignComponent* TrafficSignComponent =
        Cast<USignComponent>(TrafficSign->FindComponentByClass<USignComponent>());
  if (TrafficSignComponent)
  {
    SignModel = TrafficSignComponent->GetClass();
    SignId = TrafficSignComponent->GetSignId();
  }
}

void FTrafficSignData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  AActor* Actor = CarlaActor->GetActor();
  if (SignId.Len())
  {
    USignComponent* SignComponent =
        NewObject<USignComponent>(Actor, SignModel);
    SignComponent->SetSignId(SignId);
    SignComponent->RegisterComponent();
    SignComponent->AttachToComponent(
        Actor->GetRootComponent(),
        FAttachmentTransformRules::KeepRelativeTransform);
    ACarlaGameModeBase *GameMode = UCarlaStatics::GetGameMode(CarlaEpisode->GetWorld());
    SignComponent->InitializeSign(GameMode->GetMap().get());
  }
}

AActor* FTrafficLightData::RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info)
{
  FTransform SpawnTransform = GetLocalTransform(CarlaEpisode);
  FActorSpawnParameters SpawnParams;
  SpawnParams.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  return CarlaEpisode->GetWorld()->SpawnActor<ATrafficLightBase>(
        Model,
        SpawnTransform,
        SpawnParams);
}

void FTrafficLightData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  Model = Actor->GetClass();
  ATrafficLightBase* TrafficLight = Cast<ATrafficLightBase>(Actor);
  UTrafficLightComponent* Component = TrafficLight->GetTrafficLightComponent();
  SignId = Component->GetSignId();
  Controller = Component->GetController();
  Controller->RemoveTrafficLight(Component);
  Controller->AddCarlaActorTrafficLight(CarlaActor);
  LightState = TrafficLight->GetTrafficLightState();
  PoleIndex = TrafficLight->GetPoleIndex();
}

void FTrafficLightData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  AActor* Actor = CarlaActor->GetActor();
  ATrafficLightBase* TrafficLight = Cast<ATrafficLightBase>(Actor);
  UTrafficLightComponent* Component = TrafficLight->GetTrafficLightComponent();
  Component->SetSignId(SignId);
  Controller->RemoveCarlaActorTrafficLight(CarlaActor);
  Controller->AddTrafficLight(Component);
  ACarlaGameModeBase *GameMode = UCarlaStatics::GetGameMode(CarlaEpisode->GetWorld());
  Component->InitializeSign(GameMode->GetMap().get());
  Component->SetLightState(Controller->GetCurrentState().State);
  TrafficLight->SetPoleIndex(PoleIndex);
  TrafficLight->SetTrafficLightState(LightState);
}

void FActorSensorData::RecordActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  ASensor* Sensor = Cast<ASensor>(Actor);
  Stream = Sensor->MoveDataStream();
}

void FActorSensorData::RestoreActorData(FCarlaActor* CarlaActor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(CarlaActor, CarlaEpisode);
  AActor* Actor = CarlaActor->GetActor();
  ASensor* Sensor = Cast<ASensor>(Actor);
  Sensor->SetDataStream(std::move(Stream));
}
