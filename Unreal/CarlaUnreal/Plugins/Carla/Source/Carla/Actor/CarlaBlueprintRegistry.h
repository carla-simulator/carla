// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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

public:

  UFUNCTION(Category = "Carla Blueprint Registry", BlueprintCallable)
  static void AddToCarlaBlueprintRegistry(const TArray<FPropParameters> &PropParametersArray);

  UFUNCTION(Category = "Carla Blueprint Registry", BlueprintCallable)
  static void LoadPropDefinitions(TArray<FPropParameters> &PropParametersArray);

  /// Parse the given *.Package.json files in order (later files override
  /// earlier entries with the same name) into @a PropParametersArray.
  static void LoadPropDefinitionsFromFiles(
      const TArray<FString> &PropFileNames,
      TArray<FPropParameters> &PropParametersArray);

  /// Every <Content>/Config/**/*.Package.json of every mounted content pack.
  static TArray<FString> FindContentPackPropFiles();

};
