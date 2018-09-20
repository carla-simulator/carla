// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#include <carla/Buffer.h>
#include <carla/Optional.h>
#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/s11n/SensorHeaderSerializer.h>
#include <carla/streaming/Stream.h>
#include <compiler/enable-ue4-macros.h>

// =============================================================================
// -- FSensorMessageHeader -----------------------------------------------------
// =============================================================================

class FSensorMessageHeader {
public:

  FSensorMessageHeader(FSensorMessageHeader &&) = default;

private:

  FSensorMessageHeader(carla::Buffer InBuffer) : Buffer(std::move(InBuffer)) {}

  friend class FDataStream;

  carla::Buffer Buffer;
};

// =============================================================================
// -- FDataStream --------------------------------------------------------------
// =============================================================================

class FDataStream
{
public:

  FDataStream() = default;

  FDataStream(FDataStream &&) = default;
  FDataStream &operator=(FDataStream &&) = default;

  FDataStream(carla::streaming::Stream InStream) : Stream(std::move(InStream)) {}

  template <typename TSensor>
  FSensorMessageHeader MakeHeader(const TSensor &Sensor);

  /// Use this function to avoid memory allocations.
  carla::Buffer PopBufferFromPool();

  template <typename TSensor, typename... TArgs>
  void Send_Async(FSensorMessageHeader Header, TSensor &Sensor, TArgs &&... Args);

  template <typename TSensor, typename... TArgs>
  void Send_GameThread(TSensor &Sensor, TArgs &&... Args);

private:

  carla::Optional<carla::streaming::Stream> Stream;
};

// =============================================================================
// -- FDataStream implementation -----------------------------------------------
// =============================================================================

template <typename TSensor>
inline FSensorMessageHeader FDataStream::MakeHeader(const TSensor &Sensor)
{
  check(IsInGameThread());
  using Serializer = carla::sensor::s11n::SensorHeaderSerializer;
  return {Serializer::Serialize(Sensor, GFrameCounter)};
}

inline carla::Buffer FDataStream::PopBufferFromPool()
{
#ifdef WITH_EDITOR
  if (!Stream.has_value())
  {
    UE_LOG(LogCarla, Error, TEXT("Sensor does not have a stream!"));
    return {};
  }
#endif // WITH_EDITOR
  check(Stream.has_value());
  return (*Stream).MakeBuffer();
}

template <typename TSensor, typename... TArgs>
inline void FDataStream::Send_Async(FSensorMessageHeader Header, TSensor &Sensor, TArgs &&... Args)
{
#ifdef WITH_EDITOR
  if (!Stream.has_value())
  {
    UE_LOG(LogCarla, Error, TEXT("Sensor does not have a stream!"));
    return;
  }
#endif // WITH_EDITOR
  check(Stream.has_value());
  (*Stream).Write(
      std::move(Header.Buffer),
      carla::sensor::SensorRegistry::Serialize(Sensor, std::forward<TArgs>(Args)...));
}

template <typename TSensor, typename... TArgs>
inline void FDataStream::Send_GameThread(TSensor &Sensor, TArgs &&... Args)
{
  Send_Async(MakeHeader(Sensor), Sensor, std::forward<TArgs>(Args)...);
}
