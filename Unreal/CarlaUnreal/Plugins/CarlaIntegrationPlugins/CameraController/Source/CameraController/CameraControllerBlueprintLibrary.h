#pragma once

#include <util/ue-header-guard-begin.h>
#include "Kismet/BlueprintFunctionLibrary.h"
#include <util/ue-header-guard-end.h>

#include "CameraControllerBlueprintLibrary.generated.h"

class ACameraControllerActor;

UCLASS()
class CAMERACONTROLLER_API UCameraControllerBlueprintLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

  public:
  UFUNCTION(BlueprintCallable)
  static FActorDefinition MakeCameraControllerDefinition(TSubclassOf<ACameraControllerActor> Class);

  static void MakeCameraControllerDefinition(bool &Success, FActorDefinition &Definition, TSubclassOf<ACameraControllerActor> Class);

  UFUNCTION(BlueprintCallable)
  static void SetCameraController(const FActorDescription &Description, ACameraControllerActor *CameraController);
};