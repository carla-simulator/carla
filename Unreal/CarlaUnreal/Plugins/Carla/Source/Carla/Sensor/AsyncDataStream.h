// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/disable-ue4-macros.h>
#include <carla/Buffer.h>
#include <carla/Logging.h>
#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/s11n/SensorHeaderSerializer.h>
#include <carla/streaming/Stream.h>
#include <util/enable-ue4-macros.h>


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

  template <typename SensorT, typename... ArgsT>
  void SerializeAndSend(SensorT &Sensor, ArgsT &&... Args);

  /// allow to change the frame number of the header
  void SetFrameNumber(uint64_t FrameNumber)
  {
    carla::sensor::s11n::SensorHeaderSerializer::Header *HeaderStr =
      reinterpret_cast<carla::sensor::s11n::SensorHeaderSerializer::Header *>(Header.data());
    if (HeaderStr)
    {
      if (HeaderStr->frame != FrameNumber)
      {
        carla::log_info("Re-framing sensor type ", HeaderStr->sensor_type, " from ", HeaderStr->frame, " to ", FrameNumber);
        HeaderStr->frame = FrameNumber;
      }
    }
  }

  /// return the type of sensor of this stream
  uint64_t GetSensorType()
  {
    carla::sensor::s11n::SensorHeaderSerializer::Header *HeaderStr =
      reinterpret_cast<carla::sensor::s11n::SensorHeaderSerializer::Header *>(Header.data());
    if (HeaderStr)
    {
      return HeaderStr->sensor_type;
    }
    return 0u;
  }

  /// return the transform of the sensor
  FTransform GetSensorTransform()
  {
    carla::sensor::s11n::SensorHeaderSerializer::Header *HeaderStr =
      reinterpret_cast<carla::sensor::s11n::SensorHeaderSerializer::Header *>(Header.data());
    if (HeaderStr)
    {
      return FTransform(HeaderStr->sensor_transform);
    }
    return FTransform();
  }

  /// return the timestamp of the sensor
  double GetSensorTimestamp()
  {
    carla::sensor::s11n::SensorHeaderSerializer::Header *HeaderStr =
      reinterpret_cast<carla::sensor::s11n::SensorHeaderSerializer::Header *>(Header.data());
    if (HeaderStr)
    {
      return HeaderStr->timestamp;
    }
    return 0.0;
  }

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
inline void FAsyncDataStreamTmpl<T>::SerializeAndSend(SensorT &Sensor, ArgsT &&... Args)
{
  // serialize data
  carla::Buffer Data(carla::sensor::SensorRegistry::Serialize(Sensor, std::forward<ArgsT>(Args)...));

  // create views of buffers
  auto ViewHeader = carla::BufferView::CreateFrom(std::move(Header));
  auto ViewData = carla::BufferView::CreateFrom(std::move(Data));

  // send views
  Stream.Write(ViewHeader, ViewData);
}

template <typename T>
template <typename SensorT, typename... ArgsT>
inline void FAsyncDataStreamTmpl<T>::Send(SensorT &Sensor, ArgsT &&... Args)
{
  // create views of buffers
  auto ViewHeader = carla::BufferView::CreateFrom(std::move(Header));

  // send views
  Stream.Write(ViewHeader, std::forward<ArgsT>(Args)...);
}
