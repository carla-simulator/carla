// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Util/BoundingBox.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameFramework/Actor.h"

#include "BoundingBoxCalculator.generated.h"

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
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FBoundingBox GetVehicleBoundingBox(
    const ACarlaWheeledVehicle* Vehicle,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FBoundingBox GetCharacterBoundingBox(
    const ACharacter* Character,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void GetTrafficLightBoundingBox(
    const ATrafficLightBase* TrafficLight,
    TArray<FBoundingBox>& OutBB,
    TArray<uint8>& OutTag,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static FBoundingBox GetSkeletalMeshBoundingBox(const USkeletalMesh* SkeletalMesh);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static FBoundingBox GetStaticMeshBoundingBox(const UStaticMesh* StaticMesh);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static void GetISMBoundingBox(
    UInstancedStaticMeshComponent* ISMComp,
    TArray<FBoundingBox>& OutBoundingBox);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static void GetBBsOfStaticMeshComponents(
    const TArray<UStaticMeshComponent*>& StaticMeshComps,
    TArray<FBoundingBox>& OutBB,
    TArray<uint8>& OutTag,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static void GetBBsOfSkeletalMeshComponents(
    const TArray<USkeletalMeshComponent*>& SkeletalMeshComps,
    TArray<FBoundingBox>& OutBB,
    TArray<uint8>& OutTag,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static TArray<FBoundingBox> GetBoundingBoxOfActors(
    const TArray<AActor*>& Actors,
    uint8 InTagQueried = 0xFF);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static TArray<FBoundingBox> GetBBsOfActor(
    const AActor* Actor,
    uint8 InTagQueried = 0xFF);

  // Combines the BBs of an actor based on the distance and type of the BB
  // The BBs not combined are included too (ie: TL BBs and pole)
  // DistanceThreshold is the maximum distance between BBs to combine, if 0.0 the distance is ignored
  // TagToCombine defines the type of the BBs to combine, if 0 the type is ignored
  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static void CombineBBsOfActor(
    const AActor* Actor,
    TArray<FBoundingBox>& OutBB,
    TArray<uint8>& OutTag,
    const float DistanceThreshold = 0.0f,
    uint8 TagToCombine = 0xFF);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static FBoundingBox CombineBBs(const TArray<FBoundingBox>& BBsToCombine);

  UFUNCTION(Category = "Carla Util", BlueprintCallable)
  static FBoundingBox CombineBoxes(const TArray<UBoxComponent *>& BBsToCombine);

  // Returns Static Mesh Components that generate the InBB of the Actor
  // ie: the SMComps that creates the BB of the TL light box
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void GetMeshCompsFromActorBoundingBox(
    const AActor* Actor,
    const FBoundingBox& InBB,
    TArray<UStaticMeshComponent*>& OutStaticMeshComps);

};
