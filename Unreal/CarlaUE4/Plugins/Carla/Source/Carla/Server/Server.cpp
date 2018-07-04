// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Server.h"


// Sets default values
AServer::AServer()
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AServer::BeginPlay()
{
  Super::BeginPlay();

  _Server.Initialize(*this);
  _Server.Run();
}

void AServer::EndPlay(EEndPlayReason::Type EndPlayReason)
{
  _Server.Stop();
}

// Called every frame
void AServer::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  _Server.RunSome();
}

int32 AServer::SpawnAgent(const FTransform &Transform)
{
  check(IsInGameThread());
  UE_LOG(LogTemp, Warning, TEXT("Spawning vehicle at %s"), *Transform.ToString());

  static int32 COUNT = 0u;
  ++COUNT;

  ACarlaWheeledVehicle *Vehicle;
  SpawnVehicle(Transform, Vehicle);
  if ((Vehicle != nullptr) && !Vehicle->IsPendingKill())
  {
    // Vehicle->AIControllerClass = AWheeledVehicleAIController::StaticClass();
    Vehicle->SpawnDefaultController();
    _Agents.Add(COUNT, Vehicle);
    return COUNT;
  }
  return -1;
}

bool AServer::ApplyControl(int32 AgentId, const FVehicleControl &Control)
{
  UE_LOG(LogTemp, Log, TEXT("Applying control to vehicle %d: throttle = %f, steer = %f"), AgentId, Control.Throttle, Control.Steer);
  if (!_Agents.Contains(AgentId))
  {
    UE_LOG(LogTemp, Error, TEXT("Vehicle %d does not exist!"), AgentId);
    return false;
  }
  auto *Vehicle = _Agents[AgentId];
  Vehicle->ApplyVehicleControl(Control);
  return true;
}
