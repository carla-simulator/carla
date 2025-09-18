// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/SceneCaptureSensor_WideAngleLens.h"

#include "ShaderBasedSensor_WideAngleLens.generated.h"

UCLASS(Abstract)
class CARLA_API AShaderBasedSensor_WideAngleLens :
    public ASceneCaptureSensor_WideAngleLens
{
    GENERATED_BODY()

public:

    AShaderBasedSensor_WideAngleLens(const FObjectInitializer& ObjectInitializer);

    void Set(const FActorDescription& ActorDescription) override;

    UFUNCTION(BlueprintCallable)
    bool AddPostProcessingMaterial(const FString& Path);

    /// Add a post-processing shader.
    UFUNCTION(BlueprintCallable)
    void AddShader(const FSensorShader& Shader)
    {
        Shaders.Add(Shader);
    }

    void SetFloatShaderParameter(uint8_t ShaderIndex, const FName& ParameterName, float Value);

protected:

    void SetUpSceneCaptureComponents(TArrayView<USceneCaptureComponent2D_CARLA*> SceneCaptures) override;

private:

    UPROPERTY()
    TArray<UMaterial*> MaterialsFound;

    UPROPERTY()
    TArray<FSensorShader> Shaders;

    UPROPERTY()
    TArray<FShaderFloatParameterValue> FloatShaderParams;
};
