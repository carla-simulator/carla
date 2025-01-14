// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Sensor/DataStream.h"
#include "Carla/Util/RandomEngine.h"
#include "Carla/Game/CarlaEngine.h"

#include <util/disable-ue4-macros.h>
#include <carla/Logging.h>
#include <carla/Buffer.h>
#include <carla/BufferView.h>
#include <carla/sensor/SensorRegistry.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "GameFramework/Actor.h"
#include <util/ue-header-guard-end.h>

#include "Sensor.generated.h"

struct FActorDescription;



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



/// Base class for sensors.
UCLASS(Abstract, hidecategories = (Collision, Attachment, Actor))
class CARLA_API ASensor : public AActor
{
  GENERATED_BODY()

public:

  ASensor(const FObjectInitializer &ObjectInitializer);

  void SetEpisode(const UCarlaEpisode &InEpisode)
  {
    Episode = &InEpisode;
  }

  virtual void Set(const FActorDescription &Description);

  std::optional<FActorAttribute> GetAttribute(const FString Name);

  virtual void BeginPlay();

  /// Replace the FDataStream associated with this sensor.
  ///
  /// @warning Do not change the stream after BeginPlay. It is not thread-safe.
  void SetDataStream(FDataStream InStream)
  {
    Stream = std::move(InStream);
  }

  FDataStream MoveDataStream()
  {
    return std::move(Stream);
  }

  /// Return the token that allows subscribing to this sensor's stream.
  auto GetToken() const
  {
    return Stream.GetToken();
  }

  bool IsStreamReady()
  {
    return Stream.IsStreamReady();
  }

  bool AreClientsListening()
  { 
    return Stream.AreClientsListening();
  }

  void Tick(const float DeltaTime) final;

  virtual void PrePhysTick(float DeltaSeconds) {}
  virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) {}
  // Small interface to notify sensors when clients are listening
  virtual void OnFirstClientConnected() {};
  // Small interface to notify sensors when no clients are listening
  virtual void OnLastClientDisconnected() {};


  void PostPhysTickInternal(UWorld *World, ELevelTick TickType, float DeltaSeconds);

  UFUNCTION(BlueprintCallable)
  URandomEngine *GetRandomEngine()
  {
    return RandomEngine;
  }

  UFUNCTION(BlueprintCallable)
  int32 GetSeed() const
  {
    return Seed;
  }

  UFUNCTION(BlueprintCallable)
  void SetSeed(int32 InSeed);

  const UCarlaEpisode &GetEpisode() const
  {
    check(Episode != nullptr);
    return *Episode;
  }

  void SetSavingDataToDisk(bool bSavingData) { bSavingDataToDisk = bSavingData; }

protected:

  void PostActorCreated() override;

  void EndPlay(EEndPlayReason::Type EndPlayReason) override;

  /// Return the FDataStream associated with this sensor.
  ///
  /// You need to provide a reference to self, this is necessary for template
  /// deduction.
  template <typename SensorType>
  FAsyncDataStream GetDataStream(SensorType&& Self)
  {
    return Stream.MakeAsyncDataStream<std::remove_cvref_t<SensorType>>(
      std::forward<SensorType>(Self),
      GetEpisode().GetElapsedGameTime());
  }


  // Send sensor data to the client.
  template <
    typename SensorType,
    typename ElementType>
  static void SendDataToClient(
    SensorType&& Sensor,                  // The data's owning sensor.
    TArrayView<ElementType> SensorData,   // Data to send to the client.
    uint64_t FrameIndex                   // Current frame index.
    )
  {
    using carla::sensor::SensorRegistry;
    using SensorT = std::remove_const_t<std::remove_reference_t<SensorType>>;
    constexpr size_t HeaderOffset = SensorRegistry::get<SensorT*>::type::header_offset;

    if (!Sensor.AreClientsListening())
        return;

    auto Stream = Sensor.GetDataStream(Sensor);
    Stream.SetFrameNumber(FrameIndex);
    
    auto Buffer = Stream.PopBufferFromPool();
    Buffer.copy_from(
      HeaderOffset,
      boost::asio::buffer(
        SensorData.GetData(),
        SensorData.Num() * sizeof(ElementType)));

    if (!Buffer.data())
      return;

    auto Serialized = SensorRegistry::Serialize(Sensor, std::move(Buffer));
    auto SerializedBuffer = carla::Buffer(std::move(Serialized));
    auto BufferView = carla::BufferView::CreateFrom(std::move(SerializedBuffer));

#if defined(WITH_ROS2)
    auto ROS2 = carla::ros2::ROS2::GetInstance();
    if (ROS2->IsEnabled())
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("ROS2 SendDataToClient");
      auto StreamId = carla::streaming::detail::token_type(Sensor.GetToken()).get_stream_id();
      auto Res = std::async(std::launch::async, [&Sensor, ROS2, &Stream, StreamId, BufferView]()
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
        auto ParentActor = Sensor.GetAttachParentActor();
        auto Transform =
          ParentActor ?
          Sensor.GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform()) :
          Stream.GetSensorTransform();
        ROS2->ProcessDataFromCamera(
          Stream.GetSensorType(),
          StreamId,
          Transform,
          W, H,
          Fov,
          BufferView,
          &Sensor);
      });
    }
#endif

    if (Sensor.AreClientsListening())
      Stream.Send(Sensor, BufferView);
  }

  /// Seed of the pseudo-random engine.
  UPROPERTY(Category = "Random Engine", EditAnywhere)
  int32 Seed = 123456789;

  /// Random Engine used to provide noise for sensor output.
  UPROPERTY()
  URandomEngine *RandomEngine = nullptr;

  UPROPERTY()
  bool bIsActive = false;

  // Property used when testing with SensorSpawnerActor in editor.
  bool bSavingDataToDisk = false;

private:

  FDataStream Stream;

  FDelegateHandle OnPostTickDelegate;

  FActorDescription SensorDescription;

  const UCarlaEpisode *Episode = nullptr;

  /// Allows the sensor to tick with the tick rate from UE4.
  bool ReadyToTick = false;

  bool bClientsListening = false;

};
