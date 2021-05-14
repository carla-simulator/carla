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
#include "Carla/Walker/WalkerBase.h"

AActor* FActorData::RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info)
{
  FTransform SpawnTransform = GetLocalTransform(CarlaEpisode);
  // hack to prevent wheel intersection with geometry
  SpawnTransform.AddToTranslation(FVector(0,0,15));
  return CarlaEpisode->ReSpawnActorWithInfo(SpawnTransform, Info.Description);
}

void FActorData::RecordActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode)
{
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

void FActorData::RestoreActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode)
{
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

void FVehicleData::RecordActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(Actor, CarlaEpisode);
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
  PhysicsControl = Vehicle->GetVehiclePhysicsControl();
  Control = Vehicle->GetVehicleControl();
  LightState = Vehicle->GetVehicleLightState();
}

void FVehicleData::RestoreActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(Actor, CarlaEpisode);
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(Actor);
  Vehicle->ApplyVehiclePhysicsControl(PhysicsControl);
  Vehicle->ApplyVehicleControl(Control, EVehicleInputPriority::Client);
  Vehicle->SetVehicleLightState(LightState);
  Vehicle->SetSimulatePhysics(bSimulatePhysics);
}

AActor* FTrafficSignData::RespawnActor(UCarlaEpisode* CarlaEpisode, const FActorInfo& Info)
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

void FTrafficSignData::RecordActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(Actor, CarlaEpisode);
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

void FTrafficSignData::RestoreActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(Actor, CarlaEpisode);
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

void FTrafficLightData::RecordActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RecordActorData(Actor, CarlaEpisode);
  Model = Actor->GetClass();
  ATrafficLightBase* TrafficLight = Cast<ATrafficLightBase>(Actor);
  UTrafficLightComponent* Component = TrafficLight->GetTrafficLightComponent();
  SignId = Component->GetSignId();
  Controller = Component->GetController();
  Controller->RemoveTrafficLight(Component);
  PoleIndex = TrafficLight->GetPoleIndex();
}

void FTrafficLightData::RestoreActorData(AActor* Actor, UCarlaEpisode* CarlaEpisode)
{
  FActorData::RestoreActorData(Actor, CarlaEpisode);
  ATrafficLightBase* TrafficLight = Cast<ATrafficLightBase>(Actor);
  UTrafficLightComponent* Component = TrafficLight->GetTrafficLightComponent();
  Component->SetSignId(SignId);
  Controller->AddTrafficLight(Component);
  ACarlaGameModeBase *GameMode = UCarlaStatics::GetGameMode(CarlaEpisode->GetWorld());
  Component->InitializeSign(GameMode->GetMap().get());
  Component->SetLightState(Controller->GetCurrentState().State);
  TrafficLight->SetPoleIndex(PoleIndex);
}
