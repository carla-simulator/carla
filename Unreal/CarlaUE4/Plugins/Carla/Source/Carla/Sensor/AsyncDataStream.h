// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <compiler/disable-ue4-macros.h>
#include <carla/Buffer.h>
#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/s11n/SensorHeaderSerializer.h>
#include <carla/streaming/Stream.h>
#include <compiler/enable-ue4-macros.h>

template <typename T>
class FDataStreamTmpl;

// =============================================================================
// -- FAsyncDataStreamTmpl -----------------------------------------------------
// =============================================================================

/// A streaming channel for sending sensor data to clients, supports sending
/// data asynchronously. Data sent by the "Send" functions is passed to the
/// serializer registered with the sensor at carla::sensor:SensorRegistry before
/// being sent down the stream.
///
/// @warning This is a single-use object, a new one needs to be created for each
/// new message.
///
/// FAsyncDataStream also has a pool of carla::Buffer that allows reusing the
/// allocated memory, use it whenever possible.
template <typename T>
class FAsyncDataStreamTmpl
{
public:

  using StreamType = T;

  FAsyncDataStreamTmpl(FAsyncDataStreamTmpl &&) = default;

  /// Return the token that allows subscribing to this stream.
  auto GetToken() const
  {
    return Stream.GetToken();
  }

  /// Pop a Buffer from the pool. Buffers in the pool can reuse the memory
  /// allocated by previous messages, significantly improving performance for
  /// big messages.
  carla::Buffer PopBufferFromPool()
  {
    return Stream.MakeBuffer();
  }

  /// Send some data down the stream.
  template <typename SensorT, typename... ArgsT>
  void Send(SensorT &Sensor, ArgsT &&... Args);

private:

  friend class FDataStreamTmpl<T>;

  /// @pre This functions needs to be called in the game-thread.
  template <typename SensorT>
  explicit FAsyncDataStreamTmpl(
      const SensorT &InSensor,
      double Timestamp,
      StreamType InStream);

  StreamType Stream;

  carla::Buffer Header;
};

// =============================================================================
// -- FAsyncDataStream and FAsyncDataMultiStream -------------------------------
// =============================================================================

using FAsyncDataStream = FAsyncDataStreamTmpl<carla::streaming::Stream>;

using FAsyncDataMultiStream = FAsyncDataStreamTmpl<carla::streaming::Stream>;

// =============================================================================
// -- FAsyncDataStreamTmpl implementation --------------------------------------
// =============================================================================

template <typename T>
template <typename SensorT, typename... ArgsT>
inline void FAsyncDataStreamTmpl<T>::Send(SensorT &Sensor, ArgsT &&... Args)
{
  Stream.Write(
      std::move(Header),
      carla::sensor::SensorRegistry::Serialize(Sensor, std::forward<ArgsT>(Args)...));
}
