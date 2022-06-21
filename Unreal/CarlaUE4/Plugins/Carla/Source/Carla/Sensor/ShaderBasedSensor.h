// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/SceneCaptureSensor.h"

#include "ShaderBasedSensor.generated.h"

/// A shader parameter value to change when the material
/// instance is available.
USTRUCT(BlueprintType)
struct CARLA_API FShaderFloatParameterValue
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int ShaderIndex;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FName ParameterName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float Value = 0.0f;
};

/// A shader in AShaderBasedSensor.
USTRUCT(BlueprintType)
struct CARLA_API FSensorShader
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UMaterialInstanceDynamic *PostProcessMaterial = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float Weight = 1.0f;
};

/// A sensor that produces data by applying post-process materials (shaders) to
/// a scene capture image.
///
/// @warning Shaders must be added before BeginPlay.
UCLASS(Abstract)
class CARLA_API AShaderBasedSensor : public ASceneCaptureSensor
{
  GENERATED_BODY()

public:

  AShaderBasedSensor(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
  {
    EnablePostProcessingEffects(false);
  }

  void Set(const FActorDescription &ActorDescription) override;

  /// Load the UMaterialInstanceDynamic at the given @a Path and
  /// append it to the list of shaders with @a Weight.
  ///
  /// @return Whether it succeeded.
  UFUNCTION(BlueprintCallable)
  // bool LoadPostProcessingMaterial(const FString &Path, float Weight = 1.0f);
  bool AddPostProcessingMaterial(const FString &Path);

  /// Add a post-processing shader.
  UFUNCTION(BlueprintCallable)
  void AddShader(const FSensorShader &Shader)
  {
    Shaders.Add(Shader);
  }

  void SetFloatShaderParameter(uint8_t ShaderIndex, const FName &ParameterName, float Value);

protected:

  void SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture) override;

private:

  UPROPERTY()
  TArray<UMaterial*> MaterialsFound;

  UPROPERTY()
  TArray<FSensorShader> Shaders;

  UPROPERTY()
  TArray<FShaderFloatParameterValue> FloatShaderParams;
};
