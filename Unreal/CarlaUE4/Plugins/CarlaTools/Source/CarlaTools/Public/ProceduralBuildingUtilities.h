// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityActor.h"
#include "ProceduralBuildingUtilities.generated.h"

struct FIntRect;

/**
 * 
 */
UCLASS()
class CARLATOOLS_API AProceduralBuildingUtilities : public AEditorUtilityActor
{
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  void GenerateImpostor();

  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  void CookProceduralBuildingToMesh(const FString& DestinationPath);

};
