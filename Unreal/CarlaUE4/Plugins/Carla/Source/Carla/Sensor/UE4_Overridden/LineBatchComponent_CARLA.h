// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/LineBatchComponent.h"
#include "LineBatchComponent_CARLA.generated.h"

/**
 * 
 */
UCLASS()
class CARLA_API ULineBatchComponent_CARLA : public ULineBatchComponent
{
	GENERATED_BODY()

	ULineBatchComponent_CARLA(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	UMaterial* CosmosMeshMaterial = nullptr;
};

/** Represents a LineBatchComponent to the scene manager. */
class FLineBatcherSceneProxy_CARLA : public FLineBatcherSceneProxy
{
public:
	SIZE_T GetTypeHash() const override;

	FLineBatcherSceneProxy_CARLA(const ULineBatchComponent_CARLA* InComponent);

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	/**
	*  Returns a struct that describes to the renderer when to draw this proxy.
	*	@param		Scene view to use to determine our relevence.
	*  @return		View relevance struct
	*/
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

	UMaterial* CosmosMeshMaterial = nullptr;

private:
	TArray<FBatchedLine> Lines;
	TArray<FBatchedPoint> Points;
	TArray<FBatchedMesh> Meshes;
	
	// Cache for material render proxies to avoid memory leaks
	// Key is the color, value is the cached proxy
	mutable TMap<uint32, FColoredMaterialRenderProxy*> CachedMaterialProxies;

public:
	virtual ~FLineBatcherSceneProxy_CARLA();
};
