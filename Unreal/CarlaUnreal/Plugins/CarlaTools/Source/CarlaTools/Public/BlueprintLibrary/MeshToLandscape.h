// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "EngineMinimal.h"
#include <util/ue-header-guard-end.h>

#include "MeshToLandscape.generated.h"

class ALandscape;
class ULandscapeComponent;
class AStaticMeshActor;
class UStaticMeshComponent;

UCLASS(BlueprintType)
class CARLATOOLS_API UMeshToLandscapeUtil :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    void FilterLandscapeLikeStaticMeshComponentsByVariance(
        TArray<UStaticMeshComponent*>& StaticMeshComponents,
        double MaxZVariance);

public:

    UFUNCTION(BlueprintCallable)
    ALandscape* ConvertMeshesToLandscape(
        const TArray<UStaticMeshComponent*>& InStaticMeshComponents,
        int32 HeightmapWidth,
        int32 HeightmapHeight);

    UFUNCTION(BlueprintCallable)
    void EnumerateLandscapeLikeStaticMeshComponentsByVariance(
        AActor* WorldContextObject,
        TArray<UStaticMeshComponent*>& OutStaticMeshComponents,
        double MaxZVariance);

};
