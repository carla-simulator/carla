// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Util/BoundingBox.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BoundingBoxCalculator.generated.h"

class AActor;

UCLASS()
class CARLA_API UBoundingBoxCalculator : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  /// Compute the bounding box of the given Carla actor.
  ///
  /// @warning If the actor type is not supported a default initialized bounding
  /// box is returned.
  ///
  /// @warning Traffic signs return its trigger box instead.
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FBoundingBox GetActorBoundingBox(
    const AActor *Actor,
    uint8 InTagQueried = 0);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FBoundingBox GetVehicleBoundingBox(
    const ACarlaWheeledVehicle* Vehicle,
    uint8 InTagQueried = 0);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FBoundingBox GetCharacterBoundingBox(
    const ACharacter* Character,
    uint8 InTagQueried = 0);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void GetTrafficLightBoundingBox(
    const ATrafficLightBase* TrafficLight,
    TArray<FBoundingBox>& OutBB,
    uint8 InTagQueried = 0);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static FBoundingBox GetSkeletalMeshBoundingBox(const USkeletalMesh* SkeletalMesh);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static FBoundingBox GetStaticMeshBoundingBox(const UStaticMesh* StaticMesh);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static void GetISMBoundingBox(
    UInstancedStaticMeshComponent* HISMComp,
    TArray<FBoundingBox>& OutBoundingBox);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static void GetBBsOfStaticMeshComponents(
    const TArray<UStaticMeshComponent*>& StaticMeshComps,
    TArray<FBoundingBox>& OutBB,
    uint8 InTagQueried = 0);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static void GetBBsOfSkeletalMeshComponents(
    const TArray<USkeletalMeshComponent*>& SkeletalMeshComps,
    TArray<FBoundingBox>& OutBB,
    uint8 InTagQueried = 0);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static TArray<FBoundingBox> GetBoundingBoxOfActors(
    const TArray<AActor*>& Actors,
    uint8 InTagQueried = 0);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static TArray<FBoundingBox> GetBBsOfActor(
    const AActor* Actor,
    uint8 InTagQueried = 0);

};
