// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "ActorBlueprintFunctionLibrary.h"

#include "CarlaBlueprintRegistry.generated.h"

UCLASS()
class UCarlaBlueprintRegistry : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

private:

  UPROPERTY()
  TSet<FString> PropNames;

public:

  UFUNCTION(Category = "Carla Blueprint Registry", BlueprintCallable)
  void AddToCarlaBlueprintRegistry(const TArray<FPropParameters> &PropParametersArray);

  UFUNCTION(Category = "Carla Blueprint Registry", BlueprintCallable)
  void LoadPropDefinitions(TArray<FActorDefinition> &Definitions);

};
