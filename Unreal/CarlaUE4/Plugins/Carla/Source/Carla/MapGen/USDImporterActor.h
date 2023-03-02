// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"

#include "USDImporterActor.generated.h"

UCLASS()
class CARLA_API AUSDImporterActor : public AActor
{
    GENERATED_BODY()

public:

  AUSDImporterActor(const FObjectInitializer &ObjectInitializer);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Carla USD Importer")
  FString USDPath = "";

  UFUNCTION(CallInEditor, BlueprintCallable, Category = "Carla USD Importer")
  void LoadUSDFile();

private:
  UPROPERTY(Category="Carla USD Importer", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  USceneComponent* RootSceneComponent;

};
