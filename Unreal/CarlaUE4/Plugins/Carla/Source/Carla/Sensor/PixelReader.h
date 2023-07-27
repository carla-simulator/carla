// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreGlobals.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/ImageWriteQueue/Public/ImagePixelData.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <D3d12.h>
#endif

#include "Carla/Game/CarlaEngine.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Logging.h>
#include <carla/Buffer.h>
#include <carla/BufferView.h>
#include <carla/sensor/SensorRegistry.h>
#include <compiler/enable-ue4-macros.h>

// =============================================================================
// -- FPixelReader -------------------------------------------------------------
// =============================================================================

/// Utils for reading pixels from UTextureRenderTarget2D.
///
/// @todo This class only supports PF_R8G8B8A8 format.
class FPixelReader
{
public:

  using Payload = std::function<void(void *, uint32, uint32, uint32)>;

  /// Copy the pixels in @a RenderTarget into @a BitMap.
  ///
  /// @pre To be called from game-thread.
  static bool WritePixelsToArray(
      UTextureRenderTarget2D &RenderTarget,
      TArray<FColor> &BitMap);

  /// Dump the pixels in @a RenderTarget.
  ///
  /// @pre To be called from game-thread.
  static TUniquePtr<TImagePixelData<FColor>> DumpPixels(
      UTextureRenderTarget2D &RenderTarget);

  /// Asynchronously save the pixels in @a RenderTarget to disk.
  ///
  /// @pre To be called from game-thread.
  static TFuture<bool> SavePixelsToDisk(
      UTextureRenderTarget2D &RenderTarget,
      const FString &FilePath);

  /// Asynchronously save the pixels in @a PixelData to disk.
  ///
  /// @pre To be called from game-thread.
  static TFuture<bool> SavePixelsToDisk(
      TUniquePtr<TImagePixelData<FColor>> PixelData,
      const FString &FilePath);

  /// Convenience function to enqueue a render command that sends the pixels
  /// down the @a Sensor's data stream. It expects a sensor derived from
  /// ASceneCaptureSensor or compatible.
  ///
  /// Note that the serializer needs to define a "header_offset" that it's
  /// allocated in front of the buffer.
  ///
  /// @pre To be called from game-thread.
  template <typename TSensor, typename TPixel>
  static void SendPixelsInRenderThread(TSensor &Sensor, bool use16BitFormat = false, std::function<TArray<TPixel>(void *, uint32)> Conversor = {});

  /// Copy the pixels in @a RenderTarget into @a Buffer.
  ///
  /// @pre To be called from render-thread.
  static void WritePixelsToBuffer(
      const UTextureRenderTarget2D &RenderTarget,
      uint32 Offset,
      FRHICommandListImmediate &InRHICmdList,
      FPixelReader::Payload FuncForSending);

};

// =============================================================================
// -- FPixelReader::SendPixelsInRenderThread -----------------------------------
// =============================================================================

template <typename TSensor, typename TPixel>
void FPixelReader::SendPixelsInRenderThread(TSensor &Sensor, bool use16BitFormat, std::function<TArray<TPixel>(void *, uint32)> Conversor)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FPixelReader::SendPixelsInRenderThread);
  check(Sensor.CaptureRenderTarget != nullptr);

  if (!Sensor.HasActorBegunPlay() || Sensor.IsPendingKill())
  {
    return;
  }

  /// Blocks until the render thread has finished all it's tasks.
  Sensor.EnqueueRenderSceneImmediate();

  // Enqueue a command in the render-thread that will write the image buffer to
  // the data stream. The stream is created in the capture thus executed in the
  // game-thread.
  ENQUEUE_RENDER_COMMAND(FWritePixels_SendPixelsInRenderThread)
  (
    [&Sensor, use16BitFormat, Conversor = std::move(Conversor)](auto &InRHICmdList) mutable
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("FWritePixels_SendPixelsInRenderThread");

      /// @todo Can we make sure the sensor is not going to be destroyed?
      if (!Sensor.IsPendingKill())
      {
        FPixelReader::Payload FuncForSending =
          [&Sensor, Frame = FCarlaEngine::GetFrameCounter(), Conversor = std::move(Conversor)](void *LockedData, uint32 Size, uint32 Offset, uint32 ExpectedRowBytes)
          {
            if (Sensor.IsPendingKill()) return;

            TArray<TPixel> Converted;

            // optional conversion of data
            if (Conversor)
            {
              TRACE_CPUPROFILER_EVENT_SCOPE_STR("Data conversion");
              Converted = Conversor(LockedData, Size);
              LockedData = reinterpret_cast<void *>(Converted.GetData());
              Size = Converted.Num() * Converted.GetTypeSize();
            }

            auto Stream = Sensor.GetDataStream(Sensor);
            Stream.SetFrameNumber(Frame);
            auto Buffer = Stream.PopBufferFromPool();

            uint32 CurrentRowBytes = ExpectedRowBytes;

#ifdef _WIN32
            // DirectX uses additional bytes to align each row to 256 boundry,
            // so we need to remove that extra data
            if (IsD3DPlatform(GMaxRHIShaderPlatform, false))
            {
              CurrentRowBytes = Align(ExpectedRowBytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
              if (ExpectedRowBytes != CurrentRowBytes)
              {
                TRACE_CPUPROFILER_EVENT_SCOPE_STR("Buffer Copy (windows, row by row)");
                Buffer.reset(Offset + Size);
                auto DstRow = Buffer.begin() + Offset;
                const uint8 *SrcRow = reinterpret_cast<uint8 *>(LockedData);
                uint32 i = 0;
                while (i < Size)
                {
                  FMemory::Memcpy(DstRow, SrcRow, ExpectedRowBytes);
                  DstRow += ExpectedRowBytes;
                  SrcRow += CurrentRowBytes;
                  i += ExpectedRowBytes;
                }
              }
            }
#endif // _WIN32

            if (ExpectedRowBytes == CurrentRowBytes)
            {
              check(ExpectedRowBytes == CurrentRowBytes);
              TRACE_CPUPROFILER_EVENT_SCOPE_STR("Buffer Copy");
              Buffer.copy_from(Offset, boost::asio::buffer(LockedData, Size));
            }

            {
              // send
              TRACE_CPUPROFILER_EVENT_SCOPE_STR("Sending buffer");
              if(Buffer.data())
              {
                // serialize data
                carla::Buffer BufferReady(std::move(carla::sensor::SensorRegistry::Serialize(Sensor, std::move(Buffer))));
                carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(BufferReady));

                // ROS2
                #if defined(WITH_ROS2)
                auto ROS2 = carla::ros2::ROS2::GetInstance();
                if (ROS2->IsEnabled())
                {
                  TRACE_CPUPROFILER_EVENT_SCOPE_STR("ROS2 Send PixelReader");
                  auto StreamId = carla::streaming::detail::token_type(Sensor.GetToken()).get_stream_id();
                  auto Res = std::async(std::launch::async, [&Sensor, ROS2, &Stream, StreamId, BufView]()
                  {
                    // get resolution of camera
                    int W = -1, H = -1;
                    float Fov = -1.0f;
                    auto WidthOpt = Sensor.GetAttribute("image_size_x");
                    if (WidthOpt.has_value())
                      W = FCString::Atoi(*WidthOpt->Value);
                    auto HeightOpt = Sensor.GetAttribute("image_size_y");
                    if (HeightOpt.has_value())
                      H = FCString::Atoi(*HeightOpt->Value);
                    auto FovOpt = Sensor.GetAttribute("fov");
                    if (FovOpt.has_value())
                      Fov = FCString::Atof(*FovOpt->Value);
                    // send data to ROS2
                    AActor* ParentActor = Sensor.GetAttachParentActor();
                    if (ParentActor)
                    {
                      FTransform LocalTransformRelativeToParent = Sensor.GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
                      ROS2->ProcessDataFromCamera(Stream.GetSensorType(), StreamId, LocalTransformRelativeToParent, W, H, Fov, BufView, &Sensor);
                    }
                    else
                    {
                      ROS2->ProcessDataFromCamera(Stream.GetSensorType(), StreamId, Stream.GetSensorTransform(), W, H, Fov, BufView, &Sensor);
                    }
                  });
                }
                #endif

                // network
                SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
                TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
                Stream.Send(Sensor, BufView);
              }
            }
          };

          WritePixelsToBuffer(
              *Sensor.CaptureRenderTarget,
              carla::sensor::SensorRegistry::get<TSensor *>::type::header_offset,
              InRHICmdList,
              std::move(FuncForSending));
        }
      }
    );

  // Blocks until the render thread has finished all it's tasks
  Sensor.WaitForRenderThreadToFinish();
}
