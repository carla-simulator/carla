// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/SceneCaptureSensor.h"

#include "ShaderBasedSensor.generated.h"



/*  @CARLA_UE5

    The FPixelReader class has been deprecated, as its functionality
    is now split between ImageUtil::ReadImageDataAsync (see Sensor/ImageUtil.h)
    and ASensor::SendDataToClient.
    Here's a brief example of how to use both:

    if (!AreClientsListening()) // Ideally, check whether there are any clients.
        return;

    auto FrameIndex = FCarlaEngine::GetFrameCounter();
    ImageUtil::ReadImageDataAsync(
        *GetCaptureRenderTarget(),
        [this](
            const void* MappedPtr,
            size_t RowPitch,
            size_t BufferHeight,
            EPixelFormat Format,
            FIntPoint Extent)
        {
            TArray<FColor> ImageData;
            // Parse the raw data into ImageData...
            SendDataToClient(
                *this,
                ImageData,
                FrameIndex);
            return true;
        });

    Alternatively, if you just want to retrieve the pixels as
    FColor/FLinearColor, you can just use ReadImageDataAsyncFColor
    or ReadImageDataAsyncFLinearColor.

*/



/// A shader parameter value to change when the material
/// instance is available.
USTRUCT(BlueprintType)
struct CARLA_API FShaderFloatParameterValue
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int ShaderIndex = -1;

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

  AShaderBasedSensor(const FObjectInitializer &ObjectInitializer);

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
