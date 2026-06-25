// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Components/LineBatchComponent.h"
#include "PrimitiveSceneProxy.h"
#include <util/ue-header-guard-end.h>

#include "LineBatchComponent_CARLA.generated.h"

/**
 * Line batch component that only draws its primitives when the rendering
 * view belongs to an ACosmosControlSensor.  It loads the Cosmos debug
 * material used to colour the batched meshes.
 */
UCLASS()
class CARLA_API ULineBatchComponent_CARLA : public ULineBatchComponent
{
  GENERATED_BODY()

  ULineBatchComponent_CARLA(
      const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

  virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

  UMaterial* CosmosMeshMaterial = nullptr;
};

/**
 * Scene proxy for ULineBatchComponent_CARLA.
 *
 * In UE5.5 FLineBatcherSceneProxy is no longer exposed by any public header
 * (it lives inline in LineBatchComponent.cpp), so this proxy is implemented
 * as a standalone FPrimitiveSceneProxy subclass that re-implements the
 * line/point/mesh batching logic.  It only declares itself relevant when the
 * view is owned by an ACosmosControlSensor.
 */
class FLineBatcherSceneProxy_CARLA : public FPrimitiveSceneProxy
{
public:

  SIZE_T GetTypeHash() const override;

  FLineBatcherSceneProxy_CARLA(const ULineBatchComponent_CARLA* InComponent);

  virtual void GetDynamicMeshElements(
      const TArray<const FSceneView*>& Views,
      const FSceneViewFamily& ViewFamily,
      uint32 VisibilityMap,
      FMeshElementCollector& Collector) const override;

  virtual FPrimitiveViewRelevance GetViewRelevance(
      const FSceneView* View) const override;

  virtual uint32 GetMemoryFootprint(void) const override;

  uint32 GetAllocatedSize(void) const;

  UMaterial* CosmosMeshMaterial = nullptr;

private:

  TArray<FBatchedLine> Lines;
  TArray<FBatchedPoint> Points;
  TArray<FBatchedMesh> Meshes;
};
