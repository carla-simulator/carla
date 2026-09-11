// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityObject.h"
#include "DGTImplementable.generated.h"

UCLASS(Blueprintable, BlueprintType)
class CARLATOOLS_API UDGTImplementable : public UEditorUtilityObject
{
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "CarlaDigitalTwinsTool")
  void RunUtilityFunction(UWorld* World, class UOpenDriveToMap* OpenDriveToMap);
  
  UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "CarlaDigitalTwinsTool")
  void RunUtilityFunctionWithSplinesParamters(UWorld* World,  class UOpenDriveToMap* OpenDriveToMap, const TArray<class USplineComponent*>& Splines);

};
