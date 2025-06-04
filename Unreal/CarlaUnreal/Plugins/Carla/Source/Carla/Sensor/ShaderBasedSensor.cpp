// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/ShaderBasedSensor.h"
#include "Carla.h"
#include "Actor/ActorBlueprintFunctionLibrary.h"

#include <util/ue-header-guard-begin.h>
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SceneCaptureComponent2D.h"
#include <util/ue-header-guard-end.h>

AShaderBasedSensor::AShaderBasedSensor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
    EnablePostProcessingEffects(false);
}

bool AShaderBasedSensor::AddPostProcessingMaterial(const FString &Path)
{
  ConstructorHelpers::FObjectFinder<UMaterial> Loader(*Path);
  if (Loader.Succeeded())
  {
    MaterialsFound.Add(Loader.Object);
  }
  return Loader.Succeeded();
}

void AShaderBasedSensor::SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture)
{
  for (const auto &MaterialFound : MaterialsFound)
  {
    // Create a dynamic instance of the Material (Shader)
    AddShader({UMaterialInstanceDynamic::Create(MaterialFound, this), 1.0});
  }

  for (const auto &Shader : Shaders)
  {
    // Attach the instance into the blendables
    SceneCapture.PostProcessSettings.AddBlendable(Shader.PostProcessMaterial, Shader.Weight);
  }

  // Set the value for each Float parameter in the shader
  for (const auto &ParameterValue : FloatShaderParams)
  {
    Shaders[ParameterValue.ShaderIndex].PostProcessMaterial->SetScalarParameterValue(
        ParameterValue.ParameterName,
        ParameterValue.Value);
  }
}

void AShaderBasedSensor::Set(const FActorDescription &Description)
{
  Super::Set(Description);
  UActorBlueprintFunctionLibrary::SetCamera(Description, this);
}

void AShaderBasedSensor::SetFloatShaderParameter(
    uint8_t ShaderIndex,
    const FName &ParameterName,
    float Value)
{
  FloatShaderParams.Add({ShaderIndex, ParameterName, Value});
}
