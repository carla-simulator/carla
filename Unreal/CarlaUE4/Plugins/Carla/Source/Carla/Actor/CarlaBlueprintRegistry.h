// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
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

  // Prop Attributes
  const FString PROP_NAME = FString("name");
  const FString PROP_PATH = FString("path");
  const FString PROP_SIZE = FString("size");

public:

  UFUNCTION(Category = "Carla Blueprint Registry", BlueprintCallable)
  void AddToCarlaBlueprintRegistry(const TArray<FPropParameters> &PropParametersArray);

  UFUNCTION(Category = "Carla Blueprint Registry", BlueprintCallable)
  void LoadPropDefinitions(TArray<FActorDefinition> &Definitions);

};
