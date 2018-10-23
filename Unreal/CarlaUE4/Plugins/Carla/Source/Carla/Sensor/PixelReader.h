// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#include <carla/Buffer.h>
#include <carla/sensor/SensorRegistry.h>
#include <compiler/enable-ue4-macros.h>

#include "CoreGlobals.h"
#include "Engine/TextureRenderTarget2D.h"

// =============================================================================
// -- FPixelReader -------------------------------------------------------------
// =============================================================================

/// Utils for reading pixels from UTextureRenderTarget2D.
///
/// @todo This class only supports PF_R8G8B8A8 format.
class FPixelReader
{
public:

  /// Copy the pixels in @a RenderTarget into @a BitMap.
  ///
  /// @pre To be called from game-thread.
  static bool WritePixelsToArray(
      UTextureRenderTarget2D &RenderTarget,
      TArray<FColor> &BitMap);

  /// Save the pixels in @a RenderTarget to disk.
  ///
  /// @pre To be called from game-thread.
  static bool SavePixelsToDisk(
      UTextureRenderTarget2D &RenderTarget,
      const FString &FilePath);

  /// Copy the pixels in @a RenderTarget into @a Buffer.
  ///
  /// @pre To be called from render-thread.
  static void WritePixelsToBuffer(
      UTextureRenderTarget2D &RenderTarget,
      carla::Buffer &Buffer,
      uint32 Offset,
      FRHICommandListImmediate &InRHICmdList);

  /// Convenience function to enqueue a render command that sends the pixels
  /// down the @a Sensor's data stream. It expects a sensor derived from
  /// ASceneCaptureSensor or compatible.
  ///
  /// Note that the serializer needs to define a "header_offset" that it's
  /// allocated in front of the buffer.
  ///
  /// @pre To be called from game-thread.
  template <typename TSensor>
  static void SendPixelsInRenderThread(TSensor &Sensor);
};

// =============================================================================
// -- FPixelReader::SendPixelsInRenderThread -----------------------------------
// =============================================================================

template <typename TSensor>
void FPixelReader::SendPixelsInRenderThread(TSensor &Sensor)
{
  check(Sensor.CaptureRenderTarget != nullptr);

  // First we create the message header (needs to be created in the
  // game-thread).
  auto Header = Sensor.GetDataStream().MakeHeader(Sensor);
  // We need a shared ptr here because UE4 macros do not move the arguments -_-
  auto HeaderPtr = MakeShared<decltype(Header)>(std::move(Header));

  // Then we enqueue commands in the render-thread that will write the image
  // buffer to the data stream.

  auto WriteAndSend = [&Sensor, Hdr=std::move(HeaderPtr)](auto &InRHICmdList) mutable {
    auto &Stream = Sensor.GetDataStream();
    auto Buffer = Stream.PopBufferFromPool();
    WritePixelsToBuffer(
        *Sensor.CaptureRenderTarget,
        Buffer,
        carla::sensor::SensorRegistry::get<TSensor *>::type::header_offset,
        InRHICmdList);
    Stream.Send_Async(std::move(*Hdr), Sensor, std::move(Buffer));
  };

  ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
      FWritePixels_Vulkan,
      std::function<void(FRHICommandListImmediate &)>, WriteAndSendFunction, std::move(WriteAndSend),
  {
    WriteAndSendFunction(RHICmdList);
  });
}
