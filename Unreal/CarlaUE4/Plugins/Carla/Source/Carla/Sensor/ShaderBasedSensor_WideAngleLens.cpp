// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/ShaderBasedSensor_WideAngleLens.h"

#include "ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Actor/ActorBlueprintFunctionLibrary.h"

AShaderBasedSensor_WideAngleLens::AShaderBasedSensor_WideAngleLens(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
    EnablePostProcessingEffects(false);
}

bool AShaderBasedSensor_WideAngleLens::AddPostProcessingMaterial(const FString& Path)
{
    ConstructorHelpers::FObjectFinder<UMaterial> Loader(*Path);

    if (Loader.Succeeded())
    {
        MaterialsFound.Add(Loader.Object);
    }

    return Loader.Succeeded();
}

void AShaderBasedSensor_WideAngleLens::SetUpSceneCaptureComponents(TArrayView<USceneCaptureComponent2D_CARLA*> SceneCaptures)
{
    for (auto SceneCapture : SceneCaptures)
    {
        for (auto MaterialFound : MaterialsFound)
        {
            // Create a dynamic instance of the Material (Shader)
            AddShader({ UMaterialInstanceDynamic::Create(MaterialFound, this), 1.0 });
        }

        for (const auto& Shader : Shaders)
        {
            // Attach the instance into the blendables
            SceneCapture->PostProcessSettings.AddBlendable(Shader.PostProcessMaterial, Shader.Weight);
        }
    }

    // Set the value for each Float parameter in the shader
    if (Shaders.Num() != 0)
    {
        for (auto SceneCapture : SceneCaptures)
        {
            for (const auto& ParameterValue : FloatShaderParams)
            {
                Shaders[ParameterValue.ShaderIndex].PostProcessMaterial->SetScalarParameterValue(
                    ParameterValue.ParameterName,
                    ParameterValue.Value);
            }
        }
    }
}

void AShaderBasedSensor_WideAngleLens::Set(const FActorDescription& Description)
{
    Super::Set(Description);
    UActorBlueprintFunctionLibrary::SetCamera(Description, this);
}

void AShaderBasedSensor_WideAngleLens::SetFloatShaderParameter(
    uint8_t ShaderIndex,
    const FName& ParameterName,
    float Value)
{
    FloatShaderParams.Add({ ShaderIndex, ParameterName, Value });
}
