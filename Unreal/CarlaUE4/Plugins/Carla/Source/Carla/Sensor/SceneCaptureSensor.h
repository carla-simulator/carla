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
  template <typename SensorT>
  FAsyncDataStream GetDataStream(const SensorT &Self)
  {
    return Stream.MakeAsyncDataStream(Self, Self.GetEpisode().GetElapsedGameTime());
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
  template <typename SensorT>
  FAsyncDataStream GetDataStream(const SensorT &Self)
  {
    return Stream.MakeAsyncDataStream(Self, Self.GetEpisode().GetElapsedGameTime());
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

  struct
  {
    FCameraGBufferUint8 SceneColor;
    FCameraGBufferUint8 SceneDepth;
    FCameraGBufferUint8 SceneStencil;
    FCameraGBufferUint8 GBufferA;
    FCameraGBufferUint8 GBufferB;
    FCameraGBufferUint8 GBufferC;
    FCameraGBufferUint8 GBufferD;
    FCameraGBufferUint8 GBufferE;
    FCameraGBufferUint8 GBufferF;
    FCameraGBufferUint8 Velocity;
    FCameraGBufferUint8 SSAO;
    FCameraGBufferUint8 CustomDepth;
    FCameraGBufferUint8 CustomStencil;
  } CameraGBuffers;

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

  template <
    typename SensorT,
    typename CameraGBufferT>
  static void SendGBuffer(
      SensorT& Self,
      CameraGBufferT& CameraGBuffer,
      FGBufferRequest& GBufferData,
      EGBufferTextureID TextureID)
  {
      using PixelType = typename std::conditional<
        std::is_same<std::remove_reference_t<CameraGBufferT>, FCameraGBufferUint8>::value,
        FColor,
        FLinearColor>::type;
      FIntPoint ViewSize;
      TArray<PixelType> Pixels;
      if (GBufferData.WaitForTextureTransfer(TextureID))
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("GBuffer Decode");
        void* PixelData;
        int32 SourcePitch;
        FIntPoint SourceExtent;
        GBufferData.MapTextureData(
          TextureID,
          PixelData,
          SourcePitch,
          SourceExtent);
        auto Format = GBufferData.Readbacks[(size_t)TextureID]->GetFormat();
        ViewSize = GBufferData.ViewRect.Size();
        Pixels.AddUninitialized(ViewSize.X * ViewSize.Y);
        FReadSurfaceDataFlags Flags = {};
        Flags.SetLinearToGamma(true);
        ImageUtil::DecodePixelsByFormat(
          PixelData,
          SourcePitch,
          SourceExtent,
          ViewSize,
          Format,
          Flags,
          Pixels);
        GBufferData.UnmapTextureData(TextureID);
      }
      else
      {
        ViewSize = GBufferData.ViewRect.Size();
        Pixels.SetNum(ViewSize.X * ViewSize.Y);
        for (auto& Pixel : Pixels)
          Pixel = PixelType::Black;
      }
      auto GBufferStream = CameraGBuffer.GetDataStream(Self);
      auto Buffer = GBufferStream.PopBufferFromPool();
      Buffer.copy_from(
        carla::sensor::SensorRegistry::get<CameraGBufferT*>::type::header_offset,
        Pixels);
      if (Buffer.empty()) {
        return;
      }
      SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
      GBufferStream.Send(
        CameraGBuffer,
        std::move(Buffer),
        ViewSize.X,
        ViewSize.Y,
        Self.GetFOVAngle());
  }

protected:

  template <typename T>
  void SendGBufferTexturesInternal(T& Self, FGBufferRequest& GBufferData)
  {
    for (size_t i = 0; i != FGBufferRequest::TextureCount; ++i)
    {
      if ((GBufferData.DesiredTexturesMask & (UINT64_C(1) << i)) == 0) {
        continue;
      }
      auto& C = CameraGBuffers;
      EGBufferTextureID ID = (EGBufferTextureID)i;
      switch (ID)
      {
      case EGBufferTextureID::SceneColor:
        SendGBuffer(Self, C.SceneColor, GBufferData, ID);
        break;
      case EGBufferTextureID::SceneDepth:
        SendGBuffer(Self, C.SceneDepth, GBufferData, ID);
        break;
      case EGBufferTextureID::SceneStencil:
        SendGBuffer(Self, C.SceneStencil, GBufferData, ID);
        break;
      case EGBufferTextureID::GBufferA:
        SendGBuffer(Self, C.GBufferA, GBufferData, ID);
        break;
      case EGBufferTextureID::GBufferB:
        SendGBuffer(Self, C.GBufferB, GBufferData, ID);
        break;
      case EGBufferTextureID::GBufferC:
        SendGBuffer(Self, C.GBufferC, GBufferData, ID);
        break;
      case EGBufferTextureID::GBufferD:
        SendGBuffer(Self, C.GBufferD, GBufferData, ID);
        break;
      case EGBufferTextureID::GBufferE:
        SendGBuffer(Self, C.GBufferE, GBufferData, ID);
        break;
      case EGBufferTextureID::GBufferF:
        SendGBuffer(Self, C.GBufferF, GBufferData, ID);
        break;
      case EGBufferTextureID::Velocity:
        SendGBuffer(Self, C.Velocity, GBufferData, ID);
        break;
      case EGBufferTextureID::SSAO:
        SendGBuffer(Self, C.SSAO, GBufferData, ID);
        break;
      case EGBufferTextureID::CustomDepth:
        SendGBuffer(Self, C.CustomDepth, GBufferData, ID);
        break;
      case EGBufferTextureID::CustomStencil:
        SendGBuffer(Self, C.CustomStencil, GBufferData, ID);
        break;
      default:
          abort();
      }
    }
  }

};
