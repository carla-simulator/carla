// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


// #if WITH_EDITOR

#pragma once 

#include "CoreMinimal.h"

#include "EditorUtilityWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UnrealString.h"

#include "MapGeneratorWidget.generated.h"

/// Class UMapGeneratorWidget extends the functionality of UEditorUtilityWidget
/// to be able to generate and manage maps and largemaps tiles for procedural
/// map generation
UCLASS(BlueprintType)
class CARLATOOLS_API UMapGeneratorWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()

public:
    /// This function invokes a blueprint event defined in widget blueprint 
    /// event graph, which sets a heightmap to the @a landscape using
    /// ALandscapeProxy::LandscapeImportHeightMapFromRenderTarget(...)
    /// function, which is not exposed to be used in C++ code, only blueprints
    UFUNCTION(BlueprintImplementableEvent)
    void AssignLandscapeHeightMap(ALandscape* landscape);

//private:
    // UPROPERTY()
    // UObjectLibrary *MapObjectLibrary;

public:
    /// Function called by Widget Blueprint which generates all tiles of map
    /// @a mapName, and saves them in @a destinationPath
    /// Returns a void string is success and an error message if the process failed
    UFUNCTION(Category="Map Generator",BlueprintCallable)
    FString GenerateMapFiles(const FString& destinationPath, const FString& mapName);

private:    
    /// Loads the base template UWorld object and is return in @a WorldAssetData
    /// The funtions return true is success, otherwise false
    UFUNCTION()
    bool LoadWorld(FAssetData& WorldAssetData);

    /// Saves a world contained in @a WorldToBeSaved, in the path defined in @a DestinationPath
    /// named as @a WorldName, as a package .umap
    UFUNCTION()
    bool SaveWorld(FAssetData& WorldToBeSaved, const FString& DestinationPath, const FString& WorldName);
};
// #endif