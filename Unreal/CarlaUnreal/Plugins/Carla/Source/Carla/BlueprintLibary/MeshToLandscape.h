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
class CARLA_API UMeshToLandscapeUtil :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    static void FilterLandscapeLikeStaticMeshComponentsByVariance(
        TArray<UStaticMeshComponent*>& StaticMeshComponents,
        double MaxZVariance);

    static void FilterLandscapeLikeStaticMeshComponentsByPatterns(
        TArray<UStaticMeshComponent*>& StaticMeshComponents,
        const TArray<FString>& ActorNamePatterns);

public:

    UFUNCTION(BlueprintCallable, Category="CarlaTools")
    static ALandscape* ConvertMeshesToLandscape(
        const TArray<UStaticMeshComponent*>& StaticMeshComponents,
        int32 SubsectionSizeQuads = 255,
        int32 NumSubsections = 8);

    UFUNCTION(BlueprintCallable, Category="CarlaTools", meta=(WorldContext="WorldContextObject"))
    static void EnumerateLandscapeLikeStaticMeshComponents(
        UObject* WorldContextObject,
        const TArray<FString>& ActorNamePatterns,
        double MaxZVariance,
        TArray<UStaticMeshComponent*>& OutStaticMeshComponents);

};
