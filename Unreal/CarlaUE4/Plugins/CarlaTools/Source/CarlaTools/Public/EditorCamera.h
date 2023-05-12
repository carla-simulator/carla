// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include "CoreMinimal.h"
#include "EditorCamera.generated.h"




UCLASS(BlueprintType)
class CARLATOOLS_API AEditorCameraUtils :
    public AActor
{
    GENERATED_BODY()
public:

    UFUNCTION(BlueprintCallable, CallInEditor)
    void Get();

    UFUNCTION(BlueprintCallable, CallInEditor)
    void Set();



    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FTransform CameraTransform;

};
