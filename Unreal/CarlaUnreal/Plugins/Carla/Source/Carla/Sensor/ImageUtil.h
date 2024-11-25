// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include <CoreMinimal.h>
#include <CoreGlobals.h>
#include <Engine/TextureRenderTarget2D.h>
#include "Runtime/ImageWriteQueue/Public/ImagePixelData.h"
#include <util/ue-header-guard-end.h>

#include <functional>

class AShaderBasedSensor;
class UTextureRenderTarget2D;

namespace ImageUtil
{
  // Callback for the untyped async image reading functions below.
  using ReadImageDataAsyncCallback = std::function<
    bool(
      const void*,          // Source image data.
      size_t,               // Number of pixels (NOT BYTES) per image row.
      size_t,               // Number of rows.
      EPixelFormat,         // Image pixel format.
      FIntPoint             // Image extent.
    )
  >;



  // Callback for the FColor async image reading functions below.
  using ReadImageDataAsyncCallbackFColor = std::function<
    bool(
      TArrayView<const FColor>, // Source image data as FColor array.
      FIntPoint                 // Image extent.
    )
  >;



  // Callback for the FLinearColor async image reading functions below.
  using ReadImageDataAsyncCallbackFLinearColor = std::function<
    bool(
      TArrayView<const FLinearColor>, // Source image data as FLinearColor array.
      FIntPoint                       // Image extent.
    )
  >;



  // Reads pixels in the specified format from a region of memory
  // into an FColor array.
  bool DecodePixelsByFormat(
    const void* PixelData,          // Image data to read from.
    int32 SourcePitch,              // Number of bytes per image row.
    FIntPoint Extent,               // Image extent.
    EPixelFormat Format,            // Image pixel format.
    FReadSurfaceDataFlags Flags,    // Read image flags.
    TArrayView<FColor> Out          // Output array view.
  );



  // Reads pixels in the specified format from a region of memory.
  // into an FLinearColor array.
  bool DecodePixelsByFormat(
    const void* PixelData,          // Image data to read from.
    int32 SourcePitch,              // Number of bytes per image row.
    FIntPoint Extent,               // Image extent.
    EPixelFormat Format,            // Image pixel format.
    FReadSurfaceDataFlags Flags,    // Read image flags.
    TArrayView<FLinearColor> Out    // Output array view.
  );



  // Reads image data from a UTextureRenderTarget2D using its ReadPixels method.
  // This function is mainly for testing purposes, as we provide faster alternatives.
  bool ReadImageData(
    UTextureRenderTarget2D& RenderTarget,               // Render target to read from.
    TArray<FColor>& Out                                 // Output array.
  );



  // Reads image data from a UTextureRenderTarget2D using its ReadPixels method.
  // This function is mainly for testing purposes, as we provide faster alternatives.
  bool ReadImageData(
    UTextureRenderTarget2D& RenderTarget,               // Render target to read from.
    TArray64<FColor>& Out                               // Output array.
  );



  // Reads image data from a UTextureRenderTarget2D using its ReadPixels method.
  // This function is mainly for testing purposes, as we provide faster alternatives.
  TUniquePtr<TImagePixelData<FColor>> ReadImageData(
    UTextureRenderTarget2D& RenderTarget                // Render target to read from.
  );



  // Reads and outputs the contents of a render target to a PNG file.
  // Currently uses ReadImageData under the hood.
  TFuture<bool> SaveImageData(
    UTextureRenderTarget2D& RenderTarget,               // Render target to read from.
    const FStringView& Path                             // Output image file path.
  );



  // Given a TImagePixelData<FColor>, saves it into a PNG file.
  TFuture<bool> SaveImageData(
    TUniquePtr<TImagePixelData<FColor>> Data,           // Render target to read from.
    const FStringView& Path                             // Output image file path.
  );



  // Asynchronously reads the contents of the specified RenderTarget
  // and calls the provided Callback.
  bool ReadImageDataAsync(
    UTextureRenderTarget2D& RenderTarget,               // Render target to read from.
    ReadImageDataAsyncCallback&& Callback               // Callback to invoke when the image is available.
  );



  // Asynchronously reads the contents of the specified AShaderBasedSensor
  // and calls the provided Callback. Uses ReadImageDataAsync underneath.
  bool ReadSensorImageDataAsync(
    AShaderBasedSensor& Sensor,                         // ShaderBasedSensor to read from.
    ReadImageDataAsyncCallback&& Callback               // Callback to invoke when the image is available.
  );



  // Asynchronously reads the contents of the specified AShaderBasedSensor
  // and calls the provided Callback. Uses ReadImageDataAsync underneath.
  // This variant converts the raw pixel data to an FColor array before invoking Callback.
  bool ReadImageDataAsyncFColor(
    UTextureRenderTarget2D& RenderTarget,               // Render target to read from.
    ReadImageDataAsyncCallbackFColor&& Callback         // Callback to invoke when the image is available.
  );



  // Asynchronously reads the contents of the specified AShaderBasedSensor
  // and calls the provided Callback. Uses ReadImageDataAsync underneath.
  // This variant converts the raw pixel data to an FLinearColor array before invoking Callback.
  bool ReadSensorImageDataAsyncFColor(
    AShaderBasedSensor& Sensor,                         // ShaderBasedSensor to read from.
    ReadImageDataAsyncCallbackFColor&& Callback         // Callback to invoke when the image is available.
  );



  // Asynchronously reads the contents of the specified AShaderBasedSensor
  // and calls the provided Callback. Uses ReadImageDataAsync underneath.
  // This variant converts the raw pixel data to an FLinearColor array before invoking Callback.
  bool ReadImageDataAsyncFLinearColor(
    UTextureRenderTarget2D& RenderTarget,               // Render target to read from.
    ReadImageDataAsyncCallbackFLinearColor&& Callback   // Callback to invoke when the image is available.
  );



  // Asynchronously reads the contents of the specified AShaderBasedSensor
  // and calls the provided Callback. Uses ReadImageDataAsync underneath.
  // This variant converts the raw pixel data to an FLinearColor array before invoking Callback.
  bool ReadSensorImageDataAsyncFLinearColor(
    AShaderBasedSensor& Sensor,                         // ShaderBasedSensor to read from.
    ReadImageDataAsyncCallbackFLinearColor&& Callback   // Callback to invoke when the image is available.
  );
}
