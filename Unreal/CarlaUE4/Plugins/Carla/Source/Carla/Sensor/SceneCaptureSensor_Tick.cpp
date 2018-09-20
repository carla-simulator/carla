// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureSensor.h"

// Offset to keep clear in the buffer for later adding the image header by the
// serializer.
constexpr auto BUFFER_OFFSET =
    carla::sensor::SensorRegistry::get<ASceneCaptureSensor *>::type::offset_size;

// =============================================================================
// -- Local static methods -----------------------------------------------------
// =============================================================================

static void WritePixelsToBuffer(
    UTextureRenderTarget2D &RenderTarget,
    carla::Buffer &Buffer);

static void WritePixelsToBuffer_Vulkan(
    const UTextureRenderTarget2D &RenderTarget,
    carla::Buffer &Buffer,
    FRHICommandListImmediate &InRHICmdList);

// =============================================================================
// -- ASceneCaptureSensor::Tick ------------------------------------------------
// =============================================================================

void ASceneCaptureSensor::Tick(float DeltaSeconds)
{
  check(CaptureRenderTarget != nullptr);

  Super::Tick(DeltaSeconds);

  // First we create the message header (needs to be created in the
  // game-thread).
  auto Header = GetDataStream().MakeHeader(*this);
  // We need a shared ptr here because UE4 macros do not move the arguments -_-
  auto HeaderPtr = MakeShared<decltype(Header)>(std::move(Header));

  // Then we enqueue commands in the render-thread that will write the image
  // buffer to the data stream.

  auto WriteAndSend = [this, Hdr=std::move(HeaderPtr)](auto WritePixels) mutable {
    auto &Stream = GetDataStream();
    auto Buffer = Stream.PopBufferFromPool();
    WritePixels(*CaptureRenderTarget, Buffer);
    Stream.Send_Async(std::move(*Hdr), *this, std::move(Buffer));
  };

  if (IsVulkanPlatform(GMaxRHIShaderPlatform))
  {
    auto WriteAndSendFunction = [WaS=std::move(WriteAndSend)](FRHICommandListImmediate &InRHICmdList) mutable {
      WaS([&](auto &RenderTarget, auto &Buffer) {
        WritePixelsToBuffer_Vulkan(RenderTarget, Buffer, InRHICmdList);
      });
    };
    ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
        FWritePixels_Vulkan,
        decltype(WriteAndSendFunction), WriteAndSendFunction, std::move(WriteAndSendFunction),
    {
      WriteAndSendFunction(RHICmdList);
    });
  }
  else
  {
    ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
        FWritePixels,
        decltype(WriteAndSend), WriteAndSend, std::move(WriteAndSend),
    {
      WriteAndSend(&WritePixelsToBuffer);
    });
  }
}

// =============================================================================
// -- Local static methods (implementation) ------------------------------------
// =============================================================================

struct LockTexture {
  LockTexture(FRHITexture2D *InTexture, uint32 &Stride)
    : Texture(InTexture),
      Source(reinterpret_cast<const uint8 *>(
          RHILockTexture2D(Texture, 0, RLM_ReadOnly, Stride, false))) {}

  ~LockTexture() {
    RHIUnlockTexture2D(Texture, 0, false);
  }

  FRHITexture2D *Texture;

  const uint8 *Source;
};

static void WritePixelsToBuffer(
    UTextureRenderTarget2D &RenderTarget,
    carla::Buffer &Buffer)
{
  check(IsInRenderingThread());
  FRHITexture2D *texture = RenderTarget.GetRenderTargetResource()->GetRenderTargetTexture();
  if (!texture)
  {
    UE_LOG(LogCarla, Error, TEXT("SceneCaptureCamera: Missing render texture"));
    return;
  }
  const uint32 num_bytes_per_pixel = 4;    // PF_R8G8B8A8
  const uint32 width = texture->GetSizeX();
  const uint32 height = texture->GetSizeY();
  const uint32 dest_stride = width * height * num_bytes_per_pixel;

  uint32 src_stride;
  LockTexture Lock(texture, src_stride);
  const uint8 *src = Lock.Source;

  /// @todo We don't need to allocate memory here.
  std::unique_ptr<uint8[]> dest = nullptr;
  // Direct 3D uses additional rows in the buffer,so we need check the result
  // stride from the lock:
  if (IsD3DPlatform(GMaxRHIShaderPlatform, false) && (dest_stride != src_stride))
  {
    const uint32 copy_row_stride = width * num_bytes_per_pixel;
    dest = std::make_unique<uint8[]>(dest_stride);
    // Copy per row
    uint8 *dest_row = dest.get();
    const uint8 *src_row = src;
    for (uint32 Row = 0; Row < height; ++Row)
    {
      FMemory::Memcpy(dest_row, src_row, copy_row_stride);
      dest_row += copy_row_stride;
      src_row += src_stride;
    }
    src = dest.get();
  }

  Buffer.copy_from(BUFFER_OFFSET, src, dest_stride);
}

static void WritePixelsToBuffer_Vulkan(
    const UTextureRenderTarget2D &RenderTarget,
    carla::Buffer &Buffer,
    FRHICommandListImmediate &InRHICmdList)
{
  check(IsInRenderingThread());
  auto RenderResource =
      static_cast<const FTextureRenderTarget2DResource *>(RenderTarget.Resource);
  FTextureRHIParamRef texture = RenderResource->GetRenderTargetTexture();
  if (!texture)
  {
    UE_LOG(LogCarla, Error, TEXT("SceneCaptureCamera: Missing render target texture"));
    return;
  }

  // Extra copy here, don't know how to avoid it.
  TArray<FColor> Pixels;
  InRHICmdList.ReadSurfaceData(
      texture,
      FIntRect(0, 0, RenderResource->GetSizeXY().X, RenderResource->GetSizeXY().Y),
      Pixels,
      FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));

  Buffer.copy_from(BUFFER_OFFSET, Pixels);
}
