// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Vehicle/CarlaWheeledVehicle.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RPCServer.h"
#include "Vehicle/VehicleControl.h"
#include <future>
#include <mutex>
#include <queue>
#include "Server.generated.h"

UCLASS()
class CARLA_API AServer : public AActor
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  AServer();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;
  virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  UFUNCTION(BlueprintCallable)
  int32 SpawnAgent(const FTransform &Transform);

  UFUNCTION(BlueprintCallable)
  bool ApplyControl(int32 AgentId, const FVehicleControl &Control);

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnVehicle(const FTransform &SpawnTransform, ACarlaWheeledVehicle *&SpawnedCharacter);

private:

  FRPCServer _Server;

  UPROPERTY(VisibleAnywhere)
  TMap<uint32, ACarlaWheeledVehicle *> _Agents;
};
