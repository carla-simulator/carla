// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/PedestrianParameters.h"
#include "Carla/Actor/PropParameters.h"
#include "Carla/Actor/VehicleParameters.h"
#include "Carla/Sensor/GnssSensor.h"
#include "Carla/Sensor/Radar.h"
#include "Carla/Sensor/InertialMeasurementUnit.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "ActorBlueprintFunctionLibrary.generated.h"

class ASceneCaptureSensor;
class AShaderBasedSensor;
struct FLidarDescription;

UCLASS()
class UActorBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  /// @}
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

  static FActorDefinition MakeGenericDefinition(
      const FString &Category,
      const FString &Type,
      const FString &Id);

  static FActorDefinition MakeGenericSensorDefinition(
      const FString &Type,
      const FString &Id);

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

  static FActorDefinition MakeGnssDefinition();

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeGnssDefinition(
      bool &Success,
      FActorDefinition &Definition);

  static FActorDefinition MakeIMUDefinition();

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeIMUDefinition(
      bool &Success,
      FActorDefinition &Definition);

  static FActorDefinition MakeRadarDefinition();

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeRadarDefinition(
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

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakePedestrianDefinition(
      const FPedestrianParameters &Parameters,
      bool &Success,
      FActorDefinition &Definition);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakePedestrianDefinitions(
      const TArray<FPedestrianParameters> &ParameterArray,
      TArray<FActorDefinition> &Definitions);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeTriggerDefinitions(
      const TArray<FString> &ParameterArray,
      TArray<FActorDefinition> &Definitions);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakeTriggerDefinition(
      const FString &Id,
      bool &Success,
      FActorDefinition &Definition);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakePropDefinition(
      const FPropParameters &Parameters,
      bool &Success,
      FActorDefinition &Definition);

  UFUNCTION(Category = "Carla Actor", BlueprintCallable)
  static void MakePropDefinitions(
      const TArray<FPropParameters> &ParameterArray,
      TArray<FActorDefinition> &Definitions);

  UFUNCTION()
  static void MakeObstacleDetectorDefinitions(
      const FString &Type,
      const FString &Id,
      FActorDefinition &Definition);

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
  static void SetCamera(const FActorDescription &Description, AShaderBasedSensor *Camera);

  static void SetLidar(const FActorDescription &Description, FLidarDescription &Lidar);

  static void SetGnss(const FActorDescription &Description, AGnssSensor *Gnss);

  static void SetIMU(const FActorDescription &Description, AInertialMeasurementUnit *IMU);

  static void SetRadar(const FActorDescription &Description, ARadar *Radar);
};
