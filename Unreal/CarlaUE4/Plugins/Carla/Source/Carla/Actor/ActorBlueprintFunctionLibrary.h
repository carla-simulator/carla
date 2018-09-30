// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "ActorBlueprintFunctionLibrary.generated.h"

class ASceneCaptureSensor;
struct FLidarDescription;

USTRUCT(BlueprintType)
struct CARLA_API FVehicleParameters
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Make;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Model;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<ACarlaWheeledVehicle> Class;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 NumberOfWheels = 4;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FColor> RecommendedColors;
};

UCLASS()
class UActorBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  /// ==========================================================================
  /// @name Actor definition validators
  /// ==========================================================================
  /// @{

  /// Return whether the actor definition is valid. Prints all the errors found.
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static bool CheckActorDefinition(const FActorDefinition &ActorDefinitions);

  /// Return whether the list of actor definitions is valid. Prints all the
  /// errors found.
  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static bool CheckActorDefinitions(const TArray<FActorDefinition> &ActorDefinitions);

  /// @}
  /// ==========================================================================
  /// @name Helpers to create actor definitions
  /// ==========================================================================
  /// @{

  static FActorDefinition MakeCameraDefinition(
      const FString &Id,
      bool bEnableModifyingPostProcessEffects = false);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeCameraDefinition(
      const FString &Id,
      bool bEnableModifyingPostProcessEffects,
      bool &Success,
      FActorDefinition &Definition);

  static FActorDefinition MakeLidarDefinition(
      const FString &Id);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeLidarDefinition(
      const FString &Id,
      bool &Success,
      FActorDefinition &Definition);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeVehicleDefinition(
      const FVehicleParameters &Parameters,
      bool &Success,
      FActorDefinition &Definition);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeVehicleDefinitions(
      const TArray<FVehicleParameters> &ParameterArray,
      TArray<FActorDefinition> &Definitions);

  /// @}
  /// ==========================================================================
  /// @name Helpers to retrieve attribute values
  /// ==========================================================================
  /// @{

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static bool ActorAttributeToBool(const FActorAttribute &ActorAttribute, bool Default);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static int32 ActorAttributeToInt(const FActorAttribute &ActorAttribute, int32 Default);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static float ActorAttributeToFloat(const FActorAttribute &ActorAttribute, float Default);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FString ActorAttributeToString(const FActorAttribute &ActorAttribute, const FString &Default);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FColor ActorAttributeToColor(const FActorAttribute &ActorAttribute, const FColor &Default);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static bool RetrieveActorAttributeToBool(
      const FString &Id,
      const TMap<FString, FActorAttribute> &Attributes,
      bool Default);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static int32 RetrieveActorAttributeToInt(
      const FString &Id,
      const TMap<FString, FActorAttribute> &Attributes,
      int32 Default);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static float RetrieveActorAttributeToFloat(
      const FString &Id,
      const TMap<FString, FActorAttribute> &Attributes,
      float Default);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FString RetrieveActorAttributeToString(
      const FString &Id,
      const TMap<FString, FActorAttribute> &Attributes,
      const FString &Default);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static FColor RetrieveActorAttributeToColor(
      const FString &Id,
      const TMap<FString, FActorAttribute> &Attributes,
      const FColor &Default);

  /// @}
  /// ==========================================================================
  /// @name Helpers to set Actors
  /// ==========================================================================
  /// @{

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void SetCamera(const FActorDescription &Description, ASceneCaptureSensor *Camera);

  static void SetLidar(const FActorDescription &Description, FLidarDescription &Lidar);

  /// @}
};

