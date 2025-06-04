// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "EditorUtilityWidget.h"
#include "Math/Vector.h"
#include <util/ue-header-guard-end.h>

#include "ProceduralWaterManager.generated.h"

UENUM(BlueprintType)
enum EWaterGenerationType
{
  RIVERS = 0,
  LAKE = 1
};

USTRUCT(BlueprintType)
struct CARLATOOLS_API FProceduralRiversMetaInfo
{
  GENERATED_USTRUCT_BODY();

  UPROPERTY(BlueprintReadWrite)
  TEnumAsByte<EWaterGenerationType> WaterGenerationType = EWaterGenerationType::RIVERS;

  UPROPERTY(BlueprintReadWrite)
  FString WaterInfoPath;

  UPROPERTY(BlueprintReadWrite)
  int CustomSampling = 0;

  UPROPERTY(BlueprintReadWrite)
  float CustomScaleFactor = 0.0F;

  UPROPERTY(BlueprintReadWrite)
  float CustomRiverWidth = 0.0F;

  UPROPERTY(BlueprintReadWrite)
  FVector CustomLocationOffset = FVector::ZeroVector;

  UPROPERTY(BlueprintReadWrite)
  float CustomHeight = 0.0F;

  UPROPERTY(BlueprintReadWrite)
  int SizeOfLandscape = 0;

  UPROPERTY(BlueprintReadWrite)
  int SizeOfTexture = 0;
};

UCLASS(BlueprintType)
class CARLATOOLS_API UProceduralWaterManager : public UEditorUtilityWidget
{
  GENERATED_BODY()
  
public:  
  // Sets default values for this actor's properties
  UProceduralWaterManager();

public:  
  /// River blueprint class, set by the user using the widget interface
  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<class AActor> RiverBlueprintClass;

  /// Lake blueprint class, set by the user using the widget interface
  UPROPERTY(BlueprintReadWrite)
  TSubclassOf<class AActor> LakeBlueprintClass;

  /// Main function to be called from the widget to start all the generation process
  /// @a MetaInfo is the input data for this process
  UFUNCTION(BlueprintCallable)
  FString StartWaterGeneration(const FProceduralRiversMetaInfo MetaInfo);

  /// Add river @a riverActor spline point @a splinePoint to SplinePoint  
  /// collection to be added in later processes to the spline component.
  /// This is implemented in blueprint code.
  UFUNCTION(BlueprintImplementableEvent)
  void AddRiverPointFromCode(AActor* RiverActor, FSplinePoint SplinePoint);

  /// It checks which is the direction the flow of the river depending on the
  /// altitude of the start and the end of the river @a riverActor
  UFUNCTION(BlueprintImplementableEvent)
  void CheckAndReverseWaterFlow(AActor* RiverActor);

private:
  
  /// It is responsible of the rivers generation, parsing the file,
  /// intantiating the actors and setting its splline points
  UFUNCTION()
  FString RiverGeneration(const FProceduralRiversMetaInfo MetaInfo);

  /// It is responsible of the lakes generation, pasing the file,
  /// instantiating the actors and setting its properties
  UFUNCTION()
  FString LakeGeneration(const FProceduralRiversMetaInfo MetaInfo);

  /// Instantiate a new actor of type RiverBlueprintClass
  /// Returns the the actor created
  UFUNCTION()
  AActor* SpawnRiverBlueprintActor();

  /// Instantiate a new actor of type LakeBlueprintClass
  /// Returns the the actor created
  UFUNCTION()
  AActor* SpawnLakeBlueprintActor();

  /// Calculates the height of the landscape in an specific 2D coordinate ( @a x, @a y)
  /// throwing rays and detecting the hit point. @a bDrawDebugLines allows to visualize
  /// the rays in the viewport, only for debug purposes.
  /// Return the Z value
  UFUNCTION()
  float GetLandscapeSurfaceHeight(float x, float y, bool bDrawDebugLines = false);


  /************ RIVER PRESETS GENERATOR ************/
public:
  UFUNCTION(BlueprintCallable)
  bool CreateRiverPresetFiles(TSubclassOf<AActor> RiverParentClass);
};
