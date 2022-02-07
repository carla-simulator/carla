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

USTRUCT(BlueprintType)
struct CARLATOOLS_API FMapGeneratorMetaInfo
{
    GENERATED_USTRUCT_BODY();

    UPROPERTY(BlueprintReadWrite)
    FString destinationPath;

    UPROPERTY(BlueprintReadWrite)
    FString mapName;

    UPROPERTY(BlueprintReadWrite)   
    int sizeX;

    UPROPERTY(BlueprintReadWrite)
    int sizeY;
};

/// Class UMapGeneratorWidget extends the functionality of UEditorUtilityWidget
/// to be able to generate and manage maps and largemaps tiles for procedural
/// map generation
UCLASS(BlueprintType)
class CARLATOOLS_API UMapGeneratorWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()


private:
    // UPROPERTY()
    // UObjectLibrary *MapObjectLibrary;

public:
    /// This function invokes a blueprint event defined in widget blueprint 
    /// event graph, which sets a heightmap to the @a landscape using
    /// ALandscapeProxy::LandscapeImportHeightMapFromRenderTarget(...)
    /// function, which is not exposed to be used in C++ code, only blueprints
    UFUNCTION(BlueprintImplementableEvent)
    void AssignLandscapeHeightMap(ALandscape* landscape);

    /// Function called by Widget Blueprint which generates all tiles of map
    /// @a mapName, and saves them in @a destinationPath
    /// Returns a void string is success and an error message if the process failed
    UFUNCTION(Category="Map Generator",BlueprintCallable)
    FString GenerateMapFiles(const FMapGeneratorMetaInfo& metaInfo);

private:    
    /// Loads the base tile map and stores it in @a WorldAssetData
    /// The funtions return true is success, otherwise false
    UFUNCTION()
    bool LoadBaseTileWorld(FAssetData& WorldAssetData);

    /// Loads the base large map and stores it in @a WorldAssetData
    /// The funtions return true is success, otherwise false
    UFUNCTION()
    bool LoadBaseLargeMapWorld(FAssetData& WorldAssetData);

    /// Loads the base template UWorld object from @a baseMapPath and returns 
    /// it in @a WorldAssetData
    /// The funtions return true is success, otherwise false
    UFUNCTION()
    bool LoadWorld(FAssetData& WorldAssetData, const FString& baseMapPath);

    /// Saves a world contained in @a WorldToBeSaved, in the path defined in @a DestinationPath
    /// named as @a WorldName, as a package .umap
    UFUNCTION()
    bool SaveWorld(FAssetData& WorldToBeSaved, const FString& DestinationPath, const FString& WorldName);

    /// Takes the name of the map from @a metaInfo and created the main map
    /// including all the actors needed by large map system
    UFUNCTION()
    bool CreateMainLargeMap(const FMapGeneratorMetaInfo& metaInfo);

    /// Takes @a metaInfo as input and generates all tiles based on the
    /// dimensions specified for the map
    /// The funtions return true is success, otherwise false
    UFUNCTION()
    bool CreateTilesMaps(const FMapGeneratorMetaInfo& metaInfo);

    /// Gets the landscape from the input world @a worldAssetData and
    /// applies the heightmap to it
    /// The funtions return true is success, otherwise false
    UFUNCTION()
    bool ApplyHeightMapToLandscape(FAssetData& worldAssetData);
};
// #endif