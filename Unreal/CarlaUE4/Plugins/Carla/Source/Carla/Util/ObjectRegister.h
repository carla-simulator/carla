// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"

#include "Carla/Util/BoundingBox.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Traffic/TrafficLightBase.h"
#include "Carla/Util/EnvironmentObject.h"

#include "ObjectRegister.generated.h"

UCLASS()
class CARLA_API UObjectRegister : public UObject
{
  GENERATED_BODY()

public:

  UObjectRegister() {}
  ~UObjectRegister() {}

  UFUNCTION(Category = "Carla Object Register", BlueprintCallable, CallInEditor)
  TArray<FEnvironmentObject> GetEnvironmentObjects(uint8 InTagQueried = 0xFF) const;

  UFUNCTION(Category = "Carla Object Register")
  void RegisterObjects(TArray<AActor*> Actors);

  UFUNCTION(Category = "Carla Object Register")
  void EnableEnvironmentObjects(const TSet<uint64>& EnvObjectIds, bool Enable);

private:

  void RegisterEnvironmentObject(
    AActor* Actor,
    FBoundingBox& BoundingBox,
    EnvironmentObjectType Type,
    uint8 Tag);

  void RegisterVehicle(ACarlaWheeledVehicle* Vehicle);

  void RegisterCharacter(ACharacter* Character);

  void RegisterTrafficLight(ATrafficLightBase* TrafficLight);

  void RegisterISMComponents(AActor* Actor);

  void RegisterSMComponents(AActor* Actor);

  void RegisterSKMComponents(AActor* Actor);

  void EnableEnvironmentObject(FEnvironmentObject& EnvironmentObject, bool Enable);

  void EnableActor(FEnvironmentObject& EnvironmentObject, bool Enable);

  void EnableTrafficLight(FEnvironmentObject& EnvironmentObject, bool Enable);

  void EnableISMComp(FEnvironmentObject& EnvironmentObject, bool Enable);

  TMultiMap<uint64, const UStaticMeshComponent*> ObjectIdToComp;

  UPROPERTY(Category = "Carla Object Register", EditAnywhere)
  TArray<FEnvironmentObject> EnvironmentObjects;

  int FoliageActorInstanceCount = 0;

};
