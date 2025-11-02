// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Carla/Sensor/ShaderBasedSensor.h"
#include "Sensor/Sensor.h"
#include "Carla/Actor/ActorDefinition.h"
#include "Components/LineBatchComponent.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Sensor/UE4_Overridden/LineBatchComponent_CARLA.h"
#include "CosmosControlSensor.generated.h"

/**
 * Sensor that produces an input control visualization for Cosmos
 */
UCLASS()
class CARLA_API ACosmosControlSensor : public AShaderBasedSensor
{
	GENERATED_BODY()

public:

  struct CosmosColors
  {
    static const FColor LaneLines;
    static const FColor Lanes;
    static const FColor Poles;
    static const FColor RoadBoundaries;
    static const FColor WaitLines;
    static const FColor Crosswalks;
    static const FColor RoadMarkings;
    static const FColor TrafficSigns;
    static const FColor TrafficLights;
    static const FColor Cars;
    static const FColor Pedestrians;
  };

	static FActorDefinition GetSensorDefinition();

  ACosmosControlSensor(const FObjectInitializer& ObjectInitializer);

protected:

  void SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture) override;
  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

  //Duplicate functions from DrawDebugHelpers to guarantee they work outside editor
  void DrawDebugLine(const UWorld* InWorld, FVector const& LineStart, FVector const& LineEnd, FColor const& Color, bool bPersistentLines = false, float LifeTime = -1.f, uint8 DepthPriority = 0, float Thickness = 0);
  void DrawDebugSolidBox(const UWorld* InWorld, FVector const& Center, FVector const& Extent, FQuat const& Rotation, FColor const& Color, bool bPersistent = false, float LifeTime = -1.f, uint8 DepthPriority = 0);
  void DrawDebugBox(const UWorld* InWorld, FVector const& Center, FVector const& Box, const FQuat& Rotation, FColor const& Color, bool bPersistentLines = false, float LifeTime = -1.f, uint8 DepthPriority = 0, float Thickness = 0);
  void DrawDebugCapsule(const UWorld* InWorld, FVector const& Center, float HalfHeight, float Radius, const FQuat& Rotation, FColor const& Color, bool bPersistentLines = false, float LifeTime = -1.f, uint8 DepthPriority = 0, float Thickness = 0);
  void DrawDebugMesh(const UWorld* InWorld, const TArray<FVector>& Vertices, const TArray<int32>& Indices, const FColor& Color, bool bPersistentLines = false, float LifeTime = -1.f, uint8 DepthPriority = 0);
  void DrawCircle(const UWorld* InWorld, const FVector& Base, const FVector& X, const FVector& Y, const FColor& Color, float Radius, int32 NumSides, bool bPersistentLines = false, float LifeTime = -1.f, uint8 DepthPriority = 0, float Thickness = 0);
  void DrawHalfCircle(const UWorld* InWorld, const FVector& Base, const FVector& X, const FVector& Y, const FColor& Color, float Radius, int32 NumSides, bool bPersistentLines = false, float LifeTime = -1.f, uint8 DepthPriority = 0, float Thickness = 0);

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  ULineBatchComponent_CARLA* DynamicLines;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  ULineBatchComponent_CARLA* PersistentLines;

private:
  ULineBatchComponent_CARLA* GetDebugLineBatcher(bool bPersistentLines);
  FColor GetColorByTag(carla::rpc::CityObjectLabel Tag, uint8 alpha = 255);


private:
  bool added_persisted_stop_lines;
  bool added_persisted_route_lines;
  bool added_persisted_crosswalks;
  bool added_persisted_stencils;
};
