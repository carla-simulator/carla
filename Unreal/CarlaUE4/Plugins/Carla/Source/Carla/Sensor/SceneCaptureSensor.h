// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Sensor/PixelReader.h"
#include "Carla/Sensor/Sensor.h"
#include "Carla/Sensor/UE4_Overridden/SceneCaptureComponent2D_CARLA.h"
#include "Carla/Sensor/ImageUtil.h"

#include "Async/Async.h"
#include "Renderer/Public/GBufferView.h"

#include <type_traits>
#include <tuple>

#include "SceneCaptureSensor.generated.h"



class UDrawFrustumComponent;
class UStaticMeshComponent;
class UTextureRenderTarget2D;



struct FCameraGBufferUint8
{
  /// Prevent this sensor to be spawned by users.
  using not_spawnable = void;

  void SetDataStream(FDataStream InStream)
  {
    Stream = std::move(InStream);
  }

  /// Replace the Stream associated with this sensor.
  void SetStream(FDataMultiStream InStream)
  {
    Stream = std::move(InStream);
  }
  /// Return the token that allows subscribing to this sensor's stream.
  auto GetToken() const
  {
    bIsUsed = true;
    return Stream.GetToken();
  }

  /// Dummy. Required for compatibility with other sensors only.
  FTransform GetActorTransform() const
  {
    return {};
  }

  /// Return the FDataStream associated with this sensor.
  ///
  /// You need to provide a reference to self, this is necessary for template
  /// deduction.
  auto GetDataStream(const UCarlaEpisode& Episode)
  {
    return Stream.MakeAsyncDataStream(*this, Episode.GetElapsedGameTime());
  }

  mutable bool bIsUsed = false;
  FDataStream Stream;
};



struct FCameraGBufferFloat
{
  /// Prevent this sensor to be spawned by users.
  using not_spawnable = void;

  void SetDataStream(FDataStream InStream)
  {
    Stream = std::move(InStream);
  }

  /// Replace the Stream associated with this sensor.
  void SetStream(FDataMultiStream InStream)
  {
    Stream = std::move(InStream);
  }
  /// Return the token that allows subscribing to this sensor's stream.
  auto GetToken() const
  {
    bIsUsed = true;
    return Stream.GetToken();
  }
  /// Dummy. Required for compatibility with other sensors only.
  FTransform GetActorTransform() const
  {
    return {};
  }

  /// Return the FDataStream associated with this sensor.
  ///
  /// You need to provide a reference to self, this is necessary for template
  /// deduction.
  auto GetDataStream(const UCarlaEpisode& Episode)
  {
    return Stream.MakeAsyncDataStream(*this, Episode.GetElapsedGameTime());
  }
  
  mutable bool bIsUsed = false;
  FDataStream Stream;
};






/// Base class for sensors using a USceneCaptureComponent2D for rendering the
/// scene. This class does not capture data, use
/// `FPixelReader::SendPixelsInRenderThread(*this)` in derived classes.
///
/// To access the USceneCaptureComponent2D override the
/// SetUpSceneCaptureComponent function.
///
/// @warning All the setters should be called before BeginPlay.
UCLASS(Abstract)
class CARLA_API ASceneCaptureSensor : public ASensor
{
  GENERATED_BODY()

  friend class ACarlaGameModeBase;
  friend class FPixelReader;
  friend class FPixelReader2;

public:

  ASceneCaptureSensor(const FObjectInitializer &ObjectInitializer);

  void Set(const FActorDescription &ActorDescription) override;

  void SetImageSize(uint32 Width, uint32 Height);

  uint32 GetImageWidth() const
  {
    return ImageWidth;
  }

  uint32 GetImageHeight() const
  {
    return ImageHeight;
  }

  UFUNCTION(BlueprintCallable)
  void EnablePostProcessingEffects(bool Enable = true)
  {
    bEnablePostProcessingEffects = Enable;
  }

  UFUNCTION(BlueprintCallable)
  bool ArePostProcessingEffectsEnabled() const
  {
    return bEnablePostProcessingEffects;
  }

  UFUNCTION(BlueprintCallable)
  void Enable16BitFormat(bool Enable = false)
  {
    bEnable16BitFormat = Enable;
  }

  UFUNCTION(BlueprintCallable)
  bool Is16BitFormatEnabled() const
  {
    return bEnable16BitFormat;
  }

  UFUNCTION(BlueprintCallable)
  void SetFOVAngle(float FOVAngle);

  UFUNCTION(BlueprintCallable)
  float GetFOVAngle() const;

  UFUNCTION(BlueprintCallable)
  void SetTargetGamma(float InTargetGamma)
  {
    TargetGamma = InTargetGamma;
  }

  UFUNCTION(BlueprintCallable)
  float GetTargetGamma() const
  {
    return TargetGamma;
  }

  UFUNCTION(BlueprintCallable)
  void SetExposureMethod(EAutoExposureMethod Method);

  UFUNCTION(BlueprintCallable)
  EAutoExposureMethod GetExposureMethod() const;

  UFUNCTION(BlueprintCallable)
  void SetExposureCompensation(float Compensation);

  UFUNCTION(BlueprintCallable)
  float GetExposureCompensation() const;

  UFUNCTION(BlueprintCallable)
  void SetShutterSpeed(float Speed);

  UFUNCTION(BlueprintCallable)
  float GetShutterSpeed() const;

  UFUNCTION(BlueprintCallable)
  void SetISO(float ISO);

  UFUNCTION(BlueprintCallable)
  float GetISO() const;

  UFUNCTION(BlueprintCallable)
  void SetAperture(float Aperture);

  UFUNCTION(BlueprintCallable)
  float GetAperture() const;

  UFUNCTION(BlueprintCallable)
  void SetFocalDistance(float Distance);

  UFUNCTION(BlueprintCallable)
  float GetFocalDistance() const;

  UFUNCTION(BlueprintCallable)
  void SetDepthBlurAmount(float Amount);

  UFUNCTION(BlueprintCallable)
  float GetDepthBlurAmount() const;

  UFUNCTION(BlueprintCallable)
  void SetDepthBlurRadius(float Radius);

  UFUNCTION(BlueprintCallable)
  float GetDepthBlurRadius() const;

  UFUNCTION(BlueprintCallable)
  void SetBladeCount(int Count);

  UFUNCTION(BlueprintCallable)
  int GetBladeCount() const;

  UFUNCTION(BlueprintCallable)
  void SetDepthOfFieldMinFstop(float MinFstop);

  UFUNCTION(BlueprintCallable)
  float GetDepthOfFieldMinFstop() const;

  UFUNCTION(BlueprintCallable)
  void SetFilmSlope(float Slope);

  UFUNCTION(BlueprintCallable)
  float GetFilmSlope() const;

  UFUNCTION(BlueprintCallable)
  void SetFilmToe(float Toe);

  UFUNCTION(BlueprintCallable)
  float GetFilmToe() const;

  UFUNCTION(BlueprintCallable)
  void SetFilmShoulder(float Shoulder);

  UFUNCTION(BlueprintCallable)
  float GetFilmShoulder() const;

  UFUNCTION(BlueprintCallable)
  void SetFilmBlackClip(float BlackClip);

  UFUNCTION(BlueprintCallable)
  float GetFilmBlackClip() const;

  UFUNCTION(BlueprintCallable)
  void SetFilmWhiteClip(float WhiteClip);

  UFUNCTION(BlueprintCallable)
  float GetFilmWhiteClip() const;

  UFUNCTION(BlueprintCallable)
  void SetExposureMinBrightness(float Brightness);

  UFUNCTION(BlueprintCallable)
  float GetExposureMinBrightness() const;

  UFUNCTION(BlueprintCallable)
  void SetExposureMaxBrightness(float Brightness);

  UFUNCTION(BlueprintCallable)
  float GetExposureMaxBrightness() const;

  UFUNCTION(BlueprintCallable)
  void SetExposureSpeedDown(float Speed);

  UFUNCTION(BlueprintCallable)
  float GetExposureSpeedDown() const;

  UFUNCTION(BlueprintCallable)
  void SetExposureSpeedUp(float Speed);

  UFUNCTION(BlueprintCallable)
  float GetExposureSpeedUp() const;

  UFUNCTION(BlueprintCallable)
  void SetExposureCalibrationConstant(float Constant);

  UFUNCTION(BlueprintCallable)
  float GetExposureCalibrationConstant() const;

  UFUNCTION(BlueprintCallable)
  void SetMotionBlurIntensity(float Intensity);

  UFUNCTION(BlueprintCallable)
  float GetMotionBlurIntensity() const;

  UFUNCTION(BlueprintCallable)
  void SetMotionBlurMaxDistortion(float MaxDistortion);

  UFUNCTION(BlueprintCallable)
  float GetMotionBlurMaxDistortion() const;

  UFUNCTION(BlueprintCallable)
  void SetMotionBlurMinObjectScreenSize(float ScreenSize);

  UFUNCTION(BlueprintCallable)
  float GetMotionBlurMinObjectScreenSize() const;

  UFUNCTION(BlueprintCallable)
  void SetLensFlareIntensity(float Intensity);

  UFUNCTION(BlueprintCallable)
  float GetLensFlareIntensity() const;

  UFUNCTION(BlueprintCallable)
  void SetBloomIntensity(float Intensity);

  UFUNCTION(BlueprintCallable)
  float GetBloomIntensity() const;

  UFUNCTION(BlueprintCallable)
  void SetWhiteTemp(float Temp);

  UFUNCTION(BlueprintCallable)
  float GetWhiteTemp() const;

  UFUNCTION(BlueprintCallable)
  void SetWhiteTint(float Tint);

  UFUNCTION(BlueprintCallable)
  float GetWhiteTint() const;

  UFUNCTION(BlueprintCallable)
  void SetChromAberrIntensity(float Intensity);

  UFUNCTION(BlueprintCallable)
  float GetChromAberrIntensity() const;

  UFUNCTION(BlueprintCallable)
  void SetChromAberrOffset(float ChromAberrOffset);

  UFUNCTION(BlueprintCallable)
  float GetChromAberrOffset() const;

  /// Use for debugging purposes only.
  UFUNCTION(BlueprintCallable)
  bool ReadPixels(TArray<FColor> &BitMap) const
  {
    check(CaptureRenderTarget != nullptr);
    return FPixelReader::WritePixelsToArray(*CaptureRenderTarget, BitMap);
  }

  /// Use for debugging purposes only.
  UFUNCTION(BlueprintCallable)
  void SaveCaptureToDisk(const FString &FilePath) const
  {
    check(CaptureRenderTarget != nullptr);
    FPixelReader::SavePixelsToDisk(*CaptureRenderTarget, FilePath);
  }

  UFUNCTION(BlueprintCallable)
  USceneCaptureComponent2D *GetCaptureComponent2D()
  {
    return CaptureComponent2D;
  }

  UFUNCTION(BlueprintCallable)
  UTextureRenderTarget2D *GetCaptureRenderTarget()
  {
    return CaptureRenderTarget;
  }

  /// Immediate enqueues render commands of the scene at the current time.
  void EnqueueRenderSceneImmediate();

  /// Blocks until the render thread has finished all it's tasks.
  void WaitForRenderThreadToFinish() {
    TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureSensor::WaitForRenderThreadToFinish);
    // FlushRenderingCommands();
  }

  std::tuple<
      FCameraGBufferUint8,
      FCameraGBufferFloat,
      FCameraGBufferUint8,
      FCameraGBufferUint8,
      FCameraGBufferUint8,
      FCameraGBufferUint8,
      FCameraGBufferUint8,
      FCameraGBufferUint8,
      FCameraGBufferUint8,
      FCameraGBufferUint8,
      FCameraGBufferUint8,
      FCameraGBufferFloat,
      FCameraGBufferUint8
  > GBufferStreams;

protected:
    
  void CaptureSceneExtended();

  virtual void SendGBufferTextures(FGBufferRequest& GBuffer);

  virtual void BeginPlay() override;

  virtual void PrePhysTick(float DeltaSeconds) override;
  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;

  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  virtual void SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture) {}

  /// Render target necessary for scene capture.
  UPROPERTY(EditAnywhere)
  UTextureRenderTarget2D *CaptureRenderTarget = nullptr;

  /// Scene capture component.
  UPROPERTY(EditAnywhere)
  USceneCaptureComponent2D_CARLA *CaptureComponent2D = nullptr;

  UPROPERTY(EditAnywhere)
  float TargetGamma = 2.4f;

  /// Image width in pixels.
  UPROPERTY(EditAnywhere)
  uint32 ImageWidth = 800u;

  /// Image height in pixels.
  UPROPERTY(EditAnywhere)
  uint32 ImageHeight = 600u;

  /// Whether to render the post-processing effects present in the scene.
  UPROPERTY(EditAnywhere)
  bool bEnablePostProcessingEffects = true;

  /// Whether to change render target format to PF_A16B16G16R16, offering 16bit / channel
  UPROPERTY(EditAnywhere)
  bool bEnable16BitFormat = false;
  
private:

    template <typename PixelT>
    static void SendGBufferFillEmptyTexture(
        FGBufferRequest& GBufferData,
        FIntPoint& ImageSize,
        TArray<PixelT>& Pixels)
    {
        ImageSize = GBufferData.ViewRect.Size();
        Pixels.AddUninitialized(ImageSize.X * ImageSize.Y);
        for (auto& Pixel : Pixels)
            Pixel = PixelT::Black;
    }

  template <EGBufferTextureID TextureID, typename SensorT>
  void SendGBuffer(SensorT& Sensor, FGBufferRequest& GBufferData)
  {
      LLM_SCOPE(ELLMTag::CARLA_LLM_TAG_08);

      auto& GBufferStream = std::get<(size_t)TextureID>(GBufferStreams);

      using CameraGBufferT = std::remove_reference_t<decltype(GBufferStream)>;
      using PixelType = typename std::conditional<
          std::is_same<CameraGBufferT, FCameraGBufferUint8>::value,
          FColor,
          FLinearColor>::type;

      FIntPoint ImageSize;
      TArray<PixelType> Pixels;

      if (GBufferData.WaitForTextureTransfer(TextureID))
      {
          LLM_SCOPE(ELLMTag::CARLA_LLM_TAG_09);

        TRACE_CPUPROFILER_EVENT_SCOPE_STR("GBuffer Decode");

        FIntPoint SourceExtent = {};
        void* PixelData;
        int32 SourcePitch;

        if (GBufferData.MapTextureData(TextureID, PixelData, SourcePitch, SourceExtent))
        {
            LLM_SCOPE(ELLMTag::CARLA_LLM_TAG_10);

            auto Format = GBufferData.Readbacks[(size_t)TextureID]->GetFormat();
            ImageSize = GBufferData.ViewRect.Size();
            Pixels.AddUninitialized(ImageSize.X * ImageSize.Y);
            FReadSurfaceDataFlags Flags = {};
            Flags.SetLinearToGamma(true);
            ImageUtil::DecodePixelsByFormat(
                Pixels,
                TArrayView<uint8>((uint8*)PixelData, SourcePitch * SourceExtent.Y),
                SourcePitch,
                SourceExtent,
                ImageSize,
                Format,
                Flags);
            GBufferData.UnmapTextureData(TextureID);
        }
        else
        {
            SendGBufferFillEmptyTexture(GBufferData, ImageSize, Pixels);
        }
      }
      else
      {
          SendGBufferFillEmptyTexture(GBufferData, ImageSize, Pixels);
      }

      auto Stream = GBufferStream.GetDataStream(GetEpisode());
      auto Buffer = Stream.PopBufferFromPool();

      Buffer.copy_from(
        carla::sensor::SensorRegistry::get<CameraGBufferT*>::type::header_offset,
        boost::asio::buffer(&Pixels[0], Pixels.Num() * sizeof(PixelType)));

      if (Buffer.empty()) {
        return;
      }

      SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");

      Stream.Send(
        GBufferStream,
        std::move(Buffer),
        ImageSize.X,
        ImageSize.Y,
        Sensor.GetFOVAngle());
  }

 protected:

  template <typename SensorT>
  void SendGBufferTexturesInternal(SensorT& Sensor, FGBufferRequest& GBufferData)
  {
      LLM_SCOPE(ELLMTag::CARLA_LLM_TAG_07);

    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::SceneColor)) != 0)
        SendGBuffer<EGBufferTextureID::SceneColor>(Sensor, GBufferData);
    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::SceneDepth)) != 0)
        SendGBuffer<EGBufferTextureID::SceneDepth>(Sensor, GBufferData);
    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::SceneStencil)) != 0)
        SendGBuffer<EGBufferTextureID::SceneStencil>(Sensor, GBufferData);
    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::GBufferA)) != 0)
        SendGBuffer<EGBufferTextureID::GBufferA>(Sensor, GBufferData);
    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::GBufferB)) != 0)
        SendGBuffer<EGBufferTextureID::GBufferB>(Sensor, GBufferData);
    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::GBufferC)) != 0)
        SendGBuffer<EGBufferTextureID::GBufferC>(Sensor, GBufferData);
    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::GBufferD)) != 0)
        SendGBuffer<EGBufferTextureID::GBufferD>(Sensor, GBufferData);
    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::GBufferE)) != 0)
        SendGBuffer<EGBufferTextureID::GBufferE>(Sensor, GBufferData);
    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::GBufferF)) != 0)
        SendGBuffer<EGBufferTextureID::GBufferF>(Sensor, GBufferData);
    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::Velocity)) != 0)
        SendGBuffer<EGBufferTextureID::Velocity>(Sensor, GBufferData);
    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::SSAO)) != 0)
        SendGBuffer<EGBufferTextureID::SSAO>(Sensor, GBufferData);
    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::CustomDepth)) != 0)
        SendGBuffer<EGBufferTextureID::CustomDepth>(Sensor, GBufferData);
    if ((GBufferData.DesiredTexturesMask & (1U << (uint8_t)EGBufferTextureID::CustomStencil)) != 0)
        SendGBuffer<EGBufferTextureID::CustomStencil>(Sensor, GBufferData);
  }

};
