// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Commandlet/SetProperPositionForWorldPartitionCommandlet.h"

#include <iostream>
#include <fstream>

#if WITH_EDITOR
#include "FileHelpers.h"
#endif
#include "UObject/ConstructorHelpers.h"
#include "EditorLevelLibrary.h"
#include "MapGen/LargeMapManager.h"



DEFINE_LOG_CATEGORY(LogCarlaToolsMapSetProperPositionForWorldPartitionCommandlet);


USetProperPositionForWorldPartitionCommandlet::USetProperPositionForWorldPartitionCommandlet()
{
  
}

USetProperPositionForWorldPartitionCommandlet::USetProperPositionForWorldPartitionCommandlet(const FObjectInitializer& Initializer)
  : Super(Initializer)
{
  
}

#if WITH_EDITORONLY_DATA

int32 USetProperPositionForWorldPartitionCommandlet::Main(const FString &Params)
{
  UE_LOG(LogCarlaToolsMapSetProperPositionForWorldPartitionCommandlet, Log, TEXT("USetProperPositionForWorldPartition::Main Arguments %s"), *Params);
  TArray<FString> Tokens;
  TArray<FString> Switches;
  TMap<FString,FString> ParamsMap;

  ParseCommandLine(*Params, Tokens, Switches, ParamsMap );

  FString BaseLevelName = ParamsMap["BaseLevelName"];
  FIntVector CurrentTilesInXY = FIntVector(FCString::Atof(*ParamsMap["CTileX"]), FCString::Atof(*ParamsMap["CTileY"]), 0);
  UEditorLevelLibrary::LoadLevel(*BaseLevelName);

  AActor* QueryActor = UGameplayStatics::GetActorOfClass( GEditor->GetEditorWorldContext().World(), ALargeMapManager::StaticClass());
  if (QueryActor != nullptr) {
    ALargeMapManager* LmManager = Cast<ALargeMapManager>(QueryActor);
    
    const FIntVector NumTilesInXY = LmManager->GetNumTilesInXY();
    const float TileSize = LmManager->GetTileSize();
    UE_LOG(LogCarlaToolsMapSetProperPositionForWorldPartitionCommandlet, Warning, TEXT("NumTilesInXY is %s"), *(NumTilesInXY.ToString()));
    UE_LOG(LogCarlaToolsMapSetProperPositionForWorldPartitionCommandlet, Warning, TEXT("TileSize is %f"), (TileSize));
    
    UEditorLevelLibrary::SaveCurrentLevel();
    
    UEditorLevelLibrary::LoadLevel(*BaseLevelName);
    ProcessTile(FIntVector(CurrentTilesInXY.X, CurrentTilesInXY.Y, 0), TileSize);
    UEditorLevelLibrary::SaveCurrentLevel();
  }
  else {
    UE_LOG(LogCarlaToolsMapSetProperPositionForWorldPartitionCommandlet, Error, TEXT("Largemapmanager not found "));
  }

  UEditorLoadingAndSavingUtils::SaveDirtyPackages(false, true);
  UEditorLevelLibrary::SaveCurrentLevel();

  return 0;
}

void USetProperPositionForWorldPartitionCommandlet::ProcessTile(const FIntVector CurrentTilesInXY, const float TileSize)
{
  AActor* QueryActor = UGameplayStatics::GetActorOfClass( GEditor->GetEditorWorldContext().World(), ALargeMapManager::StaticClass());
  if (QueryActor != nullptr)
  {
    ALargeMapManager* LmManager = Cast<ALargeMapManager>(QueryActor);
    UE_LOG(LogCarlaToolsMapSetProperPositionForWorldPartitionCommandlet, Warning, TEXT("Current Tile is %s"), *(CurrentTilesInXY.ToString()));
    FCarlaMapTile* CarlaTile = LmManager->GetCarlaMapTilePointer(CurrentTilesInXY);
    if( CarlaTile != nullptr )
    {
      UE_LOG(LogCarlaToolsMapSetProperPositionForWorldPartitionCommandlet, Warning, TEXT("Tile Name is %s"), *(CarlaTile->Name));
      UEditorLevelLibrary::LoadLevel(CarlaTile->Name);

      const FVector MinPosition = FVector(CurrentTilesInXY.X * TileSize, CurrentTilesInXY.Y * -TileSize, 0.0f);

      TArray<AActor*> FoundActors;
      UGameplayStatics::GetAllActorsOfClass(GEditor->GetEditorWorldContext().World(), AActor::StaticClass(), FoundActors);
      for (AActor* CA : FoundActors) 
      {
        CA->AddActorWorldOffset(MinPosition, false, nullptr, ETeleportType::ResetPhysics);
      }
    }
    else
    {
        UE_LOG(LogCarlaToolsMapSetProperPositionForWorldPartitionCommandlet, Error, TEXT("Failed to load Tile %s"), *(CurrentTilesInXY.ToString()));
    }
    UEditorLevelLibrary::SaveCurrentLevel();
  }

}

#endif
