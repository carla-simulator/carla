// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/ShaderBasedSensor.h"
#include "Carla.h"
#include "Actor/ActorBlueprintFunctionLibrary.h"

#include <util/ue-header-guard-begin.h>
#include "Materials/Material.h"
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
  // ConstructorHelpers::FObjectFinder is documented as constructor-only and
  // has undefined behavior when used from runtime functions. FSoftObjectPath
  // is the runtime-safe path resolver; it accepts both plain object paths
  // and the class-prefixed export-text form ("Material'/Game/.../Foo'") that
  // FObjectFinder previously normalised internally.
  UMaterial *Material = Cast<UMaterial>(FSoftObjectPath(Path).TryLoad());
  if (Material != nullptr)
  {
    MaterialsFound.Add(Material);
    return true;
  }
  UE_LOG(LogCarla, Error,
      TEXT("AShaderBasedSensor::AddPostProcessingMaterial: failed to load material '%s'"),
      *Path);
  return false;
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
