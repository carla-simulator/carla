// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/SceneCaptureSensor.h"

#include "ShaderBasedSensor.generated.h"

/// A shader in AShaderBasedSensor.
USTRUCT(BlueprintType)
struct CARLA_API FSensorShader
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UMaterial *PostProcessMaterial = nullptr;

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

  /// Load the UMaterial at the given @a Path and append it to the list of
  /// shaders with @a Weight.
  ///
  /// @return Whether it succeeded.
  UFUNCTION(BlueprintCallable)
  bool LoadPostProcessingMaterial(const FString &Path, float Weight = 1.0f);

  /// Add a post-processing shader.
  UFUNCTION(BlueprintCallable)
  void AddShader(const FSensorShader &Shader)
  {
    Shaders.Add(Shader);
  }

protected:

  void SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture) override;

private:

  UPROPERTY(EditAnywhere)
  TArray<FSensorShader> Shaders;
};
