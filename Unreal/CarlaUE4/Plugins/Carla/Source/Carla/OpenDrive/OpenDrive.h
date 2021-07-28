// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/OpenDrive/OpenDriveMap.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "OpenDrive.generated.h"

UCLASS()
class CARLA_API UOpenDrive : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  /// Return the OpenDrive XML associated to @a MapName, or empty if the file
  /// is not found.
  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static FString GetXODR(const UWorld *World);

  /// Return the OpenDrive XML associated to @a MapName, or empty if the file
  /// is not found.
  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static FString GetXODRByPath(FString XODRPath, FString MapName);

  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static FString FindPathToXODRFile(const FString &InMapName);

  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static FString LoadXODR(const FString &MapName);

  /// Load OpenDriveMap associated to the given MapName. Return nullptr if no
  /// XODR can be found with same MapName.
  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static UOpenDriveMap *LoadOpenDriveMap(const FString &MapName);

  /// Load OpenDriveMap associated to the currently loaded map. Return nullptr
  /// if no XODR can be found that matches the current map.
  UFUNCTION(BlueprintPure, Category="CARLA|OpenDrive", meta=(WorldContext="WorldContextObject"))
  static UOpenDriveMap *LoadCurrentOpenDriveMap(const UObject *WorldContextObject);
};
