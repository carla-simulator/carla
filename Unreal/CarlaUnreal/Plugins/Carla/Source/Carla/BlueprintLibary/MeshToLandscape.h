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

    static void FilterInvalidComponents(
        TArray<UActorComponent*>& Components);

    static void FilterByClassList(
        TArray<UActorComponent*>& Components,
        const TArray<UClass*>& Blacklist,
        const TArray<UClass*>& Whitelist);

    static void FilterInvalidStaticMeshComponents(
        TArray<UActorComponent*>& Components);

    static void FilterStaticMeshComponentsByVariance(
        TArray<UActorComponent*>& Components,
        double MaxZVariance);

    static void FilterComponentsByPatterns(
        TArray<UActorComponent*>& Components,
        const TArray<FString>& Patterns);

public:

    UFUNCTION(BlueprintCallable, Category="CarlaTools")
    static ALandscape* ConvertMeshesToLandscape(
        const TArray<UActorComponent*>& Components,
        int32 SubsectionSizeQuads,
        int32 NumSubsections,
        bool ApplySmoothing);

    UFUNCTION(BlueprintCallable, Category="CarlaTools", meta=(WorldContext="WorldContextObject"))
    static void EnumerateLandscapeLikeStaticMeshComponents(
        UObject* WorldContextObject,
        const TArray<FString>& Patterns,
        const TArray<UClass*>& ClassWhitelist,
        const TArray<UClass*>& ClassBlacklist,
        double MaxZVariance,
        TArray<UActorComponent*>& OutComponents);

};
