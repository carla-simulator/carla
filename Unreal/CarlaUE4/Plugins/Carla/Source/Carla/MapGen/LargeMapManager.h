// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Engine/LevelStreamingDynamic.h"

#include "Math/DVector.h"

#include "LargeMapManager.generated.h"



UCLASS()
class CARLA_API ALargeMapManager : public AActor
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ALargeMapManager();

  ~ALargeMapManager();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  void PreWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin);
  void PostWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin);

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void GenerateMap(FString AssetsPath);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  ULevelStreamingDynamic* AddNewTile(FString TileName, FVector Location);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void AddNewClientPosition(AActor* InActor)
  {
    // TODO: This should be able to adapt to a multiactor environment
    // The problem is that the rebase will only be available for one
    // unless we create more instances of the simulation
    ActorToConsider = InActor;
  }

protected:

  ULevelStreamingDynamic* AddNewTileInternal(FString TileName, FVector Location);

  TMap<FString, ULevelStreamingDynamic*> TilesCreated;

  AActor* ActorToConsider = nullptr;

  FIntVector CurrentOrigin{0};

  FDVector CurrentPlayerPosition;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float LayerStreamingDistance = 2.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float RebaseOriginDistance = 2.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool ShouldTilesBlockOnLoad = false;

#if WITH_EDITOR

  void PrintMapInfo();

  FColor PositonMsgColor = FColor::Purple;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool bPrintMapInfo = true;

#endif // WITH_EDITOR

};
