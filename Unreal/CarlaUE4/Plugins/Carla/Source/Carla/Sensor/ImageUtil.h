// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <CoreMinimal.h>
#include <CoreGlobals.h>
#include <Engine/TextureRenderTarget2D.h>
#include <Runtime/ImageWriteQueue/Public/ImagePixelData.h>
#include <functional>



class AShaderBasedSensor;
class UTextureRenderTarget2D;

namespace ImageUtil
{
  bool DecodePixelsByFormat(
    const void* PixelData,
    int32 SourcePitch,
    FIntPoint SourceExtent,
    FIntPoint DestinationExtent,
    EPixelFormat Format,
    FReadSurfaceDataFlags Flags,
    TArrayView<FColor> Out);

  bool DecodePixelsByFormat(
    const void* PixelData,
    int32 SourcePitch,
    FIntPoint SourceExtent,
    FIntPoint DestinationExtent,
    EPixelFormat Format,
    FReadSurfaceDataFlags Flags,
    TArrayView<FLinearColor> Out);

  bool ReadImageData(
    UTextureRenderTarget2D& RenderTarget,
    TArray<FColor>& Out);

  bool ReadImageData(
    UTextureRenderTarget2D& RenderTarget,
    TArray64<FColor>& Out);

  TUniquePtr<TImagePixelData<FColor>> ReadImageData(
    UTextureRenderTarget2D& RenderTarget);

  TFuture<bool> SaveImageData(
    UTextureRenderTarget2D& RenderTarget,
    const FStringView& Path);

  TFuture<bool> SaveImageData(
    TUniquePtr<TImagePixelData<FColor>> Data,
    const FStringView& Path);

  using ReadImageDataAsyncCallback = std::function<
    bool(
      const void*,          // MappedData
      size_t,               // RowPitch
      size_t,               // BufferHeight
      EPixelFormat,         // Format
      FIntPoint             // Extent
    )
  >;
  
  using ReadImageDataAsyncCallbackFColor = std::function<
    bool(
      TArrayView<const FColor>, // Data
      FIntPoint                 // Extent
    )
  >;
  
  using ReadImageDataAsyncCallbackFLinearColor = std::function<
    bool(
      TArrayView<const FLinearColor>, // Data
      FIntPoint                       // Extent
    )
  >;

  bool ReadImageDataAsync(
    UTextureRenderTarget2D& RenderTarget,
    ReadImageDataAsyncCallback&& Callback);

  bool ReadSensorImageDataAsync(
    AShaderBasedSensor& Sensor,
    ReadImageDataAsyncCallback&& Callback);

  bool ReadImageDataAsyncFColor(
    UTextureRenderTarget2D& RenderTarget,
    ReadImageDataAsyncCallbackFColor&& Callback);

  bool ReadSensorImageDataAsyncFColor(
    AShaderBasedSensor& Sensor,
    ReadImageDataAsyncCallbackFColor&& Callback);

  bool ReadImageDataAsyncFLinearColor(
    UTextureRenderTarget2D& RenderTarget,
    ReadImageDataAsyncCallbackFLinearColor&& Callback);

  bool ReadSensorImageDataAsyncFLinearColor(
    AShaderBasedSensor& Sensor,
    ReadImageDataAsyncCallbackFLinearColor&& Callback);
}
